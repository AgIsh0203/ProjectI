// Squirrel Wheels prototype.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "NBInteractionSubsystem.generated.h"

class UNBInteractableComponent;

/** Keeps the live interactables of one world so squirrels can find the nearest one cheaply. */
UCLASS()
class PROJECTI_API UNBInteractionSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	void Register(UNBInteractableComponent* Interactable) { Interactables.AddUnique(Interactable); }
	void Unregister(UNBInteractableComponent* Interactable) { Interactables.Remove(Interactable); }

	const TArray<TObjectPtr<UNBInteractableComponent>>& GetInteractables() const { return Interactables; }

private:
	UPROPERTY(Transient)
	TArray<TObjectPtr<UNBInteractableComponent>> Interactables;
};
