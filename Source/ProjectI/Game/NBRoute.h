// Squirrel Wheels prototype.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NBRoute.generated.h"

class ANBFinishZone;
class UInstancedStaticMeshComponent;
class UMaterialInterface;
class UStaticMesh;

/** What sits on a stretch of road. */
UENUM(BlueprintType)
enum class ENBRouteFeature : uint8
{
	None,
	/** Low ridges across the road every few metres: acorns jump out. */
	Bumps,
	/** Scattered lumps to steer around or bounce over. */
	Rocks,
	/** Pillars alternating left and right. */
	Slalom,
	/** Ramp up, a long flat top with sheer sides, ramp down. Straight segments only. */
	Hill,
	/** A narrow raised deck with no rails. Straight segments only. */
	Bridge,
	/** A kicker ramp in the middle of the road, with room to drive round it. Straight segments only. */
	Jump,
};

/** One stretch of the route: drive Length metres while turning TurnDegrees (positive = right). */
USTRUCT(BlueprintType)
struct FNBRouteSegment
{
	GENERATED_BODY()

	/** Starts a new named section (shown on a roadside sign and the HUD). Empty continues the last one. */
	UPROPERTY(EditAnywhere, Category = "Route")
	FString Section;

	UPROPERTY(EditAnywhere, Category = "Route", meta = (ClampMin = "10"))
	float Length = 100.f;

	UPROPERTY(EditAnywhere, Category = "Route")
	float TurnDegrees = 0.f;

	UPROPERTY(EditAnywhere, Category = "Route")
	ENBRouteFeature Feature = ENBRouteFeature::None;

	/** Hill/Bridge/Jump: height in cm. Ignored by the others. */
	UPROPERTY(EditAnywhere, Category = "Route")
	float Height = 0.f;
};

/**
 * The run's course: a long greybox road from the park to the depot, built from code at BeginPlay
 * on every machine (same segments, same random seed), so it needs no level edits. The road is laid
 * on its own ground slab far from L_TestTrack's floor, with walls around the edge, and ends in an
 * ANBFinishZone spawned by the server.
 *
 * Segment lengths are in metres; everything else is in cm, in the actor's local space.
 */
UCLASS()
class PROJECTI_API ANBRoute : public AActor
{
	GENERATED_BODY()

public:
	ANBRoute();

	/** Where the crew starts: on the road, facing along it. */
	FTransform GetStartTransform() const;

	/** Server: the drop-off gate at the end of the road (spawned in BeginPlay). */
	ANBFinishZone* GetFinishZone() const { return FinishZone; }

	float GetDeadlineSeconds() const { return DeadlineSeconds; }

	/** Total road length in cm. */
	float GetLength() const { return TotalLength; }

	/**
	 * How far along the road (cm) the point is. Searches near the last answer first so the
	 * hairpin's two legs don't get mixed up; call it with one moving thing (the car).
	 */
	float GetDistanceAlong(const FVector& WorldLocation) const;

	/** The section at a distance along the road, and the distance where it starts. */
	int32 GetSectionAt(float Distance) const;
	int32 GetNumSections() const { return SectionNames.Num(); }
	const FString& GetSectionName(int32 Index) const { return SectionNames[Index]; }
	float GetSectionStart(int32 Index) const { return SectionStarts[Index]; }

	/** A point on the road at that distance, raised a little, facing along it. World space. */
	FTransform GetTransformAt(float Distance, float Lift = 0.f) const;

protected:
	/** Lays out the centreline, so start spots and distances work before BeginPlay. */
	virtual void PostInitializeComponents() override;
	/** Builds the meshes on every machine; the server also spawns the drop-off gate. */
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, Category = "Route")
	TArray<FNBRouteSegment> Segments;

	/** Time to reach the drop-off along this route. The game mode uses it as the run deadline. */
	UPROPERTY(EditAnywhere, Category = "Route")
	float DeadlineSeconds = 480.f;

	UPROPERTY(EditAnywhere, Category = "Route")
	float RoadWidth = 1400.f;

	/** Bridges are narrower than the road so falling off is a real risk. */
	UPROPERTY(EditAnywhere, Category = "Route")
	float BridgeWidth = 800.f;

	/** Ground beyond the road's furthest point on every side, before the boundary wall. */
	UPROPERTY(EditAnywhere, Category = "Route")
	float GroundMargin = 8000.f;

	/** Seeds the rock scatter; the same on every machine. */
	UPROPERTY(EditAnywhere, Category = "Route")
	int32 Seed = 1337;

private:
	/** Walk the segments into evenly spaced centreline samples (local space). */
	void BuildCentreline();
	void BuildGeometry();
	/** Ramp up, flat top, ramp down along the road from StartDistance. RampLength 0 = a lone kicker. */
	void AddRaised(UInstancedStaticMeshComponent* Mesh, float StartDistance, float Length, float RampLength, float Height, float Width, bool bKicker);
	void AddSign(const FString& Text, float Distance);

	/** A cube instance: Size in cm, at Location (local) with Rotation. */
	static void AddBox(UInstancedStaticMeshComponent* Mesh, const FVector& Location, const FRotator& Rotation, const FVector& Size);
	UInstancedStaticMeshComponent* MakeLayer(const TCHAR* Name, const FLinearColor& Color, bool bCollision, bool bShadows);

	/** Local-space sample at a distance (cm), interpolated. */
	void SampleAt(float Distance, FVector& OutLocation, float& OutYaw) const;

	UPROPERTY()
	TObjectPtr<UStaticMesh> CubeMesh;

	UPROPERTY()
	TObjectPtr<UMaterialInterface> ShapeMaterial;

	UPROPERTY()
	TObjectPtr<ANBFinishZone> FinishZone;

	UPROPERTY()
	TObjectPtr<UInstancedStaticMeshComponent> Ground;
	UPROPERTY()
	TObjectPtr<UInstancedStaticMeshComponent> Road;
	UPROPERTY()
	TObjectPtr<UInstancedStaticMeshComponent> Lines;
	UPROPERTY()
	TObjectPtr<UInstancedStaticMeshComponent> Bumps;
	UPROPERTY()
	TObjectPtr<UInstancedStaticMeshComponent> Rocks;
	UPROPERTY()
	TObjectPtr<UInstancedStaticMeshComponent> Pillars;
	UPROPERTY()
	TObjectPtr<UInstancedStaticMeshComponent> Hills;
	UPROPERTY()
	TObjectPtr<UInstancedStaticMeshComponent> Planks;
	UPROPERTY()
	TObjectPtr<UInstancedStaticMeshComponent> Markers;
	UPROPERTY()
	TObjectPtr<UInstancedStaticMeshComponent> Walls;

	/** Centreline every SampleSpacing cm, local space. */
	TArray<FVector> SamplePoints;
	TArray<float> SampleYaws;
	/** Distance along the road (cm) at each sample. */
	TArray<float> SampleDistances;
	float TotalLength = 0.f;

	TArray<FString> SectionNames;
	TArray<float> SectionStarts;

	mutable int32 LastNearestSample = INDEX_NONE;
};
