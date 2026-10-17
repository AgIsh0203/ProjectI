// Squirrel Wheels prototype.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "NBFailureDirector.generated.h"

class ANBCar;
class UNBCarPartComponent;

/**
 * Breaks car parts during a run. Failures come faster as the run goes on, and the number
 * allowed at once grows with the squirrel count, so there is always something to fix.
 * Which part goes is random but leans towards parts the driving has worn (see ANBCar's
 * "Car|Wear"), and a badly worn part can go early. A repair that frees a slot buys a full
 * interval of calm instead of an instant replacement.
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

	/** A failure left unfixed this long stops counting against the limit on failures at once,
	 *  so ignoring a cheap one (a flat tire) doesn't hold every other failure off. */
	UPROPERTY(EditAnywhere, Category = "Director")
	float IgnoredFailureSeconds = 25.f;

	/** Failures still pick at random, but a part's odds are 1 + WearBias * wear^2:
	 *  a worn-out part is this much + 1 times as likely as a fresh one. */
	UPROPERTY(EditAnywhere, Category = "Director|Wear")
	float WearBias = 5.f;

	/** Above this wear a part may also break on its own between the timed failures... */
	UPROPERTY(EditAnywhere, Category = "Director|Wear", meta = (ClampMin = "0", ClampMax = "0.99"))
	float WearHazardStart = 0.6f;

	/** ...with this chance per second when fully worn (rising from 0 at WearHazardStart). */
	UPROPERTY(EditAnywhere, Category = "Director|Wear", meta = (ClampMin = "0", ClampMax = "1"))
	float WearHazardPerSecond = 0.08f;

	/** No two failures closer together than this. */
	UPROPERTY(EditAnywhere, Category = "Director")
	float MinGap = 3.f;

private:
	/** Parts that may be failed at once at this point in the run. */
	int32 MaxConcurrent(float Progress) const;
	float NextInterval(float Progress) const;
	void FailPart(UNBCarPartComponent* Part, float Progress, const TCHAR* Why);

	UPROPERTY()
	TObjectPtr<ANBCar> Car;

	bool bActive = false;
	float RunLength = 1.f;
	float Elapsed = 0.f;
	float NextFailureAt = 0.f;

	/** Nothing breaks before this, not even a worn part. */
	float QuietUntil = 0.f;

	/** The limit was reached; the next free slot (a repair) restarts the interval. */
	bool bWasFull = false;
};
