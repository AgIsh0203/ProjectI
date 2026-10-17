// Squirrel Wheels prototype.

#include "Car/NBCar.h"

#include "ChaosVehicleWheel.h"
#include "ChaosWheeledVehicleMovementComponent.h"
#include "Components/AudioComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerController.h"
#include "Car/NBAcorn.h"
#include "Car/NBSeatComponent.h"
#include "Game/NBRoute.h"
#include "Interaction/NBInteractableComponent.h"
#include "Materials/MaterialInterface.h"
#include "Net/UnrealNetwork.h"
#include "Parts/NBCarParts.h"
#include "Player/NBPlayerState.h"
#include "Player/NBSquirrel.h"
#include "TimerManager.h"
#include "UObject/ConstructorHelpers.h"

ANBCar::ANBCar()
{
	bReplicates = true;
	SetReplicatingMovement(true);
	// Engine RPM is sampled (server) and smoothed for audio (clients) every frame.
	PrimaryActorTick.bCanEverTick = true;
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

	// The same acorn mesh the spilled acorns use (CC-BY, 1 m across, so the slots' 0.14 scale makes
	// 14 cm acorns), so a spill visibly comes out of the pile. Its own two materials (nut and cap) stay.
	AcornPile = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("AcornPile"));
	AcornPile->SetupAttachment(GetMesh());
	AcornPile->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	AcornPile->SetCanEverAffectNavigation(false);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> AcornMesh(TEXT("/Game/ThirdParty/PolyPizza/acorn_n/StaticMeshes/SM_Acorn.SM_Acorn"));
	if (AcornMesh.Succeeded())
	{
		AcornPile->SetStaticMesh(AcornMesh.Object);
	}

	DoorPart = CreateDefaultSubobject<UNBDoorPart>(TEXT("DoorPart"));
	DoorPart->SetupAttachment(GetMesh());
	DoorPart->SetRelativeLocation(FVector(30.f, 95.f, 100.f));
	DoorPart->Range = 130.f;

	// Middle of the body, so a squirrel standing at any side of the upturned car reaches it
	// (half-length ~250, half-width ~200, and the roof-down body sits ~1.5 m up).
	FlipSpot = CreateDefaultSubobject<UNBInteractableComponent>(TEXT("FlipSpot"));
	FlipSpot->SetupAttachment(GetMesh());
	FlipSpot->SetRelativeLocation(FVector(0.f, 0.f, 90.f));
	FlipSpot->Mode = ENBInteractMode::Push2;
	FlipSpot->RequiredUsers = 2;
	FlipSpot->HoldSeconds = 2.f;
	FlipSpot->Range = 360.f;
	FlipSpot->bRequiresOnFoot = true;
	FlipSpot->bDisableOnComplete = true;
	FlipSpot->Prompt = NSLOCTEXT("NB", "FlipPrompt", "Flip the car back over");

	// The engine sits under the deck at the back.
	EngineAudio = CreateDefaultSubobject<UAudioComponent>(TEXT("EngineAudio"));
	EngineAudio->SetupAttachment(GetMesh());
	EngineAudio->SetRelativeLocation(FVector(-100.f, 0.f, 120.f));
	EngineAudio->bAutoActivate = false;
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

	// The wheels drive through river water to the riverbed, never on top of it.
	GetChaosVehicleMovement()->SetWheelTraceResponseToChannel(NBCollision::Water, ECR_Ignore);

	if (HasAuthority())
	{
		FlipSpot->SetInteractEnabled(false);
		FlipSpot->OnCompleted.AddDynamic(this, &ANBCar::HandleFlipPushed);
		GetWorldTimerManager().SetTimer(FlipCheckTimerHandle, this, &ANBCar::CheckFlipped, 0.25f, true);
		GetWorldTimerManager().SetTimer(SpillTimerHandle, this, &ANBCar::CheckSpill, 0.1f, true);
		Acorns = MaxAcorns;
	}

	// Layers of 5 x 4 on the engine deck (top ~z 148 over x -60..-110), each layer nudged
	// half a step so it nests in the one below. Fixed seed: every machine builds the same pile.
	AcornPileSlots.Reset();
	FRandomStream Jitter(4242);
	constexpr int32 Columns = 5;
	constexpr int32 Rows = 4;
	for (int32 i = 0; i < MaxAcorns; ++i)
	{
		const int32 Layer = i / (Columns * Rows);
		const int32 Column = i % Columns;
		const int32 Row = (i / Columns) % Rows;
		const float Nudge = (Layer % 2) * 0.5f;
		const FVector Location(
			-125.f + (Column + Nudge) * 18.f + Jitter.FRandRange(-3.f, 3.f),
			-42.f + (Row + Nudge) * 26.f + Jitter.FRandRange(-3.f, 3.f),
			156.f + Layer * 11.f);
		const FRotator Tumble(Jitter.FRandRange(-30.f, 30.f), Jitter.FRandRange(0.f, 360.f), Jitter.FRandRange(-30.f, 30.f));
		AcornPileSlots.Add(FTransform(Tumble, Location, FVector(0.14f)));
	}
	RefreshAcornPile();

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

	EngineRPM = Movement->EngineSetup.EngineIdleRPM;
	if (EngineSound && GetNetMode() != NM_DedicatedServer)
	{
		EngineAudio->SetSound(EngineSound);
		EngineAudio->Play();
	}
}

