// Squirrel Wheels prototype.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NBFinishZone.generated.h"

class ANBCar;
class UBoxComponent;
class UStaticMeshComponent;

/** Greybox "ACORN DROP-OFF" gate. The run is delivered once the car's centre is inside the box. */
UCLASS()
class PROJECTI_API ANBFinishZone : public AActor
{
	GENERATED_BODY()

public:
	ANBFinishZone();

	/** Server: is the car's centre inside the box? */
	bool Contains(const ANBCar& Car) const;

protected:
	virtual void OnConstruction(const FTransform& Transform) override;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UBoxComponent> Box;

	/** A flat glowing slab on the ground so the gate reads from afar. */
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> Visual;

	/** Two posts either side of the track so the gate reads from far away. */
	UPROPERTY(VisibleAnywhere)
	TArray<TObjectPtr<UStaticMeshComponent>> Posts;

	UPROPERTY(EditAnywhere, Category = "Finish")
	FVector HalfExtent = FVector(300.f, 500.f, 250.f);
};
