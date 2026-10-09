// Squirrel Wheels prototype.

#include "FX/NBFeedback.h"

#include "Components/AudioComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "Sound/SoundBase.h"

const FName NBFeedback::IntensityParam(TEXT("Intensity"));

namespace
{
	bool IsCosmeticWorld(const UWorld* World)
	{
		return World && World->GetNetMode() != NM_DedicatedServer;
	}

	float VolumeFor(const FNBFeedback& Feedback, float Intensity)
	{
		return Feedback.Volume * FMath::Lerp(Feedback.QuietestVolumeFraction, 1.f, FMath::Clamp(Intensity, 0.f, 1.f));
	}

	float PitchFor(const FNBFeedback& Feedback)
	{
		return Feedback.Pitch * (1.f + FMath::FRandRange(-Feedback.PitchJitter, Feedback.PitchJitter));
	}

	FNBFeedbackLoop Spawn(const FNBFeedback& Feedback, USceneComponent* AttachTo, const FVector& RelativeLocation, float Intensity, bool bLoop)
	{
		FNBFeedbackLoop Result;
		if (!AttachTo || !Feedback.IsSet() || !IsCosmeticWorld(AttachTo->GetWorld()))
		{
			return Result;
		}
		if (Feedback.Sound)
		{
			// One-shots clean themselves up; loops are destroyed by FNBFeedbackLoop::Stop.
			Result.Audio = UGameplayStatics::SpawnSoundAttached(Feedback.Sound, AttachTo, NAME_None, RelativeLocation, FRotator::ZeroRotator,
				EAttachLocation::KeepRelativeOffset, /*bStopWhenAttachedToDestroyed*/ true, VolumeFor(Feedback, Intensity), PitchFor(Feedback),
				/*StartTime*/ 0.f, /*Attenuation*/ nullptr, /*Concurrency*/ nullptr, /*bAutoDestroy*/ true);
			if (UAudioComponent* Audio = Result.Audio.Get())
			{
				Audio->SetFloatParameter(NBFeedback::IntensityParam, Intensity);
			}
		}
		if (Feedback.Effect)
		{
			Result.Effect = UNiagaraFunctionLibrary::SpawnSystemAttached(Feedback.Effect, AttachTo, NAME_None, RelativeLocation, FRotator::ZeroRotator,
				Feedback.EffectScale, EAttachLocation::KeepRelativeOffset, /*bAutoDestroy*/ true, ENCPoolMethod::None);
			if (UNiagaraComponent* Effect = Result.Effect.Get())
			{
				Effect->SetVariableFloat(NBFeedback::IntensityParam, Intensity);
			}
		}
		if (!bLoop)
		{
			Result = FNBFeedbackLoop();
		}
		return Result;
	}
}

void FNBFeedbackLoop::SetFloat(FName Name, float Value) const
{
	if (UAudioComponent* AudioComponent = Audio.Get())
	{
		AudioComponent->SetFloatParameter(Name, Value);
	}
	if (UNiagaraComponent* EffectComponent = Effect.Get())
	{
		EffectComponent->SetVariableFloat(Name, Value);
	}
}

void FNBFeedbackLoop::Stop()
{
	if (UAudioComponent* AudioComponent = Audio.Get())
	{
		// Fade rather than cut, so a repaired engine's sizzle doesn't click off.
		AudioComponent->FadeOut(0.25f, 0.f);
	}
	if (UNiagaraComponent* EffectComponent = Effect.Get())
	{
		// Lets live particles (smoke) finish instead of popping; auto-destroys once done.
		EffectComponent->Deactivate();
	}
	Audio.Reset();
	Effect.Reset();
}

void NBFeedback::PlayAt(const UObject* WorldContext, const FNBFeedback& Feedback, const FVector& Location, const FRotator& Rotation, float Intensity)
{
	UWorld* World = WorldContext ? WorldContext->GetWorld() : nullptr;
	if (!Feedback.IsSet() || !IsCosmeticWorld(World))
	{
		return;
	}
	if (Feedback.Sound)
	{
		if (UAudioComponent* Audio = UGameplayStatics::SpawnSoundAtLocation(World, Feedback.Sound, Location, Rotation,
				VolumeFor(Feedback, Intensity), PitchFor(Feedback)))
		{
			Audio->SetFloatParameter(IntensityParam, Intensity);
		}
	}
	if (Feedback.Effect)
	{
		if (UNiagaraComponent* Effect = UNiagaraFunctionLibrary::SpawnSystemAtLocation(World, Feedback.Effect, Location, Rotation, Feedback.EffectScale))
		{
			Effect->SetVariableFloat(IntensityParam, Intensity);
		}
	}
}

void NBFeedback::PlayAttached(const FNBFeedback& Feedback, USceneComponent* AttachTo, const FVector& RelativeLocation, float Intensity)
{
	Spawn(Feedback, AttachTo, RelativeLocation, Intensity, /*bLoop*/ false);
}

FNBFeedbackLoop NBFeedback::StartLoop(const FNBFeedback& Feedback, USceneComponent* AttachTo, const FVector& RelativeLocation, float Intensity)
{
	return Spawn(Feedback, AttachTo, RelativeLocation, Intensity, /*bLoop*/ true);
}
