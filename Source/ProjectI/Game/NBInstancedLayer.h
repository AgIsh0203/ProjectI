// Squirrel Wheels prototype.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NBInstancedLayer.generated.h"

class UInstancedStaticMeshComponent;

/**
 * One layer of many identical pieces in the baked route map (the road strip, its lines): a single
 * instanced mesh instead of thousands of actors. Instances can be moved, added or removed in the
 * editor (select the actor, then the instance). Placed by Tools/Route/BakeRoute.py.
 */
UCLASS()
class PROJECTI_API ANBInstancedLayer : public AActor
{
	GENERATED_BODY()

public:
	ANBInstancedLayer();

	UInstancedStaticMeshComponent* GetInstances() const { return Instances; }

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Layer")
	TObjectPtr<UInstancedStaticMeshComponent> Instances;
};