void ANBCar::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	UpdateEngineState(DeltaSeconds);
	if (HasAuthority())
	{
		UpdateWear(DeltaSeconds);
		// The flat-tire wobble and speed cap change every frame, not only on input.
		if (FlatWheels != 0)
		{
			ApplyInputsToVehicle();
		}
	}
}

void ANBCar::UpdateWear(float DeltaSeconds)
{
	if (bRunLocked || bFlipped)
	{
		return;
	}
	const float Speed = FMath::Abs(GetForwardSpeed());
	const float Fast = FMath::GetMappedRangeValueClamped(FVector2f(WearSpeedStart, WearSpeedFull), FVector2f(0.f, 1.f), Speed);
	const float Throttle = FMath::Max(ThrottleInput, 0.f);
	// Negative pedal input brakes while rolling forward, and reverses once stopped.
	const float Brake = GetForwardSpeed() > 100.f ? FMath::Max(-ThrottleInput, 0.f) : 0.f;

	for (UNBCarPartComponent* Part : Parts)
	{
		Part->AddWear(-WearRecoveryRate * DeltaSeconds);
	}

	// Speeding: the engine and the rear tires.
	EnginePart->AddWear(EngineWearRate * Fast * (0.5f + 0.5f * Throttle) * DeltaSeconds);
	TireBL->AddWear(RearTireWearRate * Fast * DeltaSeconds);
	TireBR->AddWear(RearTireWearRate * Fast * DeltaSeconds);

	// Steering a lot: the front tires.
	const float Scrub = FMath::Abs(SteerInput) * FMath::Clamp(Speed / FrontTireWearSpeed, 0.f, 1.f);
	TireFL->AddWear(FrontTireWearRate * Scrub * DeltaSeconds);
	TireFR->AddWear(FrontTireWearRate * Scrub * DeltaSeconds);

	// Brakes: hammering them, or never touching them.
	BrakePart->AddWear(BrakeHeavyWearRate * Brake * FMath::Clamp(Speed / BrakeWearSpeed, 0.f, 1.f) * DeltaSeconds);
	if (Brake > 0.1f)
	{
		SecondsWithoutBraking = 0.f;
	}
	else if (Speed >= BrakeIdleMinSpeed)
	{
		SecondsWithoutBraking += DeltaSeconds;
		if (SecondsWithoutBraking > BrakeIdleSeconds)
		{
			BrakePart->AddWear(BrakeIdleWearRate * DeltaSeconds);
		}
	}

	// Fording: the water holds the car back and the stones nobody can see chew the tyres.
	if (const ANBRoute* Route = ANBRoute::Find(GetWorld()))
	{
		FVector Flow;
		// How deep the water is over the ground under the car.
		const FVector Location = GetActorLocation();
		const float Depth = Route->GetWaterDepth(FVector(Location.X, Location.Y, Route->GetGroundHeight(Location.X, Location.Y)), Flow);
		const float Wet = FMath::Clamp(Depth / RiverFullDepth, 0.f, 1.f);
		if (Wet > 0.f)
		{
			const float Moving = FMath::Clamp(Speed / 800.f, 0.25f, 1.f);
			for (UNBCarPartComponent* Tire : {TireFL, TireFR, TireBL, TireBR})
			{
				Tire->AddWear(RiverTireWearRate * Wet * Moving * DeltaSeconds);
			}
			GetMesh()->AddForce(-GetVelocity() * RiverDrag * Wet, NAME_None, /*bAccelChange=*/true);
		}
		if ((Wet > 0.f) != bInRiver)
		{
			bInRiver = Wet > 0.f;
			UE_LOG(LogTemp, Log, TEXT("NBRiver: car %s the water (depth %.0f cm)"), bInRiver ? TEXT("enters") : TEXT("leaves"), Depth);
		}
	}
}

