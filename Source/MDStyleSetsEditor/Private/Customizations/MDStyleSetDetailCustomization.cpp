// Copyright Dylan Dumesnil. All Rights Reserved.


#include "Customizations/MDStyleSetDetailCustomization.h"

#include "Customizations/MDStyleSetEntryCustomization.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "GameplayTagsManager.h"
#include "MDStyleSet.h"
#include "SlateOptMacros.h"
#include "SPinTypeSelector.h"

void FMDStyleSetDetailCustomization::CustomizeDetails(const TSharedPtr<IDetailLayoutBuilder>& DetailBuilder)
{
	DetailBuilderPtr = DetailBuilder;
	IDetailCustomization::CustomizeDetails(DetailBuilder);
}

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void FMDStyleSetDetailCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	UGameplayTagsManager::Get().OnGetCategoriesMetaFromPropertyHandle.AddSP(this, &FMDStyleSetDetailCustomization::OnGetCategoriesMetaFromPropertyHandle);

	StyleSetTagPropertyPtr = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UMDStyleSet, StyleSetTag), UMDStyleSet::StaticClass());
	StyleSetTagPropertyPtr->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FMDStyleSetDetailCustomization::RefreshDetails));
	FallbackValuePropertyPtr = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UMDStyleSet, FallbackValue), UMDStyleSet::StaticClass());
	StyleTypePropertyPtr = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UMDStyleSet, StyleType), UMDStyleSet::StaticClass());
	StyleTypePropertyPtr->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FMDStyleSetDetailCustomization::OnPinTypePropertyChanged));
	if (IDetailPropertyRow* StyleTypeRow = DetailBuilder.EditDefaultProperty(StyleTypePropertyPtr))
	{
		FUIAction CopyAction;
		FUIAction PasteAction;

		StyleTypePropertyPtr->CreateDefaultPropertyCopyPasteActions(CopyAction, PasteAction);

		StyleTypeRow->CustomWidget()
		.CopyAction(CopyAction)
		.PasteAction(PasteAction)
		.NameContent()
		[
			StyleTypePropertyPtr->CreatePropertyNameWidget()
		]
		.ValueContent()
		[
			SNew(SPinTypeSelector, FGetPinTypeTree::CreateUObject(GetDefault<UEdGraphSchema_K2>(), &UEdGraphSchema_K2::GetVariableTypeTree))
			.TargetPinType(this, &FMDStyleSetDetailCustomization::GetPinType)
			.OnPinTypeChanged(this, &FMDStyleSetDetailCustomization::OnPinTypeChanged)
			.TypeTreeFilter(ETypeTreeFilter::None)
			.Schema(GetDefault<UEdGraphSchema_K2>())
			.bAllowArrays(false)
			.Font(IDetailLayoutBuilder::GetDetailFont())
		]
		.ExtensionContent()
		[
			StyleTypePropertyPtr->CreateDefaultPropertyButtonWidgets()
		];
	}

	DetailBuilder.RegisterInstancedCustomPropertyTypeLayout(FMDStyleValue::StaticStruct()->GetFName(),
		FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FMDStyleSetEntryCustomization::MakeInstance, DetailBuilderPtr));

	StyleEntriesPropertyPtr = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UMDStyleSet, StyleEntries), UMDStyleSet::StaticClass());
	const TSharedPtr<IPropertyHandleMap> StyleEntriesMapProperty = StyleEntriesPropertyPtr->AsMap();
	if (IDetailPropertyRow* StyleEntriesRow = DetailBuilder.EditDefaultProperty(StyleEntriesPropertyPtr))
	{
		StyleEntriesRow->ShouldAutoExpand(true);
	}
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION

FEdGraphPinType FMDStyleSetDetailCustomization::GetPinType() const
{
	if (!StyleTypePropertyPtr.IsValid())
	{
		return FEdGraphPinType();
	}

	const IPropertyHandle* PropertyHandle = StyleTypePropertyPtr.Get();

	void* PropertyValue = nullptr;
	if (PropertyHandle->GetValueData(PropertyValue) != FPropertyAccess::Success || PropertyValue == nullptr)
	{
		return FEdGraphPinType();
	}

	const FEdGraphPinType* PropertyType = static_cast<FEdGraphPinType*>(PropertyValue);
	return *PropertyType;
}

