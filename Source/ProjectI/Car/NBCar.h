// Squirrel Wheels prototype.

#pragma once

#include "CoreMinimal.h"
#include "Car/NBSeatComponent.h"
#include "TP_VehicleAdvOffroadCar.h"
#include "NBCar.generated.h"

class UNBBrakePart;
class UNBCarPartComponent;
class UNBDoorPart;
class UNBEnginePart;
class UNBInteractableComponent;
class UNBTirePart;
class UStaticMeshComponent;

/**
 * The one car everyone shares: a Chaos wheeled vehicle built on the template offroad car.
 * Mesh, tire sockets and curves come from the Blueprint child (BP_OffroadCar_Pawn).
 *
 * Nobody possesses it. Seated squirrels send their axis to the server, which feeds
 * Chaos directly; clients follow via Predictive Interpolation physics replication.
 */
UCLASS()
class PROJECTI_API ANBCar : public ATP_VehicleAdvOffroadCar
{
	GENERATED_BODY()

public:
	ANBCar();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/** Server only. Routes a seated squirrel's axis to the control that seat owns. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Car")
	void SetSeatInput(ENBSeatRole SeatRole, float Value);

	UFUNCTION(BlueprintPure, Category = "Car")
	UNBSeatComponent* GetSeat(ENBSeatRole SeatRole) const { return SeatRole == ENBSeatRole::Wheel ? WheelSeat : PedalSeat; }

	/** Closest unoccupied seat within MaxDistance of Location, or nullptr. */
	UNBSeatComponent* FindNearestFreeSeat(const FVector& Location, float MaxDistance) const;

	/** The next free seat after From in hop order (wrapping), or nullptr if every other seat is taken. */
	UNBSeatComponent* FindNextFreeSeat(const UNBSeatComponent* From) const;

	/** Every seat, in hop order. */
	const TArray<TObjectPtr<UNBSeatComponent>>& GetSeats() const { return Seats; }

	/** Every breakable part. */
	const TArray<TObjectPtr<UNBCarPartComponent>>& GetParts() const { return Parts; }

	// --- Dev failure triggers, until the failure director (M2) exists. Server only. ---

	/** Engine, Brakes, Door, TireFL/FR/BL/BR, Tire (random tire) or All. */
	void DevFail(const FString& PartName);

	/** Dev: roll the car onto its roof so the flip-up can be tested. */
	void DevFlip();

	/** Random failures every DevChaosInterval seconds. */
	void SetDevChaos(bool bEnable);
	bool IsDevChaos() const { return DevChaosTimer.IsValid(); }

	/**
	 * Upside down or on its side and nearly stopped. Riders are thrown out, the seats are
	 * closed, and two squirrels on foot have to push together at FlipSpot to right it.
	 */
	UFUNCTION(BlueprintPure, Category = "Car|Flip")
	bool IsFlipped() const { return bFlipped; }

	/** cm/s along the car's forward axis. */
	UFUNCTION(BlueprintPure, Category = "Car")
	float GetForwardSpeed() const;

	UFUNCTION(BlueprintPure, Category = "Car")
	float GetSteerInput() const { return SteerInput; }

	UFUNCTION(BlueprintPure, Category = "Car")
	float GetThrottleInput() const { return ThrottleInput; }

	// --- Hooks for car parts. 1 = healthy. Server only. ---

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Car|Damage")
	void SetEnginePowerScale(float Scale);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Car|Damage")
	void SetBrakePowerScale(float Scale);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Car|Damage")
	void SetWheelGripScale(int32 WheelIndex, float Scale);

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Car")
	TObjectPtr<UNBSeatComponent> WheelSeat;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Car")
	TObjectPtr<UNBSeatComponent> PedalSeat;

	/** Rider spots for squirrels who aren't driving. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Car")
	TObjectPtr<UNBSeatComponent> PassengerSeat;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Car")
	TObjectPtr<UNBSeatComponent> DeckSeat;

	/** On the front hood, next to the brakes. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Car")
	TObjectPtr<UNBSeatComponent> HoodSeat;

	/** Every seat, in hop order. Filled in BeginPlay. */
	UPROPERTY(Transient)
	TArray<TObjectPtr<UNBSeatComponent>> Seats;

	// --- Parts. Each is also the spot you repair it from. ---

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Car|Parts")
	TObjectPtr<UNBEnginePart> EnginePart;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Car|Parts")
	TObjectPtr<UNBBrakePart> BrakePart;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Car|Parts")
	TObjectPtr<UNBTirePart> TireFL;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Car|Parts")
	TObjectPtr<UNBTirePart> TireFR;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Car|Parts")
	TObjectPtr<UNBTirePart> TireBL;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Car|Parts")
	TObjectPtr<UNBTirePart> TireBR;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Car|Parts")
	TObjectPtr<UNBDoorPart> DoorPart;

	/** Greybox door on the passenger side: the hinge swings, the panel hangs off it. */
	UPROPERTY(VisibleAnywhere, Category = "Car|Parts")
	TObjectPtr<USceneComponent> DoorHinge;

	UPROPERTY(VisibleAnywhere, Category = "Car|Parts")
	TObjectPtr<UStaticMeshComponent> DoorPanel;

	/** Where squirrels push to right a flipped car. Only enabled while flipped. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Car|Flip")
	TObjectPtr<UNBInteractableComponent> FlipSpot;

	/** Counts as flipped while the car's up axis is below this (0 = on its side). */
	UPROPERTY(EditAnywhere, Category = "Car|Flip")
	float FlipUpDot = 0.35f;

	/** ...and it is slower than this (cm/s)... */
	UPROPERTY(EditAnywhere, Category = "Car|Flip")
	float FlipMaxSpeed = 200.f;

	/** ...for this long, so a roll that lands on its wheels doesn't count. */
	UPROPERTY(EditAnywhere, Category = "Car|Flip")
	float FlipConfirmSeconds = 1.5f;

	/** Righted cars are lifted this far before being dropped back on their wheels. */
	UPROPERTY(EditAnywhere, Category = "Car|Flip")
	float RightingLift = 120.f;

	/** Every part. Filled in BeginPlay. */
	UPROPERTY(Transient)
	TArray<TObjectPtr<UNBCarPartComponent>> Parts;

	UPROPERTY(EditAnywhere, Category = "Car|Dev")
	float DevChaosInterval = 12.f;

private:
	void ApplyInputsToVehicle();
	UNBTirePart* CreateTire(FName Name, FName WheelBone, const FVector& Location, const FText& PartName);
	void DevFailRandomPart();
	void CheckFlipped();
	void SetFlipped(bool bNewFlipped);

	UFUNCTION()
	void HandleFlipPushed(UNBInteractableComponent* Interactable);

	FTimerHandle FlipCheckTimerHandle;
	float FlippedSeconds = 0.f;

	UPROPERTY(Replicated)
	bool bFlipped = false;

	FTimerHandle DevChaosTimer;

	UPROPERTY(Replicated)
	float SteerInput = 0.f;

	/** >0 gas, <0 brake (Chaos reverses when braking from a stop). */
	UPROPERTY(Replicated)
	float ThrottleInput = 0.f;

	float BaseMaxEngineTorque = 0.f;
	TArray<float> BaseBrakeTorque;
	TArray<float> BaseFriction;
};
