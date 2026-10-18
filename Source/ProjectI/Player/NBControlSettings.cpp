// Squirrel Wheels prototype.

#include "Player/NBControlSettings.h"

#include "Engine/GameInstance.h"
#include "Kismet/GameplayStatics.h"

void UNBControlSettings::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	LookSensitivity = FMath::Clamp(LookSensitivity, MinLookSensitivity, MaxLookSensitivity);
	CameraShake = FMath::Clamp(CameraShake, 0.f, 1.f);
}

UNBControlSettings* UNBControlSettings::Get(const UObject* WorldContext)
{
	const UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(WorldContext);
	return GameInstance ? GameInstance->GetSubsystem<UNBControlSettings>() : nullptr;
}

void UNBControlSettings::SetLookSensitivity(float Value)
{
	LookSensitivity = FMath::Clamp(Value, MinLookSensitivity, MaxLookSensitivity);
	SaveConfig();
}

void UNBControlSettings::SetLookInverted(bool bInverted)
{
	bInvertLook = bInverted;
	SaveConfig();
}

void UNBControlSettings::SetCameraShake(float Value)
{
	CameraShake = FMath::Clamp(Value, 0.f, 1.f);
	SaveConfig();
}
