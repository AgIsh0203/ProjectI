// Squirrel Wheels prototype.

#include "Game/NBRoute.h"

#include "Game/NBProp.h"

#include "Components/InstancedStaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Engine/CollisionProfile.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "Game/NBFinishZone.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"
#include "Algo/BinarySearch.h"
#include "Algo/Reverse.h"
#include "Async/ParallelFor.h"
#include "EngineUtils.h"
#include "Landscape.h"
#include "LandscapeInfo.h"
#include "LandscapeLayerInfoObject.h"
#include "LandscapeProxy.h"
#if WITH_EDITOR
#include "LandscapeUtils.h"
#endif

namespace
{
	/** Centreline sample spacing (cm). */
	constexpr float SampleSpacing = 500.f;

	/** How many samples either side of the last answer GetDistanceAlong searches (±300 m). */
	constexpr int32 SearchWindow = 60;

	/** Grid cell (cm) for GroundZ's road lookup: bigger than the road's reach (half road + shoulder + blend). */
	constexpr float GridCell = 5000.f;

	/** The bridge deck's ramps at each end (cm). */
	constexpr float BridgeRampLength = 1500.f;

	FVector RightOf(float Yaw)
	{
		return FRotator(0.f, Yaw + 90.f, 0.f).Vector();
	}

	/** One kind of roadside prop (Kenney CC0 kits, in /Game/ThirdParty/Kenney). Kenney's units are
	 *  small, so scales are big. Weight 0 = not scattered (placed by hand in AddProps). */
	struct FPropKind
	{
		const TCHAR* Path;
		float ScaleMin, ScaleMax;
		float Weight;
		/** Distance (cm) out from the road's edge. */
		float BandMin, BandMax;
		bool bShadows;
		/** Collision: an invisible box this share of the mesh's footprint (a tree's trunk, most of a rock), full height.
		 *  0 = drive through it (bushes, flowers, cones). Friend test 1: the car drove through trees. */
		float Block;
		/** Centre the box on the mesh's pivot (a trunk or pole) and size it from the narrower side of the footprint,
		 *  instead of centring it on the bounds: a lamp's bounds sit halfway along its arm. */
		bool bBlockAtPivot;
	};

	const FPropKind PropKinds[] = {
		{TEXT("Nature/tree_default/StaticMeshes/SM_tree_default.SM_tree_default"),       4.5f, 6.f, 3.f, 500.f, 4500.f, true, 0.14f, true},
		{TEXT("Nature/tree_oak/StaticMeshes/SM_tree_oak.SM_tree_oak"),                   4.5f, 6.f, 3.f, 500.f, 4500.f, true, 0.14f, true},
		{TEXT("Nature/tree_pineTallA/StaticMeshes/SM_tree_pineTallA.SM_tree_pineTallA"), 4.f, 6.f, 2.f, 500.f, 4500.f, true, 0.14f, true},
		{TEXT("Nature/tree_fat/StaticMeshes/SM_tree_fat.SM_tree_fat"),                   4.f, 5.5f, 2.f, 500.f, 4500.f, true, 0.16f, true},
		{TEXT("Nature/tree_blocks/StaticMeshes/SM_tree_blocks.SM_tree_blocks"),          4.f, 5.f, 1.f, 500.f, 4500.f, true, 0.16f, true},
		{TEXT("Nature/plant_bush/StaticMeshes/SM_plant_bush.SM_plant_bush"),             3.f, 4.5f, 3.f, 400.f, 2500.f, false, 0.f, false},
		{TEXT("Nature/plant_bushLarge/StaticMeshes/SM_plant_bushLarge.SM_plant_bushLarge"), 3.f, 4.5f, 2.f, 400.f, 2500.f, false, 0.f, false},
		{TEXT("Nature/rock_largeA/StaticMeshes/SM_rock_largeA.SM_rock_largeA"),          2.5f, 4.f, 1.5f, 500.f, 4000.f, true, 0.75f, false},
		{TEXT("Nature/rock_largeB/StaticMeshes/SM_rock_largeB.SM_rock_largeB"),          2.5f, 4.f, 1.f, 500.f, 4000.f, true, 0.75f, false},
		{TEXT("Nature/rock_tallA/StaticMeshes/SM_rock_tallA.SM_rock_tallA"),             2.f, 3.f, 0.7f, 700.f, 4000.f, true, 0.7f, false},
		{TEXT("Nature/stump_round/StaticMeshes/SM_stump_round.SM_stump_round"),          3.f, 4.f, 1.f, 400.f, 3000.f, false, 0.7f, false},
		{TEXT("Nature/log/StaticMeshes/SM_log.SM_log"),                                  3.f, 4.f, 1.f, 400.f, 3000.f, false, 0.85f, false},
		{TEXT("Nature/flower_redA/StaticMeshes/SM_flower_redA.SM_flower_redA"),          3.f, 4.f, 2.f, 400.f, 1500.f, false, 0.f, false},
		{TEXT("Nature/flower_yellowA/StaticMeshes/SM_flower_yellowA.SM_flower_yellowA"), 3.f, 4.f, 2.f, 400.f, 1500.f, false, 0.f, false},
		{TEXT("Nature/mushroom_red/StaticMeshes/SM_mushroom_red.SM_mushroom_red"),       3.f, 4.f, 1.f, 400.f, 2000.f, false, 0.f, false},
		{TEXT("Nature/grass_large/StaticMeshes/SM_grass_large.SM_grass_large"),          3.f, 4.f, 3.f, 400.f, 2000.f, false, 0.f, false},
		// Placed by AddProps, not scattered.
		{TEXT("Nature/fence_simple/StaticMeshes/SM_fence_simple.SM_fence_simple"),       3.f, 3.f, 0.f, 0.f, 0.f, false, 1.f, false},
		{TEXT("CityRoads/construction-cone/StaticMeshes/SM_construction_cone.SM_construction_cone"), 7.f, 7.f, 0.f, 0.f, 0.f, false, 0.f, false},
		{TEXT("CityRoads/construction-barrier/StaticMeshes/SM_construction_barrier.SM_construction_barrier"), 7.f, 7.f, 0.f, 0.f, 0.f, true, 0.9f, false},
		{TEXT("CityRoads/construction-light/StaticMeshes/SM_construction_light.SM_construction_light"), 7.f, 7.f, 0.f, 0.f, 0.f, true, 1.f, true},
		{TEXT("CityRoads/light-square/StaticMeshes/SM_light_square.SM_light_square"),    10.f, 10.f, 0.f, 0.f, 0.f, true, 1.f, true},
	};
	constexpr int32 FenceKind = 16, ConeKind = 17, BarrierKind = 18, WorkLightKind = 19, LampKind = 20;
}

ANBRoute::ANBRoute()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	bAlwaysRelevant = true;
	// Clients build the same road themselves; get it there before the car lands on it.
	NetPriority = 3.f;

	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	Root->SetMobility(EComponentMobility::Movable);
	SetRootComponent(Root);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> Cube(TEXT("/Engine/BasicShapes/Cube.Cube"));
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> Material(TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
	CubeMesh = Cube.Object;
	ShapeMaterial = Material.Object;

	// /Game/ThirdParty is always cooked; only this constructor references the props.
	for (const FPropKind& Kind : PropKinds)
	{
		ConstructorHelpers::FObjectFinder<UStaticMesh> Prop(*(FString(TEXT("/Game/ThirdParty/Kenney/")) + Kind.Path));
		PropMeshes.Add(Prop.Object);
	}

	// Park to depot, about 5.4 km (3.5 km until friend test 1, where run 1 finished with half the
	// deadline left and everyone called it too short). Features sit on straights where they need
	// to (hills, bridge, jumps). Checked for self-crossing: the road never comes within ~90 m of
	// another stretch of itself.
	auto Add = [this](const TCHAR* Section, float Length, float Turn, ENBRouteFeature Feature = ENBRouteFeature::None, float Height = 0.f)
	{
		FNBRouteSegment& Segment = Segments.AddDefaulted_GetRef();
		Segment.Section = Section;
		Segment.Length = Length;
		Segment.TurnDegrees = Turn;
		Segment.Feature = Feature;
		Segment.Height = Height;
	};
	Add(TEXT("THE PARK"), 200.f, 0.f);
	Add(TEXT(""), 100.f, 45.f);
	Add(TEXT(""), 100.f, -45.f);
	Add(TEXT("SQUIRREL CREEK"), 120.f, 0.f, ENBRouteFeature::Ford);
	Add(TEXT("COBBLE LANE"), 40.f, 0.f, ENBRouteFeature::Bumps);
	Add(TEXT(""), 120.f, 35.f, ENBRouteFeature::Bumps);
	Add(TEXT("ACORN HILL"), 60.f, 0.f);
	Add(TEXT(""), 200.f, 0.f, ENBRouteFeature::Hill, 800.f);
	Add(TEXT(""), 60.f, 0.f);
	Add(TEXT("PINE PASS"), 160.f, -50.f, ENBRouteFeature::Rocks);
	Add(TEXT(""), 160.f, 70.f);
	Add(TEXT(""), 140.f, -20.f, ENBRouteFeature::Rocks);
	Add(TEXT("THE HAIRPIN"), 150.f, 170.f);
	Add(TEXT(""), 100.f, 0.f);
	Add(TEXT("THE BRIDGE"), 40.f, 0.f);
	Add(TEXT(""), 200.f, 0.f, ENBRouteFeature::Bridge, 150.f);
	Add(TEXT(""), 60.f, 0.f);
	Add(TEXT("THE HUMPS"), 60.f, 0.f);
	Add(TEXT(""), 130.f, 0.f, ENBRouteFeature::Hill, 400.f);
	Add(TEXT(""), 60.f, 0.f);
	Add(TEXT(""), 130.f, 0.f, ENBRouteFeature::Hill, 500.f);
	Add(TEXT(""), 60.f, 0.f);
	Add(TEXT("ROCKY ROAD"), 120.f, -40.f, ENBRouteFeature::Rocks);
	Add(TEXT(""), 220.f, 0.f, ENBRouteFeature::Rocks);
	Add(TEXT("SQUIRREL SLALOM"), 260.f, 0.f, ENBRouteFeature::Slalom);
	Add(TEXT("THE BIG JUMP"), 160.f, 0.f, ENBRouteFeature::Jump, 220.f);
	Add(TEXT("LITTLE JUMPS"), 80.f, 0.f);
	Add(TEXT(""), 100.f, 0.f, ENBRouteFeature::Jump, 140.f);
	Add(TEXT(""), 60.f, 0.f);
	Add(TEXT(""), 100.f, 0.f, ENBRouteFeature::Jump, 160.f);
	Add(TEXT(""), 60.f, 20.f);
	Add(TEXT("WIGGLY ROAD"), 110.f, -60.f);
	Add(TEXT(""), 110.f, 60.f);
	Add(TEXT(""), 110.f, -60.f, ENBRouteFeature::Bumps);
	Add(TEXT(""), 110.f, 60.f);
	Add(TEXT(""), 110.f, -60.f);
	Add(TEXT(""), 110.f, 60.f, ENBRouteFeature::Bumps);
	Add(TEXT("COBBLE CURVE"), 360.f, -90.f, ENBRouteFeature::Bumps);
	Add(TEXT("HOME STRETCH"), 180.f, -10.f);
	Add(TEXT(""), 270.f, 0.f, ENBRouteFeature::Rocks);
	Add(TEXT("THE WASH"), 120.f, 0.f, ENBRouteFeature::Ford);
	Add(TEXT(""), 60.f, 0.f);
	Add(TEXT("THE DEPOT"), 120.f, 0.f);
}

