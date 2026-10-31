// Squirrel Wheels prototype.

#include "Audio/NBVoiceTalker.h"

#include "Audio/NBAudioSettings.h"
#include "Components/AudioComponent.h"
#include "GameFramework/Pawn.h"
#include "Player/NBPlayerState.h"

UNBVoiceTalker::UNBVoiceTalker(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickInterval = 0.1f;
}

void UNBVoiceTalker::OnTalkingBegin(UAudioComponent* AudioComponent)
{
	Super::OnTalkingBegin(AudioComponent);
	if (UNBAudioSettings* AudioSettings = UNBAudioSettings::Get(this))
	{
		AudioSettings->AddVoiceComponent(AudioComponent);
	}
	Voice = AudioComponent;
	bMuffled = IsMuffled();
	ApplyMuffle();
}

void UNBVoiceTalker::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	const bool bNow = IsMuffled();
	if (bNow != bMuffled)
	{
		bMuffled = bNow;
		ApplyMuffle();
	}
}

bool UNBVoiceTalker::IsMuffled() const
{
	const APawn* Squirrel = Cast<APawn>(GetOwner());
	const ANBPlayerState* Stats = Squirrel ? Squirrel->GetPlayerState<ANBPlayerState>() : nullptr;
	return Stats && Stats->IsMuted();
}

void UNBVoiceTalker::ApplyMuffle()
{
	if (UAudioComponent* VoiceComponent = Voice.Get())
	{
		VoiceComponent->SetLowPassFilterEnabled(bMuffled);
		VoiceComponent->SetLowPassFilterFrequency(MuffleFrequency);
	}
}
