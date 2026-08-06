// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class TheManTest : ModuleRules
{
	public TheManTest(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput", "GameplayAbilities", "GameplayTags", "GameplayTasks", "BBBAimIK", "Niagara", "AIModule", "NavigationSystem", "UMG", "AnimGraphRuntime", "CinematicCamera", "Locomotor", "ControlRig", });

		PrivateDependencyModuleNames.AddRange(new string[] {  });
		if (Target.bBuildEditor)
		{
			PrivateDependencyModuleNames.AddRange(new string[] { "UnrealEd", "AssetTools", "AnimGraph", "BlueprintGraph", "ControlRigDeveloper", "AnimationBlueprintLibrary" });
		}

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });
		
		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
