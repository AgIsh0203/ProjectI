// Squirrel Wheels prototype.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "NBControlSettings.generated.h"

/**
 * The player's control settings, saved per user (GameUserSettings.ini): look sensitivity,
 * inverted look, and how hard crashes shake the camera. ANBSquirrel reads them every use, so
 * a change applies at once. Edited on the lobby menu's Options page (Controls).
 */
UCLASS(config = GameUserSettings)
class PROJECTI_API UNBControlSettings : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	static UNBControlSettings* Get(const UObject* WorldContext);

	float GetLookSensitivity() const { return LookSensitivity; }
	bool IsLookInverted() const { return bInvertLook; }
	float GetCameraShake() const { return CameraShake; }

	void SetLookSensitivity(float Value);
	void SetLookInverted(bool bInverted);
	void SetCameraShake(float Value);

	static constexpr float MinLookSensitivity = 0.2f;
	static constexpr float MaxLookSensitivity = 3.f;

private:
	/** Multiplies mouse and right-stick look. */
	UPROPERTY(config)
	float LookSensitivity = 1.f;

	/** Up looks down, for both mouse and stick. */
	UPROPERTY(config)
	bool bInvertLook = false;

	/** 0..1: scales the crash camera punch (some players get motion sick). */
	UPROPERTY(config)
	float CameraShake = 1.f;
};