void ANBRoute::BuildGeometryNow()
{
	// An editor world never calls PostInitializeComponents, so the centreline isn't there yet.
	BuildCentreline();
	BuildGeometry();
}

float ANBRoute::GetPropBlockFraction(const UStaticMesh* Mesh) const
{
	const int32 Kind = PropMeshes.IndexOfByKey(Mesh);
	return Kind != INDEX_NONE && Kind < UE_ARRAY_COUNT(PropKinds) ? PropKinds[Kind].Block : 0.f;
}

bool ANBRoute::GetPropBlockAtPivot(const UStaticMesh* Mesh) const
{
	const int32 Kind = PropMeshes.IndexOfByKey(Mesh);
	return Kind != INDEX_NONE && Kind < UE_ARRAY_COUNT(PropKinds) && PropKinds[Kind].bBlockAtPivot;
}

void ANBRoute::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	BuildCentreline();
}

void ANBRoute::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	BuildCentreline();
}

void ANBRoute::BeginPlay()
{
	Super::BeginPlay();

	if (bBuildGeometry)
	{
		BuildGeometry();
	}

	if (HasAuthority() && TotalLength > 0.f)
	{
		FActorSpawnParameters Params;
		Params.Owner = this;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		// The gate's slab sits at its own height, so ~10 cm above the road.
		const FTransform Gate = GetTransformAt(TotalLength - 3000.f, 10.f);
		FinishZone = GetWorld()->SpawnActor<ANBFinishZone>(ANBFinishZone::StaticClass(), Gate, Params);
	}
}

// ---------------------------------------------------------------------------------------------
// Centreline

void ANBRoute::BuildCentreline()
{
	SamplePoints.Reset();
	SampleYaws.Reset();
	SampleDistances.Reset();
	SectionNames.Reset();
	SectionStarts.Reset();

	FVector Location = FVector::ZeroVector;
	float Yaw = 0.f;
	float Distance = 0.f;
	SamplePoints.Add(Location);
	SampleYaws.Add(Yaw);
	SampleDistances.Add(Distance);

	for (const FNBRouteSegment& Segment : Segments)
	{
		if (!Segment.Section.IsEmpty() || SectionNames.IsEmpty())
		{
			SectionNames.Add(Segment.Section.IsEmpty() ? FString(TEXT("THE ROAD")) : Segment.Section);
			SectionStarts.Add(Distance);
		}

		const float Length = FMath::Max(Segment.Length, 10.f) * 100.f;
		const int32 Steps = FMath::Max(1, FMath::RoundToInt(Length / SampleSpacing));
		const float Step = Length / Steps;
		const float Turn = Segment.TurnDegrees / Steps;
		for (int32 i = 0; i < Steps; ++i)
		{
			// Head along the middle of each step's turn, so arcs close up exactly.
			Location += FRotator(0.f, Yaw + Turn * 0.5f, 0.f).Vector() * Step;
			Yaw += Turn;
			SamplePoints.Add(Location);
			SampleYaws.Add(Yaw);
			SampleDistances.Add(Distance + Step * (i + 1));
		}
		Distance += Length;
	}
	TotalLength = Distance;
	BuildProfile();
}

// ---------------------------------------------------------------------------------------------
// Terrain: one deterministic height model, so every machine and every bake agree

namespace
{
	/** River centre points this far apart (cm). */
	constexpr float RiverStep = 1000.f;

	/** Water boxes and the "swept away" check stop this far (cm) short of a river's ends, where it runs dry. */
	constexpr float RiverDryEnd = 1500.f;

	float Smooth01(float X)
	{
		X = FMath::Clamp(X, 0.f, 1.f);
		return X * X * (3.f - 2.f * X);
	}
}

float ANBRoute::TerrainZ(float X, float Y) const
{
	// The offsets keep the samples off the lattice points (where Perlin noise is always 0) and come from the seed.
	const float Offset = (Seed % 1000) * 0.0137f;
	auto Noise = [X, Y, Offset](float Wavelength, float Shift)
	{
		return FMath::PerlinNoise2D(FVector2D(X / Wavelength + Offset + Shift, Y / Wavelength - Offset + Shift * 0.61f));
	};
	// Big mountains, hills, smaller hills, lumps: mostly within -0.4..0.4.
	const float Base = (Noise(90000.f, 17.31f) + 0.7f * Noise(35000.f, 41.73f) + 0.3f * Noise(14000.f, 73.17f) + 0.1f * Noise(5000.f, 5.91f)) / 2.1f;
	// Sharp crests where a slower noise crosses zero: ridges.
	const float Ridge = FMath::Square(1.f - FMath::Abs(Noise(30000.f, 29.3f)));
	float Height = TerrainMaxHeight * FMath::Clamp(0.5f + Base * TerrainContrast + (Ridge - 0.5f) * 0.4f, 0.f, 1.f);
	// Benches with steep faces between them: the cliffs. A slow noise shifts the steps up and down, so the
	// faces wander across the slopes instead of following the contours.
	if (TerraceHeight > 0.f && TerraceStrength > 0.f)
	{
		const float Shift = 0.8f * Noise(20000.f, 61.7f);
		const float Steps = Height / TerraceHeight + Shift;
		const float Terraced = (FMath::FloorToFloat(Steps) + Smooth01((FMath::Frac(Steps) - 0.3f) / 0.4f) - Shift) * TerraceHeight;
		Height = FMath::Lerp(Height, Terraced, TerraceStrength);
	}
	return Height;
}

float ANBRoute::GradeLimitAt(float Distance) const
{
	// The car spawns at 10 m; the finish gate and depot sit in the last 120 m.
	if (Distance < 6000.f || Distance > TotalLength - 12000.f)
	{
		return 0.02f;
	}
	float Limit = MaxGrade;
	float Start = 0.f;
	FString Section;
	for (const FNBRouteSegment& Segment : Segments)
	{
		const float Length = FMath::Max(Segment.Length, 10.f) * 100.f;
		if (!Segment.Section.IsEmpty())
		{
			Section = Segment.Section;
		}
		if (Segment.Feature == ENBRouteFeature::Bridge && Distance >= Start - 1000.f && Distance <= Start + Length + 1000.f)
		{
			return 0.f;
		}
		if (Segment.Feature == ENBRouteFeature::Jump && Distance >= Start - 3000.f && Distance <= Start + Length + 3000.f)
		{
			Limit = FMath::Min(Limit, 0.03f);
		}
		if (Distance >= Start && Distance <= Start + Length && (Section == TEXT("THE HAIRPIN") || Section == TEXT("SQUIRREL SLALOM")))
		{
			Limit = FMath::Min(Limit, 0.1f);
		}
		// THE PARK is where everyone learns the controls: gentle.
		if (Distance >= Start && Distance <= Start + Length && Section == TEXT("THE PARK"))
		{
			Limit = FMath::Min(Limit, 0.06f);
		}
		Start += Length;
	}
	return Limit;
}

