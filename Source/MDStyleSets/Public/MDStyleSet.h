// Copyright Dylan Dumesnil. All Rights Reserved.

#pragma once

#include "EdGraph/EdGraphPin.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "PropertyBag.h"

#include "MDStyleSet.generated.h"

class UMDStyleSetTypeHandlerBase;

USTRUCT()
struct MDSTYLESETS_API FMDStyleValue
{
	GENERATED_BODY()

public:
	static const FName ValuePropertyName;

	UPROPERTY(EditDefaultsOnly, Category = "Style Set", meta = (ShowOnlyInnerProperties, FixedLayout))
	FInstancedPropertyBag Value;

	TTuple<FPropertyBagPropertyDesc, const uint8*> GetValue() const;
};

UCLASS(BlueprintType)
class MDSTYLESETS_API UMDStyleSet : public UDataAsset
{
	GENERATED_BODY()

public:
	static const FName ConvertibleTypesAssetTagName;

#if WITH_EDITOR
	static EPropertyBagPropertyType GetValueTypeFromPinType(const FEdGraphPinType& PinType);
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual EDataValidationResult IsDataValid(FDataValidationContext& Context) const override;
#endif

	virtual void GetAssetRegistryTags(FAssetRegistryTagsContext Context) const override;

	TTuple<FPropertyBagPropertyDesc, const uint8*> GetStyleValue(const FGameplayTag& ValueTag) const;

	bool TrySetPropertyValue(const FGameplayTag& ValueTag, const FProperty* DestProp, void* DestPtr) const;

	FText GetDisplayName() const { return DisplayName.IsEmpty() ? FText::FromName(GetFName()) : FText::FromString(DisplayName); }

	FText GetValueDisplayName(const FGameplayTag& ValueTag) const;

	bool DoesHaveValueWithTag(const FGameplayTag& ValueTag) const;

	UPROPERTY(EditDefaultsOnly, Category = "Style Set")
	FEdGraphPinType StyleType;

	UPROPERTY(EditDefaultsOnly, Category = "Style Set")
	FString DisplayName;

	UPROPERTY(EditDefaultsOnly, Category = "Style Set", meta = (Categories = "Style"))
	FGameplayTag StyleSetTag;

	UPROPERTY(EditDefaultsOnly, Instanced, Category = "Style Set")
	TObjectPtr<UMDStyleSetTypeHandlerBase> TypeHandler = nullptr;

	// The value to use if the requested tag is not found
	UPROPERTY(EditDefaultsOnly, Category = "Style Set")
	FMDStyleValue FallbackValue;

	UPROPERTY(EditDefaultsOnly, Category = "Style Set", meta = (ForceInlineRow))
	TMap<FGameplayTag, FMDStyleValue> StyleEntries;

private:
	// Sort entries alphabetically by their tag
	UFUNCTION(CallInEditor, Category = "Style Set")
	void SortEntries();
};
