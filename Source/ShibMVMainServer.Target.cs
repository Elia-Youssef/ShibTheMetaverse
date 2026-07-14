// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class ShibMVMainServerTarget : TargetRules
{
	public ShibMVMainServerTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Server;
		DefaultBuildSettings = BuildSettingsVersion.V5;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_4;
		bUseLoggingInShipping = true;
		ExtraModuleNames.Add("ShibMVMain");
	}
}