void ANBRoute::BuildProfile()
{
	Rivers.Reset();
	StepGrid.Reset();
	const int32 Num = SamplePoints.Num();
	SampleRoughness.Init(0.f, Num);
	SampleCamber.Init(0.f, Num);
	if (!bUseTerrain || Num < 2)
	{
		return;
	}

	struct FSpan { float Start, End; ENBRouteFeature Feature; float Height; };
	TArray<FSpan> Spans;
	{
		float Start = 0.f;
		for (const FNBRouteSegment& Segment : Segments)
		{
			const float Length = FMath::Max(Segment.Length, 10.f) * 100.f;
			Spans.Add({Start, Start + Length, Segment.Feature, Segment.Height});
			Start += Length;
		}
	}
	auto Smooth = [Num](TArray<float>& Values, int32 HalfWindow, const TArray<bool>* Keep = nullptr)
	{
		TArray<double> Sum;
		Sum.SetNumZeroed(Num + 1);
		for (int32 i = 0; i < Num; ++i)
		{
			Sum[i + 1] = Sum[i] + Values[i];
		}
		for (int32 i = 0; i < Num; ++i)
		{
			if (Keep && (*Keep)[i])
			{
				continue;
			}
			const int32 A = FMath::Max(0, i - HalfWindow), B = FMath::Min(Num - 1, i + HalfWindow);
			Values[i] = static_cast<float>((Sum[B + 1] - Sum[A]) / (B - A + 1));
		}
	};

	// 1. The mountains under the centreline, averaged over ProfileSmoothing.
	TArray<float> Heights;
	Heights.SetNum(Num);
	for (int32 i = 0; i < Num; ++i)
	{
		Heights[i] = TerrainZ(SamplePoints[i].X, SamplePoints[i].Y);
	}
	Smooth(Heights, FMath::Max(1, FMath::RoundToInt(ProfileSmoothing * 0.5f / SampleSpacing)));

	// 2. ACORN HILL and THE HUMPS: bumps on the profile.
	for (const FSpan& Span : Spans)
	{
		if (Span.Feature != ENBRouteFeature::Hill)
		{
			continue;
		}
		for (int32 i = 0; i < Num; ++i)
		{
			const float T = (SampleDistances[i] - Span.Start) / (Span.End - Span.Start);
			if (T > 0.f && T < 1.f)
			{
				Heights[i] += Span.Height * 0.5f * (1.f - FMath::Cos(T * UE_TWO_PI));
			}
		}
	}

	// 3. Limit the grade both ways (each step to its zone's limit), round off the kinks, and limit again.
	//    Pinned samples (the fords, later) hold their height and the others give way.
	TArray<float> Limits;
	Limits.SetNum(Num);
	for (int32 i = 1; i < Num; ++i)
	{
		Limits[i] = GradeLimitAt((SampleDistances[i - 1] + SampleDistances[i]) * 0.5f);
	}
	TArray<bool> Pinned;
	Pinned.Init(false, Num);
	auto LimitGrades = [&]()
	{
		for (int32 i = 1; i < Num; ++i)
		{
			const float Rise = Limits[i] * (SampleDistances[i] - SampleDistances[i - 1]);
			if (!Pinned[i])
			{
				Heights[i] = FMath::Clamp(Heights[i], Heights[i - 1] - Rise, Heights[i - 1] + Rise);
			}
		}
		for (int32 i = Num - 2; i >= 0; --i)
		{
			const float Rise = Limits[i + 1] * (SampleDistances[i + 1] - SampleDistances[i]);
			if (!Pinned[i])
			{
				Heights[i] = FMath::Clamp(Heights[i], Heights[i + 1] - Rise, Heights[i + 1] + Rise);
			}
		}
	};
	LimitGrades();
	Smooth(Heights, 2);
	LimitGrades();
	for (int32 i = 0; i < Num; ++i)
	{
		SamplePoints[i].Z = Heights[i];
	}

	// 4. Rivers: one across each ford and under the bridge, wandering off either side of the trail.
	for (int32 SpanIndex = 0; SpanIndex < Spans.Num(); ++SpanIndex)
	{
		const FSpan& Span = Spans[SpanIndex];
		if (Span.Feature != ENBRouteFeature::Ford && Span.Feature != ENBRouteFeature::Bridge)
		{
			continue;
		}
		FRiver& River = Rivers.AddDefaulted_GetRef();
		River.bFord = Span.Feature == ENBRouteFeature::Ford;
		River.Crossing = (Span.Start + Span.End) * 0.5f;
		FVector Centre;
		float Yaw;
		SampleAt(River.Crossing, Centre, Yaw);
		// A ford is a dip: its water is at the lowest the trail gets near the crossing, so both approaches come
		// down into it. The bridge's river runs in a gorge under the deck.
		if (River.bFord)
		{
			River.Level = Centre.Z;
			for (int32 i = 0; i < Num; ++i)
			{
				if (FMath::Abs(SampleDistances[i] - River.Crossing) <= RiverHalfWidth + RiverBank + 1000.f)
				{
					River.Level = FMath::Min(River.Level, SamplePoints[i].Z);
				}
			}
		}
		else
		{
			River.Level = Centre.Z - BridgeClearance;
		}
		const int32 CrossingSample = FMath::Clamp(Algo::UpperBound(SampleDistances, River.Crossing), 0, Num - 1);

		const FVector2D Across(RightOf(Yaw)), Along(FRotator(0.f, Yaw, 0.f).Vector());
		FRandomStream Stream(Seed + 101 * SpanIndex);
		const float Phase = Stream.FRandRange(0.f, 10.f);
		auto PointAt = [&](float S)
		{
			// Straight across the trail, then wandering.
			const float Wander = 3000.f * FMath::Sin(S / 12000.f + Phase) - 3000.f * FMath::Sin(Phase);
			return FVector2D(Centre) + Across * S + Along * Wander * Smooth01((FMath::Abs(S) - 2000.f) / 6000.f);
		};
		// Out to RiverReach each way, stopping short of any other stretch of trail.
		const float KeepClear = RoadWidth * 0.5f + ShoulderWidth + BlendWidth + RiverHalfWidth + RiverBank;
		auto Reach = [&](float Direction)
		{
			float S = 0.f;
			while (S + RiverStep <= RiverReach)
			{
				const FVector2D P = PointAt(Direction * (S + RiverStep));
				bool bBlocked = false;
				for (int32 i = 0; i < Num && !bBlocked; ++i)
				{
					bBlocked = FMath::Abs(i - CrossingSample) > 12 && FVector2D::DistSquared(FVector2D(SamplePoints[i]), P) < FMath::Square(KeepClear);
				}
				if (bBlocked)
				{
					break;
				}
				S += RiverStep;
			}
			return FMath::Max(S, KeepClear);
		};
		// Downstream is the way the ground falls away.
		const float Ends[2] = {-Reach(-1.f), Reach(1.f)};
		const int32 Down = TerrainZ(PointAt(Ends[1]).X, PointAt(Ends[1]).Y) < TerrainZ(PointAt(Ends[0]).X, PointAt(Ends[0]).Y) ? 1 : 0;
		const float DownSign = Down == 1 ? 1.f : -1.f;
		// The water is level at the crossing. Downstream it follows the ground down (never up); upstream it
		// climbs gently toward the ground, and the river starts where the ground no longer rises above it.
		auto LevelAlong = [&](float Direction, float End, TArray<TPair<FVector2D, float>>& Out)
		{
			float Level = River.Level;
			for (float S = RiverStep; S <= FMath::Abs(End) + 1.f; S += RiverStep)
			{
				const FVector2D P = PointAt(Direction * S);
				const float Ground = TerrainZ(P.X, P.Y) - 150.f;
				if (S > KeepClear)
				{
					if (Direction == DownSign)
					{
						Level = FMath::Min(Level, Ground);
					}
					else if (Ground < Level)
					{
						break;
					}
					else
					{
						Level = FMath::Min(Ground, Level + RiverStep * 0.06f);
					}
				}
				Out.Add({P, Level});
			}
		};
		TArray<TPair<FVector2D, float>> Upstream, Downstream;
		LevelAlong(-DownSign, Ends[1 - Down], Upstream);
		LevelAlong(DownSign, Ends[Down], Downstream);
		for (int32 i = Upstream.Num() - 1; i >= 0; --i)
		{
			River.Points.Add(Upstream[i].Key);
			River.Levels.Add(Upstream[i].Value);
		}
		River.Points.Add(FVector2D(Centre));
		River.Levels.Add(River.Level);
		for (const TPair<FVector2D, float>& Point : Downstream)
		{
			River.Points.Add(Point.Key);
			River.Levels.Add(Point.Value);
		}
		if (River.bFord)
		{
			// The trail dips into the water across the channel and climbs out again.
			for (int32 i = 0; i < Num; ++i)
			{
				if (FMath::Abs(SampleDistances[i] - River.Crossing) <= RiverHalfWidth)
				{
					Heights[i] = River.Level - FordDepth;
					Pinned[i] = true;
				}
			}
		}
	}
	if (Pinned.Contains(true))
	{
		LimitGrades();
		for (int32 i = 0; i < Num; ++i)
		{
			SamplePoints[i].Z = Heights[i];
		}
	}

	// 5. How rough the trail is and how it leans: calm where the car starts and stops, at the jumps and on the
	//    bridge; extra lumpy on the cobbles and the stony fords.
	for (int32 i = 0; i < Num; ++i)
	{
		const float D = SampleDistances[i];
		float Rough = 1.f;
		for (const FSpan& Span : Spans)
		{
			const bool bInside = D >= Span.Start && D <= Span.End;
			if (bInside && Span.Feature == ENBRouteFeature::Bumps)
			{
				Rough = 2.5f;
			}
			else if (bInside && Span.Feature == ENBRouteFeature::Ford)
			{
				Rough = 1.5f;
			}
			if ((Span.Feature == ENBRouteFeature::Jump && D >= Span.Start - 3000.f && D <= Span.End + 3000.f)
				|| (Span.Feature == ENBRouteFeature::Bridge && D >= Span.Start - 1000.f && D <= Span.End + 1000.f))
			{
				Rough = 0.f;
				break;
			}
		}
		if (D < 6000.f || D > TotalLength - 12000.f)
		{
			Rough = 0.f;
		}
		SampleRoughness[i] = Rough;
	}
	Smooth(SampleRoughness, 4);
	const float Offset = (Seed % 1000) * 0.0137f;
	for (int32 i = 0; i < Num; ++i)
	{
		SampleCamber[i] = TrailCamber * FMath::Clamp(1.6f * FMath::PerlinNoise1D(SampleDistances[i] / 6000.f + Offset + 3.3f), -1.f, 1.f)
			* FMath::Min(SampleRoughness[i], 1.f);
	}

	for (int32 i = 0; i + 1 < Num; ++i)
	{
		StepGrid.FindOrAdd(FIntPoint(FMath::FloorToInt(SamplePoints[i].X / GridCell), FMath::FloorToInt(SamplePoints[i].Y / GridCell))).Add(i);
	}
}

float ANBRoute::RiverDistance(const FRiver& River, const FVector2D& P, FVector2D* OutFlow, float* OutFromEnd, float* OutLevel) const
{
	float Best = TNumericLimits<float>::Max();
	const float Total = (River.Points.Num() - 1) * RiverStep;
	for (int32 i = 0; i + 1 < River.Points.Num(); ++i)
	{
		const FVector2D A = River.Points[i], AB = River.Points[i + 1] - A;
		const float T = FMath::Clamp(FVector2D::DotProduct(P - A, AB) / FMath::Max(AB.SizeSquared(), 1.0), 0.0, 1.0);
		const float Dist = FVector2D::Distance(A + AB * T, P);
		if (Dist < Best)
		{
			Best = Dist;
			if (OutFlow)
			{
				*OutFlow = AB.GetSafeNormal();
			}
			if (OutFromEnd)
			{
				const float Along = (i + T) * RiverStep;
				*OutFromEnd = FMath::Min(Along, Total - Along);
			}
			if (OutLevel)
			{
				*OutLevel = FMath::Lerp(River.Levels[i], River.Levels[i + 1], T);
			}
		}
	}
	return Best;
}

