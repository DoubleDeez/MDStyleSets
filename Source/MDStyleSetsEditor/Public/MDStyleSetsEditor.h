// Copyright Dylan Dumesnil. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FMDStyleSetNodeFactory;
class FMDStyleSetsPropertyBindingExtension;

class FMDStyleSetsEditorModule : public IModuleInterface
{
public:
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;

private:
	TSharedPtr<FMDStyleSetsPropertyBindingExtension> PropertyBinding;
	TSharedPtr<FMDStyleSetNodeFactory> StyleSetNodeFactory;
};
