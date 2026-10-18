// Squirrel Wheels prototype.

#pragma once

#include "CoreMinimal.h"
#include "NBFeedback.generated.h"

class UAudioComponent;
class UNiagaraComponent;
class UNiagaraSystem;
class USceneComponent;
class USoundBase;

class ANBPuffFX;

UENUM(BlueprintType)
enum class ENBPuffShape : uint8
{
	Sphere,
	Cube,
};

/**
 * A burst (or, in a loop, a stream) of chunky toon particles: plain spheres or cubes that fly,
 * swell, shrink and vanish. Built in code like the rest of the greybox, so effects need no assets.
 * Simulated on every machine by ANBPuffFX, in world space (a moving car leaves its smoke behind).
 */
USTRUCT(BlueprintType)
struct PROJECTI_API FNBPuff
{
	GENERATED_BODY()

	/** Particles per play; in a loop, particles per second. 0 = this puff is off. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Puff", meta = (ClampMin = "0"))
	float Count = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Puff")
	ENBPuffShape Shape = ENBPuffShape::Sphere;

	/** Each particle takes one of the two colours at random. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Puff")
	FLinearColor ColorA = FLinearColor::White;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Puff")
	FLinearColor ColorB = FLinearColor::White;

	/** 0 = lit like the rest of the greybox (dust, smoke); above 0 = unlit and glowing by this much (fire, sparks). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Puff", meta = (ClampMin = "0"))
	float Glow = 0.f;

	/** Biggest size (cm), reached a quarter of the way through its life; it starts and ends at 0. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Puff", meta = (ClampMin = "0"))
	float Size = 20.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Puff", meta = (ClampMin = "0.05"))
	float Lifetime = 0.6f;

	/** Launch speed (cm/s), within SpreadDegrees of straight up. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Puff", meta = (ClampMin = "0"))
	float Speed = 300.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Puff", meta = (ClampMin = "0", ClampMax = "180"))
	float SpreadDegrees = 60.f;

	/** Downward pull (cm/s^2). Negative rises, like smoke and flames. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Puff")
	float Gravity = 980.f;

	/** How fast particles slow down (1/s). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Puff", meta = (ClampMin = "0"))
	float Drag = 1.f;

	/** Particles start this far (cm) from the spawn point, at random. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Puff", meta = (ClampMin = "0"))
	float SpawnRadius = 10.f;

	/** Size, speed and lifetime vary by up to this fraction per particle. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Puff", meta = (ClampMin = "0", ClampMax = "1"))
	float Jitter = 0.35f;

	bool IsSet() const { return Count > 0.f && Size > 0.f; }
};

/**
 * A sound, toon puffs and/or a Niagara effect for one game event (a part breaking, a crash...).
 * Both are optional, so events can be wired in code now and given assets later in the
 * Blueprint defaults (BP_OffroadCar_Pawn, BP squirrel) without touching C++.
 *
 * Every play passes an Intensity (0..1) through as the float parameter "Intensity" on
 * the sound (MetaSound input / SoundCue parameter) and the effect (user parameter); it also scales the puffs.
 */
USTRUCT(BlueprintType)
struct PROJECTI_API FNBFeedback
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Feedback")
	TObjectPtr<USoundBase> Sound;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Feedback")
	TObjectPtr<UNiagaraSystem> Effect;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Feedback", meta = (ClampMin = "0"))
	float Volume = 1.f;

	/** Volume at Intensity 0; it rises to Volume at Intensity 1. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Feedback", meta = (ClampMin = "0", ClampMax = "1"))
	float QuietestVolumeFraction = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Feedback", meta = (ClampMin = "0.1"))
	float Pitch = 1.f;

	/** Random +- pitch per play, so repeated one-shots don't sound identical. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Feedback", meta = (ClampMin = "0", ClampMax = "0.5"))
	float PitchJitter = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Feedback")
	FVector EffectScale = FVector::OneVector;

	/** Toon particles, e.g. sparks plus dust. Intensity scales their count and speed; in a loop the
	 *  float parameter (Severity) scales their rate and size. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Feedback")
	TArray<FNBPuff> Puffs;

	bool HasPuffs() const { return Puffs.ContainsByPredicate([](const FNBPuff& Puff) { return Puff.IsSet(); }); }
	bool IsSet() const { return Sound || Effect || HasPuffs(); }
};

/** A looping FNBFeedback that is playing (e.g. smoke and sizzle while the engine is failed). */
struct PROJECTI_API FNBFeedbackLoop
{
	TWeakObjectPtr<UAudioComponent> Audio;
	TWeakObjectPtr<UNiagaraComponent> Effect;
	TWeakObjectPtr<ANBPuffFX> Puffs;

	bool IsPlaying() const { return Audio.IsValid() || Effect.IsValid() || Puffs.IsValid(); }

	/** Sets a float parameter on the sound and the effect, and scales the puffs' rate and size by it. */
	void SetFloat(FName Name, float Value) const;

	/** Damps the puffs (rate and size, 1 = none) without touching the parameter. */
	void SetDamp(float Value) const;

	void Stop();
};

namespace NBFeedback
{
	/** The parameter every play sets from its Intensity argument. */
	extern PROJECTI_API const FName IntensityParam;

	/** One-shot at a fixed spot in the world. Does nothing on a dedicated server or if nothing is set. */
	PROJECTI_API void PlayAt(const UObject* WorldContext, const FNBFeedback& Feedback, const FVector& Location,
		const FRotator& Rotation = FRotator::ZeroRotator, float Intensity = 1.f);

	/** One-shot that rides along on AttachTo (e.g. a part of the moving car), offset by RelativeLocation. */
	PROJECTI_API void PlayAttached(const FNBFeedback& Feedback, USceneComponent* AttachTo,
		const FVector& RelativeLocation = FVector::ZeroVector, float Intensity = 1.f);

	/** Starts Feedback attached to AttachTo and keeps it going until the returned loop is stopped.
	 *  The assets themselves must loop (looping sound wave / MetaSound, looping emitter). */
	PROJECTI_API FNBFeedbackLoop StartLoop(const FNBFeedback& Feedback, USceneComponent* AttachTo,
		const FVector& RelativeLocation = FVector::ZeroVector, float Intensity = 1.f);
}
