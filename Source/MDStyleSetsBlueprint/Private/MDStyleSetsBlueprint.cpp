// Copyright Dylan Dumesnil. All Rights Reserved.

#include "MDStyleSetsBlueprint.h"

#include "BlueprintCompilationManager.h"
#include "Extensions/MDStyleSetBlueprintCompiler.h"

#define LOCTEXT_NAMESPACE "FMDStyleSetsBlueprintModule"

void FMDStyleSetsBlueprintModule::StartupModule()
{
	UMDStyleSetBlueprintCompiler* Compiler = NewObject<UMDStyleSetBlueprintCompiler>();
	Compiler->BindPreCompile();
	FBlueprintCompilationManager::RegisterCompilerExtension(UBlueprint::StaticClass(), Compiler);
}

void FMDStyleSetsBlueprintModule::ShutdownModule()
{
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FMDStyleSetsBlueprintModule, MDStyleSetsBlueprint)