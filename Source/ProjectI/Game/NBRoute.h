// Squirrel Wheels prototype.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NBRoute.generated.h"

class ANBFinishZone;
class UInstancedStaticMeshComponent;
class UMaterialInterface;
class UStaticMesh;

namespace NBCollision
{
	/** River water's object channel ("Water" in DefaultEngine.ini): everything ignores it by default. */
	constexpr ECollisionChannel Water = ECC_GameTraceChannel1;
	/** Its profile: query only, ignoring every channel. */
	inline const FName WaterProfile(TEXT("Water"));
}

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
	/** A river across the trail, driven through (terrain only; plain road without). Hidden pebbles in the
	 *  water, and a squirrel on foot in it is swept away. Straight segments only. */
	Ford,
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

	float GetParSeconds() const { return ParSeconds; }

	/** Bake tool (Tools/Route/BakeRoute.py): builds the road, features, signs and props now, in whatever
	 *  world this is (the editor's), so the script can copy them into level actors. */
	UFUNCTION(BlueprintCallable, Category = "Route|Bake")
	void BuildGeometryNow();

	/** Bake tool: the collision share (FPropKind::Block) of the prop table entry using this mesh; 0 if none. */
	UFUNCTION(BlueprintCallable, Category = "Route|Bake")
	float GetPropBlockFraction(const UStaticMesh* Mesh) const;

	/** Bake tool: whether that entry's box sits on the pivot (trees, lamps). */
	UFUNCTION(BlueprintCallable, Category = "Route|Bake")
	bool GetPropBlockAtPivot(const UStaticMesh* Mesh) const;

	/** Off in a baked level (L_Route): its geometry is saved in the map as ordinary actors you can edit,
	 *  and this actor only supplies the centreline (progress, sections, the finish gate). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Route")
	bool bBuildGeometry = true;

	/** The dog (#13): the server spawns an ANBDog whose kennel sits beside this section, and which
	 *  chases the car from there to the next named section. Empty or not found: no dog. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Route|Dog")
	FString DogSection = TEXT("WIGGLY ROAD");

	/** The kennel: this far into the section (cm) ... */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Route|Dog")
	float DogKennelDistance = 3000.f;

	/** ... and this far right of the centreline (cm; the road is RoadWidth wide). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Route|Dog")
	float DogKennelSide = 1100.f;

	/** Total road length in cm. */
	UFUNCTION(BlueprintPure, Category = "Route")
	float GetLength() const { return TotalLength; }

	/** Off: everything at z = 0 on a flat slab (the runtime route on L_TestTrack). On: the road follows a
	 *  height profile over rolling hills and a Landscape (BuildLandscape, baked into L_Route) is the ground. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Route|Terrain")
	bool bUseTerrain = false;

	/** The road's height (world Z, cm) at a distance along it. */
	UFUNCTION(BlueprintPure, Category = "Route|Terrain")
	float GetRoadHeight(float Distance) const;

	/** The road's grade at a distance along it (rise over run; 0.08 = 8 %). */
	UFUNCTION(BlueprintPure, Category = "Route|Terrain")
	float GetRoadGrade(float Distance) const;

	/** What the landscape gets at a world X/Y (world Z, cm): the road's height across the road and its
	 *  shoulders, blending into the natural hills further out. */
	UFUNCTION(BlueprintPure, Category = "Route|Terrain")
	float GetGroundHeight(float X, float Y) const;

	/** How much a world X/Y is on the dirt trail: 1 on it (and everywhere without terrain), fading to 0
	 *  across its shoulders. The car loses grip the longer its wheels stay off it. */
	UFUNCTION(BlueprintPure, Category = "Route|Terrain")
	float GetTrailWeight(float X, float Y) const;

	/** The ground's height model without the road (world Z, cm). */
	UFUNCTION(BlueprintPure, Category = "Route|Terrain")
	float GetTerrainHeight(float X, float Y) const;

	/** How deep (cm) the water of a river is at a world location (0 = dry), and the way it flows (world, unit). */
	UFUNCTION(BlueprintCallable, Category = "Route|Terrain")
	float GetWaterDepth(const FVector& WorldLocation, FVector& OutFlow) const;

	/** The route in this world, if any (cached per world). */
	static ANBRoute* Find(const UWorld* World);

	/** The height of whatever landscape is in this world at a world X/Y; false if there is none there. */
	UFUNCTION(BlueprintCallable, Category = "Route|Terrain")
	bool GetLandscapeHeight(float X, float Y, float& OutZ) const;

	/** Grade checker: per section, the max grade, the min and max height, and any stretch over its limit. */
	UFUNCTION(BlueprintCallable, Category = "Route|Terrain")
	FString CheckProfile() const;

	/** Bake tool, editor only: spawns a Landscape covering the route plus Margin cm on every side, QuadSize cm
	 *  per quad, heights from GetGroundHeight, and paints the trail on its "Trail" layer (TrailLayerInfo, which
	 *  the material samples). Returns it (null outside the editor). */
	UFUNCTION(BlueprintCallable, Category = "Route|Bake")
	AActor* BuildLandscape(UMaterialInterface* Material, UObject* TrailLayerInfo, float QuadSize = 200.f, float Margin = 30000.f);

	/** Bake tool, editor only: creates the "Trail" landscape layer asset at PackagePath (e.g. /Game/Maps/LI_Trail). */
	UFUNCTION(BlueprintCallable, Category = "Route|Bake")
	static UObject* CreateTrailLayerInfo(const FString& PackagePath);

	/**
	 * How far along the road (cm) the point is. Searches near the last answer first so the
	 * hairpin's two legs don't get mixed up; call it with one moving thing (the car).
	 */
	float GetDistanceAlong(const FVector& WorldLocation) const;

	/** The section at a distance along the road, and the distance where it starts. */
	UFUNCTION(BlueprintPure, Category = "Route")
	int32 GetSectionAt(float Distance) const;
	UFUNCTION(BlueprintPure, Category = "Route")
	int32 GetNumSections() const { return SectionNames.Num(); }
	UFUNCTION(BlueprintPure, Category = "Route")
	FString GetSectionName(int32 Index) const { return SectionNames.IsValidIndex(Index) ? SectionNames[Index] : FString(); }
	UFUNCTION(BlueprintPure, Category = "Route")
	float GetSectionStart(int32 Index) const { return SectionStarts.IsValidIndex(Index) ? SectionStarts[Index] : 0.f; }

	/** A point on the road at that distance, raised a little, facing along it and pitched to its grade. World space. */
	UFUNCTION(BlueprintPure, Category = "Route")
	FTransform GetTransformAt(float Distance, float Lift = 0.f) const;

