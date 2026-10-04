// Squirrel Wheels prototype.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Car/NBSeatComponent.h"
#include "NBCar.generated.h"

class UBoxComponent;
class UStaticMeshComponent;

USTRUCT(BlueprintType)
struct FNBWheelSetup
{
	GENERATED_BODY()

	/** Suspension mount point, local to the car root. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wheel")
	FVector LocalOffset = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wheel")
	bool bSteers = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wheel")
	bool bDriven = true;

	/** Multiplier applied by part damage (e.g. flat tire). 1 = healthy. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "Wheel")
	float GripScale = 1.f;
};

/**
 * The one car everyone shares. Arcade raycast vehicle: a physics box held up by four
 * spring raycasts, with drive, brake and lateral-grip forces applied at each grounded wheel.
 *
 * Server-authoritative: only the server applies forces; clients receive replicated physics.
 * Seated squirrels feed input through SetSeatInput (called from their server RPC), so a
 * client never drives the car directly.
 */
UCLASS()
class PROJECTI_API ANBCar : public AActor
{
	GENERATED_BODY()

public:
	ANBCar();

	virtual void Tick(float DeltaSeconds) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/** Server only. Routes a seated squirrel's axis to the control that seat owns. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Car")
	void SetSeatInput(ENBSeatRole SeatRole, float Value);

	UFUNCTION(BlueprintPure, Category = "Car")
	UNBSeatComponent* GetSeat(ENBSeatRole SeatRole) const { return SeatRole == ENBSeatRole::Wheel ? WheelSeat : PedalSeat; }

	/** Closest unoccupied seat within MaxDistance of Location, or nullptr. */
	UNBSeatComponent* FindNearestFreeSeat(const FVector& Location, float MaxDistance) const;

	UFUNCTION(BlueprintPure, Category = "Car")
	float GetForwardSpeed() const;

	UFUNCTION(BlueprintPure, Category = "Car")
	float GetSteerInput() const { return SteerInput; }

	UFUNCTION(BlueprintPure, Category = "Car")
	float GetThrottleInput() const { return ThrottleInput; }

	// --- Hooks for car parts (engine, brakes, tires). 1 = healthy. ---

	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "Car|Damage")
	float EnginePowerScale = 1.f;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "Car|Damage")
	float BrakePowerScale = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Car|Wheels")
	TArray<FNBWheelSetup> Wheels;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Car")
	TObjectPtr<UBoxComponent> Chassis;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Car")
	TObjectPtr<UStaticMeshComponent> BodyMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Car")
	TArray<TObjectPtr<UStaticMeshComponent>> WheelMeshes;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Car")
	TObjectPtr<UNBSeatComponent> WheelSeat;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Car")
	TObjectPtr<UNBSeatComponent> PedalSeat;

	// --- Tuning (arcade: forgiving, big air, hard to flip) ---

	UPROPERTY(EditAnywhere, Category = "Car|Tuning")
	float MassKg = 1200.f;

	UPROPERTY(EditAnywhere, Category = "Car|Tuning")
	float WheelRadius = 38.f;

	UPROPERTY(EditAnywhere, Category = "Car|Tuning")
	float SuspensionRestLength = 35.f;

	UPROPERTY(EditAnywhere, Category = "Car|Tuning")
	float SpringStrength = 26000.f;

	UPROPERTY(EditAnywhere, Category = "Car|Tuning")
	float SpringDamping = 3200.f;

	/** Total drive force across driven wheels, in Newtons*100 (UE units). */
	UPROPERTY(EditAnywhere, Category = "Car|Tuning")
	float EngineForce = 900000.f;

	UPROPERTY(EditAnywhere, Category = "Car|Tuning")
	float BrakeForce = 1200000.f;

	UPROPERTY(EditAnywhere, Category = "Car|Tuning")
	float ReverseForceScale = 0.5f;

	/** cm/s. Drive force fades to zero as speed approaches this. */
	UPROPERTY(EditAnywhere, Category = "Car|Tuning")
	float MaxSpeed = 2600.f;

	UPROPERTY(EditAnywhere, Category = "Car|Tuning")
	float MaxSteerAngle = 32.f;

	/** Steering angle multiplier at MaxSpeed (keeps high speed stable). */
	UPROPERTY(EditAnywhere, Category = "Car|Tuning")
	float HighSpeedSteerScale = 0.45f;

	/** How hard tires cancel sideways sliding (1/s). Lower = driftier. */
	UPROPERTY(EditAnywhere, Category = "Car|Tuning")
	float LateralGrip = 9.f;

	/** Passive slowdown when no throttle (1/s). */
	UPROPERTY(EditAnywhere, Category = "Car|Tuning")
	float RollingResistance = 0.35f;

private:
	void SimulateWheels(float DeltaSeconds, bool bApplyForces);
	FVector GetWheelForward(int32 WheelIndex) const;

	UPROPERTY(Replicated)
	float SteerInput = 0.f;

	UPROPERTY(Replicated)
	float ThrottleInput = 0.f;

	TArray<float> LastCompression;
};