ANBRoute::FGroundSample ANBRoute::SampleGround(float X, float Y) const
{
	FGroundSample Out;
	if (!bUseTerrain)
	{
		return Out;
	}
	const float Terrain = TerrainZ(X, Y);
	Out.Z = Terrain;
	if (StepGrid.IsEmpty())
	{
		return Out;
	}
	const FVector2D P(X, Y);
	const float HalfRoad = RoadWidth * 0.5f;
	const float Inner = HalfRoad + ShoulderWidth;
	const float Outer = Inner + BlendWidth;
	// Lumps in the trail: two scales of noise, a few metres across.
	const float Lumps = TrailRoughness * 1.6f * (0.6f * FMath::PerlinNoise2D(FVector2D(X / 600.f + 0.37f, Y / 600.f + 0.71f))
		+ 0.4f * FMath::PerlinNoise2D(FVector2D(X / 350.f + 5.13f, Y / 350.f + 2.29f)));

	// The nearest point of each separate stretch of trail within reach (the hairpin's legs, the wiggles).
	struct FHit { int32 Step; float Dist; float Z; };
	TArray<FHit, TInlineAllocator<32>> Hits;
	const FIntPoint Cell(FMath::FloorToInt(X / GridCell), FMath::FloorToInt(Y / GridCell));
	for (int32 DX = -1; DX <= 1; ++DX)
	{
		for (int32 DY = -1; DY <= 1; ++DY)
		{
			const TArray<int32>* Steps = StepGrid.Find(Cell + FIntPoint(DX, DY));
			if (!Steps)
			{
				continue;
			}
			for (const int32 i : *Steps)
			{
				const FVector2D A(SamplePoints[i]), B(SamplePoints[i + 1]);
				const FVector2D AB = B - A;
				const float T = FMath::Clamp(FVector2D::DotProduct(P - A, AB) / FMath::Max(AB.SizeSquared(), 1.0), 0.0, 1.0);
				const FVector2D Nearest = A + AB * T;
				const float Dist = FVector2D::Distance(Nearest, P);
				if (Dist < Outer)
				{
					// The trail's own surface: its profile, leaning by its camber (the shoulders carry on the lean), plus lumps.
					const float Lateral = FMath::Clamp(FVector2D::CrossProduct(AB.GetSafeNormal(), P - Nearest), -Inner, Inner);
					const float Z = FMath::Lerp(SamplePoints[i].Z, SamplePoints[i + 1].Z, static_cast<double>(T))
						+ Lateral * FMath::Lerp(SampleCamber[i], SampleCamber[i + 1], T)
						+ Lumps * FMath::Lerp(SampleRoughness[i], SampleRoughness[i + 1], T);
					Hits.Add({i, Dist, Z});
				}
			}
		}
	}
	Hits.Sort([](const FHit& L, const FHit& R) { return L.Step < R.Step; });

	float Height = Terrain;
	float RoadWeight = 0.f;
	if (!Hits.IsEmpty())
	{
		// Steps less than ~20 m apart along the trail are the same stretch: keep its nearest.
		TArray<FHit, TInlineAllocator<4>> Stretches;
		for (const FHit& Hit : Hits)
		{
			if (!Stretches.IsEmpty() && Hit.Step - Stretches.Last().Step <= 4)
			{
				FHit& Last = Stretches.Last();
				if (Hit.Dist < Last.Dist)
				{
					Last.Dist = Hit.Dist;
					Last.Z = Hit.Z;
				}
				Last.Step = Hit.Step;
				continue;
			}
			Stretches.Add(Hit);
		}
		// Each stretch pulls the ground to its height, fully across the trail and shoulders, fading out over
		// BlendWidth. Where two overlap they share; where none reaches, the mountains show.
		float WeightSum = 0.f, Pulled = 0.f;
		for (const FHit& Stretch : Stretches)
		{
			const float Weight = 1.f - Smooth01((Stretch.Dist - Inner) / (Outer - Inner));
			WeightSum += Weight;
			Pulled += Weight * Stretch.Z;
			Out.Trail = FMath::Max(Out.Trail, 1.f - Smooth01((Stretch.Dist - HalfRoad + 50.f) / 250.f));
		}
		Height = WeightSum >= 1.f ? Pulled / WeightSum : Pulled + (1.f - WeightSum) * Terrain;
		RoadWeight = FMath::Min(WeightSum, 1.f);
	}

	for (const FRiver& River : Rivers)
	{
		FVector2D Flow;
		float FromEnd, Level;
		const float Dist = RiverDistance(River, P, &Flow, &FromEnd, &Level);
		const float Reach = RiverHalfWidth + RiverBank;
		if (Dist > Reach + BlendWidth)
		{
			continue;
		}
		// The channel shallows to nothing over the last 30 m of each end.
		const float Taper = Smooth01(FromEnd / 3000.f);
		// Banks at least a little above the water, so it never spills over a dip (the trail fords it instead).
		const float Levee = (1.f - Smooth01((Dist - Reach) / BlendWidth)) * Taper * (River.bFord ? 1.f - RoadWeight : 1.f);
		Height = FMath::Max(Height, FMath::Lerp(Height, Level + 30.f, Levee));
		// The channel: shallow where the trail fords it, deep elsewhere and in the gorge under the bridge.
		const float Depth = River.bFord ? FMath::Lerp(RiverDepth, FordDepth, RoadWeight) : RiverDepth;
		// At a ford the trail's own profile is the riverbed: the banks don't cut into it.
		const float Cut = (1.f - Smooth01((Dist - RiverHalfWidth) / RiverBank)) * (River.bFord ? 1.f - RoadWeight : 1.f);
		Height = FMath::Min(Height, FMath::Lerp(Height, Level - Depth * Taper + (River.bFord ? Lumps * RoadWeight * 1.5f : 0.f), Cut));
	}
	Out.Z = Height;
	return Out;
}

float ANBRoute::WaterDepthAt(const FVector& Local, FVector2D* OutFlow) const
{
	for (const FRiver& River : Rivers)
	{
		FVector2D Flow;
		float FromEnd, Level;
		if (RiverDistance(River, FVector2D(Local), &Flow, &FromEnd, &Level) <= RiverHalfWidth + RiverBank && FromEnd > RiverDryEnd && Level > Local.Z)
		{
			if (OutFlow)
			{
				*OutFlow = Flow;
			}
			return Level - Local.Z;
		}
	}
	return 0.f;
}

float ANBRoute::GetWaterDepth(const FVector& WorldLocation, FVector& OutFlow) const
{
	FVector2D Flow(ForceInitToZero);
	const float Depth = WaterDepthAt(GetActorTransform().InverseTransformPosition(WorldLocation), &Flow);
	OutFlow = GetActorTransform().TransformVectorNoScale(FVector(Flow, 0.f));
	return Depth;
}

ANBRoute* ANBRoute::Find(const UWorld* World)
{
	static TWeakObjectPtr<ANBRoute> Cached;
	if (Cached.IsValid() && Cached->GetWorld() == World)
	{
		return Cached.Get();
	}
	for (TActorIterator<ANBRoute> It(World); It; ++It)
	{
		Cached = *It;
		return *It;
	}
	return nullptr;
}

FVector ANBRoute::GroundNormal(float X, float Y) const
{
	constexpr float Step = 100.f;
	const float DX = GroundZ(X + Step, Y) - GroundZ(X - Step, Y);
	const float DY = GroundZ(X, Y + Step) - GroundZ(X, Y - Step);
	return FVector(-DX, -DY, 2.f * Step).GetSafeNormal();
}

float ANBRoute::GetRoadHeight(float Distance) const
{
	FVector Location;
	float Yaw;
	SampleAt(Distance, Location, Yaw);
	return GetActorTransform().TransformPosition(Location).Z;
}

float ANBRoute::GetRoadGrade(float Distance) const
{
	FVector A, B;
	float Yaw;
	SampleAt(Distance - SampleSpacing * 0.5f, A, Yaw);
	SampleAt(Distance + SampleSpacing * 0.5f, B, Yaw);
	const float Run = FVector::DistXY(A, B);
	return Run > 1.f ? (B.Z - A.Z) / Run : 0.f;
}

float ANBRoute::GetGroundHeight(float X, float Y) const
{
	const FVector Local = GetActorTransform().InverseTransformPosition(FVector(X, Y, 0.f));
	return GetActorTransform().TransformPosition(FVector(Local.X, Local.Y, GroundZ(Local.X, Local.Y))).Z;
}

float ANBRoute::GetTerrainHeight(float X, float Y) const
{
	const FVector Local = GetActorTransform().InverseTransformPosition(FVector(X, Y, 0.f));
	return GetActorTransform().TransformPosition(FVector(Local.X, Local.Y, TerrainZ(Local.X, Local.Y))).Z;
}

bool ANBRoute::GetLandscapeHeight(float X, float Y, float& OutZ) const
{
	for (TActorIterator<ALandscapeProxy> It(GetWorld()); It; ++It)
	{
		const TOptional<float> Height = It->GetHeightAtLocation(FVector(X, Y, 0.f));
		if (Height.IsSet())
		{
			OutZ = Height.GetValue();
			return true;
		}
	}
	return false;
}

