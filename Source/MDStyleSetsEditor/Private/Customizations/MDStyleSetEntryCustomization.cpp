// Copyright Dylan Dumesnil. All Rights Reserved.

#include "Customizations/MDStyleSetEntryCustomization.h"

#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "IDetailChildrenBuilder.h"
#include "IStructureDataProvider.h"
#include "MDStyleSet.h"
#include "Widgets/Text/STextBlock.h"

class FMDStyleValueInstancedStructProvider : public IStructureDataProvider
{
public:
	FMDStyleValueInstancedStructProvider() = default;

	explicit FMDStyleValueInstancedStructProvider(const TSharedPtr<IPropertyHandle>& InStructProperty) : StructProperty(InStructProperty) {}
	virtual ~FMDStyleValueInstancedStructProvider() override {}

	void Reset() { StructProperty = nullptr; }

	virtual bool IsValid() const override
	{
		bool bHasValidData = false;
		EnumerateInstances([&bHasValidData](const UScriptStruct* ScriptStruct, uint8* Memory, UPackage* Package)
		{
			if (ScriptStruct && Memory)
			{
				bHasValidData = true;
				return false; // Stop
			}
			return true; // Continue
		});

		return bHasValidData;
	}

	virtual const UStruct* GetBaseStructure() const override
	{
		// Taken from UClass::FindCommonBase
		auto FindCommonBaseStruct = [](const UScriptStruct* StructA, const UScriptStruct* StructB)
		{
			const UScriptStruct* CommonBaseStruct = StructA;
			while (CommonBaseStruct && StructB && !StructB->IsChildOf(CommonBaseStruct))
			{
				CommonBaseStruct = Cast<UScriptStruct>(CommonBaseStruct->GetSuperStruct());
			}
			return CommonBaseStruct;
		};

		const UScriptStruct* CommonStruct = nullptr;
		EnumerateInstances([&CommonStruct, &FindCommonBaseStruct](const UScriptStruct* ScriptStruct, uint8* Memory, UPackage* Package)
		{
			if (ScriptStruct)
			{
				CommonStruct = FindCommonBaseStruct(ScriptStruct, CommonStruct);
			}
			return true; // Continue
		});

		return CommonStruct;
	}

	virtual void GetInstances(TArray<TSharedPtr<FStructOnScope>>& OutInstances, const UStruct* ExpectedBaseStructure) const override
	{
		// The returned instances need to be compatible with base structure.
		// This function returns empty instances in case they are not compatible, with the idea that we have as many instances as we have outer objects.
		EnumerateInstances([&OutInstances, ExpectedBaseStructure](const UScriptStruct* ScriptStruct, uint8* Memory, UPackage* Package)
		{
			TSharedPtr<FStructOnScope> Result;

			if (ExpectedBaseStructure && ScriptStruct && ScriptStruct->IsChildOf(ExpectedBaseStructure))
			{
				Result = MakeShared<FStructOnScope>(ScriptStruct, Memory);
				Result->SetPackage(Package);
			}

			OutInstances.Add(Result);

			return true; // Continue
		});
	}

	virtual bool IsPropertyIndirection() const override { return true; }

	virtual uint8* GetValueBaseAddress(uint8* ParentValueAddress, const UStruct* ExpectedBaseStructure) const override
	{
		if (ParentValueAddress != nullptr)
		{
			FInstancedStruct& InstancedStruct = *reinterpret_cast<FInstancedStruct*>(ParentValueAddress);
			if (ExpectedBaseStructure && InstancedStruct.GetScriptStruct() && InstancedStruct.GetScriptStruct()->IsChildOf(ExpectedBaseStructure))
			{
				return InstancedStruct.GetMutableMemory();
			}
		}

		return nullptr;
	}

protected:
	void EnumerateInstances(TFunctionRef<bool(const UScriptStruct* ScriptStruct, uint8* Memory, UPackage* Package)> InFunc) const
	{
		if (!StructProperty.IsValid())
		{
			return;
		}

		TArray<UPackage*> Packages;
		StructProperty->GetOuterPackages(Packages);

		StructProperty->EnumerateRawData([&InFunc, &Packages](void* RawData, const int32 DataIndex, const int32 /*NumDatas*/)
		{
			const UScriptStruct* ScriptStruct = nullptr;
			uint8* Memory = nullptr;
			UPackage* Package = nullptr;
			if (FInstancedStruct* InstancedStruct = static_cast<FInstancedStruct*>(RawData))
			{
				ScriptStruct = InstancedStruct->GetScriptStruct();
				Memory = InstancedStruct->GetMutableMemory();
				if (ensureMsgf(Packages.IsValidIndex(DataIndex), TEXT("Expecting packges and raw data to match.")))
				{
					Package = Packages[DataIndex];
				}
			}

			return InFunc(ScriptStruct, Memory, Package);
		});
	}

