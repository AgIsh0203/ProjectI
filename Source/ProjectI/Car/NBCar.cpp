// Squirrel Wheels prototype.

#include "Car/NBCar.h"

#include "ChaosVehicleWheel.h"
#include "ChaosWheeledVehicleMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "Net/UnrealNetwork.h"
#include "Parts/NBCarParts.h"
#include "TimerManager.h"
#include "UObject/ConstructorHelpers.h"

ANBCar::ANBCar()
{
	bReplicates = true;
	SetReplicatingMovement(true);
	// Clients interpolate toward the server's car instead of simulating it themselves.
	SetPhysicsReplicationMode(EPhysicsReplicationMode::PredictiveInterpolation);

	// The car is never possessed; the server feeds inputs from seated squirrels.
	GetChaosVehicleMovement()->SetRequiresControllerForInputs(false);
	// Chaos reverses when brake is held at a stop.
	GetChaosVehicleMovement()->bReverseAsBrake = true;

	// Flipping back is a 2-squirrel job in this game, so disable the template's auto-reset.
	FlipCheckMinDot = -2.f;

	// Seat locations are the squirrel's capsule centre (half-height 22), measured off
	// SM_Offroad_Body: steering wheel ring ~(105, -37, 90..122), pedals ~(115, -37, 60),
	// seat cushions ~(30, +-35, 74), engine deck top ~z 148 over x -60..-110.

	// Perched on top of the steering wheel rim.
	WheelSeat = CreateDefaultSubobject<UNBSeatComponent>(TEXT("WheelSeat"));
	WheelSeat->SetupAttachment(GetMesh());
	WheelSeat->Role = ENBSeatRole::Wheel;
	WheelSeat->SetRelativeLocation(FVector(98.f, -37.f, 142.f));

	// Down in the driver's footwell, on the pedals.
	PedalSeat = CreateDefaultSubobject<UNBSeatComponent>(TEXT("PedalSeat"));
	PedalSeat->SetupAttachment(GetMesh());
	PedalSeat->Role = ENBSeatRole::Pedals;
	PedalSeat->SetRelativeLocation(FVector(114.f, -37.f, 84.f));

	PassengerSeat = CreateDefaultSubobject<UNBSeatComponent>(TEXT("PassengerSeat"));
	PassengerSeat->SetupAttachment(GetMesh());
	PassengerSeat->Role = ENBSeatRole::Rider;
	PassengerSeat->SetRelativeLocation(FVector(28.f, 35.f, 96.f));

	// On the engine deck behind the seats.
	DeckSeat = CreateDefaultSubobject<UNBSeatComponent>(TEXT("DeckSeat"));
	DeckSeat->SetupAttachment(GetMesh());
	DeckSeat->Role = ENBSeatRole::Rider;
	DeckSeat->SetRelativeLocation(FVector(-85.f, 0.f, 172.f));

	// On the front hood (surface ~z 110 at x 160).
	HoodSeat = CreateDefaultSubobject<UNBSeatComponent>(TEXT("HoodSeat"));
	HoodSeat->SetupAttachment(GetMesh());
	HoodSeat->Role = ENBSeatRole::Rider;
	HoodSeat->SetRelativeLocation(FVector(160.f, 0.f, 132.f));

	// Parts sit where they're repaired from. Ranges are chosen so each in-car part is
	// reachable from one rider seat only (engine: deck, brakes: hood, door: passenger),
	// never from the wheel or pedals: fixing things means leaving the controls.
	EnginePart = CreateDefaultSubobject<UNBEnginePart>(TEXT("EnginePart"));
	EnginePart->SetupAttachment(GetMesh());
	EnginePart->SetRelativeLocation(FVector(-100.f, 0.f, 150.f));

	BrakePart = CreateDefaultSubobject<UNBBrakePart>(TEXT("BrakePart"));
	BrakePart->SetupAttachment(GetMesh());
	BrakePart->SetRelativeLocation(FVector(180.f, 0.f, 112.f));
	BrakePart->Range = 60.f;

	// Tire spots hang just outside each wheel; the squirrel clings there while patching.
	TireFL = CreateTire(TEXT("TireFL"), TEXT("PhysWheel_FL"), FVector(168.f, -175.f, 51.f), NSLOCTEXT("NB", "TireFL", "TIRE FL"));
	TireFR = CreateTire(TEXT("TireFR"), TEXT("PhysWheel_FR"), FVector(168.f, 175.f, 51.f), NSLOCTEXT("NB", "TireFR", "TIRE FR"));
	TireBL = CreateTire(TEXT("TireBL"), TEXT("PhysWheel_BL"), FVector(-135.f, -190.f, 51.f), NSLOCTEXT("NB", "TireBL", "TIRE BL"));
	TireBR = CreateTire(TEXT("TireBR"), TEXT("PhysWheel_BR"), FVector(-135.f, 190.f, 51.f), NSLOCTEXT("NB", "TireBR", "TIRE BR"));

	// Greybox door over the passenger-side opening (x 0..60). The hinge is its front edge.
	DoorHinge = CreateDefaultSubobject<USceneComponent>(TEXT("DoorHinge"));
	DoorHinge->SetupAttachment(GetMesh());
	DoorHinge->SetRelativeLocation(FVector(60.f, 90.f, 100.f));

	DoorPanel = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DoorPanel"));
	DoorPanel->SetupAttachment(DoorHinge);
	DoorPanel->SetRelativeLocation(FVector(-30.f, 0.f, 0.f));
	DoorPanel->SetRelativeScale3D(FVector(0.6f, 0.04f, 0.5f));
	DoorPanel->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> ShapeMaterial(TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
	if (CubeMesh.Succeeded())
	{
		DoorPanel->SetStaticMesh(CubeMesh.Object);
	}
	if (ShapeMaterial.Succeeded())
	{
		DoorPanel->SetMaterial(0, ShapeMaterial.Object);
	}

	DoorPart = CreateDefaultSubobject<UNBDoorPart>(TEXT("DoorPart"));
	DoorPart->SetupAttachment(GetMesh());
	DoorPart->SetRelativeLocation(FVector(30.f, 95.f, 100.f));
	DoorPart->Range = 130.f;
}

UNBTirePart* ANBCar::CreateTire(FName Name, FName WheelBone, const FVector& Location, const FText& PartName)
{
	UNBTirePart* Tire = CreateDefaultSubobject<UNBTirePart>(Name);
	Tire->SetupAttachment(GetMesh());
	Tire->SetRelativeLocation(Location);
	Tire->WheelBone = WheelBone;
	Tire->PartName = PartName;
	return Tire;
}

void ANBCar::BeginPlay()
{
	Super::BeginPlay();

	// Hop order goes round the car: wheel -> pedals -> hood -> passenger seat -> engine deck.
	Seats = {WheelSeat, PedalSeat, HoodSeat, PassengerSeat, DeckSeat};
	Parts = {EnginePart, BrakePart, TireFL, TireFR, TireBL, TireBR, DoorPart};
	DoorPart->Hinge = DoorHinge;

	UChaosWheeledVehicleMovementComponent* Movement = GetChaosVehicleMovement();
	// Only the server consumes raw inputs. Clients, still requiring a controller they
	// don't have, fall back to Chaos's ReplicatedState (the server's live steering,
	// throttle, brake and gear), so their wheels steer/spin in step with the
	// interpolated body instead of fighting it with zero input.
	Movement->SetRequiresControllerForInputs(!HasAuthority());

	BaseMaxEngineTorque = Movement->EngineSetup.MaxTorque;
	BaseBrakeTorque.Reset();
	BaseFriction.Reset();
	for (const UChaosVehicleWheel* Wheel : Movement->Wheels)
	{
		BaseBrakeTorque.Add(Wheel ? Wheel->MaxBrakeTorque : 0.f);
		BaseFriction.Add(Wheel ? Wheel->FrictionForceMultiplier : 1.f);
	}
}

void ANBCar::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ANBCar, SteerInput);
	DOREPLIFETIME(ANBCar, ThrottleInput);
}

