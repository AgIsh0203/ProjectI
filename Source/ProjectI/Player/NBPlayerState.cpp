// Squirrel Wheels prototype.

#include "Player/NBPlayerState.h"

#include "Net/UnrealNetwork.h"

void ANBPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ANBPlayerState, Falls);
	DOREPLIFETIME(ANBPlayerState, Respawns);
}