FString ANBRoute::CheckProfile() const
{
	FString Report;
	int32 Problems = 0;
	float MinZ = TNumericLimits<float>::Max(), MaxZ = TNumericLimits<float>::Lowest();
	for (int32 Section = 0; Section < SectionNames.Num(); ++Section)
	{
		const float Start = SectionStarts[Section];
		const float End = SectionStarts.IsValidIndex(Section + 1) ? SectionStarts[Section + 1] : TotalLength;
		float MaxUp = 0.f, MaxDown = 0.f, Low = TNumericLimits<float>::Max(), High = TNumericLimits<float>::Lowest();
		for (int32 i = 1; i < SamplePoints.Num(); ++i)
		{
			const float Mid = (SampleDistances[i - 1] + SampleDistances[i]) * 0.5f;
			if (Mid < Start || Mid >= End)
			{
				continue;
			}
			const float Run = SampleDistances[i] - SampleDistances[i - 1];
			const float Grade = (SamplePoints[i].Z - SamplePoints[i - 1].Z) / Run;
			const float Limit = GradeLimitAt(Mid);
			MaxUp = FMath::Max(MaxUp, Grade);
			MaxDown = FMath::Min(MaxDown, Grade);
			Low = FMath::Min(Low, SamplePoints[i].Z);
			High = FMath::Max(High, SamplePoints[i].Z);
			if (FMath::Abs(Grade) > Limit + 0.001f)
			{
				Report += FString::Printf(TEXT("  OVER LIMIT at %.0f m: %.1f %% (limit %.1f %%)\n"), Mid / 100.f, Grade * 100.f, Limit * 100.f);
				++Problems;
			}
		}
		MinZ = FMath::Min(MinZ, Low);
		MaxZ = FMath::Max(MaxZ, High);
		Report += FString::Printf(TEXT("%-16s %5.0f-%5.0f m  climbs %4.1f %%  descents %5.1f %%  height %5.1f-%5.1f m\n"),
			*SectionNames[Section], Start / 100.f, End / 100.f, MaxUp * 100.f, MaxDown * 100.f, Low / 100.f, High / 100.f);
	}
	for (const FRiver& River : Rivers)
	{
		Report += FString::Printf(TEXT("river (%s) at %.0f m: water at %.1f m there, %.1f-%.1f m along it, %.0f m long\n"), River.bFord ? TEXT("ford") : TEXT("bridge"),
			River.Crossing / 100.f, River.Level / 100.f, FMath::Min(River.Levels) / 100.f, FMath::Max(River.Levels) / 100.f, (River.Points.Num() - 1) * RiverStep / 100.f);
	}
	Report += FString::Printf(TEXT("trail height %.1f-%.1f m, %d steps over their limit\n"), MinZ / 100.f, MaxZ / 100.f, Problems);
	return Report;
}

UObject* ANBRoute::CreateTrailLayerInfo(const FString& PackagePath)
{
#if WITH_EDITOR
	if (ULandscapeLayerInfoObject* Existing = LoadObject<ULandscapeLayerInfoObject>(nullptr, *PackagePath, nullptr, LOAD_NoWarn | LOAD_Quiet))
	{
		return Existing;
	}
	FString Path, Name;
	PackagePath.Split(TEXT("/"), &Path, &Name, ESearchCase::IgnoreCase, ESearchDir::FromEnd);
	ULandscapeLayerInfoObject* LayerInfo = UE::Landscape::CreateTargetLayerInfo(FName(TEXT("Trail")), Path, Name);
	// Painted on its own, not traded off against other layers.
	LayerInfo->SetBlendMethod(ELandscapeTargetLayerBlendMethod::None, false);
	return LayerInfo;
#else
	return nullptr;
#endif
}

AActor* ANBRoute::BuildLandscape(UMaterialInterface* Material, UObject* TrailLayerInfo, float QuadSize, float Margin)
{
#if WITH_EDITOR
	UWorld* World = GetWorld();
	if (!World || SamplePoints.Num() < 2 || QuadSize < 50.f)
	{
		return nullptr;
	}
	// Components of 127 quads, one section each; enough of them to cover the trail plus Margin.
	constexpr int32 QuadsPerComponent = 127;
	constexpr float ScaleZ = 100.f; // 65536 steps of 100/128 cm: +-256 m around the actor's Z
	FBox2D Bounds(ForceInit);
	for (const FVector& Point : SamplePoints)
	{
		Bounds += FVector2D(GetActorTransform().TransformPosition(Point));
	}
	Bounds = Bounds.ExpandBy(Margin);
	const int32 ComponentsX = FMath::CeilToInt(Bounds.GetSize().X / (QuadSize * QuadsPerComponent));
	const int32 ComponentsY = FMath::CeilToInt(Bounds.GetSize().Y / (QuadSize * QuadsPerComponent));
	const int32 VertsX = ComponentsX * QuadsPerComponent + 1;
	const int32 VertsY = ComponentsY * QuadsPerComponent + 1;
	const FVector Origin(Bounds.GetCenter() - FVector2D(VertsX - 1, VertsY - 1) * QuadSize * 0.5f, GetActorLocation().Z);

	TArray<uint16> Heights;
	Heights.SetNumUninitialized(VertsX * VertsY);
	TArray<uint8> Trail;
	Trail.SetNumUninitialized(VertsX * VertsY);
	ParallelFor(VertsY, [&](int32 Y)
	{
		for (int32 X = 0; X < VertsX; ++X)
		{
			const FVector Local = GetActorTransform().InverseTransformPosition(Origin + FVector(X * QuadSize, Y * QuadSize, 0.f));
			const FGroundSample Ground = SampleGround(Local.X, Local.Y);
			const float Z = GetActorTransform().TransformPosition(FVector(Local.X, Local.Y, Ground.Z)).Z - Origin.Z;
			Heights[Y * VertsX + X] = static_cast<uint16>(FMath::Clamp(FMath::RoundToInt(32768.f + Z * 128.f / ScaleZ), 0, 65535));
			Trail[Y * VertsX + X] = static_cast<uint8>(FMath::RoundToInt(Ground.Trail * 255.f));
		}
	});

	ALandscape* Landscape = World->SpawnActor<ALandscape>(Origin, FRotator::ZeroRotator);
	if (!Landscape)
	{
		return nullptr;
	}
	Landscape->LandscapeMaterial = Material;
	Landscape->SetActorRelativeScale3D(FVector(QuadSize, QuadSize, ScaleZ));
	Landscape->StaticLightingLOD = FMath::DivideAndRoundUp(FMath::CeilLogTwo((VertsX * VertsY) / (2048 * 2048) + 1), (uint32)2);

	ULandscapeLayerInfoObject* LayerInfo = Cast<ULandscapeLayerInfoObject>(TrailLayerInfo);
	TMap<FGuid, TArray<uint16>> HeightData;
	HeightData.Add(FGuid(), MoveTemp(Heights));
	TMap<FGuid, TArray<FLandscapeImportLayerInfo>> ImportLayers;
	TArray<FLandscapeImportLayerInfo>& TrailLayers = ImportLayers.Add(FGuid());
	if (LayerInfo)
	{
		FLandscapeImportLayerInfo& Layer = TrailLayers.Emplace_GetRef(LayerInfo->GetLayerName());
		Layer.LayerInfo = LayerInfo;
		Layer.LayerData = MoveTemp(Trail);
	}
	Landscape->Import(FGuid::NewGuid(), 0, 0, VertsX - 1, VertsY - 1, 1, QuadsPerComponent, HeightData, TEXT(""), ImportLayers,
		ELandscapeImportAlphamapType::Additive, TArrayView<const FLandscapeLayer>());
	if (LayerInfo)
	{
		Landscape->AddTargetLayer(LayerInfo->GetLayerName(), FLandscapeTargetLayerSettings(LayerInfo));
		if (ULandscapeInfo* Info = Landscape->GetLandscapeInfo())
		{
			Info->UpdateLayerInfoMap(Landscape);
		}
	}
	UE_LOG(LogTemp, Log, TEXT("NBRoute: landscape %d x %d vertices (%d x %d components) at %.0f cm per quad"), VertsX, VertsY, ComponentsX, ComponentsY, QuadSize);
	return Landscape;
#else
	return nullptr;
#endif
}


void ANBRoute::SampleAt(float Distance, FVector& OutLocation, float& OutYaw) const
{
	const int32 Last = SamplePoints.Num() - 1;
	if (Last <= 0)
	{
		OutLocation = FVector::ZeroVector;
		OutYaw = 0.f;
		return;
	}
	Distance = FMath::Clamp(Distance, 0.f, TotalLength);
	// First sample past Distance; the answer lies between it and the one before.
	const int32 Next = FMath::Clamp(Algo::UpperBound(SampleDistances, Distance), 1, Last);
	const float StepLength = SampleDistances[Next] - SampleDistances[Next - 1];
	const float Alpha = StepLength > 0.f ? FMath::Clamp((Distance - SampleDistances[Next - 1]) / StepLength, 0.f, 1.f) : 0.f;
	OutLocation = FMath::Lerp(SamplePoints[Next - 1], SamplePoints[Next], Alpha);
	OutYaw = FMath::Lerp(SampleYaws[Next - 1], SampleYaws[Next], Alpha);
}

FTransform ANBRoute::GetTransformAt(float Distance, float Lift) const
{
	FVector Location;
	float Yaw;
	SampleAt(Distance, Location, Yaw);
	const float Pitch = FMath::RadiansToDegrees(FMath::Atan(GetRoadGrade(Distance)));
	return FTransform(FRotator(Pitch, Yaw, 0.f), Location + FVector(0.f, 0.f, Lift)) * GetActorTransform();
}

FTransform ANBRoute::GetStartTransform() const
{
	return GetTransformAt(1000.f);
}

float ANBRoute::GetDistanceAlong(const FVector& WorldLocation) const
{
	const int32 Num = SamplePoints.Num();
	if (Num < 2)
	{
		return 0.f;
	}
	const FVector Local = GetActorTransform().InverseTransformPosition(WorldLocation);

	auto FindNearest = [&](int32 From, int32 To)
	{
		int32 Best = From;
		double BestDistSq = TNumericLimits<double>::Max();
		for (int32 i = From; i <= To; ++i)
		{
			const double DistSq = FVector::DistSquaredXY(SamplePoints[i], Local);
			if (DistSq < BestDistSq)
			{
				BestDistSq = DistSq;
				Best = i;
			}
		}
		return Best;
	};

	int32 Nearest = INDEX_NONE;
	if (LastNearestSample != INDEX_NONE)
	{
		Nearest = FindNearest(FMath::Max(0, LastNearestSample - SearchWindow), FMath::Min(Num - 1, LastNearestSample + SearchWindow));
		// Teleported or way off the road: start over.
		if (FVector::DistSquaredXY(SamplePoints[Nearest], Local) > FMath::Square(RoadWidth * 4.f))
		{
			Nearest = INDEX_NONE;
		}
	}
	if (Nearest == INDEX_NONE)
	{
		Nearest = FindNearest(0, Num - 1);
	}
	LastNearestSample = Nearest;

	// Project onto the better of the two steps either side of the nearest sample.
	auto Project = [&](int32 A, int32 B, double& OutDistSq)
	{
		const FVector2D PA(SamplePoints[A]), PB(SamplePoints[B]), P(Local);
		const FVector2D AB = PB - PA;
		const double T = FMath::Clamp(FVector2D::DotProduct(P - PA, AB) / FMath::Max(AB.SizeSquared(), 1.0), 0.0, 1.0);
		OutDistSq = FVector2D::DistSquared(PA + AB * T, P);
		return FMath::Lerp(SampleDistances[A], SampleDistances[B], static_cast<float>(T));
	};
	double BestDistSq = TNumericLimits<double>::Max();
	float Along = SampleDistances[Nearest];
	if (Nearest + 1 < Num)
	{
		Along = Project(Nearest, Nearest + 1, BestDistSq);
	}
	if (Nearest > 0)
	{
		double BackDistSq;
		const float Back = Project(Nearest - 1, Nearest, BackDistSq);
		if (BackDistSq < BestDistSq)
		{
			Along = Back;
		}
	}
	return Along;
}

