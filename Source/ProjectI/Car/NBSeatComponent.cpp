// Squirrel Wheels prototype.

#include "Car/NBSeatComponent.h"

#include "Net/UnrealNetwork.h"
#include "Player/NBSquirrel.h"

UNBSeatComponent::UNBSeatComponent()
{
	SetIsReplicatedByDefault(true);
}

void UNBSeatComponent::SetOccupant(ANBSquirrel* NewOccupant)
{
	check(GetOwner() && GetOwner()->HasAuthority());
	Occupant = NewOccupant;
}

void UNBSeatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UNBSeatComponent, Occupant);
}