void ANBCar::NoteSeatHop()
{
	check(HasAuthority());
	if (!bRunLocked)
	{
		DoorPart->AddWear(DoorWearPerHop);
	}
}

void ANBCar::UpdateEngineState(float DeltaSeconds)
{
	const UChaosWheeledVehicleMovementComponent* Movement = GetChaosVehicleMovement();
	if (HasAuthority())
	{
		EngineRPM = Movement->GetEngineRotationSpeed();
		RepEngineRPM = static_cast<uint16>(FMath::Clamp(FMath::RoundToInt(EngineRPM / 10.f) * 10, 0, MAX_uint16));
		RepGear = static_cast<int8>(FMath::Clamp(Movement->GetCurrentGear(), -128, 127));
	}
	else
	{
		EngineRPM = FMath::FInterpTo(EngineRPM, static_cast<float>(RepEngineRPM), DeltaSeconds, RPMSmoothingSpeed);
	}

	if (!EngineAudio->IsPlaying())
	{
		return;
	}
	const float Alpha = GetEngineRPMAlpha();
	EngineAudio->SetFloatParameter(EngineRPMParam, EngineRPM);
	EngineAudio->SetFloatParameter(EngineThrottleParam, ThrottleInput);
	EngineAudio->SetFloatParameter(EngineFailedParam, EnginePart->IsFailed() ? 1.f : 0.f);
	EngineAudio->SetVolumeMultiplier(FMath::Lerp(EngineIdleVolume, 1.f, FMath::Clamp(ThrottleInput, 0.f, 1.f)));
	if (bPitchFromRPM)
	{
		EngineAudio->SetPitchMultiplier(FMath::Lerp(EnginePitchAtIdle, EnginePitchAtRedline, Alpha));
	}
}

float ANBCar::GetEngineRPMAlpha() const
{
	const auto& Engine = GetChaosVehicleMovement()->EngineSetup;
	return Engine.MaxRPM > Engine.EngineIdleRPM
		? FMath::Clamp((EngineRPM - Engine.EngineIdleRPM) / (Engine.MaxRPM - Engine.EngineIdleRPM), 0.f, 1.f)
		: 0.f;
}

void ANBCar::Multicast_Feedback_Implementation(ENBCarFeedback Event, float Intensity)
{
	switch (Event)
	{
	case ENBCarFeedback::Crash:
		NBFeedback::PlayAttached(CrashFeedback, GetMesh(), FVector::ZeroVector, Intensity);
		ShakeLocalCameras(Intensity);
		break;
	case ENBCarFeedback::Spill:
		// From the open bed behind the seats, where the acorns fly out.
		NBFeedback::PlayAttached(SpillFeedback, GetMesh(), FVector(-85.f, 0.f, 170.f), Intensity);
		break;
	case ENBCarFeedback::Flipped:
		NBFeedback::PlayAttached(FlippedFeedback, GetMesh(), FVector::ZeroVector, Intensity);
		break;
	case ENBCarFeedback::Righted:
		NBFeedback::PlayAttached(RightedFeedback, GetMesh(), FVector::ZeroVector, Intensity);
		break;
	case ENBCarFeedback::Horn:
		LastHornTime = GetWorld()->GetTimeSeconds();
		// Behind the front grille.
		NBFeedback::PlayAttached(HornFeedback, GetMesh(), FVector(190.f, 0.f, 100.f), Intensity);
		break;
	}
}

