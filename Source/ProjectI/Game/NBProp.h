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
	virtual void BeginPlay() override;

	/** For the bake script. */
	UFUNCTION(BlueprintCallable, Category = "Prop")
	void SetProp(UStaticMesh* NewMesh, float NewBlockFraction, bool bNewBlockAtPivot);

	/** The collision box for a prop mesh, in the mesh's own space (shared with the runtime-built route). */
	static void BlockerBox(const UStaticMesh* Mesh, float Fraction, bool bAtPivot, FVector& OutCentre, FVector& OutExtent);

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Prop")
	TObjectPtr<UStaticMeshComponent> Mesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Prop")
	TObjectPtr<UBoxComponent> Blocker;

	/** Share of the mesh's footprint the car bumps into, full height; 0 = drive through it. */
	UPROPERTY(EditAnywhere, Category = "Prop", meta = (ClampMin = "0", ClampMax = "1"))
	float BlockFraction = 0.f;

	/** Centre the box on the pivot (a trunk or a lamp's pole), sized from the narrower side of the footprint;
	 *  off = centred on the mesh's bounds (rocks, logs, fences). */
	UPROPERTY(EditAnywhere, Category = "Prop")
	bool bBlockAtPivot = false;

	/** A light physics body the car (or a squirrel) knocks over instead of a fixed blocker: traffic cones.
	 *  Each machine simulates it on its own (not replicated), so a cone may lie a little differently per player.
	 *  Needs simple collision on the mesh. BlockFraction is ignored. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prop")
	bool bKnockable = false;

	/** Mass of a knockable prop: light enough that hitting it doesn't slow the car. */
	UPROPERTY(EditAnywhere, Category = "Prop", meta = (ClampMin = "0.1", EditCondition = "bKnockable"))
	float KnockMass = 4.f;

private:
	void FitBlocker();
};
