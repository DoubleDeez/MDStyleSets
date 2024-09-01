// Copyright Dylan Dumesnil. All Rights Reserved.

using UnrealBuildTool;

public class MDStyleSetsBlueprint : ModuleRules
{
    public MDStyleSetsBlueprint(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
	            "BlueprintGraph",
                "Core",
                "Kismet",
                "MDStyleSets",
                "PropertyBindingUtils",
                "PropertyPath",
                "StructUtils",
                "UMG"
            }
        );

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "CoreUObject",
                "Engine",
                "GameplayTags",
                "Slate",
                "SlateCore",
                "UMGEditor",
                "UnrealEd"
            }
        );
    }
}