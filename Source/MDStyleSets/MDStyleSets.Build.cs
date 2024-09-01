// Copyright Dylan Dumesnil. All Rights Reserved.

using UnrealBuildTool;

public class MDStyleSets : ModuleRules
{
	public MDStyleSets(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"DeveloperSettings",
				"GameplayTags",
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
				"Slate",
				"SlateCore",
			}
		);
	}
}
