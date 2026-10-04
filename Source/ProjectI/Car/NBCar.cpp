// Squirrel Wheels prototype.

#include "Car/NBCar.h"

#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "Net/UnrealNetwork.h"
#include "UObject/ConstructorHelpers.h"

namespace
{
	const FVector ChassisExtent(220.f, 95.f, 35.f);
	constexpr float WheelWidth = 28.f;
}

ANBCar::ANBCar()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickGroup = TG_PrePhysics;

	bReplicates = true;
	SetReplicatingMovement(true);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderMesh(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));

	Chassis = CreateDefaultSubobject<UBoxComponent>(TEXT("Chassis"));
	Chassis->SetBoxExtent(ChassisExtent);
	Chassis->SetCollisionProfileName(UCollisionProfile::PhysicsActor_ProfileName);
	Chassis->SetSimulatePhysics(true);
	Chassis->SetLinearDamping(0.05f);
	Chassis->SetAngularDamping(1.5f);
	// A low centre of mass makes the car hard (but not impossible) to flip.
	Chassis->BodyInstance.COMNudge = FVector(0.f, 0.f, -40.f);
	RootComponent = Chassis;

	// Greybox body: an open-top tub. Replaced by a Kenney car mesh later.
	BodyMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BodyMesh"));
	BodyMesh->SetupAttachment(Chassis);
	BodyMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	BodyMesh->SetRelativeScale3D(ChassisExtent / 50.f);
	if (CubeMesh.Succeeded())
	{
		BodyMesh->SetStaticMesh(CubeMesh.Object);
	}

	const float X = ChassisExtent.X - 55.f;
	const float Y = ChassisExtent.Y - 5.f;
	const float Z = -ChassisExtent.Z + 10.f;
	auto MakeWheel = [](const FVector& Offset, bool bSteers)
	{
		FNBWheelSetup Wheel;
		Wheel.LocalOffset = Offset;
		Wheel.bSteers = bSteers;
		return Wheel;
	};
	Wheels = {
		MakeWheel(FVector(X, -Y, Z), true),
		MakeWheel(FVector(X, Y, Z), true),
		MakeWheel(FVector(-X, -Y, Z), false),
		MakeWheel(FVector(-X, Y, Z), false),
	};

	for (int32 i = 0; i < Wheels.Num(); ++i)
	{
		UStaticMeshComponent* WheelMesh = CreateDefaultSubobject<UStaticMeshComponent>(*FString::Printf(TEXT("WheelMesh%d"), i));
		WheelMesh->SetupAttachment(Chassis);
		WheelMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		WheelMesh->SetRelativeLocation(Wheels[i].LocalOffset);
		// The engine cylinder is 100 units tall along Z; roll it onto its side.
		WheelMesh->SetRelativeRotation(FRotator(0.f, 0.f, 90.f));
		WheelMesh->SetRelativeScale3D(FVector(WheelRadius / 50.f, WheelRadius / 50.f, WheelWidth / 100.f));
		if (CylinderMesh.Succeeded())
		{
			WheelMesh->SetStaticMesh(CylinderMesh.Object);
		}
		WheelMeshes.Add(WheelMesh);
	}

	WheelSeat = CreateDefaultSubobject<UNBSeatComponent>(TEXT("WheelSeat"));
	WheelSeat->SetupAttachment(Chassis);
	WheelSeat->Role = ENBSeatRole::Wheel;
	WheelSeat->SetRelativeLocation(FVector(40.f, -45.f, ChassisExtent.Z + 30.f));

	PedalSeat = CreateDefaultSubobject<UNBSeatComponent>(TEXT("PedalSeat"));
	PedalSeat->SetupAttachment(Chassis);
	PedalSeat->Role = ENBSeatRole::Pedals;
	PedalSeat->SetRelativeLocation(FVector(85.f, -45.f, ChassisExtent.Z + 5.f));
}

void ANBCar::BeginPlay()
{
	Super::BeginPlay();
	Chassis->SetMassOverrideInKg(NAME_None, MassKg, true);
	LastCompression.Init(0.f, Wheels.Num());
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
	return FVector::DotProduct(Chassis->GetPhysicsLinearVelocity(), GetActorForwardVector());
}

void ANBCar::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	// Every machine runs the same force model from the replicated inputs, so clients
	// predict the car locally and physics replication only corrects drift.
	SimulateWheels(DeltaSeconds, true);
}

FVector ANBCar::GetWheelForward(int32 WheelIndex) const
{
	const FVector Forward = GetActorForwardVector();
	if (!Wheels[WheelIndex].bSteers)
	{
		return Forward;
	}
	const float SpeedAlpha = FMath::Clamp(FMath::Abs(GetForwardSpeed()) / MaxSpeed, 0.f, 1.f);
	const float Angle = SteerInput * MaxSteerAngle * FMath::Lerp(1.f, HighSpeedSteerScale, SpeedAlpha);
	return Forward.RotateAngleAxis(Angle, GetActorUpVector());
}

