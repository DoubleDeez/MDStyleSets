// Copyright Dylan Dumesnil. All Rights Reserved.


#include "Customizations/MDStyleSetsPropertyBindingExtension.h"

#include "Components/Widget.h"
#include "Editor.h"
#include "Extensions/MDStyleSetBlueprintCompiler.h"
#include "Extensions/MDStyleSetBlueprintExtension.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "MDStyleSet.h"
#include "MDStyleSetFunctionLibrary.h"
#include "PropertyBag.h"
#include "PropertyHandle.h"
#include "ScopedTransaction.h"
#include "Slate/SObjectWidget.h"
#include "Subsystems/AssetEditorSubsystem.h"
#include "TypeHandlers/MDStyleSetTypeHandlerBase.h"
#include "UObject/Package.h"
#include "UObject/PropertyOptional.h"
#include "WidgetBlueprint.h"
#include "Widgets/Layout/SScaleBox.h"

namespace MDStyleSetsPropertyBindingExtension
{
	static UUserWidget* FindOutermostUserWidget(TSharedPtr<SWidget> Widget)
	{
		UUserWidget* UserWidget = nullptr;

		while (Widget.IsValid())
		{
			if (Widget && Widget->GetType().IsEqual(TEXT("SObjectWidget")))
			{
				if (UUserWidget* TestUserWidget = Cast<UUserWidget>(StaticCastSharedPtr<SObjectWidget>(Widget)->GetWidgetObject()))
				{
					UserWidget = TestUserWidget;
				}
			}
			Widget = Widget->GetParentWidget();
		}

		return UserWidget;
	}

	static FPropertyBindingPath BuildPropertyPath(const UWidgetBlueprint* WidgetBP, UWidget* Widget, const TSharedPtr<IPropertyHandle>& PropertyHandle)
	{
		if (!PropertyHandle.IsValid() || !IsValid(WidgetBP) || !IsValid(Widget))
		{
			return {};
		}

		FCachedPropertyPath CachedPropertyPath(PropertyHandle->GeneratePathToProperty());
		CachedPropertyPath.Resolve(Widget);

		FPropertyBindingPath PropertyPath;
		if (Widget->GetClass()->ClassGeneratedBy != WidgetBP)
		{
			PropertyPath.AddPathSegment(Widget->GetFName());
		}

		for (int32 SegNum = 0; SegNum < CachedPropertyPath.GetNumSegments(); SegNum++)
		{
			// Skip the top-level array property if we're going to end up appending the array entry (we want MyObject.MyArray[2] not MyObject.MyArray.MyArray[2])
			const bool bIsFinalSegment = SegNum == (CachedPropertyPath.GetNumSegments() - 1);
			const FPropertyPathSegment& Segment = CachedPropertyPath.GetSegment(SegNum);
			const bool bIsArraySegment = Segment.GetField().IsA<FArrayProperty>();
			if (bIsFinalSegment || !bIsArraySegment || Segment.GetArrayIndex() != INDEX_NONE)
			{
				PropertyPath.AddPathSegment(Segment.GetName(), Segment.GetArrayIndex(), nullptr);
			}
		}

		return PropertyPath;
	}

	static void CreateStyleBinding(TWeakObjectPtr<UMDStyleSet> StyleSetPtr, FGameplayTag Tag, TWeakObjectPtr<const UWidgetBlueprint> WidgetBPPtr, TWeakObjectPtr<UWidget> WidgetPtr, TSharedPtr<IPropertyHandle> PropertyHandle)
	{
		UWidgetBlueprint* WidgetBP = const_cast<UWidgetBlueprint*>(WidgetBPPtr.Get());
		UMDStyleSetBlueprintExtension* Extension = UMDStyleSetBlueprintExtension::GetOrCreateExtension(WidgetBP);
		UWidget* Widget = WidgetPtr.Get();
		UMDStyleSet* StyleSet = StyleSetPtr.Get();
		if (IsValid(Widget) && IsValid(Extension) && IsValid(StyleSet) && PropertyHandle.IsValid() && PropertyHandle->GetProperty() != nullptr)
		{
			FMDStyleSetPropertyBinding Binding = {
				{
					StyleSet,
					Tag
				},
				BuildPropertyPath(WidgetBP, Widget, PropertyHandle)
			};

			FScopedTransaction CreateBindingTransaction(FText::Format(INVTEXT("Set Style Binding for '{0}' to '{1} - {2}'"),
				FText::FromString(Binding.TargetProperty.ToString()), Binding.Value.StyleSet->GetDisplayName(), FText::FromString(Binding.Value.StyleSet->StyleSetTag.ToString())));
			Extension->AddBinding(MoveTemp(Binding));
		}
	}

