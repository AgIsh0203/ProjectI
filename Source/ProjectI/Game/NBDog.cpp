// Squirrel Wheels prototype.

#include "Game/NBDog.h"

#include "Car/NBCar.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "EngineUtils.h"
#include "Game/NBRoute.h"
#include "Game/NBRunGameState.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "Net/UnrealNetwork.h"
#include "Parts/NBCarParts.h"
#include "Player/NBSquirrel.h"
#include "UObject/ConstructorHelpers.h"

namespace
{
	// Greybox body, in cm, facing +X with the origin on the ground between the feet.
	constexpr float HipHeight = 46.f;
	constexpr float LegLength = 46.f;
	constexpr float TailLength = 34.f;
	const FVector TailRoot(-55.f, 0.f, 78.f);

	bool IsBlackPart(const UStaticMeshComponent* Part)
	{
		const FString Name = Part->GetName();
		return Name.StartsWith(TEXT("Nose")) || Name.StartsWith(TEXT("Eye"));
	}
	const FVector HipSpots[4] = {
		FVector(40.f, -16.f, HipHeight), FVector(40.f, 16.f, HipHeight),
		FVector(-40.f, -16.f, HipHeight), FVector(-40.f, 16.f, HipHeight)};
}

ANBDog::ANBDog()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	SetReplicateMovement(true);
	bAlwaysRelevant = true;
	SetNetUpdateFrequency(30.f);

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);
	VisualRoot = CreateDefaultSubobject<USceneComponent>(TEXT("VisualRoot"));
	VisualRoot->SetupAttachment(Root);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderMesh(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	UStaticMesh* Cube = CubeMesh.Object;
	UStaticMesh* Cylinder = CylinderMesh.Object;
	UStaticMesh* Sphere = SphereMesh.Object;

	Body = MakePart(TEXT("Body"), VisualRoot, Cube, FVector(0.f, 0.f, 64.f), FVector(1.1f, 0.46f, 0.4f));
	HeadRoot = CreateDefaultSubobject<USceneComponent>(TEXT("HeadRoot"));
	HeadRoot->SetupAttachment(VisualRoot);
	HeadRoot->SetRelativeLocation(FVector(62.f, 0.f, 88.f));
	MakePart(TEXT("Head"), HeadRoot, Cube, FVector::ZeroVector, FVector(0.38f, 0.36f, 0.32f));
	MakePart(TEXT("Snout"), HeadRoot, Cube, FVector(24.f, 0.f, -7.f), FVector(0.2f, 0.22f, 0.15f));
	MakePart(TEXT("EarL"), HeadRoot, Cube, FVector(-6.f, -15.f, 18.f), FVector(0.08f, 0.1f, 0.18f));
	MakePart(TEXT("EarR"), HeadRoot, Cube, FVector(-6.f, 15.f, 18.f), FVector(0.08f, 0.1f, 0.18f));
	Tail = MakePart(TEXT("Tail"), VisualRoot, Cylinder, TailRoot, FVector(0.07f, 0.07f, TailLength / 100.f));
	for (int32 i = 0; i < 4; ++i)
	{
		Legs.Add(MakePart(*FString::Printf(TEXT("Leg%d"), i), VisualRoot, Cylinder,
			HipSpots[i] - FVector(0.f, 0.f, LegLength * 0.5f), FVector(0.11f, 0.11f, LegLength / 100.f)));
	}
	// Black, not fur: BeginPlay tints by name.
	MakePart(TEXT("Nose"), HeadRoot, Sphere, FVector(35.f, 0.f, -3.f), FVector(0.08f));
	MakePart(TEXT("EyeL"), HeadRoot, Sphere, FVector(19.f, -9.f, 6.f), FVector(0.06f));
	MakePart(TEXT("EyeR"), HeadRoot, Sphere, FVector(19.f, 9.f, 6.f), FVector(0.06f));
}

