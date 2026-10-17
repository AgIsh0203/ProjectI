// Squirrel Wheels prototype.

#include "Parts/NBCarParts.h"

#include "Car/NBCar.h"
#include "Car/NBSeatComponent.h"
#include "ChaosWheeledVehicleMovementComponent.h"
#include "Player/NBSquirrel.h"

// --- Engine ---

UNBEnginePart::UNBEnginePart()
{
	Mode = ENBInteractMode::Mash;
	PartName = NSLOCTEXT("NB", "EngineName", "ENGINE");
	FailureText = NSLOCTEXT("NB", "EngineFail", "OVERHEATING");
	CalloutText = NSLOCTEXT("NB", "EngineCallout", "ENGINE ON FIRE!");
	Prompt = NSLOCTEXT("NB", "EnginePrompt", "Cool engine");
	Range = 90.f;
	// The fire/smoke loop peaks as the engine stalls.
	SeverityRampSeconds = StallSeconds;
}

void UNBEnginePart::ApplyFailedEffect(float FailedSeconds)
{
	const float Power = FailedSeconds >= StallSeconds
		? 0.f
		: FMath::Lerp(1.f, MinPowerBeforeStall, FMath::Clamp(FailedSeconds / FadeSeconds, 0.f, 1.f));
	GetCar()->SetEnginePowerScale(Power);
}

void UNBEnginePart::ClearEffect()
{
	GetCar()->SetEnginePowerScale(1.f);
}

// --- Brakes ---

UNBBrakePart::UNBBrakePart()
{
	Mode = ENBInteractMode::TimingRing;
	PartName = NSLOCTEXT("NB", "BrakesName", "BRAKES");
	FailureText = NSLOCTEXT("NB", "BrakesFail", "FADING");
	CalloutText = NSLOCTEXT("NB", "BrakesCallout", "THE BRAKES ARE GOING!");
	Prompt = NSLOCTEXT("NB", "BrakesPrompt", "Pump brakes");
	Range = 70.f;
	SeverityRampSeconds = GoneSeconds;
}

void UNBBrakePart::ApplyFailedEffect(float FailedSeconds)
{
	ANBCar* Car = GetCar();
	if (FailedSeconds >= NextGrabAt)
	{
		GrabUntil = FailedSeconds + GrabSeconds;
		NextGrabAt = FailedSeconds + FMath::FRandRange(GrabIntervalMin, GrabIntervalMax);
	}
	// Seizing: full torque for a moment, whatever the pedals say, so the car lurches even when
	// nobody brakes (friend test 1: "the brakes are going ... we didn't even press the brake").
	const bool bGrab = FailedSeconds < GrabUntil && FMath::Abs(Car->GetForwardSpeed()) >= GrabMinSpeed;
	const float Power = FMath::Lerp(FadedBrakePower, 0.f, FMath::Clamp(FailedSeconds / GoneSeconds, 0.f, 1.f));
	Car->SetBrakePowerScale(bGrab ? 1.f : Power);
	Car->SetBrakeGrab(bGrab ? GrabStrength : 0.f);
}

void UNBBrakePart::ClearEffect()
{
	GetCar()->SetBrakePowerScale(1.f);
	GetCar()->SetBrakeGrab(0.f);
}

void UNBBrakePart::OnFailedChanged()
{
	NextGrabAt = FMath::FRandRange(GrabIntervalMin * 0.5f, GrabIntervalMin);
	GrabUntil = 0.f;
}

// --- Tire ---

UNBTirePart::UNBTirePart()
{
	Mode = ENBInteractMode::Hold;
	HoldSeconds = 3.f;
	bRequiresOnFoot = true;
	bAttachUser = true;
	PartName = NSLOCTEXT("NB", "TireName", "TIRE");
	FailureText = NSLOCTEXT("NB", "TireFail", "FLAT");
	CalloutText = NSLOCTEXT("NB", "TireCallout", "FLAT TIRE!");
	Prompt = NSLOCTEXT("NB", "TirePrompt", "Patch tire");
	Range = 110.f;
}

