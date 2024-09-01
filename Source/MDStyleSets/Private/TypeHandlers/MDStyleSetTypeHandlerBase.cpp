// Copyright Dylan Dumesnil. All Rights Reserved.


#include "TypeHandlers/MDStyleSetTypeHandlerBase.h"

#include "Components/Widget.h"
#include "MDStyleSet.h"
#include "MDStyleSetPreviewWidgetInterface.h"
#include "UObject/Package.h"
#include "Widgets/SNullWidget.h"

void SMDStyleSetPreviewUWidgetWrapper::Construct(const FArguments& InArgs, UWidget* Widget)
{
	if (IsValid(Widget))
	{
		StrongWidgetPtr.Reset(Widget);

		ChildSlot
		[
			Widget->TakeWidget()
		];
	}
}

TSharedRef<SWidget> UMDStyleSetTypeHandlerBase::CreateValuePreviewWidget(UMDStyleSet* StyleSet, const FGameplayTag& StyleTag) const
{
	if (IsValid(PreviewWidgetClass))
	{
		if (UWidget* PreviewWidget = NewObject<UWidget>(GetTransientPackage(), PreviewWidgetClass))
		{
			IMDStyleSetPreviewWidgetInterface::Execute_SetStyleSetPreviewValue(PreviewWidget, StyleSet, StyleTag);
			return SNew(SMDStyleSetPreviewUWidgetWrapper, PreviewWidget);
		}
	}

	return SNullWidget::NullWidget;
}

FText UMDStyleSetTypeHandlerBase::CreateValuePreviewText_Implementation(UMDStyleSet* StyleSet, const FGameplayTag& StyleTag) const
{
	return FText::GetEmpty();
}

FText UMDStyleSetTypeHandlerBase::GetValueAsText_Implementation(const UMDStyleSet* StyleSet, const FGameplayTag& StyleTag) const
{
	if (!IsValid(StyleSet))
	{
		return INVTEXT("Invalid Style Set");
	}

	TTuple<FPropertyBagPropertyDesc, const uint8*> StyleValue = StyleSet->GetStyleValue(StyleTag);
	const FProperty* Property = StyleValue.Key.CachedProperty;
	if (Property == nullptr || StyleValue.Value == nullptr)
	{
		return INVTEXT("Invalid Style Value");
	}

	FString OutStringValue;
	if (!Property->ExportText_Direct(OutStringValue, StyleValue.Value, StyleValue.Value, nullptr, PPF_None))
	{
		return INVTEXT("Invalid Style Value");
	}

	return FText::FromString(OutStringValue);
}