void ANBCar::SetSeatInput(ENBSeatRole SeatRole, float Value)
{
	check(HasAuthority());
	Value = FMath::Clamp(Value, -1.f, 1.f);
	switch (SeatRole)
	{
	case ENBSeatRole::Wheel:
		SteerInput = Value;
		break;
	case ENBSeatRole::Pedals:
		ThrottleInput = Value;
		break;
	case ENBSeatRole::Rider:
		return;
	}
	ApplyInputsToVehicle();
}

void ANBCar::ApplyInputsToVehicle()
{
	UChaosWheeledVehicleMovementComponent* Movement = GetChaosVehicleMovement();
	Movement->SetSteeringInput(SteerInput);
	Movement->SetThrottleInput(FMath::Max(ThrottleInput, 0.f));
	Movement->SetBrakeInput(FMath::Max(-ThrottleInput, 0.f));
}

UNBSeatComponent* ANBCar::FindNearestFreeSeat(const FVector& Location, float MaxDistance) const
{
	UNBSeatComponent* Best = nullptr;
	float BestDistSq = FMath::Square(MaxDistance);
	for (UNBSeatComponent* Seat : Seats)
	{
		if (!Seat || !Seat->IsFree())
		{
			continue;
		}
		const float DistSq = FVector::DistSquared(Seat->GetComponentLocation(), Location);
		if (DistSq < BestDistSq)
		{
			BestDistSq = DistSq;
			Best = Seat;
		}
	}
	return Best;
}

