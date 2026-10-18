// Squirrel Wheels prototype.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "NBAudioSettings.generated.h"

class UAudioComponent;
struct FAudioOutputDeviceInfo;

/**
 * The player's sound and voice settings, saved per user (GameUserSettings.ini) and applied
 * on start. Game volume scales everything; voice volume scales the other squirrels' voices
 * on top of it; mic boost amplifies what this player sends. The mic itself is Windows'
 * default input: UE's Steam voice capture opens the default device and has no public way to
 * pick another.
 */
UCLASS(config = GameUserSettings)
class PROJECTI_API UNBAudioSettings : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	static UNBAudioSettings* Get(const UObject* WorldContext);

	float GetGameVolume() const { return GameVolume; }
	float GetVoiceVolume() const { return VoiceVolume; }
	float GetMicBoost() const { return MicBoost; }
	const FString& GetOutputDeviceId() const { return OutputDeviceId; }

	void SetGameVolume(float Value);
	void SetVoiceVolume(float Value);
	void SetMicBoost(float Value);
	/** Empty = the system default. */
	void SetOutputDevice(const FString& DeviceId);

	/** A remote squirrel started talking: keep its voice at VoiceVolume, now and on later changes. */
	void AddVoiceComponent(UAudioComponent* Voice);

	static constexpr float MaxGameVolume = 1.f;
	static constexpr float MaxVoiceVolume = 3.f;
	static constexpr float MaxMicBoost = 4.f;

private:
	void ApplyGameVolume() const;
	void ApplyMicBoost() const;
	void ApplyVoiceVolume();
	/** What each voice component's volume is set to: VoiceVolume, independent of GameVolume. */
	float GetVoiceMultiplier() const;
	void Save();

	/** Below 1 by default so voices sit on top of the engine and music (friend test 2: "could not hear her over the game sound"). */
	UPROPERTY(config)
	float GameVolume = 0.7f;

	/** Above 1 by default: voices were too quiet to use in the 17 Oct friend test. */
	UPROPERTY(config)
	float VoiceVolume = 1.5f;

	UPROPERTY(config)
	float MicBoost = 1.5f;

	UPROPERTY(config)
	FString OutputDeviceId;

	TArray<TWeakObjectPtr<UAudioComponent>> VoiceComponents;
};
