// Copyright Epic Games, Inc. All Rights Reserved.

using System.IO;
using UnrealBuildTool;

public class ProjectI : ModuleRules
{
	public ProjectI(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		// Lets sources include by feature folder, e.g. "Car/NBCar.h".
		PublicIncludePaths.Add(ModuleDirectory);

		// Imported C++ Vehicle template: its files include each other by bare name.
		foreach (string Dir in new string[] { "", "OffroadCar", "SportsCar", "Variant_OffRoad", "Variant_TimeTrial", "Variant_TimeTrial/UI" })
		{
			PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "TP_VehicleAdv", Dir));
		}

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput", "PhysicsCore", "ChaosVehicles" });

		PrivateDependencyModuleNames.AddRange(new string[] { "NetCore", "OnlineSubsystem", "OnlineSubsystemUtils", "UMG", "Slate", "SlateCore", "Niagara", "AudioExtensions" });
	}
}