UNBSeatComponent* ANBCar::FindNextFreeSeat(const UNBSeatComponent* From) const
{
	const int32 Start = Seats.IndexOfByKey(From);
	for (int32 Step = 1; Step < Seats.Num(); ++Step)
	{
		UNBSeatComponent* Seat = Seats[(Start + Step + Seats.Num()) % Seats.Num()];
		if (Seat && Seat->IsFree())
		{
			return Seat;
		}
	}
	return nullptr;
}

void ANBCar::DevFail(const FString& PartName)
{
	check(HasAuthority());
	if (PartName.Equals(TEXT("All"), ESearchCase::IgnoreCase))
	{
		for (UNBCarPartComponent* Part : Parts)
		{
			Part->Fail();
		}
		return;
	}
	if (PartName.Equals(TEXT("Tire"), ESearchCase::IgnoreCase))
	{
		UNBTirePart* Tires[] = {TireFL, TireFR, TireBL, TireBR};
		Tires[FMath::RandRange(0, 3)]->Fail();
		return;
	}
	for (UNBCarPartComponent* Part : Parts)
	{
		// Component names are EnginePart, BrakePart, TireFL, ..., DoorPart.
		const FString Name = Part->GetName();
		if (Name.Equals(PartName, ESearchCase::IgnoreCase) || Name.Equals(PartName + TEXT("Part"), ESearchCase::IgnoreCase)
			|| (PartName.Equals(TEXT("Brakes"), ESearchCase::IgnoreCase) && Part == BrakePart))
		{
			Part->Fail();
			return;
		}
	}
	UE_LOG(LogTemp, Warning, TEXT("NBFail: unknown part '%s'"), *PartName);
}

void ANBCar::SetDevChaos(bool bEnable)
{
	check(HasAuthority());
	GetWorldTimerManager().ClearTimer(DevChaosTimer);
	if (bEnable)
	{
		GetWorldTimerManager().SetTimer(DevChaosTimer, this, &ANBCar::DevFailRandomPart, DevChaosInterval, true);
	}
	UE_LOG(LogTemp, Log, TEXT("NBChaos %s"), bEnable ? TEXT("on") : TEXT("off"));
}

void ANBCar::DevFailRandomPart()
{
	TArray<UNBCarPartComponent*> Healthy;
	for (UNBCarPartComponent* Part : Parts)
	{
		if (!Part->IsFailed())
		{
			Healthy.Add(Part);
		}
	}
	if (Healthy.Num() > 0)
	{
		Healthy[FMath::RandRange(0, Healthy.Num() - 1)]->Fail();
	}
}

float ANBCar::GetForwardSpeed() const
{
	return GetChaosVehicleMovement()->GetForwardSpeed();
}

void ANBCar::SetEnginePowerScale(float Scale)
{
	GetChaosVehicleMovement()->SetMaxEngineTorque(BaseMaxEngineTorque * FMath::Max(Scale, 0.f));
}

void ANBCar::SetBrakePowerScale(float Scale)
{
	for (int32 i = 0; i < BaseBrakeTorque.Num(); ++i)
	{
		GetChaosVehicleMovement()->SetBrakeTorque(BaseBrakeTorque[i] * FMath::Max(Scale, 0.f), i);
	}
}

void ANBCar::SetWheelGripScale(int32 WheelIndex, float Scale)
{
	if (BaseFriction.IsValidIndex(WheelIndex))
	{
		GetChaosVehicleMovement()->SetWheelFrictionMultiplier(WheelIndex, BaseFriction[WheelIndex] * FMath::Max(Scale, 0.f));
	}
}