void ANBCar::ShakeLocalCameras(float Intensity) const
{
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		const APlayerController* PC = It->Get();
		ANBSquirrel* Squirrel = PC && PC->IsLocalController() ? Cast<ANBSquirrel>(PC->GetPawn()) : nullptr;
		if (!Squirrel)
		{
			continue;
		}
		// Riders take the full hit; squirrels on foot feel it less the farther away they are.
		const bool bRiding = Squirrel->GetAttachParentActor() == this;
		const float Falloff = bRiding ? 1.f
			: 1.f - FMath::Clamp(FVector::Dist(Squirrel->GetActorLocation(), GetActorLocation()) / FMath::Max(CrashShakeRadius, 1.f), 0.f, 1.f);
		if (Falloff > 0.f)
		{
			Squirrel->AddCameraPunch(Intensity * Falloff);
		}
	}
}

void ANBCar::Honk()
{
	check(HasAuthority());
	const float Now = GetWorld()->GetTimeSeconds();
	if (Now < NextHornTime)
	{
		return;
	}
	NextHornTime = Now + HornCooldown;
	Multicast_Feedback(ENBCarFeedback::Horn, 1.f);
}

void ANBCar::OnRep_Acorns()
{
	RefreshAcornPile();
}

void ANBCar::RefreshAcornPile()
{
	// Rebuilt whole: at 40 instances that's cheaper to reason about than removing by index.
	const int32 Count = FMath::Clamp(Acorns, 0, AcornPileSlots.Num());
	if (AcornPile->GetInstanceCount() == Count)
	{
		return;
	}
	AcornPile->ClearInstances();
	AcornPile->AddInstances(TArray<FTransform>(AcornPileSlots.GetData(), Count), false, false, false);
}

void ANBCar::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ANBCar, SteerInput);
	DOREPLIFETIME(ANBCar, ThrottleInput);
	DOREPLIFETIME(ANBCar, bFlipped);
	DOREPLIFETIME(ANBCar, Acorns);
	DOREPLIFETIME(ANBCar, LastJumpPoints);
	DOREPLIFETIME(ANBCar, JumpCount);
	DOREPLIFETIME(ANBCar, RepEngineRPM);
	DOREPLIFETIME(ANBCar, RepGear);
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
	float Steer = SteerInput;
	float Throttle = FMath::Max(ThrottleInput, 0.f);
	if (FlatWheels != 0)
	{
		// Each flat drags its side and shakes the wheel; past the top speed the engine gives up.
		const TPair<uint8, float> Pulls[] = {{1, -FlatFrontPull}, {2, FlatFrontPull}, {4, -FlatRearPull}, {8, FlatRearPull}};
		int32 NumFlat = 0;
		for (const TPair<uint8, float>& Pull : Pulls)
		{
			if (FlatWheels & Pull.Key)
			{
				Steer += Pull.Value;
				++NumFlat;
			}
		}
		Steer += NumFlat * FlatWobble * FMath::Sin(GetWorld()->GetTimeSeconds() * 9.f);
		if (FMath::Abs(GetForwardSpeed()) > FlatTopSpeed - (NumFlat - 1) * FlatTopSpeedPerExtra)
		{
			Throttle = 0.f;
		}
	}
	Movement->SetSteeringInput(FMath::Clamp(Steer, -1.f, 1.f));
	Movement->SetThrottleInput(Throttle);
	Movement->SetBrakeInput(bRunLocked ? 1.f : FMath::Max(FMath::Max(-ThrottleInput, 0.f), BrakeGrab));
	Movement->SetHandbrakeInput(bRunLocked);
}

void ANBCar::SetWheelFlat(const UNBTirePart* Tire, bool bFlat)
{
	check(HasAuthority());
	const uint8 Bit = Tire == TireFL ? 1 : Tire == TireFR ? 2 : Tire == TireBL ? 4 : Tire == TireBR ? 8 : 0;
	const uint8 Old = FlatWheels;
	FlatWheels = bFlat ? (FlatWheels | Bit) : (FlatWheels & ~Bit);
	if (FlatWheels != Old)
	{
		ApplyInputsToVehicle();
	}
}