int32 ANBRoute::GetSectionAt(float Distance) const
{
	int32 Section = 0;
	for (int32 i = 0; i < SectionStarts.Num(); ++i)
	{
		if (SectionStarts[i] <= Distance)
		{
			Section = i;
		}
	}
	return Section;
}

// ---------------------------------------------------------------------------------------------
// Geometry

UInstancedStaticMeshComponent* ANBRoute::MakeLayer(const TCHAR* Name, const FLinearColor& Color, bool bCollision, bool bShadows)
{
	UInstancedStaticMeshComponent* Mesh = NewObject<UInstancedStaticMeshComponent>(this, Name);
	// Every machine builds the same named layers; this lets the replicated movement base (a squirrel standing on the ground) resolve on clients.
	Mesh->SetNetAddressable();
	Mesh->SetMobility(EComponentMobility::Movable);
	Mesh->SetupAttachment(GetRootComponent());
	Mesh->SetStaticMesh(CubeMesh);
	if (ShapeMaterial)
	{
		UMaterialInstanceDynamic* Tint = UMaterialInstanceDynamic::Create(ShapeMaterial, this);
		Tint->SetVectorParameterValue(TEXT("Color"), Color);
		Mesh->SetMaterial(0, Tint);
	}
	if (bCollision)
	{
		Mesh->SetCollisionProfileName(UCollisionProfile::BlockAll_ProfileName);
	}
	else
	{
		Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
	Mesh->SetCastShadow(bShadows);
	Mesh->RegisterComponent();
	return Mesh;
}

void ANBRoute::AddBox(UInstancedStaticMeshComponent* Mesh, const FVector& Location, const FRotator& Rotation, const FVector& Size)
{
	// The engine cube is 100 cm on a side, centred on its origin.
	Mesh->AddInstance(FTransform(Rotation, Location, Size / 100.f));
}

void ANBRoute::BuildGeometry()
{
	if (!CubeMesh || SamplePoints.Num() < 2)
	{
		return;
	}

	Ground  = MakeLayer(TEXT("Ground"),  FLinearColor(0.16f, 0.32f, 0.12f), true,  false);
	Road    = MakeLayer(TEXT("Road"),    FLinearColor(0.12f, 0.12f, 0.13f), false, false);
	Lines   = MakeLayer(TEXT("Lines"),   FLinearColor(0.9f, 0.9f, 0.85f),   false, false);
	Bumps   = MakeLayer(TEXT("Bumps"),   FLinearColor(0.9f, 0.75f, 0.1f),   true,  true);
	Rocks   = MakeLayer(TEXT("Rocks"),   FLinearColor(0.35f, 0.3f, 0.27f),  true,  true);
	Pillars = MakeLayer(TEXT("Pillars"), FLinearColor(1.f, 0.4f, 0.05f),    true,  true);
	Hills   = MakeLayer(TEXT("Hills"),   FLinearColor(0.45f, 0.5f, 0.2f),   true,  true);
	Planks  = MakeLayer(TEXT("Planks"),  FLinearColor(0.45f, 0.28f, 0.14f), true,  true);
	Markers = MakeLayer(TEXT("Markers"), FLinearColor(0.85f, 0.1f, 0.1f),   false, true);
	Walls   = MakeLayer(TEXT("Walls"),   FLinearColor(0.5f, 0.5f, 0.55f),   true,  true);
	Pebbles = MakeLayer(TEXT("Pebbles"), FLinearColor(0.3f, 0.27f, 0.24f), true,  false);
	if (bUseTerrain)
	{
		// The landscape is the ground; the edge of the world is an invisible wall, so the hills beyond stay in view.
		Boundary = MakeLayer(TEXT("Boundary"), FLinearColor::White, true, false);
		Boundary->SetHiddenInGame(true);
		Boundary->SetVisibility(false);
	}
	else
	{
		Boundary = Walls;
	}

	// Without terrain, a ground slab under the whole road (top at z = 0). Either way, walled in so nothing
	// drives off the world.
	FBox2D Bounds(ForceInit);
	for (const FVector& Point : SamplePoints)
	{
		Bounds += FVector2D(Point);
	}
	Bounds = Bounds.ExpandBy(GroundMargin);
	const FVector2D Centre = Bounds.GetCenter();
	const FVector2D Size = Bounds.GetSize();
	if (!bUseTerrain)
	{
		AddBox(Ground, FVector(Centre, -100.f), FRotator::ZeroRotator, FVector(Size, 200.f));
	}
	// With terrain the walls reach from below the lowest ground to well above the highest hill.
	const float WallHeight = bUseTerrain ? TerrainMaxHeight + 2500.f : 400.f;
	const float WallZ = bUseTerrain ? WallHeight * 0.5f - 1000.f : WallHeight * 0.5f;
	constexpr float WallThick = 200.f;
	AddBox(Boundary, FVector(Bounds.Min.X, Centre.Y, WallZ), FRotator::ZeroRotator, FVector(WallThick, Size.Y, WallHeight));
	AddBox(Boundary, FVector(Bounds.Max.X, Centre.Y, WallZ), FRotator::ZeroRotator, FVector(WallThick, Size.Y, WallHeight));
	AddBox(Boundary, FVector(Centre.X, Bounds.Min.Y, WallZ), FRotator::ZeroRotator, FVector(Size.X, WallThick, WallHeight));
	AddBox(Boundary, FVector(Centre.X, Bounds.Max.Y, WallZ), FRotator::ZeroRotator, FVector(Size.X, WallThick, WallHeight));

	// Road surface (visual only; the ground slab or the landscape is what the wheels touch), edge lines and a
	// dashed centre line. Each piece runs from sample to sample, so on terrain it takes the road's pitch.
	auto AddStrip = [this](UInstancedStaticMeshComponent* Mesh, int32 A, int32 B, float Offset, float Width, float Z, float Overlap)
	{
		const FVector Start = SamplePoints[A] + RightOf(SampleYaws[A]) * Offset;
		const FVector End = SamplePoints[B] + RightOf(SampleYaws[B]) * Offset;
		const FVector Delta = End - Start;
		AddBox(Mesh, (Start + End) * 0.5f + FVector(0.f, 0.f, Z), Delta.Rotation(), FVector(Delta.Size() + Overlap, Width, 2.f));
	};
	const float EdgeOffset = RoadWidth * 0.5f - 40.f;
	// A landscape is a mesh of 2 m quads, not a perfect plane: sit a little higher on it.
	const float RoadZ = bUseTerrain ? 3.f : 1.f, LineZ = RoadZ + 1.5f;
	// With terrain the trail is the landscape itself, painted dirt (its "Trail" layer): no strip, no lines.
	for (int32 i = 0; i + 1 < SamplePoints.Num() && !bUseTerrain; ++i)
	{
		AddStrip(Road, i, i + 1, 0.f, RoadWidth, RoadZ, 80.f);
		AddStrip(Lines, i, i + 1, -EdgeOffset, 30.f, LineZ, 20.f);
		AddStrip(Lines, i, i + 1, EdgeOffset, 30.f, LineZ, 20.f);
		if (i % 2 == 0)
		{
			AddStrip(Lines, i, i + 1, 0.f, 20.f, LineZ, -150.f);
		}
	}

	// Start line.
	if (!bUseTerrain)
	{
		FVector Location;
		float Yaw;
		SampleAt(1500.f, Location, Yaw);
		AddBox(Lines, Location + FVector(0.f, 0.f, LineZ), FRotator(0.f, Yaw, 0.f), FVector(60.f, RoadWidth, 2.f));
	}

	// Per-segment features, corner markers on the outside of bends, and a sign at each section.
	float Distance = 0.f;
	for (int32 Index = 0; Index < Segments.Num(); ++Index)
	{
		const FNBRouteSegment& Segment = Segments[Index];
		const float Length = FMath::Max(Segment.Length, 10.f) * 100.f;

		if (FMath::Abs(Segment.TurnDegrees) >= 20.f)
		{
			const float Side = Segment.TurnDegrees > 0.f ? -1.f : 1.f;
			for (float D = Distance + 600.f; D < Distance + Length; D += 1500.f)
			{
				FVector Location;
				float Yaw;
				SampleAt(D, Location, Yaw);
				Location += RightOf(Yaw) * Side * (RoadWidth * 0.5f + 150.f);
				Location.Z = GroundZ(Location.X, Location.Y) - (bUseTerrain ? 10.f : 0.f);
				AddBox(Markers, Location + FVector(0.f, 0.f, 75.f), FRotator(0.f, Yaw, 0.f), FVector(40.f, 40.f, 150.f));
			}
		}

		FRandomStream Stream(Seed + Index * 7919);
		switch (Segment.Feature)
		{
		case ENBRouteFeature::Bumps:
			// Ridges with 45-degree faces, ~14 cm tall: enough to rattle acorns loose at speed. On terrain the
			// cobbles are extra lumps in the trail instead (BuildProfile).
			for (float D = Distance + 600.f; D < Distance + Length - 300.f && !bUseTerrain; D += 900.f)
			{
				FVector Location;
				float Yaw;
				SampleAt(D, Location, Yaw);
				AddBox(Bumps, Location, FRotator(45.f, Yaw, 0.f), FVector(20.f, RoadWidth, 20.f));
			}
			break;

		case ENBRouteFeature::Rocks:
		{
			const int32 Count = FMath::RoundToInt(Length / 800.f);
			for (int32 i = 0; i < Count; ++i)
			{
				FVector Location;
				float Yaw;
				SampleAt(Distance + Length * Stream.FRandRange(0.05f, 0.95f), Location, Yaw);
				Location += RightOf(Yaw) * RoadWidth * Stream.FRandRange(-0.42f, 0.42f);
				const FRotator Tumble(Stream.FRandRange(-10.f, 10.f), Stream.FRandRange(0.f, 360.f), Stream.FRandRange(-10.f, 10.f));
				const FVector RockSize(Stream.FRandRange(80.f, 180.f), Stream.FRandRange(80.f, 180.f), Stream.FRandRange(30.f, 55.f));
				Location.Z = GroundZ(Location.X, Location.Y);
				AddBox(Rocks, Location, Tumble, RockSize);
			}
			break;
		}

		case ENBRouteFeature::Slalom:
		{
			float Side = 1.f;
			for (float D = Distance + 1500.f; D < Distance + Length - 1000.f; D += 2800.f)
			{
				FVector Location;
				float Yaw;
				SampleAt(D, Location, Yaw);
				Location += RightOf(Yaw) * Side * RoadWidth * 0.22f;
				Location.Z = GroundZ(Location.X, Location.Y) - (bUseTerrain ? 30.f : 0.f);
				AddBox(Pillars, Location + FVector(0.f, 0.f, 160.f), FRotator(0.f, Yaw, 0.f), FVector(120.f, 120.f, 320.f));
				Side = -Side;
			}
			break;
		}

		case ENBRouteFeature::Hill:
			// With terrain the hill is a smooth bump in the road's profile (BuildProfile) instead.
			if (!bUseTerrain)
			{
				AddRaised(Hills, Distance, Length, Length * 0.3f, Segment.Height, RoadWidth + 600.f, false);
			}
			break;

		case ENBRouteFeature::Bridge:
		{
			AddRaised(Planks, Distance, Length, BridgeRampLength, Segment.Height, BridgeWidth, false);
			// On terrain a river runs in a gorge under the deck (its water is added with the other rivers below).
			if (!bUseTerrain)
			{
				// A "river" across the ground under the deck (visual only; falling in just lands on grass).
				FVector Location;
				float Yaw;
				SampleAt(Distance + Length * 0.5f, Location, Yaw);
				UInstancedStaticMeshComponent* Water = MakeLayer(*FString::Printf(TEXT("Water%d"), Index), FLinearColor(0.1f, 0.3f, 0.7f), false, false);
				AddBox(Water, Location + FVector(0.f, 0.f, 4.f), FRotator(0.f, Yaw, 0.f), FVector(Length - BridgeRampLength * 2.f - 400.f, 8000.f, 2.f));
			}
			break;
		}

		case ENBRouteFeature::Ford:
		{
			// Pebbles hidden in the water across the trail: unseen jolts, spilled acorns, worn tyres.
			const FRiver* River = Rivers.FindByPredicate([Distance, Length](const FRiver& R) { return R.Crossing > Distance && R.Crossing < Distance + Length; });
			if (!River)
			{
				break;
			}
			for (int32 i = 0; i < 18; ++i)
			{
				FVector Location;
				float Yaw;
				SampleAt(River->Crossing + RiverHalfWidth * Stream.FRandRange(-0.85f, 0.85f), Location, Yaw);
				Location += RightOf(Yaw) * RoadWidth * Stream.FRandRange(-0.45f, 0.45f);
				Location.Z = GroundZ(Location.X, Location.Y);
				const FRotator Tumble(Stream.FRandRange(-15.f, 15.f), Stream.FRandRange(0.f, 360.f), Stream.FRandRange(-15.f, 15.f));
				AddBox(Pebbles, Location, Tumble, FVector(Stream.FRandRange(35.f, 70.f), Stream.FRandRange(35.f, 70.f), Stream.FRandRange(25.f, 40.f)));
			}
			break;
		}

		case ENBRouteFeature::Jump:
		{
			// A kicker in the middle of the road; the brave go over it, the careful go round.
			constexpr float RampLength = 1000.f;
			AddRaised(Hills, Distance + (Length - RampLength) * 0.5f, RampLength, RampLength, Segment.Height, 600.f, true);
			break;
		}

		case ENBRouteFeature::None:
			break;
		}

		if (!Segment.Section.IsEmpty())
		{
			AddSign(Segment.Section, Distance + 300.f);
		}
		Distance += Length;
	}

	// The rivers' water: flat boxes along each river at its level, as wide as the channel and its banks
	// (the banks rise above it). Opaque, so the pebbles stay hidden.
	if (!Rivers.IsEmpty())
	{
		UInstancedStaticMeshComponent* Water = MakeLayer(TEXT("Water"), FLinearColor(0.12f, 0.27f, 0.3f), false, false);
		Water->SetCollisionProfileName(NBCollision::WaterProfile);
		for (const FRiver& River : Rivers)
		{
			const int32 Skip = FMath::CeilToInt(RiverDryEnd / RiverStep);
			for (int32 i = Skip; i + 1 + Skip < River.Points.Num(); ++i)
			{
				// Each piece runs from one point's level to the next, so the water runs downhill.
				const FVector A(River.Points[i], River.Levels[i] - 1.f), B(River.Points[i + 1], River.Levels[i + 1] - 1.f);
				AddBox(Water, (A + B) * 0.5f, (B - A).Rotation(), FVector(RiverStep + 300.f, (RiverHalfWidth + RiverBank) * 2.f, 2.f));
			}
		}
	}

	// THE PARK teaches the controls on the left, in the order they're needed. The car starts at 10 m;
	// the first board is in view from the start, where everyone spawns on foot.
	AddSign(TEXT("E: GET IN / HOP SEAT"), 2200.f, true);
	AddSign(TEXT("LMB: FIX BROKEN PARTS"), 5500.f, true);
	AddSign(TEXT("SPACE: JUMP OUT"), 9000.f, true);
	AddSign(TEXT("H: HONK!"), 13000.f, true);

	// The depot shed past the drop-off gate.
	{
		FVector Location;
		float Yaw;
		SampleAt(TotalLength, Location, Yaw);
		AddBox(Walls, Location + FRotator(0.f, Yaw, 0.f).Vector() * 1500.f + FVector(0.f, 0.f, 400.f), FRotator(0.f, Yaw, 0.f), FVector(1200.f, 3000.f, 800.f));
	}
	AddProps(Bounds);
}

void ANBRoute::AddRaised(UInstancedStaticMeshComponent* Mesh, float StartDistance, float Length, float RampLength, float Height, float Width, bool bKicker)
{
	if (Height <= 0.f || RampLength <= 0.f)
	{
		return;
	}
	// Raised features assume a straight segment: lay them along the heading at the start.
	FVector Origin;
	float Yaw;
	SampleAt(StartDistance, Origin, Yaw);
	// Tilted to the road's grade where it starts (the profile keeps these stretches nearly level).
	const float Pitch = FMath::RadiansToDegrees(FMath::Atan(GetRoadGrade(StartDistance)));
	const FRotationMatrix Frame(FRotator(Pitch, Yaw, 0.f));
	const FVector Forward = Frame.GetUnitAxis(EAxis::X);
	const FVector Up = Frame.GetUnitAxis(EAxis::Z);

	const float SlopeDegrees = FMath::RadiansToDegrees(FMath::Atan2(Height, RampLength));
	const float SlopeLength = FMath::Sqrt(FMath::Square(RampLength) + FMath::Square(Height));
	// Thick enough that the slab reaches below the ground under its high end.
	const float Thickness = Height / FMath::Cos(FMath::DegreesToRadians(SlopeDegrees)) + 100.f;

	// A slab whose top face runs from (Along, 0) to (Along + RampLength, Height), or back down.
	auto AddRamp = [&](float Along, bool bUp)
	{
		const FRotator Rotation(Pitch + (bUp ? SlopeDegrees : -SlopeDegrees), Yaw, 0.f);
		const FVector TopCentre = Origin + Forward * (Along + RampLength * 0.5f) + Up * (Height * 0.5f);
		const FVector Normal = FRotationMatrix(Rotation).GetUnitAxis(EAxis::Z);
		AddBox(Mesh, TopCentre - Normal * Thickness * 0.5f, Rotation, FVector(SlopeLength, Width, Thickness));
	};

	AddRamp(0.f, true);
	if (bKicker)
	{
		return;
	}
	const float Top = FMath::Max(Length - RampLength * 2.f, 0.f);
	if (Top > 0.f)
	{
		AddBox(Mesh, Origin + Forward * (RampLength + Top * 0.5f) + Up * (Height * 0.5f), FRotator(Pitch, Yaw, 0.f), FVector(Top, Width, Height));
	}
	AddRamp(RampLength + Top, false);
}

void ANBRoute::AddSign(const FString& Text, float Distance, bool bLeftSide)
{
	FVector Location;
	float Yaw;
	SampleAt(Distance, Location, Yaw);
	// The board grows with the text (about 55 cm per letter at this world size).
	const float BoardWidth = FMath::Max(760.f, Text.Len() * 55.f + 120.f);
	const FVector Side = RightOf(Yaw) * (RoadWidth * 0.5f + 100.f + BoardWidth * 0.5f) * (bLeftSide ? -1.f : 1.f);
	const FRotator Facing(0.f, Yaw, 0.f);
	// The post stands on the ground beside the road, not at the road's height.
	Location.Z = GroundZ(Location.X + Side.X, Location.Y + Side.Y);

	AddBox(Walls, Location + Side + FVector(0.f, 0.f, 175.f), Facing, FVector(30.f, 30.f, 350.f));
	AddBox(Markers, Location + Side + FVector(0.f, 0.f, 380.f), Facing, FVector(20.f, BoardWidth, 160.f));
	// Props keep clear of the board: Z holds the clearance radius.
	SignSpots.Add(FVector(FVector2D(Location + Side), BoardWidth * 0.5f + 500.f));

	// Readable from the road behind it: text faces its local +X, so turn it to face oncoming cars.
	UTextRenderComponent* Label = NewObject<UTextRenderComponent>(this);
	Label->SetMobility(EComponentMobility::Movable);
	Label->SetupAttachment(GetRootComponent());
	Label->SetRelativeLocationAndRotation(Location + Side + FVector(0.f, 0.f, 380.f) - Facing.Vector() * 15.f, FRotator(0.f, Yaw + 180.f, 0.f));
	Label->SetHorizontalAlignment(EHTA_Center);
	Label->SetVerticalAlignment(EVRTA_TextCenter);
	Label->SetWorldSize(90.f);
	Label->SetTextRenderColor(FColor::White);
	Label->SetText(FText::FromString(Text));
	Label->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Label->RegisterComponent();
}

UInstancedStaticMeshComponent* ANBRoute::PropLayer(int32 Kind)
{
	if (!PropMeshes.IsValidIndex(Kind) || !PropMeshes[Kind])
	{
		return nullptr;
	}
	PropLayers.SetNum(PropMeshes.Num());
	if (!PropLayers[Kind])
	{
		UInstancedStaticMeshComponent* Mesh = NewObject<UInstancedStaticMeshComponent>(this, *FString::Printf(TEXT("Prop%d"), Kind));
		Mesh->SetMobility(EComponentMobility::Movable);
		Mesh->SetupAttachment(GetRootComponent());
		Mesh->SetStaticMesh(PropMeshes[Kind]);
		Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Mesh->SetCanEverAffectNavigation(false);
		Mesh->SetCastShadow(PropKinds[Kind].bShadows);
		Mesh->RegisterComponent();
		PropLayers[Kind] = Mesh;
	}
	return PropLayers[Kind];
}

float ANBRoute::DistanceToRoad(const FVector& Location) const
{
	double Best = TNumericLimits<double>::Max();
	for (const FVector& Point : SamplePoints)
	{
		Best = FMath::Min(Best, FVector::DistSquaredXY(Point, Location));
	}
	return FMath::Sqrt(static_cast<float>(Best));
}

void ANBRoute::AddProps(const FBox2D& GroundBounds)
{
	// Same seed on every machine, like the rocks, so everyone sees the same trees.
	FRandomStream Stream(Seed + 7);
	const float HalfRoad = RoadWidth * 0.5f;
	const FBox2D Inside = GroundBounds.ExpandBy(-400.f);

	auto ClearOfSigns = [this](const FVector& Location)
	{
		return !SignSpots.ContainsByPredicate([&Location](const FVector& Spot) { return FVector2D::Distance(FVector2D(Spot), FVector2D(Location)) < Spot.Z; });
	};
	// Solid props get an invisible box in one collision layer (the Kenney meshes' own collision is
	// canopy-sized, and the visible layers stay collision-free).
	UInstancedStaticMeshComponent* Blockers = MakeLayer(TEXT("PropBlockers"), FLinearColor::White, true, false);
	Blockers->SetHiddenInGame(true);
	Blockers->SetVisibility(false);
	auto Add = [this, Blockers](int32 Kind, FVector Location, float Yaw, float Scale)
	{
		UInstancedStaticMeshComponent* Mesh = PropLayer(Kind);
		if (!Mesh)
		{
			return;
		}
		// Dropped onto the ground, sunk a little so no base floats on a slope. Trees and posts stay
		// upright; rocks, stumps and logs lean with the ground.
		Location.Z = GroundZ(Location.X, Location.Y) - (bUseTerrain ? 10.f : 0.f);
		FQuat Rotation = FRotator(0.f, Yaw, 0.f).Quaternion();
		const TCHAR* Path = PropKinds[Kind].Path;
		if (bUseTerrain && (FCString::Strstr(Path, TEXT("/rock_")) || FCString::Strstr(Path, TEXT("/stump_")) || FCString::Strstr(Path, TEXT("/log/"))))
		{
			Rotation = FQuat::FindBetweenNormals(FVector::UpVector, GroundNormal(Location.X, Location.Y)) * Rotation;
		}
		const FTransform Transform(Rotation, Location, FVector(Scale));
		Mesh->AddInstance(Transform);
		if (PropKinds[Kind].Block > 0.f)
		{
			FVector Centre, Extent;
			ANBProp::BlockerBox(PropMeshes[Kind], PropKinds[Kind].Block, PropKinds[Kind].bBlockAtPivot, Centre, Extent);
			AddBox(Blockers, Transform.TransformPosition(Centre), Transform.Rotator(), Extent * 2.f * Scale);
		}
	};

	// Where each named section starts and ends, for the props that belong to one.
	auto SectionRange = [this](const TCHAR* Name, float& OutStart, float& OutEnd)
	{
		float Distance = 0.f;
		bool bInside = false;
		OutStart = OutEnd = -1.f;
		for (const FNBRouteSegment& Segment : Segments)
		{
			if (!Segment.Section.IsEmpty())
			{
				if (bInside)
				{
					break;
				}
				bInside = Segment.Section.Equals(Name);
				if (bInside)
				{
					OutStart = Distance;
				}
			}
			Distance += FMath::Max(Segment.Length, 10.f) * 100.f;
			if (bInside)
			{
				OutEnd = Distance;
			}
		}
	};

	// Nature both sides of the whole road, one try every 7 m per side, picked by weight.
	float TotalWeight = 0.f;
	for (const FPropKind& Kind : PropKinds)
	{
		TotalWeight += Kind.Weight;
	}
	for (float D = 0.f; D < TotalLength; D += 700.f)
	{
		for (const float Side : {-1.f, 1.f})
		{
			float Pick = Stream.FRandRange(0.f, TotalWeight);
			int32 Kind = 0;
			while (Kind < UE_ARRAY_COUNT(PropKinds) - 1 && Pick > PropKinds[Kind].Weight)
			{
				Pick -= PropKinds[Kind].Weight;
				++Kind;
			}
			const FPropKind& Prop = PropKinds[Kind];
			FVector Location;
			float Yaw;
			SampleAt(D + Stream.FRandRange(-300.f, 300.f), Location, Yaw);
			const float Band = Stream.FRandRange(Prop.BandMin, Prop.BandMax);
			Location += RightOf(Yaw) * Side * (HalfRoad + Band);
			// Never on the road, including the other legs of the hairpin and wiggles; on terrain, never in a river
			// or on a cliff face.
			if (Prop.Weight <= 0.f || DistanceToRoad(Location) < HalfRoad + Prop.BandMin * 0.7f
				|| !Inside.IsInside(FVector2D(Location)) || !ClearOfSigns(Location))
			{
				continue;
			}
			if (bUseTerrain)
			{
				const FVector2D Spot(Location);
				const bool bWet = Rivers.ContainsByPredicate([this, &Spot](const FRiver& River)
				{
					return RiverDistance(River, Spot, nullptr, nullptr, nullptr) < RiverHalfWidth + RiverBank + 300.f;
				});
				if (bWet || GroundNormal(Location.X, Location.Y).Z < 0.8f)
				{
					continue;
				}
			}
			Add(Kind, Location, Stream.FRandRange(0.f, 360.f), Stream.FRandRange(Prop.ScaleMin, Prop.ScaleMax));
		}
	}

	// THE PARK: a picket fence and street lamps either side.
	float Start, End;
	SectionRange(TEXT("THE PARK"), Start, End);
	if (End > Start)
	{
		for (float D = Start; D < End; D += 300.f)
		{
			FVector Location;
			float Yaw;
			SampleAt(D, Location, Yaw);
			for (const float Side : {-1.f, 1.f})
			{
				const FVector Spot = Location + RightOf(Yaw) * Side * (HalfRoad + 300.f);
				if (ClearOfSigns(Spot))
				{
					Add(FenceKind, Spot, Yaw, PropKinds[FenceKind].ScaleMin);
				}
			}
		}
	}

	// Street lamps along THE PARK and the HOME STRETCH, arms over the road.
	for (const TCHAR* Section : {TEXT("THE PARK"), TEXT("HOME STRETCH")})
	{
		SectionRange(Section, Start, End);
		for (float D = Start + 1000.f; D < End; D += 3000.f)
		{
			FVector Location;
			float Yaw;
			SampleAt(D, Location, Yaw);
			for (const float Side : {-1.f, 1.f})
			{
				const FVector Spot = Location + RightOf(Yaw) * Side * (HalfRoad + 150.f);
				// The lamp's arm points along its -Y: turn it to reach over the road.
				if (ClearOfSigns(Spot))
				{
					Add(LampKind, Spot, Side > 0.f ? Yaw : Yaw + 180.f, PropKinds[LampKind].ScaleMin);
				}
			}
		}
	}

	// Cones on both edges of every real corner, so the turns read from the chase cam.
	float Distance = 0.f;
	for (const FNBRouteSegment& Segment : Segments)
	{
		const float Length = FMath::Max(Segment.Length, 10.f) * 100.f;
		if (FMath::Abs(Segment.TurnDegrees) >= 35.f)
		{
			for (float D = Distance + Length * 0.25f; D < Distance + Length * 0.8f; D += 600.f)
			{
				FVector Location;
				float Yaw;
				SampleAt(D, Location, Yaw);
				for (const float Side : {-1.f, 1.f})
				{
					Add(ConeKind, Location + RightOf(Yaw) * Side * (HalfRoad + 60.f), Stream.FRandRange(0.f, 360.f), PropKinds[ConeKind].ScaleMin);
				}
			}
		}
		Distance += Length;
	}

	// Road-works at the SQUIRREL SLALOM and THE BIG JUMP: barriers and a flashing-light post each side.
	for (const TCHAR* Section : {TEXT("SQUIRREL SLALOM"), TEXT("THE BIG JUMP")})
	{
		SectionRange(Section, Start, End);
		if (End <= Start)
		{
			continue;
		}
		FVector Location;
		float Yaw;
		SampleAt(Start + 800.f, Location, Yaw);
		for (const float Side : {-1.f, 1.f})
		{
			const FVector Edge = Location + RightOf(Yaw) * Side * (HalfRoad + 200.f);
			for (int32 i = -1; i <= 1; ++i)
			{
				Add(BarrierKind, Edge + FRotator(0.f, Yaw, 0.f).Vector() * (i * 180.f), Yaw + 90.f, PropKinds[BarrierKind].ScaleMin);
			}
			Add(WorkLightKind, Edge + FRotator(0.f, Yaw, 0.f).Vector() * 400.f, Yaw, PropKinds[WorkLightKind].ScaleMin);
		}
	}
}
