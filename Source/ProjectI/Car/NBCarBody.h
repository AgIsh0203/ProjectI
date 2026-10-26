// Squirrel Wheels prototype.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "NBCarBody.generated.h"

class UStaticMesh;

/**
 * A car's look and layout on the shared template skeleton (SKM_Offroad): the meshes that replace the
 * buggy's, and where the seats, parts and cargo go. ANBCar applies it in OnConstruction, so every
 * machine builds the same car from its class defaults. Chaos, the wheel bones and the physics asset
 * stay the buggy's, so a body changes the look and the jobs' layout, not the driving.
 *
 * Positions are relative to the car's mesh (the actor origin), in cm. A seat's position is the
 * squirrel's capsule centre (half-height 22), so put it 22 cm above the surface it sits on.
 */
UCLASS(BlueprintType)
class PROJECTI_API UNBCarBody : public UDataAsset
{
	GENERATED_BODY()

public:
	/** Replaces the template's Chassis mesh. */
	UPROPERTY(EditAnywhere, Category = "Body")
	TObjectPtr<UStaticMesh> BodyMesh;

	/** Replaces the 4 tire meshes (origin at the axle centre; the right side is turned 180 by the template). */
	UPROPERTY(EditAnywhere, Category = "Body")
	TObjectPtr<UStaticMesh> WheelMesh;

	/** Replaces the greybox door panel; its origin is the hinge (DoorHinge's location). It opens outward on its side. */
	UPROPERTY(EditAnywhere, Category = "Body")
	TObjectPtr<UStaticMesh> DoorMesh;

	/** Optional: the engine you can see (the engine part's fire plays at the EnginePart location). */
	UPROPERTY(EditAnywhere, Category = "Body")
	TObjectPtr<UStaticMesh> EngineMesh;

	UPROPERTY(EditAnywhere, Category = "Body")
	FTransform EngineTransform;

	/**
	 * Relative locations by component name: WheelSeat, PedalSeat, PassengerSeat (Gears), DeckSeat,
	 * HoodSeat, EnginePart, BrakePart, DoorPart, DoorHinge, TireFL/FR/BL/BR, FlipSpot, EngineAudio.
	 * Components left out keep the buggy's.
	 */
	UPROPERTY(EditAnywhere, Category = "Layout")
	TMap<FName, FVector> Locations;

	/** Reach (cm) by interactable component name (EnginePart, BrakePart, DoorPart, ...). Left out keep theirs. */
	UPROPERTY(EditAnywhere, Category = "Layout")
	TMap<FName, float> Ranges;

	/** The acorn pile's first acorn (layers of 5 along x by 4 along y, 18 x 26 cm apart, 11 cm per layer). */
	UPROPERTY(EditAnywhere, Category = "Layout")
	FVector AcornPileCorner = FVector(-125.f, -42.f, 156.f);
};