	static void OpenStyleAsset(TWeakObjectPtr<UMDStyleSet> StyleSetPtr)
	{
		if (StyleSetPtr.IsValid() && GEditor != nullptr)
		{
			if (UAssetEditorSubsystem* AssetEditorSubsystem = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>())
			{
				AssetEditorSubsystem->OpenEditorForAsset(StyleSetPtr.Get());
			}
		}
	}

	static void AddStyleSetEntriesBindingMenuItems(FMenuBuilder& MenuBuilder, TWeakObjectPtr<UMDStyleSet> StyleSetPtr, TWeakObjectPtr<const UWidgetBlueprint> WidgetBPPtr, TWeakObjectPtr<UWidget> WidgetPtr, TSharedPtr<IPropertyHandle> PropertyHandle)
	{
		if (!StyleSetPtr.IsValid())
		{
			return;
		}

		UMDStyleSet* StyleSet = StyleSetPtr.Get();
		MenuBuilder.AddMenuEntry(
			FText::Format(INVTEXT("Open '{0}' Style Asset"), StyleSet->GetDisplayName()),
			FText::GetEmpty(),
			FSlateIcon(FAppStyle::GetAppStyleSetName(), "SystemWideCommands.SummonOpenAssetDialog"),
			FUIAction(FExecuteAction::CreateStatic(&OpenStyleAsset, StyleSetPtr))
		);

		MenuBuilder.AddSeparator();

		for (TPair<FGameplayTag, FMDStyleValue>& Pair : StyleSet->StyleEntries)
		{
			TSharedRef<SWidget> PreviewWidget = IsValid(StyleSet->TypeHandler) ? StyleSet->TypeHandler->CreateValuePreviewWidget(StyleSet, Pair.Key) : SNullWidget::NullWidget;
			const FText ValuePreviewText = IsValid(StyleSet->TypeHandler) ? StyleSet->TypeHandler->CreateValuePreviewText(StyleSet, Pair.Key) : FText::GetEmpty();
			const FText TagText = FText::FromString(Pair.Key.ToString().RightChop(StyleSet->StyleSetTag.ToString().Len() + 1));
			const FText LabelText = ValuePreviewText.IsEmptyOrWhitespace() ? TagText : FText::Format(INVTEXT("{0} ({1})"), TagText, ValuePreviewText);
			const FText ToolTipText = FText::Format(INVTEXT("{0}: {1}"), FText::FromString(Pair.Key.ToString()), StyleSet->GetValueDisplayName(Pair.Key));

			if (PreviewWidget == SNullWidget::NullWidget)
			{
				MenuBuilder.AddMenuEntry(
					LabelText,
					ToolTipText,
					FSlateIcon(),
					FUIAction(FExecuteAction::CreateStatic(&CreateStyleBinding, StyleSetPtr, Pair.Key, WidgetBPPtr, WidgetPtr, PropertyHandle))
				);
			}
			else
			{
				TSharedRef<SWidget> MenuWidget = SNew(SHorizontalBox)
					+SHorizontalBox::Slot()
					.VAlign(VAlign_Center)
					.AutoWidth()
					.Padding(0, 0, 4.f, 0)
					[
						SNew(SBox)
						.HeightOverride(24.f)
						[
							SNew(SScaleBox)
							.Stretch(EStretch::ScaleToFitY)
							[
								PreviewWidget
							]
						]
					]
					+SHorizontalBox::Slot()
					.VAlign(VAlign_Center)
					.FillWidth(1.f)
					[
						SNew(STextBlock)
						.Text(LabelText)
					];
				MenuBuilder.AddMenuEntry(
					FUIAction(FExecuteAction::CreateStatic(&CreateStyleBinding, StyleSetPtr, Pair.Key, WidgetBPPtr, WidgetPtr, PropertyHandle)),
					MenuWidget,
					NAME_None,
					ToolTipText
				);
			}
		}
	}

