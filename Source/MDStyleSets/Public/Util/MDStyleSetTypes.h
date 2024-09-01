// Copyright Dylan Dumesnil. All Rights Reserved.

#pragma once

#include "GameplayTagContainer.h"
#include "PropertyBindingPath.h"
#include "PropertyPathHelpers.h"

#include "MDStyleSetTypes.generated.h"

class UMDStyleSet;

USTRUCT(BlueprintType)
struct FMDStyleSetValueReference
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Style Set Value")
	TObjectPtr<UMDStyleSet> StyleSet;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Style Set Value", meta = (Categories = "Style"))
	FGameplayTag StyleValueTag;
};

USTRUCT(BlueprintType)
struct FMDStyleSetPropertyBinding
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Style Set Value")
	FMDStyleSetValueReference Value;

	UPROPERTY(EditAnywhere, Category = "Style Set Value")
	FPropertyBindingPath TargetProperty;
};
