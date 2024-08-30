// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class AnimDataFromJSON : ModuleRules
{
	public AnimDataFromJSON(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicIncludePaths.AddRange(
			new string[] {
				// ... add public include paths required here ...
			}
			);
				
		
		PrivateIncludePaths.AddRange(
			new string[] {
				// ... add other private include paths required here ...
			}
			);
			
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
                "Core",
                "Projects",
                "InputCore",
                "LevelSequence",
                "CoreUObject",
                "UnrealEd",
                "Engine",
                "MovieScene",
                "MovieSceneTracks",
                "LevelSequenceEditor",
                "DesktopPlatform",
                "MovieSceneTracks",
                "Sequencer",
                "Json",
                "JsonUtilities",
                "HeadMountedDisplay",
                "FunctionalTesting",
                "Slate",
                "SlateCore",
                "UMG",
                "UnrealEd",
                "EditorFramework",
                "DesktopPlatform",
                "ToolMenus",
				// ... add other public dependencies that you statically link with here ...
			}
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
                "Projects",
                "InputCore",
                "EditorFramework",
                "UnrealEd",
                "CoreUObject",
                "MovieScene",
                "MovieSceneTracks",
                "Engine",
                "Slate",
                "SlateCore",
                "HeadMountedDisplay",
                "FunctionalTesting",
				// ... add private dependencies that you statically link with here ...	
			}
			);
		
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
			);
	}
}



