// Copyright Dylan Dumesnil. All Rights Reserved.

#pragma once

#include "GameplayTagContainer.h"
#include "K2Node_CallFunction.h"
#include "MDStyleSetNode_GetStyleValue.generated.h"

class UMDStyleSet;
/**
 *
 */
UCLASS()
class MDSTYLESETSBLUEPRINT_API UMDStyleSetNode_GetStyleValue : public UK2Node_CallFunction
{
	GENERATED_BODY()

public:
	UMDStyleSetNode_GetStyleValue();

	virtual void BeginDestroy() override;

	virtual FLinearColor GetNodeTitleColor() const override;
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	virtual FText GetPinDisplayName(const UEdGraphPin* Pin) const override;
	virtual FText GetTooltipText() const override;
	virtual FString GetPinMetaData(FName InPinName, FName InKey) override;
	virtual bool DrawNodeAsVariable() const override { return true; }

	virtual void AllocateDefaultPins() override;
	virtual void ReconstructNode() override;
	virtual void PinConnectionListChanged(UEdGraphPin* Pin) override;
	virtual void PinDefaultValueChanged(UEdGraphPin* Pin) override;
	virtual void ValidateNodeDuringCompilation(FCompilerResultsLog& MessageLog) const override;

	virtual UObject* GetJumpTargetForDoubleClick() const override;
	virtual bool CanJumpToDefinition() const override;
	virtual void JumpToDefinition() const override;

	virtual void GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const override;

	FGameplayTag GetStyleTag() const;

	UPROPERTY()
	TObjectPtr<UMDStyleSet> BoundStyleSet;

	FSimpleMulticastDelegate OnUpdatePreview;

private:
	void UpdatePinData();
	void UpdatePreviewWidget();

	void OnPropertyValueChanged(UObject* Object, FPropertyChangedEvent& Event);

};
