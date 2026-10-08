// Squirrel Wheels prototype.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NBAcorn.generated.h"

class UStaticMeshComponent;

/** A spilled acorn: a small physics prop that bounces off the track and disappears after a while. */
UCLASS()
class PROJECTI_API ANBAcorn : public AActor
{
	GENERATED_BODY()

public:
	ANBAcorn();

	/** Server only. */
	static ANBAcorn* SpawnSpilled(UWorld* World, const FVector& Location, const FVector& Velocity);

private:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> Mesh;
};
