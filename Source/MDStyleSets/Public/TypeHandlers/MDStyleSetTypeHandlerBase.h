// Copyright Dylan Dumesnil. All Rights Reserved.

#pragma once

#include "PropertyBag.h"
#include "Templates/SubclassOf.h"
#include "UObject/Object.h"
#include "UObject/StrongObjectPtr.h"
#include "Widgets/SCompoundWidget.h"
#include "MDStyleSetTypeHandlerBase.generated.h"

struct FGameplayTag;
class SWidget;
class UWidget;
class UMDStyleSet;

class SMDStyleSetPreviewUWidgetWrapper : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SMDStyleSetPreviewUWidgetWrapper) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, UWidget* Widget);

private:
	TStrongObjectPtr<UWidget> StrongWidgetPtr;
};

/**
 * Utility class for handling additional operations for specific types used by Style Sets
 */
UCLASS(EditInlineNew, DefaultToInstanced, Blueprintable, CollapseCategories, DisplayName = "Base Style Handler")
class MDSTYLESETS_API UMDStyleSetTypeHandlerBase : public UObject
{
	GENERATED_BODY()

public:
	virtual void GetConvertibleTypes(TArray<FPropertyBagPropertyDesc>& OutConvertibleTypes) const {}

	virtual bool TrySetValue(const TTuple<FPropertyBagPropertyDesc, const uint8*>& Value, const FPropertyBagPropertyDesc& DestDesc, void* DestPtr) const { return false; }

	virtual TSharedRef<SWidget> CreateValuePreviewWidget(UMDStyleSet* StyleSet, const FGameplayTag& StyleTag) const;

	// Return a preview of the value of a style entry as text (can leave empty if a Preview Widget is used instead)
	UFUNCTION(BlueprintNativeEvent, Category = "Style Set")
	FText CreateValuePreviewText(UMDStyleSet* StyleSet, const FGameplayTag& StyleTag) const;
	virtual FText CreateValuePreviewText_Implementation(UMDStyleSet* StyleSet, const FGameplayTag& StyleTag) const;

	// Return the full value of a style entry as text
	UFUNCTION(BlueprintNativeEvent, Category = "Style Set")
	FText GetValueAsText(const UMDStyleSet* StyleSet, const FGameplayTag& StyleTag) const;
	virtual FText GetValueAsText_Implementation(const UMDStyleSet* StyleSet, const FGameplayTag& StyleTag) const;

protected:
	// Optional widget class to instance when previewing the values of the supported type, must implement MDStyleSetPreviewWidgetInterface
	UPROPERTY(EditDefaultsOnly, Category = "Type Handler", meta = (MustImplement = "/Script/MDStyleSets.MDStyleSetPreviewWidgetInterface"))
	TSubclassOf<UWidget> PreviewWidgetClass;

};
