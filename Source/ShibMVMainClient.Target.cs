// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class ShibMVMainClientTarget : TargetRules
{
	public ShibMVMainClientTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Client;
		DefaultBuildSettings = BuildSettingsVersion.V5;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_4;
		bUseLoggingInShipping = true;
		ExtraModuleNames.Add("ShibMVMain");
	}
}
