// Squirrel Wheels prototype.

#include "Player/NBSquirrelAnimInstance.h"

#include "AnimationRuntime.h"
#include "Animation/AnimationPoseData.h"
#include "Animation/AnimNodeBase.h"
#include "Animation/AnimSequence.h"

void FNBAnimPlayback::Advance(float DeltaSeconds)
{
	if (!Sequence)
	{
		return;
	}
	const float Length = Sequence->GetPlayLength();
	Time += DeltaSeconds * PlayRate;
	if (bLoop && Length > KINDA_SMALL_NUMBER)
	{
		Time = FMath::Fmod(Time, Length);
		if (Time < 0.f)
		{
			Time += Length;
		}
	}
	else
	{
		// A one-shot holds its last frame.
		Time = FMath::Clamp(Time, 0.f, Length);
	}
}

void FNBSquirrelAnimProxy::PreUpdate(UAnimInstance* InAnimInstance, float DeltaSeconds)
{
	FAnimInstanceProxy::PreUpdate(InAnimInstance, DeltaSeconds);
	const UNBSquirrelAnimInstance* Instance = CastChecked<UNBSquirrelAnimInstance>(InAnimInstance);
	Current = Instance->Current;
	Previous = Instance->Previous;
	BlendAlpha = Instance->BlendTime > KINDA_SMALL_NUMBER ? FMath::Clamp(Instance->BlendElapsed / Instance->BlendTime, 0.f, 1.f) : 1.f;
}

bool FNBSquirrelAnimProxy::Evaluate(FPoseContext& Output)
{
	auto Sample = [](const FNBAnimPlayback& Playback, FPoseContext& Pose)
	{
		FAnimationPoseData PoseData(Pose);
		Playback.Sequence->GetAnimationPose(PoseData, FAnimExtractContext(static_cast<double>(Playback.Time), false, {}, Playback.bLoop));
	};

	if (!Current.Sequence)
	{
		Output.ResetToRefPose();
		return true;
	}
	if (BlendAlpha >= 1.f || !Previous.Sequence)
	{
		Sample(Current, Output);
		return true;
	}
	FPoseContext From(this);
	FPoseContext To(this);
	Sample(Previous, From);
	Sample(Current, To);
	const FAnimationPoseData FromData(From);
	const FAnimationPoseData ToData(To);
	FAnimationPoseData OutData(Output);
	FAnimationRuntime::BlendTwoPosesTogether(FromData, ToData, 1.f - BlendAlpha, OutData);
	return true;
}

void UNBSquirrelAnimInstance::Play(UAnimSequence* Sequence, bool bLoop, float PlayRate, float InBlendTime, bool bRestart)
{
	if (Sequence == Current.Sequence && !bRestart)
	{
		Current.PlayRate = PlayRate;
		return;
	}
	Previous = Current;
	Current.Sequence = Sequence;
	Current.Time = 0.f;
	Current.PlayRate = PlayRate;
	Current.bLoop = bLoop;
	BlendTime = Previous.Sequence ? InBlendTime : 0.f;
	BlendElapsed = 0.f;
}

void UNBSquirrelAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);
	Current.Advance(DeltaSeconds);
	Previous.Advance(DeltaSeconds);
	BlendElapsed += DeltaSeconds;
}

FAnimInstanceProxy* UNBSquirrelAnimInstance::CreateAnimInstanceProxy()
{
	return new FNBSquirrelAnimProxy(this);
}
