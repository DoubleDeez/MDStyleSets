// Copyright Dylan Dumesnil. All Rights Reserved.

#pragma once

#include "UObject/Interface.h"
#include "MDStyleSetPreviewWidgetInterface.generated.h"

struct FGameplayTag;
class UMDStyleSet;

UINTERFACE(Blueprintable)
class UMDStyleSetPreviewWidgetInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * Interface for implement Style Set preview widgets
 */
class MDSTYLESETS_API IMDStyleSetPreviewWidgetInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintImplementableEvent, Category = "Style Set")
	void SetStyleSetPreviewValue(UMDStyleSet* StyleSet, const FGameplayTag& StyleTag);
};
