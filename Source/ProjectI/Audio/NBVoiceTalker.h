// Squirrel Wheels prototype.

#pragma once

#include "CoreMinimal.h"
#include "Net/VoiceConfig.h"
#include "NBVoiceTalker.generated.h"

/** A squirrel's voice: hands each voice stream to UNBAudioSettings so the voice volume applies. */
UCLASS()
class PROJECTI_API UNBVoiceTalker : public UVOIPTalker
{
	GENERATED_BODY()

public:
	UNBVoiceTalker(const FObjectInitializer& ObjectInitializer);

	virtual void OnTalkingBegin(UAudioComponent* AudioComponent) override;
};
