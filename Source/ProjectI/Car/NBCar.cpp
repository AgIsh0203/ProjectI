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

	WheelSeat = CreateDefaultSubobject<UNBSeatComponent>(TEXT("WheelSeat"));
	WheelSeat->SetupAttachment(GetMesh());
	WheelSeat->Role = ENBSeatRole::Wheel;
	WheelSeat->SetRelativeLocation(FVector(10.f, -38.f, 150.f));

	PedalSeat = CreateDefaultSubobject<UNBSeatComponent>(TEXT("PedalSeat"));
	PedalSeat->SetupAttachment(GetMesh());
	PedalSeat->Role = ENBSeatRole::Pedals;
	PedalSeat->SetRelativeLocation(FVector(45.f, -38.f, 95.f));
}

void ANBCar::BeginPlay()
{
	Super::BeginPlay();

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
	for (UNBSeatComponent* Seat : {WheelSeat.Get(), PedalSeat.Get()})
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
