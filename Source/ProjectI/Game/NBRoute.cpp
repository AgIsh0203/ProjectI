// Squirrel Wheels prototype.

#include "Game/NBRoute.h"

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

namespace
{
	/** Centreline sample spacing (cm). */
	constexpr float SampleSpacing = 500.f;

	/** How many samples either side of the last answer GetDistanceAlong searches (±300 m). */
	constexpr int32 SearchWindow = 60;

	FVector RightOf(float Yaw)
	{
		return FRotator(0.f, Yaw + 90.f, 0.f).Vector();
	}
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

	// Park to depot, about 3.5 km. A tidy crew at ~12 m/s needs ~5 min; breakdowns, flips and
	// falls should push a typical run to 6-8 min (GDD: 6-10 min). Features sit on straights where
	// they need to (hills, bridge, jump).
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
	Add(TEXT("COBBLE LANE"), 160.f, 0.f, ENBRouteFeature::Bumps);
	Add(TEXT(""), 120.f, 35.f, ENBRouteFeature::Bumps);
	Add(TEXT("ACORN HILL"), 60.f, 0.f);
	Add(TEXT(""), 200.f, 0.f, ENBRouteFeature::Hill, 800.f);
	Add(TEXT(""), 60.f, 0.f);
	Add(TEXT("THE HAIRPIN"), 150.f, 170.f);
	Add(TEXT(""), 100.f, 0.f);
	Add(TEXT("THE BRIDGE"), 40.f, 0.f);
	Add(TEXT(""), 200.f, 0.f, ENBRouteFeature::Bridge, 150.f);
	Add(TEXT(""), 60.f, 0.f);
	Add(TEXT("ROCKY ROAD"), 120.f, -40.f, ENBRouteFeature::Rocks);
	Add(TEXT(""), 220.f, 0.f, ENBRouteFeature::Rocks);
	Add(TEXT("SQUIRREL SLALOM"), 260.f, 0.f, ENBRouteFeature::Slalom);
	Add(TEXT("THE BIG JUMP"), 160.f, 0.f, ENBRouteFeature::Jump, 220.f);
	Add(TEXT("WIGGLY ROAD"), 110.f, -60.f);
	Add(TEXT(""), 110.f, 60.f);
	Add(TEXT(""), 110.f, -60.f, ENBRouteFeature::Bumps);
	Add(TEXT(""), 110.f, 60.f);
	Add(TEXT("HOME STRETCH"), 180.f, -100.f);
	Add(TEXT(""), 450.f, 0.f, ENBRouteFeature::Rocks);
	Add(TEXT("THE DEPOT"), 120.f, 0.f);
}

void ANBRoute::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	BuildCentreline();
}