UStaticMeshComponent* ANBDog::MakePart(FName Name, USceneComponent* Parent, UStaticMesh* Mesh, const FVector& Location, const FVector& Scale)
{
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> ShapeMaterial(TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
	UStaticMeshComponent* Part = CreateDefaultSubobject<UStaticMeshComponent>(Name);
	Part->SetupAttachment(Parent);
	Part->SetStaticMesh(Mesh);
	Part->SetMaterial(0, ShapeMaterial.Object);
	Part->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Part->SetGenerateOverlapEvents(false);
	Part->SetRelativeLocation(Location);
	Part->SetRelativeScale3D(Scale);
	return Part;
}

void ANBDog::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ANBDog, State);
	DOREPLIFETIME(ANBDog, BiteTire);
}

void ANBDog::BeginPlay()
{
	Super::BeginPlay();
	if (GetNetMode() != NM_DedicatedServer)
	{
		TArray<UStaticMeshComponent*> Parts;
		GetComponents(Parts);
		for (UStaticMeshComponent* Part : Parts)
		{
			if (UMaterialInstanceDynamic* Material = Part->CreateDynamicMaterialInstance(0))
			{
				Material->SetVectorParameterValue(TEXT("Color"), IsBlackPart(Part) ? FLinearColor(0.02f, 0.02f, 0.02f) : FurColor);
			}
		}
	}
	for (const UStaticMeshComponent* Leg : Legs)
	{
		LegRest.Add(Leg->GetRelativeTransform());
	}
}

void ANBDog::SetHome(const FTransform& Home, float ZoneStart, float ZoneEnd)
{
	HomeTransform = Home;
	ZoneStartDistance = ZoneStart;
	ZoneEndDistance = ZoneEnd;
	FVector Location = Home.GetLocation();
	SnapToGround(Location);
	HomeTransform.SetLocation(Location);
	SetActorTransform(HomeTransform);
}

void ANBDog::DevChase()
{
	bDevChase = true;
	UE_LOG(LogTemp, Log, TEXT("NBDog: dev chase on"));
}

FVector ANBDog::GetBubbleLocation() const
{
	return HeadRoot->GetComponentLocation() + FVector(0.f, 0.f, 70.f);
}

void ANBDog::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (HasAuthority())
	{
		UpdateServer(DeltaSeconds);
	}
	if (GetNetMode() != NM_DedicatedServer)
	{
		UpdateVisuals(DeltaSeconds);
	}
}

// ---------------------------------------------------------------------------------------------
// Server

ANBCar* ANBDog::FindCar()
{
	if (!Car.IsValid())
	{
		for (TActorIterator<ANBCar> It(GetWorld()); It; ++It)
		{
			Car = *It;
			break;
		}
	}
	return Car.Get();
}

bool ANBDog::IsCarInZone(const ANBCar& TheCar) const
{
	const ANBRunGameState* RunState = GetWorld()->GetGameState<ANBRunGameState>();
	if (!RunState || RunState->GetPhase() != ENBRunPhase::Driving)
	{
		return false;
	}
	if (bDevChase)
	{
		return true;
	}
	const ANBRoute* Route = ANBRoute::Find(GetWorld());
	if (!Route || ZoneEndDistance < 0.f)
	{
		return false;
	}
	const float Distance = Route->GetDistanceAlong(TheCar.GetActorLocation());
	return Distance >= ZoneStartDistance - ZoneMargin && Distance <= ZoneEndDistance;
}

void ANBDog::SetState(ENBDogState NewState)
{
	if (State != NewState)
	{
		UE_LOG(LogTemp, Log, TEXT("NBDog: %s -> %s"), *UEnum::GetValueAsString(State), *UEnum::GetValueAsString(NewState));
		State = NewState;
	}
}

void ANBDog::SnapToGround(FVector& Location) const
{
	FCollisionObjectQueryParams Objects(ECC_WorldStatic);
	FCollisionQueryParams Params(SCENE_QUERY_STAT(NBDogGround), false, this);
	if (Car.IsValid())
	{
		Params.AddIgnoredActor(Car.Get());
	}
	FHitResult Hit;
	if (GetWorld()->LineTraceSingleByObjectType(Hit, Location + FVector(0.f, 0.f, 600.f), Location - FVector(0.f, 0.f, 5000.f), Objects, Params))
	{
		Location.Z = Hit.ImpactPoint.Z;
	}
}

