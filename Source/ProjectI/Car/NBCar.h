// Squirrel Wheels prototype.

#pragma once

#include "CoreMinimal.h"
#include "Car/NBSeatComponent.h"
#include "TP_VehicleAdvOffroadCar.h"
#include "NBCar.generated.h"

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

private:
	void ApplyInputsToVehicle();

	UPROPERTY(Replicated)
	float SteerInput = 0.f;

	/** >0 gas, <0 brake (Chaos reverses when braking from a stop). */
	UPROPERTY(Replicated)
	float ThrottleInput = 0.f;

	float BaseMaxEngineTorque = 0.f;
	TArray<float> BaseBrakeTorque;
	TArray<float> BaseFriction;
};