	static void AddStyleSetBindingMenuItems(FMenuBuilder& MenuBuilder, TWeakObjectPtr<const UWidgetBlueprint> WidgetBlueprint, TWeakObjectPtr<UWidget> Widget, TSharedPtr<IPropertyHandle> PropertyHandle)
	{
		if (PropertyHandle.IsValid() && PropertyHandle->GetProperty() != nullptr)
		{
			MenuBuilder.BeginSection("MDStyleSets", INVTEXT("Style Sets"));

			const FProperty* PropertyPtr = PropertyHandle->GetProperty();
			const FPropertyBagPropertyDesc Desc(PropertyPtr->GetFName(), PropertyPtr);

			TArray<UMDStyleSet*> StyleSets = UMDStyleSetFunctionLibrary::GetConvertibleStyleSets(Desc);

			for (UMDStyleSet* StyleSet : StyleSets)
			{
				if (IsValid(StyleSet))
				{
					MenuBuilder.AddSubMenu(
						StyleSet->GetDisplayName(),
						StyleSet->GetPackage()->GetLoadedPath().GetDebugNameText(),
						FNewMenuDelegate::CreateStatic(&AddStyleSetEntriesBindingMenuItems, MakeWeakObjectPtr(StyleSet), WidgetBlueprint, Widget, PropertyHandle),
						false
					);
				}
			}

			MenuBuilder.EndSection();
		}
	}
}

bool FMDStyleSetsPropertyBindingExtension::CanExtend(const UWidgetBlueprint* WidgetBlueprint, const UWidget* Widget, TSharedPtr<const IPropertyHandle> PropertyHandle) const
{
	if (!IsValid(WidgetBlueprint) || !IsValid(Widget) || !PropertyHandle.IsValid() || PropertyHandle->GetProperty() == nullptr || !IsValid(WidgetBlueprint->GeneratedClass))
	{
		return false;
	}

	// We can't support Set, Map, nor Optional properties since they're not supported in PropertyPathHelpersInternal::ResolvePropertyPath and FPropertyBindingPath::ResolveIndirectionsWithValue
	{
		TSharedPtr<const IPropertyHandle> TestHandle = PropertyHandle;
		while (TestHandle.IsValid() && TestHandle->GetProperty() != nullptr)
		{
			const FProperty* TestProperty = TestHandle->GetProperty();
			if (TestProperty->IsA<FSetProperty>() || TestProperty->IsA<FMapProperty>() || TestProperty->IsA<FOptionalProperty>())
			{
				return false;
			}

			TestHandle = TestHandle->GetParentHandle();
		}
	}

	const FProperty* Property = PropertyHandle->GetProperty();
	if (Widget->GetClass()->ClassGeneratedBy == WidgetBlueprint || (Widget->bIsVariable && WidgetBlueprint->GeneratedClass->FindPropertyByName(Widget->GetFName()) != nullptr))
	{
		const FPropertyBagPropertyDesc Desc(Property->GetFName(), Property);
		return UMDStyleSetFunctionLibrary::HasConvertibleStyleSets(Desc);
	}

	return false;
}