float ANBDog::MoveTowards(const FVector& Target, float Speed, float DeltaSeconds)
{
	FVector Location = GetActorLocation();
	FVector Delta = Target - Location;
	Delta.Z = 0.f;
	const float Distance = Delta.Size();
	float Step = 0.f;
	FRotator Rotation = GetActorRotation();
	if (Distance > 1.f)
	{
		Step = FMath::Min(Distance, Speed * DeltaSeconds);
		Location += Delta / Distance * Step;
		Rotation = FMath::RInterpTo(Rotation, FRotator(0.f, Delta.Rotation().Yaw, 0.f), DeltaSeconds, 10.f);
	}
	SnapToGround(Location);
	SetActorLocationAndRotation(Location, Rotation);
	return Distance - Step;
}

UNBTirePart* ANBDog::PickTire(const ANBCar& TheCar) const
{
	UNBTirePart* Best = nullptr;
	float BestScore = TNumericLimits<float>::Max();
	for (UNBCarPartComponent* Part : TheCar.GetParts())
	{
		UNBTirePart* Tire = Cast<UNBTirePart>(Part);
		if (!Tire)
		{
			continue;
		}
		// Nearest, but a tyre that's already flat only if there's nothing else.
		const float Score = FVector::Dist2D(Tire->GetComponentLocation(), GetActorLocation()) + (Tire->IsFailed() ? 100000.f : 0.f);
		if (Score < BestScore)
		{
			BestScore = Score;
			Best = Tire;
		}
	}
	return Best;
}

FVector ANBDog::TireSpot(const ANBCar& TheCar, const UNBTirePart& Tire) const
{
	const FVector Right = TheCar.GetActorRightVector();
	const FVector TireLocation = Tire.GetComponentLocation();
	const float Side = FVector::DotProduct(TireLocation - TheCar.GetActorLocation(), Right) >= 0.f ? 1.f : -1.f;
	return TireLocation + Right * Side * 75.f;
}

ANBSquirrel* ANBDog::FindSquirrelOnFoot() const
{
	ANBSquirrel* Best = nullptr;
	float BestDistance = SquirrelInterest;
	for (TActorIterator<ANBSquirrel> It(GetWorld()); It; ++It)
	{
		if (It->IsSeated() || It->IsRagdolled() || It->IsClinging())
		{
			continue;
		}
		const float Distance = FVector::Dist2D(It->GetActorLocation(), GetActorLocation());
		if (Distance < BestDistance)
		{
			BestDistance = Distance;
			Best = *It;
		}
	}
	return Best;
}