protected:
	/** Lays out the centreline, so start spots and distances work before BeginPlay. */
	virtual void PostInitializeComponents() override;
	/** The same in the editor, where PostInitializeComponents doesn't run (bake and snap scripts, screenshots). */
	virtual void OnConstruction(const FTransform& Transform) override;
	/** Builds the meshes on every machine; the server also spawns the drop-off gate. */
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, Category = "Route")
	TArray<FNBRouteSegment> Segments;

	/** Par time to the drop-off: delivered runs score per second under it, and the failure director ramps over it. No deadline since 31 Oct. */
	UPROPERTY(EditAnywhere, Category = "Route")
	float ParSeconds = 540.f;

	UPROPERTY(EditAnywhere, Category = "Route")
	float RoadWidth = 1400.f;

	/** Bridges are narrower than the road so falling off is a real risk. */
	UPROPERTY(EditAnywhere, Category = "Route")
	float BridgeWidth = 800.f;

	/** Ground beyond the road's furthest point on every side, before the boundary wall. */
	UPROPERTY(EditAnywhere, Category = "Route")
	float GroundMargin = 8000.f;

	/** Seeds the rock scatter and the hills; the same on every machine. */
	UPROPERTY(EditAnywhere, Category = "Route")
	int32 Seed = 1337;

	/** The mountains go from 0 to this (cm). */
	UPROPERTY(EditAnywhere, Category = "Route|Terrain")
	float TerrainMaxHeight = 16000.f;

	/** How much of 0..TerrainMaxHeight the mountains use: higher is more rugged, with flat tops and floors where it clips. */
	UPROPERTY(EditAnywhere, Category = "Route|Terrain")
	float TerrainContrast = 1.3f;

	/** The ground steps in benches this tall (cm) with steep rock faces between: the cliffs. */
	UPROPERTY(EditAnywhere, Category = "Route|Terrain")
	float TerraceHeight = 2500.f;

	/** 0 = smooth mountains, 1 = sharp steps. */
	UPROPERTY(EditAnywhere, Category = "Route|Terrain")
	float TerraceStrength = 0.45f;

	/** Steepest the trail gets (rise over run) outside the stricter zones (the start, the end, the jumps,
	 *  the hairpin and the slalom, the bridge). A climb this steep is slow with a healthy engine and a
	 *  crawl or a stall with a broken one. */
	UPROPERTY(EditAnywhere, Category = "Route|Terrain")
	float MaxGrade = 0.18f;

	/** The trail follows the mountains averaged over this length (cm). */
	UPROPERTY(EditAnywhere, Category = "Route|Terrain")
	float ProfileSmoothing = 3000.f;

	/** Ground at trail height either side of the trail (cm), then BlendWidth out to the natural ground. A short
	 *  blend leaves a rock wall on the uphill side and a drop on the other. */
	UPROPERTY(EditAnywhere, Category = "Route|Terrain")
	float ShoulderWidth = 200.f;

	UPROPERTY(EditAnywhere, Category = "Route|Terrain")
	float BlendWidth = 1500.f;

	/** Lumps in the trail (cm, up and down), doubled on cobbled sections; none at the start, the end, the jumps or the bridge. */
	UPROPERTY(EditAnywhere, Category = "Route|Terrain")
	float TrailRoughness = 15.f;

	/** Most the trail leans sideways (rise over run across it), varying along the way. */
	UPROPERTY(EditAnywhere, Category = "Route|Terrain")
	float TrailCamber = 0.05f;

	/** Rivers: the channel's half width and its banks (cm), and how deep it is (cm below the water). */
	UPROPERTY(EditAnywhere, Category = "Route|Terrain")
	float RiverHalfWidth = 1400.f;

	UPROPERTY(EditAnywhere, Category = "Route|Terrain")
	float RiverBank = 600.f;

	UPROPERTY(EditAnywhere, Category = "Route|Terrain")
	float RiverDepth = 250.f;

	/** Water over the trail at a ford (cm): deep enough to sweep a squirrel away, shallow enough to drive. */
	UPROPERTY(EditAnywhere, Category = "Route|Terrain")
	float FordDepth = 60.f;

	/** How far each river runs either side of the trail (cm). */
	UPROPERTY(EditAnywhere, Category = "Route|Terrain")
	float RiverReach = 30000.f;

	/** The bridge's deck stands this far (cm) above its river. */
	UPROPERTY(EditAnywhere, Category = "Route|Terrain")
	float BridgeClearance = 900.f;

