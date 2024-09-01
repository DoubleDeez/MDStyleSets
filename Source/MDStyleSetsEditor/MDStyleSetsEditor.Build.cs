// Copyright Dylan Dumesnil. All Rights Reserved.

using UnrealBuildTool;

public class MDStyleSetsEditor : ModuleRules
{
    public MDStyleSetsEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "GraphEditor",
                "StructUtils",
                "StructUtilsEditor",
                "UMGEditor",
            }
        );

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
	            "BlueprintGraph",
                "CoreUObject",
                "Engine",
                "GameplayTags",
                "KismetWidgets",
                "MDStyleSetsBlueprint",
                "MDStyleSets",
                "PropertyBindingUtils",
                "PropertyPath",
                "Slate",
                "SlateCore",
                "UMG",
                "UnrealEd"
            }
        );
    }
}