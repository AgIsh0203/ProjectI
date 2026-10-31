// Squirrel Wheels prototype.

#pragma once

#include "CoreMinimal.h"
#include "Net/VoiceConfig.h"
#include "NBVoiceTalker.generated.h"

/**
 * A squirrel's voice: hands each voice stream to UNBAudioSettings so the voice volume applies, and
 * muffles it (a low-pass filter, on every listener's machine) while the squirrel has an acorn in
 * its mouth (31 Oct: muting it read as an annoyance, so players would leave for Discord).
 */
UCLASS()
class PROJECTI_API UNBVoiceTalker : public UVOIPTalker
{
	GENERATED_BODY()

public:
	UNBVoiceTalker(const FObjectInitializer& ObjectInitializer);

	virtual void OnTalkingBegin(UAudioComponent* AudioComponent) override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/** Hz: the low-pass cutoff on a squirrel talking with an acorn in its mouth. */
	UPROPERTY(EditAnywhere, Category = "Voice")
	float MuffleFrequency = 450.f;

private:
	/** The owning squirrel's Acorn in Mouth state. */
	bool IsMuffled() const;
	void ApplyMuffle();

	/** The engine's component playing this squirrel's incoming voice (none for the local squirrel). */
	TWeakObjectPtr<UAudioComponent> Voice;
	bool bMuffled = false;
};
