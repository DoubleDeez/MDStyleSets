// Copyright Dylan Dumesnil. All Rights Reserved.

#pragma once

#include "BlueprintCompilerExtension.h"
#include "PropertyBindingPath.h"
#include "MDStyleSetBlueprintCompiler.generated.h"

class UMDStyleSetBlueprintExtension;
struct FMDStyleSetPropertyBinding;

enum class EMDStyleSetBindingExecutionResult : uint8
{
	Success,
	StyleNotFound,
	PropertyNotFound,
	CouldNotSetValue
};

/**
 *
 */
UCLASS()
class MDSTYLESETSBLUEPRINT_API UMDStyleSetBlueprintCompiler : public UBlueprintCompilerExtension
{
	GENERATED_BODY()

public:
	static EMDStyleSetBindingExecutionResult ExecuteBindingOnBlueprint(UBlueprint* Blueprint, const FPropertyBindingDataView BaseValueView, const FMDStyleSetPropertyBinding& Binding);
	static void ExecuteBindingsOnBlueprint(UBlueprint* Blueprint, UMDStyleSetBlueprintExtension* BPExtension, bool bShouldRemoveFailedBindings);

	virtual void BeginDestroy() override;

	void BindPreCompile();

protected:
	void OnBlueprintPreCompile(UBlueprint* Blueprint);

private:
	FDelegateHandle PreCompileHandle;
};