void FMDStyleSetDetailCustomization::OnPinTypeChanged(const FEdGraphPinType& PinType)
{
	if (StyleTypePropertyPtr.IsValid())
	{
		void* StyleTypeValuePtr = nullptr;
		if (StyleTypePropertyPtr->GetValueData(StyleTypeValuePtr) == FPropertyAccess::Success && StyleTypeValuePtr != nullptr)
		{
			FEdGraphPinType* PropertyType = static_cast<FEdGraphPinType*>(StyleTypeValuePtr);
			if (*PropertyType != PinType)
			{
				StyleTypePropertyPtr->NotifyPreChange();

				*PropertyType = PinType;

				StyleTypePropertyPtr->NotifyPostChange(EPropertyChangeType::ValueSet);
				StyleTypePropertyPtr->NotifyFinishedChangingProperties();
			}
		}
	}

	const EPropertyBagPropertyType ValueType = UMDStyleSet::GetValueTypeFromPinType(PinType);
	UObject* TypeObject = PinType.PinSubCategoryObject.Get();

	if (FallbackValuePropertyPtr.IsValid())
	{
		void* FallbackValuePtr = nullptr;
		if (FallbackValuePropertyPtr->GetValueData(FallbackValuePtr) == FPropertyAccess::Success && FallbackValuePtr != nullptr)
		{
			FallbackValuePropertyPtr->NotifyPreChange();

			FMDStyleValue* FallbackValue = static_cast<FMDStyleValue*>(FallbackValuePtr);
			if (ValueType == EPropertyBagPropertyType::None)
			{
				FallbackValue->Value.Reset();
			}
			else
			{
				FallbackValue->Value.AddProperty(FMDStyleValue::ValuePropertyName, ValueType, TypeObject);
			}

			FallbackValuePropertyPtr->NotifyPostChange(EPropertyChangeType::ValueSet);
			FallbackValuePropertyPtr->NotifyFinishedChangingProperties();
		}
	}

	if (StyleEntriesPropertyPtr.IsValid())
	{
		TSharedPtr<IPropertyHandleMap> MapProperty = StyleEntriesPropertyPtr->AsMap();

		uint32 NumEntries = 0;
		MapProperty->GetNumElements(NumEntries);
		for (uint32 i = 0; i < NumEntries; ++i)
		{
			TSharedRef<IPropertyHandle> EntryProp = MapProperty->GetElement(i);
			void* EntryPtr = nullptr;
			if (EntryProp->GetValueData(EntryPtr) == FPropertyAccess::Success && EntryPtr != nullptr)
			{
				EntryProp->NotifyPreChange();

				FMDStyleValue* Entry = static_cast<FMDStyleValue*>(EntryPtr);
				if (ValueType == EPropertyBagPropertyType::None)
				{
					Entry->Value.Reset();
				}
				else
				{
					Entry->Value.AddProperty(FMDStyleValue::ValuePropertyName, ValueType, TypeObject);
				}

				EntryProp->NotifyPostChange(EPropertyChangeType::ValueSet);
				EntryProp->NotifyFinishedChangingProperties();
			}
		}
	}

	RefreshDetails();
}

void FMDStyleSetDetailCustomization::OnPinTypePropertyChanged()
{
	if (StyleTypePropertyPtr.IsValid())
	{
		void* StyleTypeValuePtr = nullptr;
		if (StyleTypePropertyPtr->GetValueData(StyleTypeValuePtr) == FPropertyAccess::Success && StyleTypeValuePtr != nullptr)
		{
			const FEdGraphPinType* PropertyType = static_cast<FEdGraphPinType*>(StyleTypeValuePtr);
			OnPinTypeChanged(*PropertyType);
		}
	}
}

void FMDStyleSetDetailCustomization::OnGetCategoriesMetaFromPropertyHandle(TSharedPtr<IPropertyHandle> PropertyHandle, FString& MetaString) const
{
	if (StyleSetTagPropertyPtr.IsValid() && StyleEntriesPropertyPtr->IsSamePropertyNode(PropertyHandle->GetParentHandle()))
	{
		StyleSetTagPropertyPtr->EnumerateRawData([&MetaString](void* RawData, const int32 /*DataIndex*/, const int32 /*NumDatas*/)
		{
			const FGameplayTag& Tag = *static_cast<FGameplayTag*>(RawData);
			MetaString += Tag.ToString() + TEXT(",");
			return true;
		});
	}
}

void FMDStyleSetDetailCustomization::RefreshDetails() const
{
	IDetailLayoutBuilder* DetailBuilder = DetailBuilderPtr.Pin().Get();
	if (DetailBuilder != nullptr)
	{
		DetailBuilder->ForceRefreshDetails();
	}
}
