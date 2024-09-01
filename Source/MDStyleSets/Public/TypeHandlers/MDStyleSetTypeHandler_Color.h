// Copyright Dylan Dumesnil. All Rights Reserved.

#pragma once

#include "MDStyleSetTypeHandlerBase.h"
#include "MDStyleSetTypeHandler_Color.generated.h"

/**
 * Allows converting between different color types and displays a preview of the color
 */
UCLASS(DisplayName = "Color Style Handler")
class MDSTYLESETS_API UMDStyleSetTypeHandler_Color : public UMDStyleSetTypeHandlerBase
{
	GENERATED_BODY()

public:
	virtual void GetConvertibleTypes(TArray<FPropertyBagPropertyDesc>& OutConvertibleTypes) const override;

	virtual bool TrySetValue(const TTuple<FPropertyBagPropertyDesc, const uint8*>& Value, const FPropertyBagPropertyDesc& DestDesc, void* DescPtr) const override;

	virtual TSharedRef<SWidget> CreateValuePreviewWidget(UMDStyleSet* StyleSet, const FGameplayTag& StyleTag) const override;

	virtual FText GetValueAsText_Implementation(const UMDStyleSet* StyleSet, const FGameplayTag& StyleTag) const override;
};
