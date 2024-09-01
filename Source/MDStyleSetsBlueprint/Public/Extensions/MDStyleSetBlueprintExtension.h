// Copyright Dylan Dumesnil. All Rights Reserved.

#pragma once

#include "Blueprint/BlueprintExtension.h"
#include "Util/MDStyleSetTypes.h"

#include "MDStyleSetBlueprintExtension.generated.h"

/**
 * Stores a blueprint's Style Set Property Bindings
 */
UCLASS()
class MDSTYLESETSBLUEPRINT_API UMDStyleSetBlueprintExtension : public UBlueprintExtension
{
	GENERATED_BODY()

public:
	static UMDStyleSetBlueprintExtension* GetOrCreateExtension(UBlueprint* Blueprint);
	static UMDStyleSetBlueprintExtension* GetExtension(const UBlueprint* Blueprint);

	void ExecuteBindingOnCDO(const FMDStyleSetPropertyBinding& Binding) const;
	void AddBinding(FMDStyleSetPropertyBinding&& Binding);
	const FMDStyleSetPropertyBinding* FindBindingForProperty(const FPropertyBindingPath& PropertyPath) const;
	bool RemoveBindingForProperty(const FPropertyBindingPath& PropertyPath);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Style Set")
	TArray<FMDStyleSetPropertyBinding> Bindings;
};