void ANBRoute::BeginPlay()
{
	Super::BeginPlay();

	BuildGeometry();

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
	return FTransform(FRotator(0.f, Yaw, 0.f), Location + FVector(0.f, 0.f, Lift)) * GetActorTransform();
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

	// Ground slab under the whole road (top at z = 0), walled in so nothing drives off the world.
	FBox2D Bounds(ForceInit);
	for (const FVector& Point : SamplePoints)
	{
		Bounds += FVector2D(Point);
	}
	Bounds = Bounds.ExpandBy(GroundMargin);
	const FVector2D Centre = Bounds.GetCenter();
	const FVector2D Size = Bounds.GetSize();
	AddBox(Ground, FVector(Centre, -100.f), FRotator::ZeroRotator, FVector(Size, 200.f));
	constexpr float WallHeight = 400.f, WallThick = 200.f;
	AddBox(Walls, FVector(Bounds.Min.X, Centre.Y, WallHeight * 0.5f), FRotator::ZeroRotator, FVector(WallThick, Size.Y, WallHeight));
	AddBox(Walls, FVector(Bounds.Max.X, Centre.Y, WallHeight * 0.5f), FRotator::ZeroRotator, FVector(WallThick, Size.Y, WallHeight));
	AddBox(Walls, FVector(Centre.X, Bounds.Min.Y, WallHeight * 0.5f), FRotator::ZeroRotator, FVector(Size.X, WallThick, WallHeight));
	AddBox(Walls, FVector(Centre.X, Bounds.Max.Y, WallHeight * 0.5f), FRotator::ZeroRotator, FVector(Size.X, WallThick, WallHeight));

	// Road surface (visual only; the ground slab is what the wheels touch), edge lines and a dashed centre line.
	auto AddStrip = [this](UInstancedStaticMeshComponent* Mesh, int32 A, int32 B, float Offset, float Width, float Z, float Overlap)
	{
		const FVector Start = SamplePoints[A] + RightOf(SampleYaws[A]) * Offset;
		const FVector End = SamplePoints[B] + RightOf(SampleYaws[B]) * Offset;
		const FVector Delta = End - Start;
		AddBox(Mesh, (Start + End) * 0.5f + FVector(0.f, 0.f, Z), Delta.Rotation(), FVector(Delta.Size() + Overlap, Width, 2.f));
	};
	const float EdgeOffset = RoadWidth * 0.5f - 40.f;
	for (int32 i = 0; i + 1 < SamplePoints.Num(); ++i)
	{
		AddStrip(Road, i, i + 1, 0.f, RoadWidth, 1.f, 80.f);
		AddStrip(Lines, i, i + 1, -EdgeOffset, 30.f, 2.5f, 20.f);
		AddStrip(Lines, i, i + 1, EdgeOffset, 30.f, 2.5f, 20.f);
		if (i % 2 == 0)
		{
			AddStrip(Lines, i, i + 1, 0.f, 20.f, 2.5f, -150.f);
		}
	}

	// Start line.
	{
		FVector Location;
		float Yaw;
		SampleAt(1500.f, Location, Yaw);
		AddBox(Lines, Location + FVector(0.f, 0.f, 2.5f), FRotator(0.f, Yaw, 0.f), FVector(60.f, RoadWidth, 2.f));
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
				AddBox(Markers, Location + RightOf(Yaw) * Side * (RoadWidth * 0.5f + 150.f) + FVector(0.f, 0.f, 75.f), FRotator(0.f, Yaw, 0.f), FVector(40.f, 40.f, 150.f));
			}
		}

		FRandomStream Stream(Seed + Index * 7919);
		switch (Segment.Feature)
		{
		case ENBRouteFeature::Bumps:
			// Ridges with 45-degree faces, ~14 cm tall: enough to rattle acorns loose at speed.
			for (float D = Distance + 600.f; D < Distance + Length - 300.f; D += 900.f)
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
				AddBox(Pillars, Location + RightOf(Yaw) * Side * RoadWidth * 0.22f + FVector(0.f, 0.f, 160.f), FRotator(0.f, Yaw, 0.f), FVector(120.f, 120.f, 320.f));
				Side = -Side;
			}
			break;
		}

		case ENBRouteFeature::Hill:
			AddRaised(Hills, Distance, Length, Length * 0.3f, Segment.Height, RoadWidth + 600.f, false);
			break;

		case ENBRouteFeature::Bridge:
		{
			constexpr float RampLength = 1500.f;
			AddRaised(Planks, Distance, Length, RampLength, Segment.Height, BridgeWidth, false);
			// A "river" across the ground under the deck (visual only; falling in just lands on grass).
			FVector Location;
			float Yaw;
			SampleAt(Distance + Length * 0.5f, Location, Yaw);
			UInstancedStaticMeshComponent* Water = MakeLayer(*FString::Printf(TEXT("Water%d"), Index), FLinearColor(0.1f, 0.3f, 0.7f), false, false);
			AddBox(Water, Location + FVector(0.f, 0.f, 4.f), FRotator(0.f, Yaw, 0.f), FVector(Length - RampLength * 2.f - 400.f, 8000.f, 2.f));
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

	// The depot shed past the drop-off gate.
	{
		FVector Location;
		float Yaw;
		SampleAt(TotalLength, Location, Yaw);
		AddBox(Walls, Location + FRotator(0.f, Yaw, 0.f).Vector() * 1500.f + FVector(0.f, 0.f, 400.f), FRotator(0.f, Yaw, 0.f), FVector(1200.f, 3000.f, 800.f));
	}
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
	const FVector Forward = FRotator(0.f, Yaw, 0.f).Vector();

	const float SlopeDegrees = FMath::RadiansToDegrees(FMath::Atan2(Height, RampLength));
	const float SlopeLength = FMath::Sqrt(FMath::Square(RampLength) + FMath::Square(Height));
	// Thick enough that the slab reaches below the ground under its high end.
	const float Thickness = Height / FMath::Cos(FMath::DegreesToRadians(SlopeDegrees)) + 100.f;

	// A slab whose top face runs from (Along, 0) to (Along + RampLength, Height), or back down.
	auto AddRamp = [&](float Along, bool bUp)
	{
		const FRotator Rotation(bUp ? SlopeDegrees : -SlopeDegrees, Yaw, 0.f);
		const FVector TopCentre = Origin + Forward * (Along + RampLength * 0.5f) + FVector(0.f, 0.f, Height * 0.5f);
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
		AddBox(Mesh, Origin + Forward * (RampLength + Top * 0.5f) + FVector(0.f, 0.f, Height * 0.5f), FRotator(0.f, Yaw, 0.f), FVector(Top, Width, Height));
	}
	AddRamp(RampLength + Top, false);
}

void ANBRoute::AddSign(const FString& Text, float Distance)
{
	FVector Location;
	float Yaw;
	SampleAt(Distance, Location, Yaw);
	const FVector Side = RightOf(Yaw) * (RoadWidth * 0.5f + 450.f);
	const FRotator Facing(0.f, Yaw, 0.f);

	AddBox(Walls, Location + Side + FVector(0.f, 0.f, 175.f), Facing, FVector(30.f, 30.f, 350.f));
	AddBox(Markers, Location + Side + FVector(0.f, 0.f, 380.f), Facing, FVector(20.f, 760.f, 160.f));

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
