// Squirrel Wheels prototype.

#pragma once

#include "CoreMinimal.h"
#include "NBFeedback.generated.h"

class UAudioComponent;
class UNiagaraComponent;
class UNiagaraSystem;
class USceneComponent;
class USoundBase;

/**
 * A sound and/or a Niagara effect for one game event (a part breaking, a crash...).
 * Both are optional, so events can be wired in code now and given assets later in the
 * Blueprint defaults (BP_OffroadCar_Pawn, BP squirrel) without touching C++.
 *
 * Every play passes an Intensity (0..1) through as the float parameter "Intensity" on
 * both the sound (MetaSound input / SoundCue parameter) and the effect (user parameter).
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

	bool IsSet() const { return Sound || Effect; }
};

/** A looping FNBFeedback that is playing (e.g. smoke and sizzle while the engine is failed). */
struct PROJECTI_API FNBFeedbackLoop
{
	TWeakObjectPtr<UAudioComponent> Audio;
	TWeakObjectPtr<UNiagaraComponent> Effect;

	bool IsPlaying() const { return Audio.IsValid() || Effect.IsValid(); }

	/** Sets a float parameter on both the sound and the effect. */
	void SetFloat(FName Name, float Value) const;

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
