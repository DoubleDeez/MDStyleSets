// Copyright Dylan Dumesnil. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class UMDStyleSetBlueprintCompiler;

class FMDStyleSetsBlueprintModule : public IModuleInterface
{
public:
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;
};
