// Copyright Dylan Dumesnil. All Rights Reserved.

#pragma once

#include "MDStyleSetTypeHandlerBase.h"
#include "MDStyleSetTypeHandler_Numeric.generated.h"

/**
 * Allows converting between different numeric types
 */
UCLASS(DisplayName = "Numeric Style Handler")
class MDSTYLESETS_API UMDStyleSetTypeHandler_Numeric : public UMDStyleSetTypeHandlerBase
{
	GENERATED_BODY()

public:
	virtual void GetConvertibleTypes(TArray<FPropertyBagPropertyDesc>& OutConvertibleTypes) const override;

	virtual bool TrySetValue(const TTuple<FPropertyBagPropertyDesc, const uint8*>& Value, const FPropertyBagPropertyDesc& DestDesc, void* DestPtr) const override;

	virtual FText CreateValuePreviewText_Implementation(UMDStyleSet* StyleSet, const FGameplayTag& StyleTag) const override;
};
