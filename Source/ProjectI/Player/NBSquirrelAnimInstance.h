// Squirrel Wheels prototype.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimInstanceProxy.h"
#include "NBSquirrelAnimInstance.generated.h"

class UAnimSequence;

/** One sequence playing: where it is, how fast, and whether it wraps. */
USTRUCT()
struct FNBAnimPlayback
{
	GENERATED_BODY()

	UPROPERTY(Transient)
	TObjectPtr<UAnimSequence> Sequence;

	float Time = 0.f;
	float PlayRate = 1.f;
	bool bLoop = true;

	void Advance(float DeltaSeconds);
};

/** Samples the playing sequence (and the one fading out) on the animation thread. */
struct FNBSquirrelAnimProxy : public FAnimInstanceProxy
{
	using FAnimInstanceProxy::FAnimInstanceProxy;

	virtual void PreUpdate(UAnimInstance* InAnimInstance, float DeltaSeconds) override;
	virtual bool Evaluate(FPoseContext& Output) override;

	FNBAnimPlayback Current;
	FNBAnimPlayback Previous;

	/** 0 = all Previous, 1 = all Current. */
	float BlendAlpha = 1.f;
};

/**
 * The rigged squirrel's animation, with no Animation Blueprint: ANBSquirrel calls Play with the
 * sequence for what it is doing, and this cross-fades from the last one. Cosmetic, per machine.
 */
UCLASS(Transient, NotBlueprintable)
class PROJECTI_API UNBSquirrelAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	/** Fades to Sequence over BlendTime. If it is already playing, only its rate changes, unless bRestart. */
	void Play(UAnimSequence* Sequence, bool bLoop, float PlayRate = 1.f, float BlendTime = 0.2f, bool bRestart = false);

	UAnimSequence* GetPlaying() const { return Current.Sequence; }

protected:
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;
	virtual FAnimInstanceProxy* CreateAnimInstanceProxy() override;

private:
	friend struct FNBSquirrelAnimProxy;

	UPROPERTY(Transient)
	FNBAnimPlayback Current;

	UPROPERTY(Transient)
	FNBAnimPlayback Previous;

	float BlendTime = 0.f;
	float BlendElapsed = 0.f;
};
