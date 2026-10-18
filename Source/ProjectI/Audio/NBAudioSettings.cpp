// Squirrel Wheels prototype.

#include "Audio/NBAudioSettings.h"

#include "AudioDevice.h"
#include "AudioMixerBlueprintLibrary.h"
#include "Components/AudioComponent.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "HAL/IConsoleManager.h"
#include "Kismet/GameplayStatics.h"

void UNBAudioSettings::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	GameVolume = FMath::Clamp(GameVolume, 0.f, MaxGameVolume);
	VoiceVolume = FMath::Clamp(VoiceVolume, 0.f, MaxVoiceVolume);
	MicBoost = FMath::Clamp(MicBoost, 0.f, MaxMicBoost);
	ApplyGameVolume();
	ApplyMicBoost();
	if (!OutputDeviceId.IsEmpty())
	{
		SetOutputDevice(OutputDeviceId);
	}
}

UNBAudioSettings* UNBAudioSettings::Get(const UObject* WorldContext)
{
	const UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(WorldContext);
	return GameInstance ? GameInstance->GetSubsystem<UNBAudioSettings>() : nullptr;
}

void UNBAudioSettings::SetGameVolume(float Value)
{
	GameVolume = FMath::Clamp(Value, 0.f, MaxGameVolume);
	ApplyGameVolume();
	// The primary volume scales voices too; re-balance them so this slider only moves the game.
	ApplyVoiceVolume();
	Save();
}

void UNBAudioSettings::SetVoiceVolume(float Value)
{
	VoiceVolume = FMath::Clamp(Value, 0.f, MaxVoiceVolume);
	ApplyVoiceVolume();
	Save();
}

void UNBAudioSettings::SetMicBoost(float Value)
{
	MicBoost = FMath::Clamp(Value, 0.f, MaxMicBoost);
	ApplyMicBoost();
	Save();
}

void UNBAudioSettings::SetOutputDevice(const FString& DeviceId)
{
	OutputDeviceId = DeviceId;
	Save();
	// Asynchronous; an empty id falls back to the system default device.
	UAudioMixerBlueprintLibrary::SwapAudioOutputDevice(GetGameInstance(), DeviceId, FOnCompletedDeviceSwap());
}

void UNBAudioSettings::AddVoiceComponent(UAudioComponent* Voice)
{
	if (!Voice)
	{
		return;
	}
	VoiceComponents.RemoveAll([](const TWeakObjectPtr<UAudioComponent>& Each) { return !Each.IsValid(); });
	VoiceComponents.AddUnique(Voice);
	Voice->SetVolumeMultiplier(GetVoiceMultiplier());
}

void UNBAudioSettings::ApplyGameVolume() const
{
	if (GEngine)
	{
		if (FAudioDeviceHandle Device = GEngine->GetMainAudioDevice())
		{
			Device->SetTransientPrimaryVolume(GameVolume);
		}
	}
}

void UNBAudioSettings::ApplyMicBoost() const
{
	if (IConsoleVariable* Gain = IConsoleManager::Get().FindConsoleVariable(TEXT("voice.MicInputGain")))
	{
		Gain->Set(MicBoost, ECVF_SetByGameSetting);
	}
}

float UNBAudioSettings::GetVoiceMultiplier() const
{
	// SetTransientPrimaryVolume turns everything down, voices included (friend test 2: the game drowned
	// the voices and there was no way to fix it), so divide it back out of the voice. Capped where the
	// engine's own volume clamp would cut in anyway.
	return FMath::Min(VoiceVolume / FMath::Max(GameVolume, 0.05f), 4.f);
}

void UNBAudioSettings::ApplyVoiceVolume()
{
	for (const TWeakObjectPtr<UAudioComponent>& Voice : VoiceComponents)
	{
		if (Voice.IsValid())
		{
			Voice->SetVolumeMultiplier(GetVoiceMultiplier());
		}
	}
}

void UNBAudioSettings::Save()
{
	SaveConfig();
}
