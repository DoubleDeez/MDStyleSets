// Copyright Dylan Dumesnil. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "PropertyBag.h"

#include "MDStyleSetFunctionLibrary.generated.h"

struct FMDStyleSetValueReference;
struct FGameplayTag;
class UMDStyleSet;

/**
 *
 */
UCLASS()
class MDSTYLESETS_API UMDStyleSetFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, CustomThunk, meta = (Category = "Style Set", CustomStructureParam = "OutValue", AutoCreateRefTerm = "StyleTag"))
	static void GetStyleValue(UMDStyleSet* StyleSet, UPARAM(meta = (Categories = "Style")) const FGameplayTag& StyleTag, int32& OutValue);
	DECLARE_FUNCTION(execGetStyleValue);

	static TTuple<FPropertyBagPropertyDesc, const uint8*> GetStyleSetValue(const FMDStyleSetValueReference& ValueReference);

	static bool HasConvertibleStyleSets(const FPropertyBagPropertyDesc& Desc);

	static TArray<UMDStyleSet*> GetConvertibleStyleSets(const FPropertyBagPropertyDesc& Desc);

	static TArray<FPropertyBagPropertyDesc> GetConvertibleTypes(const FAssetData& Asset);

};
