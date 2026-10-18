// Squirrel Wheels prototype.

#pragma once

#include "CoreMinimal.h"
#include "InputCoreTypes.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "NBControlSettings.generated.h"

/** The keyboard/mouse actions a player can rebind. The menu keys (Esc, Tab), the 1-8 pings and the gamepad stay fixed. */
enum class ENBBinding : uint8
{
	Forward,
	Back,
	Left,
	Right,
	Jump,
	Interact,
	Work,
	Horn,
	ResetCar,
	Count
};

/**
 * The player's control settings, saved per user (GameUserSettings.ini): look sensitivity,
 * inverted look, how hard crashes shake the camera, and the key for each ENBBinding.
 * ANBSquirrel reads them every use and remaps its keys when GetBindingsRevision changes, so a
 * change applies at once. Edited on the lobby menu's Options page (Controls).
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

	// --- Key bindings. ---

	static FKey GetDefaultKey(ENBBinding Binding);
	static const TCHAR* GetBindingLabel(ENBBinding Binding);
	/** Keyboard keys and mouse buttons, except the fixed ones (Esc, Tab, 1-8). */
	static bool IsBindable(const FKey& Key);
	/** Short on-screen name: "LMB", "Space", "F". */
	static FString GetKeyLabel(const FKey& Key);

	FKey GetKey(ENBBinding Binding) const;
	FString GetKeyLabel(ENBBinding Binding) const { return GetKeyLabel(GetKey(Binding)); }

	/** Binds Key to Binding. If another binding had Key, the two swap. Returns that other binding, or Count. */
	ENBBinding SetKey(ENBBinding Binding, const FKey& Key);
	void ResetKeys();

	/** Bumped on every binding change, so squirrels know to remap. */
	int32 GetBindingsRevision() const { return BindingsRevision; }

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

	/** Key names in ENBBinding order; empty or invalid entries use the default. */
	UPROPERTY(config)
	TArray<FString> KeyBindings;

	int32 BindingsRevision = 0;
};
