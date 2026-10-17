// Squirrel Wheels prototype.

#include "Audio/NBVoiceTalker.h"

#include "Audio/NBAudioSettings.h"

UNBVoiceTalker::UNBVoiceTalker(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UNBVoiceTalker::OnTalkingBegin(UAudioComponent* AudioComponent)
{
	Super::OnTalkingBegin(AudioComponent);
	if (UNBAudioSettings* AudioSettings = UNBAudioSettings::Get(this))
	{
		AudioSettings->AddVoiceComponent(AudioComponent);
	}
}