void ANBCar::SimulateWheels(float DeltaSeconds, bool bApplyForces)
{
	if (DeltaSeconds <= KINDA_SMALL_NUMBER || Wheels.Num() == 0)
	{
		return;
	}

	const FTransform& CarTransform = GetActorTransform();
	const FVector Up = GetActorUpVector();
	const float TraceLength = SuspensionRestLength + WheelRadius;
	const float MassPerWheel = MassKg / Wheels.Num();
	const float ForwardSpeed = GetForwardSpeed();

	int32 DrivenWheels = 0;
	for (const FNBWheelSetup& Wheel : Wheels)
	{
		DrivenWheels += Wheel.bDriven ? 1 : 0;
	}

	FCollisionQueryParams Params(SCENE_QUERY_STAT(NBCarWheel), false, this);
	TArray<AActor*> Attached;
	GetAttachedActors(Attached, true, true);
	Params.AddIgnoredActors(Attached);

	for (int32 i = 0; i < Wheels.Num(); ++i)
	{
		const FNBWheelSetup& Wheel = Wheels[i];
		const FVector Mount = CarTransform.TransformPosition(Wheel.LocalOffset);
		const FVector End = Mount - Up * TraceLength;

		FHitResult Hit;
		const bool bGrounded = GetWorld()->LineTraceSingleByChannel(Hit, Mount, End, ECC_Visibility, Params);
		const float HitDistance = bGrounded ? Hit.Distance : TraceLength;

		if (WheelMeshes.IsValidIndex(i))
		{
			const FVector WheelCentre = Mount - Up * (HitDistance - WheelRadius);
			WheelMeshes[i]->SetWorldLocation(WheelCentre);
			const FVector WheelForward = GetWheelForward(i);
			WheelMeshes[i]->SetWorldRotation(FRotationMatrix::MakeFromXZ(WheelForward, Up).Rotator() + FRotator(0.f, 0.f, 90.f));
		}

		if (!bApplyForces || !bGrounded)
		{
			LastCompression[i] = 0.f;
			continue;
		}

		// Suspension spring + damper.
		const float Compression = FMath::Clamp(TraceLength - Hit.Distance, 0.f, SuspensionRestLength);
		const float CompressionSpeed = (Compression - LastCompression[i]) / DeltaSeconds;
		LastCompression[i] = Compression;
		const float SpringForce = FMath::Max(0.f, SpringStrength * Compression + SpringDamping * CompressionSpeed);
		Chassis->AddForceAtLocation(Up * SpringForce, Mount);

		const FVector ContactPoint = Hit.ImpactPoint;
		const FVector WheelForward = GetWheelForward(i);
		const FVector WheelRight = FVector::CrossProduct(Up, WheelForward).GetSafeNormal();
		const FVector PointVelocity = Chassis->GetPhysicsLinearVelocityAtPoint(ContactPoint);

		// Sideways grip: cancel lateral sliding, scaled by tire health.
		const float LateralSpeed = FVector::DotProduct(PointVelocity, WheelRight);
		Chassis->AddForceAtLocation(-WheelRight * LateralSpeed * MassPerWheel * LateralGrip * Wheel.GripScale, ContactPoint);

		// Drive / brake / reverse along the wheel's heading.
		const bool bBraking = ThrottleInput * ForwardSpeed < 0.f && FMath::Abs(ForwardSpeed) > 50.f;
		if (bBraking)
		{
			const float Brake = BrakeForce * BrakePowerScale / Wheels.Num();
			Chassis->AddForceAtLocation(-WheelForward * FMath::Sign(ForwardSpeed) * Brake * FMath::Abs(ThrottleInput), ContactPoint);
		}
		else if (Wheel.bDriven && DrivenWheels > 0 && !FMath::IsNearlyZero(ThrottleInput))
		{
			const float SpeedFade = 1.f - FMath::Clamp(FMath::Abs(ForwardSpeed) / MaxSpeed, 0.f, 1.f);
			const float Direction = ThrottleInput > 0.f ? 1.f : ReverseForceScale;
			const float Drive = EngineForce * EnginePowerScale * ThrottleInput * Direction * SpeedFade / DrivenWheels;
			Chassis->AddForceAtLocation(WheelForward * Drive, ContactPoint);
		}
		else
		{
			const float RollingSpeed = FVector::DotProduct(PointVelocity, WheelForward);
			Chassis->AddForceAtLocation(-WheelForward * RollingSpeed * MassPerWheel * RollingResistance, ContactPoint);
		}
	}
}
