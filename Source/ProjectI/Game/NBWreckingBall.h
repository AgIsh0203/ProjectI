// Squirrel Wheels prototype.

#pragma once

#include "CoreMinimal.h"
#include "FX/NBFeedback.h"
#include "GameFramework/Actor.h"
#include "NBWreckingBall.generated.h"

class UStaticMeshComponent;

/**
 * G6's moving hazard: a wrecking ball on a gantry, swinging across the road (the actor's Y axis).
 * Placed in L_Route over THE BRIDGE by Tools/Route/PlaceRoadblocks.py. A crew that waits for it to
 * swing away gets through; one that rushes gets hit: the ball is a kinematic body, so it shoves the
 * car (the car's own crash detection does the damage), and it knocks out any squirrel it touches.
 *
 * Not replicated: the swing is a function of the server's clock (GetServerWorldTimeSeconds), so every
 * machine moves its own copy in step. Only the server knocks squirrels out.
 */
UCLASS()
class PROJECTI_API ANBWreckingBall : public AActor
{
	GENERATED_BODY()

public:
	ANBWreckingBall();

	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void Tick(float DeltaSeconds) override;

	/** Seconds for a full swing there and back. */
	UPROPERTY(EditAnywhere, Category = "Wrecking Ball")
	float Period = 4.5f;

	/** Largest angle from straight down (degrees). */
	UPROPERTY(EditAnywhere, Category = "Wrecking Ball")
	float Amplitude = 55.f;

	/** Where in the swing it is at server time 0 (0..1 of a period), to offset several balls. */
	UPROPERTY(EditAnywhere, Category = "Wrecking Ball")
	float PhaseOffset = 0.f;

	/** The pivot's height above the actor (the road surface), cm. */
	UPROPERTY(EditAnywhere, Category = "Wrecking Ball")
	float PivotHeight = 900.f;

	/** Pivot to ball centre, cm. At rest the ball's centre is PivotHeight - ChainLength above the road. */
	UPROPERTY(EditAnywhere, Category = "Wrecking Ball")
	float ChainLength = 760.f;

	UPROPERTY(EditAnywhere, Category = "Wrecking Ball")
	float BallRadius = 90.f;

	/** The gantry's posts stand this far either side of the centre (cm) ... */
	UPROPERTY(EditAnywhere, Category = "Wrecking Ball")
	float PostSpan = 650.f;

	/** ... and reach this far below the road, so they stand on the ground of a gorge (cm). */
	UPROPERTY(EditAnywhere, Category = "Wrecking Ball")
	float PostDepth = 3000.f;

	/** A squirrel whose centre comes this close to the ball's surface is knocked out (cm). */
	UPROPERTY(EditAnywhere, Category = "Wrecking Ball")
	float KnockReach = 45.f;

	/** The knock: the ball's velocity times this, plus this much up (cm/s). */
	UPROPERTY(EditAnywhere, Category = "Wrecking Ball")
	FVector2D KnockVelocity = FVector2D(1.3f, 500.f);

	/** Played where a squirrel is knocked out (the car's hits use its own CrashFeedback). */
	UPROPERTY(EditAnywhere, Category = "Wrecking Ball|Feedback")
	FNBFeedback KnockFeedback;

private:
	/** Swing angle (degrees, + towards the actor's +Y) and its rate (degrees/s) at a time. */
	void GetSwing(double Time, float& OutAngle, float& OutRate) const;
	void PlaceParts();

	UPROPERTY(VisibleAnywhere, Category = "Wrecking Ball")
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(VisibleAnywhere, Category = "Wrecking Ball")
	TObjectPtr<UStaticMeshComponent> PostL;

	UPROPERTY(VisibleAnywhere, Category = "Wrecking Ball")
	TObjectPtr<UStaticMeshComponent> PostR;

	UPROPERTY(VisibleAnywhere, Category = "Wrecking Ball")
	TObjectPtr<UStaticMeshComponent> Beam;

	UPROPERTY(VisibleAnywhere, Category = "Wrecking Ball")
	TObjectPtr<UStaticMeshComponent> Chain;

	UPROPERTY(VisibleAnywhere, Category = "Wrecking Ball")
	TObjectPtr<UStaticMeshComponent> Ball;
};
