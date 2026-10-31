// Squirrel Wheels prototype.

#include "Car/NBVehicleMovement.h"

#include "Engine/World.h"

namespace
{
	/** Re-sweeps after a rejected hit, each ignoring one more component. Plenty for a squirrel or two on a fender. */
	constexpr int32 MaxRejectedHits = 4;

	/** Ground faces back up the sweep. A surface facing along it (or sideways) was only met from inside or above. */
	bool FacesAlongSweep(const FHitResult& Hit, const FVector& SweepDir)
	{
		return Hit.bBlockingHit && FVector::DotProduct(Hit.ImpactNormal, SweepDir) >= 0.f;
	}
}

void FNBWheeledVehicleSimulation::PerformSuspensionTraces(const TArray<Chaos::FSuspensionTrace>& SuspensionTrace, FCollisionQueryParams& TraceParams,
	FCollisionResponseContainer& CollisionResponse, TArray<FWheelTraceParams>& WheelTraceParams)
{
	UChaosWheeledVehicleSimulation::PerformSuspensionTraces(SuspensionTrace, TraceParams, CollisionResponse, WheelTraceParams);

	FCollisionResponseParams ResponseParams;
	ResponseParams.CollisionResponse = CollisionResponse;

	for (int32 WheelIdx = 0; WheelIdx < SuspensionTrace.Num(); ++WheelIdx)
	{
		FHitResult& HitResult = WheelState.TraceResult[WheelIdx];
		const FVector TraceStart = SuspensionTrace[WheelIdx].Start;
		const FVector TraceEnd = SuspensionTrace[WheelIdx].End;
		const FVector SweepDir = (TraceEnd - TraceStart).GetSafeNormal();
		if (!FacesAlongSweep(HitResult, SweepDir))
		{
			continue;
		}

		// Sweep again as the engine does (same channel, shape and start), ignoring each rejected component.
		FCollisionQueryParams Params = TraceParams;
		Params.bTraceComplex = (WheelTraceParams[WheelIdx].SweepType == ESweepType::ComplexSweep);
		const bool bSphere = WheelTraceParams[WheelIdx].SweepShape == ESweepShape::Spherecast;
		const float WheelRadius = PVehicle->Wheels[WheelIdx].GetEffectiveRadius();
		for (int32 Try = 0; Try < MaxRejectedHits && FacesAlongSweep(HitResult, SweepDir); ++Try)
		{
			Params.AddIgnoredComponent(HitResult.GetComponent());
			HitResult = FHitResult();
			if (bSphere)
			{
				World->SweepSingleByChannel(HitResult, TraceStart - SweepDir * WheelRadius, TraceEnd, FQuat::Identity, ECC_WorldDynamic,
					FCollisionShape::MakeSphere(WheelRadius), Params, ResponseParams);
			}
			else
			{
				World->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_WorldDynamic, Params, ResponseParams);
			}
		}
		if (FacesAlongSweep(HitResult, SweepDir))
		{
			HitResult = FHitResult();
		}
	}
}

TUniquePtr<Chaos::FSimpleWheeledVehicle> UNBVehicleMovementComponent::CreatePhysicsVehicle()
{
	// As UChaosWheeledVehicleMovementComponent::CreatePhysicsVehicle, with our simulation.
	VehicleSimulationPT = MakeUnique<FNBWheeledVehicleSimulation>();
	return UChaosVehicleMovementComponent::CreatePhysicsVehicle();
}