void ANBCar::SetBrakeGrab(float Strength)
{
	check(HasAuthority());
	if (BrakeGrab != Strength)
	{
		BrakeGrab = Strength;
		ApplyInputsToVehicle();
	}
}

void ANBCar::SetRunLocked(bool bLocked)
{
	check(HasAuthority());
	bRunLocked = bLocked;
	LastSpillVelocity = GetVelocity();
	ApplyInputsToVehicle();
}

int32 ANBCar::SpillAcorns(int32 Count)
{
	check(HasAuthority());
	Count = FMath::Clamp(Count, 0, Acorns);
	const FTransform Body = GetMesh()->GetComponentTransform();
	for (int32 i = 0; i < Count; ++i)
	{
		// The bed is the open space behind the seats; acorns pop up and out over the sides.
		const FVector Local(FMath::FRandRange(-130.f, -40.f), FMath::FRandRange(-60.f, 60.f), 200.f);
		const FVector Side = Body.GetUnitAxis(EAxis::Y) * (FMath::RandBool() ? 1.f : -1.f) * FMath::FRandRange(150.f, 450.f);
		const FVector Velocity = GetVelocity() * 0.7f + Side + FVector(0.f, 0.f, FMath::FRandRange(250.f, 550.f));
		ANBAcorn::SpawnSpilled(GetWorld(), Body.TransformPosition(Local), Velocity);
	}
	Acorns -= Count;
	RefreshAcornPile();

	// Blame whoever is driving: the wheel squirrel, or the pedal squirrel if the wheel is empty.
	const ANBSquirrel* Driver = WheelSeat->GetOccupant() ? WheelSeat->GetOccupant() : PedalSeat->GetOccupant();
	if (ANBPlayerState* Stats = (Driver && Count > 0) ? Driver->GetPlayerState<ANBPlayerState>() : nullptr)
	{
		Stats->AddAcornsSpilled(Count);
	}
	return Count;
}

void ANBCar::CheckSpill()
{
	const FVector Velocity = GetVelocity();
	const FVector LastSpillVelocityBefore = LastSpillVelocity;
	const float DeltaV = (Velocity - LastSpillVelocity).Size();
	LastSpillVelocity = Velocity;

	const float Now = GetWorld()->GetTimeSeconds();
	// A clean landing off a jump: the thump straight down doesn't shake the cargo out, only sideways jolts.
	const float SpillDeltaV = UpdateJump(Now) ? FVector::VectorPlaneProject(Velocity - LastSpillVelocityBefore, FVector::UpVector).Size() : DeltaV;
	if (DeltaV >= CrashMinDeltaV && Now >= NextCrashFeedbackTime)
	{
		NextCrashFeedbackTime = Now + CrashCooldown;
		Multicast_Feedback(ENBCarFeedback::Crash, FMath::GetMappedRangeValueClamped(FVector2f(CrashMinDeltaV, CrashMaxDeltaV), FVector2f(0.f, 1.f), DeltaV));
	}

	if (bRunLocked || Acorns <= 0 || SpillDeltaV < SpillMinDeltaV)
	{
		return;
	}
	SpillAcorns(1 + FMath::FloorToInt((SpillDeltaV - SpillMinDeltaV) / SpillDeltaVPerAcorn));
}

bool ANBCar::UpdateJump(double Now)
{
	const UChaosWheeledVehicleMovementComponent* Movement = GetChaosVehicleMovement();
	int32 InContact = 0;
	for (int32 i = 0; i < Movement->GetNumWheels(); ++i)
	{
		InContact += Movement->GetWheelState(i).bInContact ? 1 : 0;
	}
	if (InContact == 0)
	{
		if (AirborneSince < 0.0)
		{
			AirborneSince = Now;
		}
		// The landing thump often arrives a sample before the wheels report contact, so a long-enough
		// flight already counts as landing clean.
		return Now - AirborneSince >= MinJumpAirtime;
	}
	if (AirborneSince >= 0.0)
	{
		const double Airtime = Now - AirborneSince;
		AirborneSince = -1.0;
		const bool bUpright = FVector::DotProduct(GetActorUpVector(), FVector::UpVector) > 0.8f;
		if (Airtime >= MinJumpAirtime && GetVelocity().Size2D() >= MinJumpSpeed)
		{
			UE_LOG(LogTemp, Log, TEXT("NBJump: %.2f s in the air, %s"), Airtime, bUpright ? TEXT("clean landing") : TEXT("crashed"));
		}
		// A real jump lands moving; a stuck car whose wheels flicker off the ground, or a teleport, doesn't count.
		const bool bMoving = GetVelocity().Size2D() >= MinJumpSpeed;
		if (Airtime >= MinJumpAirtime && bUpright && bMoving && !bRunLocked && !bFlipped)
		{
			CleanLandingUntil = Now + CleanLandingGrace;
			LastJumpPoints = FMath::RoundToInt(Airtime * AirPointsPerSecond);
			AirPoints += LastJumpPoints;
			++JumpCount;
			OnRep_JumpCount();
		}
	}
	return Now <= CleanLandingUntil;
}

