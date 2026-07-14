// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class ShibMVMain : ModuleRules
{
	public ShibMVMain(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"UMG",
			"OnlineSubsystem",
			"OnlineSubsystemUtils",
			"OnlineSubsystemBlueprints",
			"Networking",
			"ShibMultiplayer",
			"VoiceChat",
			"ShibAsyncLoadingScreen",
			"Slate",
			"SlateCore",
			"MoviePlayer",
			"Matchmaking",
			"ShibUiNavigation",
			"DeveloperSettings",
            "ShibAPIs",
            "ShibAvatarBuilder",
            "ShibPlotBuilder",
			"Json",
			"JsonUtilities",
			"Riverology_Plugin", 
			"MediaAssets",
			"AudioExtensions"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore", });
	}
}