// Squirrel Wheels prototype.

#pragma once

#include "CoreMinimal.h"
#include "ChaosWheeledVehicleMovementComponent.h"
#include "NBVehicleMovement.generated.h"

/**
 * Chaos's wheeled simulation with one change to the suspension traces: a hit only counts as ground
 * if its surface faces back up the sweep. Each wheel's sweep starts a wheel radius above the wheel,
 * so anything sitting on the fender (a squirrel, 31 Oct) started inside it and came back as a hit at
 * the very top: the suspension read fully compressed and the jeep tipped. Those hits face along the
 * sweep and are skipped, while a squirrel under a wheel still faces up and is driven over like rubble.
 */
class FNBWheeledVehicleSimulation : public UChaosWheeledVehicleSimulation
{
public:
	virtual void PerformSuspensionTraces(const TArray<Chaos::FSuspensionTrace>& SuspensionTrace, FCollisionQueryParams& TraceParams,
		FCollisionResponseContainer& CollisionResponse, TArray<FWheelTraceParams>& WheelTraceParams) override;
};

/** The car's movement component: Chaos's, running FNBWheeledVehicleSimulation. ANBCar swaps it in. */
UCLASS()
class PROJECTI_API UNBVehicleMovementComponent : public UChaosWheeledVehicleMovementComponent
{
	GENERATED_BODY()

protected:
	virtual TUniquePtr<Chaos::FSimpleWheeledVehicle> CreatePhysicsVehicle() override;
};