void ANBCar::OnRep_JumpCount()
{
	LastJumpSeenTime = GetWorld()->GetTimeSeconds();
}

UNBSeatComponent* ANBCar::FindNearestFreeSeat(const FVector& Location, float MaxDistance) const
{
	// Seats are closed until the car is back on its wheels.
	if (bFlipped)
	{
		return nullptr;
	}
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
	if (bFlipped)
	{
		return nullptr;
	}
	// Without a From seat, scan every seat starting at the first (the wheel).
	const int32 Start = From ? Seats.IndexOfByKey(From) : INDEX_NONE;
	for (int32 Step = 1; Step <= Seats.Num(); ++Step)
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

void ANBCar::CheckFlipped()
{
	const float UpDot = FVector::DotProduct(GetMesh()->GetUpVector(), FVector::UpVector);
	if (bFlipped)
	{
		// Rolled back onto its wheels by itself (a bump, a push from the other car...).
		if (UpDot > 0.8f)
		{
			SetFlipped(false);
		}
		return;
	}

	const bool bLooksFlipped = UpDot < FlipUpDot && GetVelocity().Size() < FlipMaxSpeed;
	FlippedSeconds = bLooksFlipped ? FlippedSeconds + GetWorldTimerManager().GetTimerRate(FlipCheckTimerHandle) : 0.f;
	if (FlippedSeconds >= FlipConfirmSeconds)
	{
		SetFlipped(true);
	}
}

void ANBCar::SetFlipped(bool bNewFlipped)
{
	check(HasAuthority());
	FlippedSeconds = 0.f;
	if (bFlipped == bNewFlipped)
	{
		return;
	}
	bFlipped = bNewFlipped;
	FlipSpot->SetInteractEnabled(bFlipped);
	if (!bFlipped)
	{
		return;
	}
	Multicast_Feedback(ENBCarFeedback::Flipped, 1.f);

	// Tipping over dumps part of the load.
	SpillAcorns(FMath::CeilToInt(Acorns * FlipSpillFraction));

	// Open top: whoever is still inside falls out.
	for (UNBSeatComponent* Seat : Seats)
	{
		if (ANBSquirrel* Occupant = Seat ? Seat->GetOccupant() : nullptr)
		{
			Occupant->EjectFromCar();
		}
	}
	SteerInput = 0.f;
	ThrottleInput = 0.f;
	ApplyInputsToVehicle();
}

void ANBCar::HandleFlipPushed(UNBInteractableComponent* Interactable)
{
	// Back on its wheels, keeping its heading, dropped from a little height.
	const FRotator Upright(0.f, GetActorRotation().Yaw, 0.f);
	SetActorLocationAndRotation(GetActorLocation() + FVector(0.f, 0.f, RightingLift), Upright, false, nullptr, ETeleportType::TeleportPhysics);
	GetMesh()->SetPhysicsLinearVelocity(FVector::ZeroVector);
	GetMesh()->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
	SetFlipped(false);
	Multicast_Feedback(ENBCarFeedback::Righted, 1.f);
}

void ANBCar::DevFlip()
{
	check(HasAuthority());
	const FRotator Roof(0.f, GetActorRotation().Yaw, 180.f);
	SetActorLocationAndRotation(GetActorLocation() + FVector(0.f, 0.f, 250.f), Roof, false, nullptr, ETeleportType::TeleportPhysics);
	GetMesh()->SetPhysicsLinearVelocity(FVector::ZeroVector);
	GetMesh()->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
}

void ANBCar::DevSoundTour()
{
	check(HasAuthority());
	// The flip only screams if someone is riding: seat every squirrel on foot.
	for (TActorIterator<ANBSquirrel> It(GetWorld()); It; ++It)
	{
		if (!It->IsSeated() && !It->IsRagdolled())
		{
			for (UNBSeatComponent* Seat : Seats)
			{
				if (Seat && Seat->IsFree())
				{
					It->EnterSeat(Seat);
					break;
				}
			}
		}
	}
	SoundTourIndex = 0;
	SoundTourStep();
}

void ANBCar::SoundTourStep()
{
	ANBSquirrel* Squirrel = nullptr;
	for (TActorIterator<ANBSquirrel> It(GetWorld()); It; ++It)
	{
		Squirrel = *It;
		break;
	}

	// Each step: what the screen says, what it does, and how long until the next one.
	FString Label;
	float Wait = 4.f;
	switch (SoundTourIndex++)
	{
	case 0:  Label = TEXT("ENGINE LOOP: full throttle"); SetSeatInput(ENBSeatRole::Pedals, 1.f); break;
	case 1:  Label = TEXT("ENGINE LOOP: back to idle"); SetSeatInput(ENBSeatRole::Pedals, 0.f); break;
	case 2:  Label = TEXT("HORN"); Honk(); Wait = 3.f; break;
	case 3:  Label = TEXT("CRASH (light)"); Multicast_Feedback(ENBCarFeedback::Crash, 0.2f); Wait = 3.f; break;
	case 4:  Label = TEXT("CRASH (hard)"); Multicast_Feedback(ENBCarFeedback::Crash, 1.f); Wait = 3.f; break;
	case 5:  Label = TEXT("ACORN SPILL"); Multicast_Feedback(ENBCarFeedback::Spill, 0.6f); Wait = 3.f; break;
	case 6:  Label = TEXT("ENGINE BREAKS: clang, then the fire loop"); EnginePart->Fail(); Wait = 6.f; break;
	case 7:  Label = TEXT("ENGINE REPAIRED: ding"); EnginePart->Repair(); Wait = 3.f; break;
	case 8:  Label = TEXT("BRAKES BREAK"); BrakePart->Fail(); Wait = 3.f; break;
	case 9:  Label = TEXT("BRAKES REPAIRED"); BrakePart->Repair(); Wait = 3.f; break;
	case 10: Label = TEXT("TIRE BREAKS"); TireFL->Fail(); Wait = 3.f; break;
	case 11: Label = TEXT("TIRE REPAIRED"); TireFL->Repair(); Wait = 3.f; break;
	case 12: Label = TEXT("DOOR BREAKS"); DoorPart->Fail(); Wait = 3.f; break;
	case 13: Label = TEXT("DOOR REPAIRED"); DoorPart->Repair(); Wait = 3.f; break;
	case 14: Label = TEXT("SQUIRREL GRABS A PART: chatter"); if (Squirrel) { Squirrel->DevPlayGrabFeedback(); } Wait = 3.f; break;
	// The flip registers after ~1.5 s upside down: then the thud, the riders' screams and their landing thuds.
	case 15: Label = TEXT("CAR FLIPS: thud, squirrel scream, landing thuds"); DevFlip(); Wait = 7.f; break;
	case 16: Label = TEXT("CAR RIGHTED: boing"); HandleFlipPushed(nullptr); Wait = 4.f; break;
	default:
		GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Green, TEXT("SOUND TOUR DONE. End jingles: NBEnd Delivered, or NBEnd Wrecked"), true, FVector2D(1.6f));
		return;
	}
	UE_LOG(LogTemp, Log, TEXT("NBSoundTour %d: %s"), SoundTourIndex, *Label);
	GEngine->AddOnScreenDebugMessage(-1, Wait, FColor::Yellow, FString::Printf(TEXT("%d/17  %s"), SoundTourIndex, *Label), true, FVector2D(1.6f));
	GetWorldTimerManager().SetTimer(SoundTourTimer, this, &ANBCar::SoundTourStep, Wait, false);
}
