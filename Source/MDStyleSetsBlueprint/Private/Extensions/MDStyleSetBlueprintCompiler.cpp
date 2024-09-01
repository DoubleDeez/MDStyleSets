// Copyright Dylan Dumesnil. All Rights Reserved.


#include "Extensions/MDStyleSetBlueprintCompiler.h"

#include "Editor.h"
#include "Extensions/MDStyleSetBlueprintExtension.h"
#include "MDStyleSet.h"
#include "WidgetBlueprintCompiler.h"

DEFINE_LOG_CATEGORY_STATIC(LogMDStyleSetCompiler, Warning, All);

EMDStyleSetBindingExecutionResult UMDStyleSetBlueprintCompiler::ExecuteBindingOnBlueprint(UBlueprint* Blueprint, const FPropertyBindingDataView BaseValueView, const FMDStyleSetPropertyBinding& Binding)
{
	if (!IsValid(Binding.Value.StyleSet))
	{
		UE_LOG(LogMDStyleSetCompiler, Error, TEXT("Error executing binding BP: [%s] | Property: [%s] | Error: [Invalid StyleSet]"), *GetNameSafe(Blueprint), *Binding.TargetProperty.ToString());
		return EMDStyleSetBindingExecutionResult::StyleNotFound;
	}

	if (Binding.Value.StyleSet->HasAnyFlags(RF_NeedLoad))
	{
		Binding.Value.StyleSet->GetLinker()->Preload(Binding.Value.StyleSet);
	}

	if (Binding.Value.StyleSet->HasAnyFlags(RF_NeedPostLoad))
	{
		Binding.Value.StyleSet->ConditionalPostLoad();
	}

	if (!Binding.Value.StyleSet->DoesHaveValueWithTag(Binding.Value.StyleValueTag))
	{
		UE_LOG(LogMDStyleSetCompiler, Error, TEXT("Error executing binding BP: [%s] | Property: [%s] | StyleSet [%s] does not have value for tag [%s]"), *GetNameSafe(Blueprint), *Binding.TargetProperty.ToString(), *Binding.Value.StyleSet->GetDisplayName().ToString(), *Binding.Value.StyleValueTag.ToString());
		return EMDStyleSetBindingExecutionResult::StyleNotFound;
	}

	FString Error;
	TArray<FPropertyBindingPathIndirection> Indirections;
	if (Binding.TargetProperty.ResolveIndirectionsWithValue(BaseValueView, Indirections, &Error, true) && !Indirections.IsEmpty())
	{
		// If we didn't find a value, check if we're binding to a widget, since it doesn't have sub-widgets on the CDO
		if (Indirections.Last().GetContainerAddress() == nullptr)
		{
			if (UWidgetBlueprint* WidgetBP = Cast<UWidgetBlueprint>(Blueprint))
			{
				const FPropertyBindingPathSegment& FirstSegment = Binding.TargetProperty.GetSegment(0);
				UWidget** FirstWidgetPtr = WidgetBP->GetAllSourceWidgets().FindByPredicate([WidgetName = FirstSegment.GetName()](UWidget* Widget)
				{
					return IsValid(Widget) && Widget->GetFName() == WidgetName;
				});

				if (FirstWidgetPtr != nullptr)
				{
					FPropertyBindingPath TrimmedPath;
					for (int32 i = 1; i < Binding.TargetProperty.NumSegments(); ++i)
					{
						TrimmedPath.AddPathSegment(Binding.TargetProperty.GetSegment(i));
					}

					TrimmedPath.ResolveIndirectionsWithValue(*FirstWidgetPtr, Indirections, &Error, true);
				}
			}
		}
	}

	UE_CLOG(!Error.IsEmpty(), LogMDStyleSetCompiler, Error, TEXT("Error executing binding BP: [%s] | Property: [%s] | Error: [%s]"), *GetNameSafe(Blueprint), *Binding.TargetProperty.ToString(), *Error);

	if (Indirections.IsEmpty() || Indirections.Last().GetContainerAddress() == nullptr)
	{
		return EMDStyleSetBindingExecutionResult::PropertyNotFound;
	}

	const FProperty* Property = Indirections.Last().GetProperty();
	void* TargetAddress = Indirections.Last().GetMutablePropertyAddress();
	const bool bDidSetValue = Binding.Value.StyleSet->TrySetPropertyValue(Binding.Value.StyleValueTag, Property, TargetAddress);

	return bDidSetValue ? EMDStyleSetBindingExecutionResult::Success : EMDStyleSetBindingExecutionResult::CouldNotSetValue;
}

void UMDStyleSetBlueprintCompiler::ExecuteBindingsOnBlueprint(UBlueprint* Blueprint, UMDStyleSetBlueprintExtension* BPExtension, bool bShouldRemoveFailedBindings)
{
	if (IsValid(Blueprint) && IsValid(BPExtension))
	{
		for (auto It = BPExtension->Bindings.CreateIterator(); It; ++It)
		{
			const EMDStyleSetBindingExecutionResult Result = ExecuteBindingOnBlueprint(Blueprint, Blueprint->GeneratedClass->GetDefaultObject(), *It);
			if (bShouldRemoveFailedBindings && Result == EMDStyleSetBindingExecutionResult::PropertyNotFound)
			{
				It.RemoveCurrent();
			}
		}
	}
}

void UMDStyleSetBlueprintCompiler::BeginDestroy()
{
	if (GEditor != nullptr)
	{
		GEditor->OnBlueprintPreCompile().Remove(PreCompileHandle);
		PreCompileHandle.Reset();
	}

	Super::BeginDestroy();
}

void UMDStyleSetBlueprintCompiler::BindPreCompile()
{
	FCoreDelegates::OnPostEngineInit.AddWeakLambda(this, [this]()
	{
		if (GEditor != nullptr)
		{
			PreCompileHandle = GEditor->OnBlueprintPreCompile().AddUObject(this, &UMDStyleSetBlueprintCompiler::OnBlueprintPreCompile);
		}
	});
}

void UMDStyleSetBlueprintCompiler::OnBlueprintPreCompile(UBlueprint* Blueprint)
{
	if (IsValid(Blueprint))
	{
		UMDStyleSetBlueprintExtension* BPExtension = UMDStyleSetBlueprintExtension::GetExtension(Blueprint);
		if (!IsValid(BPExtension))
		{
			return;
		}
		else if (BPExtension->Bindings.IsEmpty())
		{
			Blueprint->RemoveExtension(BPExtension);
		}
		else if (Blueprint->GeneratedClass != nullptr)
		{
			constexpr bool bShouldRemoveFailedBindings = true;
			ExecuteBindingsOnBlueprint(Blueprint, BPExtension, bShouldRemoveFailedBindings);
		}
	}
}