void UNBTirePart::BeginPlay()
{
	Super::BeginPlay();
	const UChaosWheeledVehicleMovementComponent* Movement = GetCar()->GetChaosVehicleMovement();
	WheelIndex = Movement->WheelSetups.IndexOfByPredicate([this](const FChaosWheelSetup& Setup) { return Setup.BoneName == WheelBone; });
	ensureMsgf(WheelIndex != INDEX_NONE, TEXT("Tire part %s: no wheel on bone %s"), *GetName(), *WheelBone.ToString());
}

void UNBTirePart::ApplyFailedEffect(float FailedSeconds)
{
	GetCar()->SetWheelGripScale(WheelIndex, FlatGrip);
	GetCar()->SetWheelFlat(this, true);
}

void UNBTirePart::ClearEffect()
{
	GetCar()->SetWheelGripScale(WheelIndex, 1.f);
	GetCar()->SetWheelFlat(this, false);
}

// --- Door ---

UNBDoorPart::UNBDoorPart()
{
	Mode = ENBInteractMode::Mash;
	// A few hard slams: it bounces open the first couple of times.
	MashPresses = 3;
	MashDrainPerSecond = 0.25f;
	PartName = NSLOCTEXT("NB", "DoorName", "DOOR");
	FailureText = NSLOCTEXT("NB", "DoorFail", "OPEN");
	CalloutText = NSLOCTEXT("NB", "DoorCallout", "THE DOOR FLEW OPEN!");
	Prompt = NSLOCTEXT("NB", "DoorPrompt", "Slam door");
	Range = 110.f;
}

void UNBDoorPart::BeginPlay()
{
	Super::BeginPlay();
	// The flapping door is cosmetic, so it ticks everywhere (progress still only moves on the server).
	SetComponentTickEnabled(true);
}

void UNBDoorPart::ApplyFailedEffect(float FailedSeconds)
{
	if (FailedSeconds < NextEjectTime)
	{
		return;
	}
	NextEjectTime = FailedSeconds + EjectIntervalSeconds;
	if (FMath::Abs(GetCar()->GetForwardSpeed()) >= EjectMinSpeed)
	{
		EjectSquirrelOnDoorSide();
	}
}

void UNBDoorPart::EjectSquirrelOnDoorSide()
{
	ANBCar* Car = GetCar();
	const FVector Right = Car->GetActorRightVector();
	const float DoorSide = FMath::Sign(FVector::DotProduct(GetComponentLocation() - Car->GetActorLocation(), Right));
	for (UNBSeatComponent* Seat : Car->GetSeats())
	{
		ANBSquirrel* Occupant = Seat ? Seat->GetOccupant() : nullptr;
		const float SeatSide = Seat ? FVector::DotProduct(Seat->GetComponentLocation() - Car->GetActorLocation(), Right) : 0.f;
		// Only seats clearly on the door's side; the deck seat sits on the centre line.
		if (Occupant && SeatSide * DoorSide > 10.f)
		{
			Occupant->EjectFromCar();
			return;
		}
	}
	// Nobody on that side: the cargo rattles out through the gap instead, so an open door always
	// costs something (friend test 1: "does the door really matter? No").
	Car->SpillAcorns(AcornsLostWhenEmpty);
}

void UNBDoorPart::OnFailedChanged()
{
	NextEjectTime = EjectIntervalSeconds;
	if (Hinge && !IsFailed())
	{
		Hinge->SetRelativeRotation(FRotator::ZeroRotator);
	}
}

void UNBDoorPart::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	if (Hinge && IsFailed())
	{
		// Flap around the open position so it reads as "loose" from a distance.
		const float Flap = FMath::Sin(GetWorld()->GetTimeSeconds() * 7.f) * 12.f;
		Hinge->SetRelativeRotation(FRotator(0.f, OpenYaw + Flap, 0.f));
	}
}
