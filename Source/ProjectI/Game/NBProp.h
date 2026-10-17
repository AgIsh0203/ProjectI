// Squirrel Wheels prototype.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NBProp.generated.h"

class UBoxComponent;

/**
 * A roadside prop in the baked route map (L_Route): a mesh with no collision of its own, plus an
 * invisible box covering BlockFraction of its footprint (a tree's trunk, most of a rock). Kenney's
 * meshes only have canopy-sized collision, and the box travels with the prop when it's moved in the
 * editor. Placed by Tools/Route/BakeRoute.py; edit, copy or add them freely.
 */
UCLASS()
class PROJECTI_API ANBProp : public AActor
{
	GENERATED_BODY()

public:
	ANBProp();

	virtual void OnConstruction(const FTransform& Transform) override;

	/** For the bake script. */
	UFUNCTION(BlueprintCallable, Category = "Prop")
	void SetProp(UStaticMesh* NewMesh, float NewBlockFraction);

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Prop")
	TObjectPtr<UStaticMeshComponent> Mesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Prop")
	TObjectPtr<UBoxComponent> Blocker;

	/** Share of the mesh's footprint the car bumps into, full height; 0 = drive through it. */
	UPROPERTY(EditAnywhere, Category = "Prop", meta = (ClampMin = "0", ClampMax = "1"))
	float BlockFraction = 0.f;

private:
	void FitBlocker();
};
