// Squirrel Wheels prototype.

#include "Audio/NBAudioSettings.h"

#include "AudioDevice.h"
#include "AudioMixerBlueprintLibrary.h"
#include "Components/AudioComponent.h"
#include "Components/SynthComponent.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "HAL/IConsoleManager.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/UObjectIterator.h"
#include "VoiceCaptureDevices.h"

void UNBAudioSettings::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	GameVolume = FMath::Clamp(GameVolume, 0.f, MaxGameVolume);
	VoiceVolume = FMath::Clamp(VoiceVolume, 0.f, MaxVoiceVolume);
	MicBoost = FMath::Clamp(MicBoost, 0.f, MaxMicBoost);
	ApplyGameVolume();
	ApplyMicBoost();
	ApplyMicDevice();
	// In every log, so a tester's "nobody hears me" can be read from it.
	UE_LOG(LogTemp, Log, TEXT("NBVoice: mic '%s' (empty = default); mics here: %s"), *MicDevice, *FString::Join(GetMicDevices(), TEXT(" | ")));
	if (!OutputDeviceId.IsEmpty())
	{
		SetOutputDevice(OutputDeviceId);
	}
	WorldCleanupHandle = FWorldDelegates::OnWorldCleanup.AddUObject(this, &UNBAudioSettings::OnWorldCleanup);
}

void UNBAudioSettings::Deinitialize()
{
	FWorldDelegates::OnWorldCleanup.Remove(WorldCleanupHandle);
	Super::Deinitialize();
}

void UNBAudioSettings::OnWorldCleanup(UWorld* World, bool bSessionEnded, bool bCleanupResources)
{
	// The class isn't exported from OnlineSubsystemUtils, so match it by name.
	static const FName VoipClassName(TEXT("VoipListenerSynthComponent"));
	for (TObjectIterator<USynthComponent> It; It; ++It)
	{
		USynthComponent* Voice = *It;
		if (Voice->GetClass()->GetFName() != VoipClassName || Voice->GetWorld() != World)
		{
			continue;
		}
		UE_LOG(LogTemp, Log, TEXT("NBVoice: releasing %s from %s"), *Voice->GetName(), *World->GetName());
		// Stop first: the audio device drops the sound before the world's garbage is collected.
		// The engine's talker sees the component idle on its next tick, resets it, and the next
		// voice packet makes a new one in the new world.
		Voice->Stop();
		if (UAudioComponent* Audio = Voice->GetAudioComponent())
		{
			Audio->Stop();
			if (Audio->IsRegistered())
			{
				Audio->UnregisterComponent();
			}
		}
		if (Voice->IsRegistered())
		{
			Voice->UnregisterComponent();
		}
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

void UNBAudioSettings::SetPushToTalk(bool bOn)
{
	// ANBSquirrel::UpdateVoiceMute reads it every frame.
	bPushToTalk = bOn;
	Save();
}

void UNBAudioSettings::SetOutputDevice(const FString& DeviceId)
{
	OutputDeviceId = DeviceId;
	Save();
	// Asynchronous; an empty id falls back to the system default device.
	UAudioMixerBlueprintLibrary::SwapAudioOutputDevice(GetGameInstance(), DeviceId, FOnCompletedDeviceSwap());
}

TArray<FString> UNBAudioSettings::GetMicDevices()
{
	return VoiceCaptureDevices::GetNames();
}

void UNBAudioSettings::SetMicDevice(const FString& DeviceName)
{
	MicDevice = DeviceName;
	UE_LOG(LogTemp, Log, TEXT("NBVoice: mic set to '%s' (empty = default)"), *MicDevice);
	ApplyMicDevice();
	Save();
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

void UNBAudioSettings::ApplyMicDevice() const
{
	// The voice engine reads it when it opens the mic and switches when it changes. A mic that
	// isn't there any more falls back to the default there.
	if (IConsoleVariable* Device = IConsoleManager::Get().FindConsoleVariable(TEXT("voice.CaptureDevice")))
	{
		Device->Set(*MicDevice, ECVF_SetByGameSetting);
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
