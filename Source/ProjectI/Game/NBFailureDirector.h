// Squirrel Wheels prototype.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "NBFailureDirector.generated.h"

class ANBCar;

/**
 * Breaks car parts during a run. Failures come faster as the run goes on, and the number
 * allowed at once grows with the squirrel count, so there is always something to fix.
 * Server only; lives on the game mode.
 */
UCLASS()
class PROJECTI_API UNBFailureDirector : public UActorComponent
{
	GENERATED_BODY()

public:
	UNBFailureDirector();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/** RunSeconds is the length of the run, used to ramp up the pace. */
	void Begin(ANBCar* InCar, float RunSeconds);
	void End();

protected:
	/** Seconds between failures at the start of the run... */
	UPROPERTY(EditAnywhere, Category = "Director")
	float StartInterval = 14.f;

	/** ...and at the end. */
	UPROPERTY(EditAnywhere, Category = "Director")
	float EndInterval = 6.f;

	/** A part that was just repaired can't break again for this long. */
	UPROPERTY(EditAnywhere, Category = "Director")
	float RepairCooldown = 8.f;

	/** Grace period before the first failure. */
	UPROPERTY(EditAnywhere, Category = "Director")
	float FirstFailureDelay = 8.f;

private:
	/** Parts that may be failed at once at this point in the run. */
	int32 MaxConcurrent(float Progress) const;
	float NextInterval(float Progress) const;

	UPROPERTY()
	TObjectPtr<ANBCar> Car;

	bool bActive = false;
	float RunLength = 1.f;
	float Elapsed = 0.f;
	float NextFailureAt = 0.f;
};
