// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class ProjectI : ModuleRules
{
	public ProjectI(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		// Lets sources include by feature folder, e.g. "Car/NBCar.h".
		PublicIncludePaths.Add(ModuleDirectory);

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput", "PhysicsCore" });

		PrivateDependencyModuleNames.AddRange(new string[] { "NetCore", "OnlineSubsystem", "OnlineSubsystemUtils", "UMG", "Slate", "SlateCore", "Niagara" });
	}
}