void ANBDog::UpdateServer(float DeltaSeconds)
{
	ANBCar* TheCar = FindCar();
	const double Now = GetWorld()->GetTimeSeconds();
	const bool bInZone = TheCar && IsCarInZone(*TheCar);

	// The horn: the car stamps its local time on every machine, the server included.
	if (TheCar && TheCar->GetLastHornTime() > LastHornSeen)
	{
		LastHornSeen = TheCar->GetLastHornTime();
		if ((State == ENBDogState::Chasing || State == ENBDogState::Biting)
			&& FVector::Dist(TheCar->GetActorLocation(), GetActorLocation()) < ScareRadius)
		{
			BiteTire = nullptr;
			StateEndTime = Now + ScaredSeconds;
			SetState(ENBDogState::Scared);
			Multicast_Sound(static_cast<uint8>(EDogSound::Yelp));
		}
	}

	switch (State)
	{
	case ENBDogState::Sleeping:
		if (bInZone)
		{
			SetState(ENBDogState::Chasing);
			NextBarkTime = Now;
		}
		break;

	case ENBDogState::GoingHome:
		if (bInZone)
		{
			SetState(ENBDogState::Chasing);
		}
		else if (MoveTowards(HomeTransform.GetLocation(), TrotSpeed, DeltaSeconds) < 30.f)
		{
			SetActorTransform(HomeTransform);
			SetState(ENBDogState::Sleeping);
		}
		break;

	case ENBDogState::Chasing:
	{
		if (!bInZone)
		{
			SetState(ENBDogState::GoingHome);
			break;
		}
		if (Now >= NextBarkTime)
		{
			NextBarkTime = Now + FMath::FRandRange(BarkInterval.X, BarkInterval.Y);
			Multicast_Sound(static_cast<uint8>(EDogSound::Bark));
		}
		// A squirrel on foot nearby beats the car (not straight after a bite or grab).
		if (ANBSquirrel* Squirrel = Now >= NextBiteTime ? FindSquirrelOnFoot() : nullptr)
		{
			if (MoveTowards(Squirrel->GetActorLocation(), RunSpeed, DeltaSeconds) < GrabRange)
			{
				const FVector Fling = GetActorForwardVector() * FlingVelocity.X + FVector(0.f, 0.f, FlingVelocity.Y);
				Squirrel->Eject(Fling, ENBEjectReason::Dog);
				NextBiteTime = Now + BiteCooldown;
				Multicast_Sound(static_cast<uint8>(EDogSound::Bite));
				UE_LOG(LogTemp, Log, TEXT("NBDog: flings %s"), *Squirrel->GetName());
			}
			break;
		}
		UNBTirePart* Tire = PickTire(*TheCar);
		if (!Tire)
		{
			break;
		}
		if (MoveTowards(TireSpot(*TheCar, *Tire), RunSpeed, DeltaSeconds) < BiteRange && Now >= NextBiteTime)
		{
			BiteTire = Tire;
			StateEndTime = Now + BiteSeconds;
			SetState(ENBDogState::Biting);
			Multicast_Sound(static_cast<uint8>(EDogSound::Bite));
			UE_LOG(LogTemp, Log, TEXT("NBDog: bites %s"), *Tire->GetName());
		}
		break;
	}

	case ENBDogState::Biting:
	{
		if (!BiteTire || !TheCar)
		{
			BiteTire = nullptr;
			SetState(ENBDogState::Chasing);
			break;
		}
		// Hangs on: dragged along beside the wheel, facing it.
		FVector Location = TireSpot(*TheCar, *BiteTire);
		SnapToGround(Location);
		const FVector ToTire = BiteTire->GetComponentLocation() - Location;
		SetActorLocationAndRotation(Location, FRotator(0.f, ToTire.Rotation().Yaw, 0.f));
		BiteTire->AddWear(BiteWearRate * DeltaSeconds);
		if (Now >= StateEndTime)
		{
			const bool bFlat = !BiteTire->IsFailed() && FMath::FRand() < FlatChance;
			if (bFlat)
			{
				BiteTire->Fail();
			}
			UE_LOG(LogTemp, Log, TEXT("NBDog: lets go of %s (%s)"), *BiteTire->GetName(), bFlat ? TEXT("flat") : TEXT("worn"));
			BiteTire = nullptr;
			NextBiteTime = Now + BiteCooldown;
			SetState(ENBDogState::Chasing);
		}
		break;
	}

	case ENBDogState::Scared:
	{
		const FVector Away = TheCar ? (GetActorLocation() - TheCar->GetActorLocation()).GetSafeNormal2D() : GetActorForwardVector();
		MoveTowards(GetActorLocation() + Away * 1000.f, RunSpeed * 0.8f, DeltaSeconds);
		if (Now >= StateEndTime)
		{
			SetState(bInZone ? ENBDogState::Chasing : ENBDogState::GoingHome);
		}
		break;
	}
	}
}

void ANBDog::Multicast_Sound_Implementation(uint8 Sound)
{
	switch (static_cast<EDogSound>(Sound))
	{
	case EDogSound::Bark:
		LastBarkTime = GetWorld()->GetTimeSeconds();
		NBFeedback::PlayAttached(BarkFeedback, HeadRoot);
		break;
	case EDogSound::Bite:
		NBFeedback::PlayAttached(BiteFeedback, HeadRoot);
		break;
	case EDogSound::Yelp:
		NBFeedback::PlayAttached(YelpFeedback, HeadRoot);
		break;
	}
}

// ---------------------------------------------------------------------------------------------
// Every machine: smoothing and a code-driven trot.

