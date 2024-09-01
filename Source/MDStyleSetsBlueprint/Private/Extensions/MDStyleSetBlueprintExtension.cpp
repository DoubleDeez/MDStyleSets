// Copyright Dylan Dumesnil. All Rights Reserved.

#include "Extensions/MDStyleSetBlueprintExtension.h"

#include "BlueprintCompilationManager.h"
#include "Engine/Blueprint.h"
#include "Extensions/MDStyleSetBlueprintCompiler.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "MDStyleSet.h"

UMDStyleSetBlueprintExtension* UMDStyleSetBlueprintExtension::GetOrCreateExtension(UBlueprint* Blueprint)
{
	if (IsValid(Blueprint))
	{
		UMDStyleSetBlueprintExtension* BPExtension = GetExtension(Blueprint);
		if (IsValid(BPExtension))
		{
			BPExtension->SetFlags(RF_Transactional);
			return BPExtension;
		}

		BPExtension = NewObject<UMDStyleSetBlueprintExtension>(Blueprint, NAME_None, RF_Transactional);
		Blueprint->AddExtension(BPExtension);
		return BPExtension;
	}

	return nullptr;
}

UMDStyleSetBlueprintExtension* UMDStyleSetBlueprintExtension::GetExtension(const UBlueprint* Blueprint)
{
	if (IsValid(Blueprint))
	{
		for (UBlueprintExtension* Extension : Blueprint->GetExtensions())
		{
			auto* BPExtension = Cast<UMDStyleSetBlueprintExtension>(Extension);
			if (IsValid(BPExtension))
			{
				BPExtension->SetFlags(RF_Transactional);
				return BPExtension;
			}
		}
	}

	return nullptr;
}

void UMDStyleSetBlueprintExtension::ExecuteBindingOnCDO(const FMDStyleSetPropertyBinding& Binding) const
{
	if (UBlueprint* Blueprint = GetTypedOuter<UBlueprint>())
	{
		if (IsValid(Blueprint->GeneratedClass))
		{
			if (UObject* CDO = Blueprint->GeneratedClass->GetDefaultObject())
			{
				UMDStyleSetBlueprintCompiler::ExecuteBindingOnBlueprint(Blueprint, CDO, Binding);
				FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(Blueprint);
			}
		}
	}
}

void UMDStyleSetBlueprintExtension::AddBinding(FMDStyleSetPropertyBinding&& Binding)
{
	if (IsValid(Binding.Value.StyleSet))
	{
		Modify();
		Bindings.RemoveAll([&Binding](const FMDStyleSetPropertyBinding& B)
			{
				return B.TargetProperty == Binding.TargetProperty;
			});
		const int32 BindingIndex = Bindings.Emplace(MoveTemp(Binding));

		FBlueprintEditorUtils::MarkBlueprintAsModified(GetTypedOuter<UBlueprint>());

		ExecuteBindingOnCDO(Bindings[BindingIndex]);
	}
}

const FMDStyleSetPropertyBinding* UMDStyleSetBlueprintExtension::FindBindingForProperty(const FPropertyBindingPath& PropertyPath) const
{
	for (const FMDStyleSetPropertyBinding& Binding : Bindings)
	{
		if (Binding.TargetProperty == PropertyPath)
		{
			return &Binding;
		}
	}

	return nullptr;
}

bool UMDStyleSetBlueprintExtension::RemoveBindingForProperty(const FPropertyBindingPath& PropertyPath)
{
	for (auto It = Bindings.CreateIterator(); It; ++It)
	{
		const FMDStyleSetPropertyBinding& Binding = *It;
		if (Binding.TargetProperty == PropertyPath)
		{
			Modify();
			It.RemoveCurrent();
			return true;
		}
	}

	return false;
}