private:
	/** Walk the segments into evenly spaced centreline samples (local space). */
	void BuildCentreline();

	/** Server, BeginPlay: the dog's kennel beside DogSection. */
	void SpawnDog();
	void BuildGeometry();
	/** Ramp up, flat top, ramp down along the road from StartDistance. RampLength 0 = a lone kicker. */
	void AddRaised(UInstancedStaticMeshComponent* Mesh, float StartDistance, float Length, float RampLength, float Height, float Width, bool bKicker);
	/** A roadside board at Distance along the road, sized to its text, facing oncoming cars. */
	void AddSign(const FString& Text, float Distance, bool bLeftSide = false);

	/** Dressing: Kenney trees, bushes, rocks, flowers either side of the road (fixed seed, never on any
	 *  stretch of road), fences and lamps along THE PARK, lamps on the HOME STRETCH, cones at the corners.
	 *  Decoration only: no collision, so nothing new to get stuck on. */
	void AddProps(const FBox2D& GroundBounds);

	/** One instanced layer per prop mesh, keeping the mesh's own materials. */
	UInstancedStaticMeshComponent* PropLayer(int32 Kind);

	/** Nearest distance (cm, XY) from Location to the road centreline anywhere along the route. */
	float DistanceToRoad(const FVector& Location) const;

	/** A cube instance: Size in cm, at Location (local) with Rotation. */
	static void AddBox(UInstancedStaticMeshComponent* Mesh, const FVector& Location, const FRotator& Rotation, const FVector& Size);
	UInstancedStaticMeshComponent* MakeLayer(const TCHAR* Name, const FLinearColor& Color, bool bCollision, bool bShadows);

	/** Local-space sample at a distance (cm), interpolated; Z is the road's height. */
	void SampleAt(float Distance, FVector& OutLocation, float& OutYaw) const;

	/** Terrain: the trail's heights into SamplePoints' Z, its roughness and camber, the rivers, and the lookup
	 *  grid GroundZ uses. */
	void BuildProfile();
	/** The grade limit at a distance along the road (the strict zones). */
	float GradeLimitAt(float Distance) const;
	/** The mountains without the trail or rivers (local space). */
	float TerrainZ(float X, float Y) const;

	/** Everything the landscape needs at a local X/Y. */
	struct FGroundSample
	{
		float Z = 0.f;
		/** 0..1: how much of the dirt-track paint this point gets. */
		float Trail = 0.f;
	};
	FGroundSample SampleGround(float X, float Y) const;
	float GroundZ(float X, float Y) const { return SampleGround(X, Y).Z; }
	/** Up vector of the ground at a local X/Y. */
	FVector GroundNormal(float X, float Y) const;

	/** A river crossing the trail at a ford or under the bridge (terrain only). Local space. */
	struct FRiver
	{
		/** The river's middle every RiverStep cm, upstream first. */
		TArray<FVector2D> Points;
		/** The water's surface (local Z) at the crossing, and at each point (it runs downhill away from the trail). */
		float Level = 0.f;
		TArray<float> Levels;
		/** Distance along the trail where it crosses. */
		float Crossing = 0.f;
		bool bFord = false;
	};
	TArray<FRiver> Rivers;

	/** Depth of water (cm, 0 = dry) at a local point, and the way it flows. */
	float WaterDepthAt(const FVector& Local, FVector2D* OutFlow = nullptr) const;
	/** Nearest point of a river: distance (cm, XY), the flow direction there, how far from either end (cm), and the water's level. */
	float RiverDistance(const FRiver& River, const FVector2D& P, FVector2D* OutFlow, float* OutFromEnd, float* OutLevel = nullptr) const;

	/** Per centreline sample: how rough the trail is (0 = smooth) and how much it leans (rise over run, + = right side up). */
	TArray<float> SampleRoughness;
	TArray<float> SampleCamber;

	/** Road steps (sample i to i+1) by the grid cell of sample i, for GroundZ. */
	TMap<FIntPoint, TArray<int32>> StepGrid;

	UPROPERTY()
	TObjectPtr<UStaticMesh> CubeMesh;

	UPROPERTY()
	TObjectPtr<UMaterialInterface> ShapeMaterial;

	/** Prop meshes in the order of the table in NBRoute.cpp; null where an asset is missing. */
	UPROPERTY()
	TArray<TObjectPtr<UStaticMesh>> PropMeshes;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UInstancedStaticMeshComponent>> PropLayers;


	/** Where the roadside boards stand (local XY) and, in Z, how far props keep clear of each. */
	TArray<FVector> SignSpots;

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
	/** Stones hidden in the fords' water. */
	UPROPERTY()
	TObjectPtr<UInstancedStaticMeshComponent> Pebbles;
	/** The edge of the world: the grey walls, or invisible ones with terrain (the hills behind stay in view). */
	UPROPERTY()
	TObjectPtr<UInstancedStaticMeshComponent> Boundary;

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