	TSharedPtr<IPropertyHandle> StructProperty;
};

void FMDStyleSetEntryCustomization::CustomizeHeader(TSharedRef<IPropertyHandle> PropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	TSharedPtr<IPropertyHandle> ValueHandle = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FMDStyleValue, Value));
	TSharedRef<FMDStyleValueInstancedStructProvider> NewStructProvider = MakeShared<FMDStyleValueInstancedStructProvider>(ValueHandle->GetChildHandle(TEXT("Value")));
	TArray<TSharedPtr<IPropertyHandle>> ChildProperties = ValueHandle->AddChildStructure(NewStructProvider);
	EntryValueHandle = ChildProperties.IsEmpty() ? nullptr : ChildProperties[0];

	const FIsResetToDefaultVisible IsResetToDefaultVisible = FIsResetToDefaultVisible::CreateSP(this, &FMDStyleSetEntryCustomization::IsResetToDefaultVisible);
	const FResetToDefaultHandler ResetToDefaultClicked = FResetToDefaultHandler::CreateSP(this, &FMDStyleSetEntryCustomization::ResetValueToDefault);

	HeaderRow
	.NameContent()
	[
		PropertyHandle->CreatePropertyNameWidget()
	]
	.ValueContent()
	[
		SNew(SOverlay)
		+SOverlay::Slot()
		.VAlign(VAlign_Center)
		.HAlign(HAlign_Fill)
		[
			EntryValueHandle.IsValid()
				? EntryValueHandle->CreatePropertyValueWidgetWithCustomization(DetailBuilderPtr.Pin()->GetDetailsView())
				: SNullWidget::NullWidget
		]
	]
	.OverrideResetToDefault(FResetToDefaultOverride::Create(IsResetToDefaultVisible, ResetToDefaultClicked));
}

void FMDStyleSetEntryCustomization::CustomizeChildren(TSharedRef<IPropertyHandle> PropertyHandle, IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	if (EntryValueHandle.IsValid())
	{
		uint32 NumChildren = 0;
		EntryValueHandle->GetNumChildren(NumChildren);
		for (uint32 i = 0; i < NumChildren; ++i)
		{
			ChildBuilder.AddProperty(EntryValueHandle->GetChildHandle(i).ToSharedRef());
		}
	}
}

bool FMDStyleSetEntryCustomization::IsResetToDefaultVisible(TSharedPtr<IPropertyHandle> Handle) const
{
	return EntryValueHandle.IsValid() && EntryValueHandle->IsValidHandle() && EntryValueHandle->IsEditable() && EntryValueHandle->DiffersFromDefault();
}

void FMDStyleSetEntryCustomization::ResetValueToDefault(TSharedPtr<IPropertyHandle> Handle)
{
	if (EntryValueHandle.IsValid() && EntryValueHandle->IsValidHandle() && EntryValueHandle->IsEditable())
	{
		if (const FProperty* Property = EntryValueHandle->GetProperty())
		{
			void* ValuePtr = nullptr;
			if (EntryValueHandle->GetValueData(ValuePtr) == FPropertyAccess::Success && ValuePtr != nullptr)
			{
				EntryValueHandle->NotifyPreChange();
				Property->DestroyValue(ValuePtr);
				Property->InitializeValue(ValuePtr);
				EntryValueHandle->NotifyPostChange(EPropertyChangeType::ValueSet);
				EntryValueHandle->NotifyFinishedChangingProperties();
			}
		}
	}
}
