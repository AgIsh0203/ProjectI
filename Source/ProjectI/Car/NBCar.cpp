// Squirrel Wheels prototype.

#include "Car/NBCar.h"

#include "ChaosVehicleWheel.h"
#include "ChaosWheeledVehicleMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Net/UnrealNetwork.h"

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
}

void ANBCar::BeginPlay()
{
	Super::BeginPlay();

	// Hop order goes round the car: wheel -> pedals -> passenger seat -> engine deck.
	Seats = {WheelSeat, PedalSeat, PassengerSeat, DeckSeat};

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