void ANBDog::UpdateVisuals(float DeltaSeconds)
{
	const FVector Target = GetActorLocation();
	const FRotator TargetRotation = GetActorRotation();
	if (!bVisualsPlaced || HasAuthority() || FVector::DistSquared(SmoothedLocation, Target) > FMath::Square(1500.f))
	{
		// The server moves the actor every tick; a client eases after its ~30 Hz updates.
		SmoothedLocation = Target;
		SmoothedRotation = TargetRotation;
		bVisualsPlaced = true;
	}
	else
	{
		SmoothedLocation = FMath::VInterpTo(SmoothedLocation, Target, DeltaSeconds, 15.f);
		SmoothedRotation = FMath::RInterpTo(SmoothedRotation, TargetRotation, DeltaSeconds, 12.f);
	}

	const float Measured = DeltaSeconds > 0.f ? FVector::Dist2D(SmoothedLocation, LastVisualLocation) / DeltaSeconds : 0.f;
	LastVisualLocation = SmoothedLocation;
	VisualSpeed = FMath::FInterpTo(VisualSpeed, FMath::Min(Measured, 3000.f), DeltaSeconds, 8.f);

	const float Time = GetWorld()->GetTimeSeconds();
	const bool bSleeping = State == ENBDogState::Sleeping;
	const bool bBiting = State == ENBDogState::Biting;
	const float Amp = bBiting ? 0.6f : FMath::Clamp(VisualSpeed / 900.f, 0.f, 1.f);
	GaitPhase += DeltaSeconds * (bBiting ? 14.f : VisualSpeed / 18.f);

	// Whole-body pose: bob while running, pull back while biting, lie down asleep.
	FRotator Pose = SmoothedRotation;
	FVector Offset = FVector(0.f, 0.f, FMath::Abs(FMath::Sin(GaitPhase)) * 6.f * Amp);
	if (bBiting)
	{
		Pose.Pitch += 12.f;
		Pose.Roll += FMath::Sin(Time * 28.f) * 8.f;
	}
	if (bSleeping)
	{
		Offset = FVector(0.f, 0.f, -36.f);
	}
	VisualRoot->SetWorldLocationAndRotation(SmoothedLocation + Offset, Pose);

	// Legs swing about the hip; diagonal pairs move together. Tucked flat asleep.
	for (int32 i = 0; i < Legs.Num() && i < LegRest.Num(); ++i)
	{
		const float Swing = bSleeping ? 80.f : FMath::Sin(GaitPhase + ((i == 0 || i == 3) ? 0.f : PI)) * 40.f * Amp;
		const FQuat Rotation = FRotator(Swing, 0.f, 0.f).Quaternion();
		Legs[i]->SetRelativeLocationAndRotation(HipSpots[i] + Rotation.RotateVector(FVector(0.f, 0.f, -LegLength * 0.5f)), Rotation);
	}

	// Head: shakes the tyre while biting, droops asleep, barks with a jerk.
	const float BarkAge = Time - LastBarkTime;
	const float BarkJerk = BarkAge < 0.25f ? FMath::Sin(BarkAge / 0.25f * PI) * 18.f : 0.f;
	FRotator Head(BarkJerk, 0.f, 0.f);
	if (bBiting)
	{
		Head = FRotator(-20.f, FMath::Sin(Time * 30.f) * 25.f, 0.f);
	}
	else if (bSleeping)
	{
		Head = FRotator(-25.f, 20.f, 0.f);
	}
	HeadRoot->SetRelativeRotation(Head);

	// Tail: wags hard when chasing, tucked when scared.
	const bool bScared = State == ENBDogState::Scared;
	const float Wag = bScared || bSleeping ? 0.f : FMath::Sin(Time * (State == ENBDogState::Chasing ? 18.f : 8.f)) * 35.f;
	// Tilted back from the rump (up and back, or down between the legs when scared), then wagged side to side.
	const FQuat TailRotation = FRotator(bScared ? 150.f : 40.f, Wag, 0.f).Quaternion();
	Tail->SetRelativeLocationAndRotation(TailRoot + TailRotation.RotateVector(FVector(0.f, 0.f, TailLength * 0.5f)), TailRotation);
}