TSharedPtr<FExtender> FMDStyleSetsPropertyBindingExtension::CreateMenuExtender(const UWidgetBlueprint* WidgetBlueprint, UWidget* Widget, TSharedPtr<IPropertyHandle> PropertyHandle)
{
	TSharedPtr<FExtender> Extender = MakeShared<FExtender>();
	Extender->AddMenuExtension("BindingActions", EExtensionHook::Before, nullptr, FMenuExtensionDelegate::CreateStatic(&MDStyleSetsPropertyBindingExtension::AddStyleSetBindingMenuItems,
		MakeWeakObjectPtr(WidgetBlueprint),
		MakeWeakObjectPtr(Widget),
		PropertyHandle
	));
	return Extender;
}

void FMDStyleSetsPropertyBindingExtension::ClearCurrentValue(const UWidgetBlueprint* WidgetBlueprint, const UWidget* Widget, TSharedPtr<IPropertyHandle> PropertyHandle)
{
	if (PropertyHandle.IsValid() && IsValid(WidgetBlueprint))
	{
		UMDStyleSetBlueprintExtension* Extension = UMDStyleSetBlueprintExtension::GetExtension(WidgetBlueprint);
		if (IsValid(Extension))
		{
			const FPropertyBindingPath PropertyBindingPath = MDStyleSetsPropertyBindingExtension::BuildPropertyPath(WidgetBlueprint, const_cast<UWidget*>(Widget), PropertyHandle);
			if (const FMDStyleSetPropertyBinding* Binding = Extension->FindBindingForProperty(PropertyBindingPath))
			{
				FScopedTransaction ClearBindingTransaction(FText::Format(INVTEXT("Removed Style Binding for '{0}' to '{1} - {2}'"),
					FText::FromString(Binding->TargetProperty.ToString()), Binding->Value.StyleSet->GetDisplayName(), FText::FromString(Binding->Value.StyleSet->StyleSetTag.ToString())));
				Extension->RemoveBindingForProperty(PropertyBindingPath);
			}
		}
	}
}

TOptional<FName> FMDStyleSetsPropertyBindingExtension::GetCurrentValue(const UWidgetBlueprint* WidgetBlueprint, const UWidget* Widget, TSharedPtr<IPropertyHandle> PropertyHandle) const
{
	if (PropertyHandle.IsValid() && IsValid(WidgetBlueprint))
	{
		UMDStyleSetBlueprintExtension* Extension = UMDStyleSetBlueprintExtension::GetExtension(WidgetBlueprint);
		if (IsValid(Extension))
		{
			const FPropertyBindingPath PropertyBindingPath = MDStyleSetsPropertyBindingExtension::BuildPropertyPath(WidgetBlueprint, const_cast<UWidget*>(Widget), PropertyHandle);
			if (const FMDStyleSetPropertyBinding* Binding = Extension->FindBindingForProperty(PropertyBindingPath))
			{
				return Binding->Value.StyleValueTag.GetTagName();
			}
		}
	}

	return {};
}

const FSlateBrush* FMDStyleSetsPropertyBindingExtension::GetCurrentIcon(const UWidgetBlueprint* WidgetBlueprint, const UWidget* Widget, TSharedPtr<IPropertyHandle> PropertyHandle) const
{
	if (PropertyHandle.IsValid() && IsValid(WidgetBlueprint))
	{
		UMDStyleSetBlueprintExtension* Extension = UMDStyleSetBlueprintExtension::GetExtension(WidgetBlueprint);
		if (IsValid(Extension))
		{
			const FPropertyBindingPath PropertyBindingPath = MDStyleSetsPropertyBindingExtension::BuildPropertyPath(WidgetBlueprint, const_cast<UWidget*>(Widget), PropertyHandle);
			if (const FMDStyleSetPropertyBinding* Binding = Extension->FindBindingForProperty(PropertyBindingPath))
			{
				if (const UMDStyleSet* StyleSet = Binding->Value.StyleSet)
				{
					return FBlueprintEditorUtils::GetIconFromPin(StyleSet->StyleType);
				}
			}
		}
	}

	return nullptr;
}
