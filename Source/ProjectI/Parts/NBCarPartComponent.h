// Squirrel Wheels prototype.

#pragma once

#include "CoreMinimal.h"
#include "FX/NBFeedback.h"
#include "Interaction/NBInteractableComponent.h"
#include "NBCarPartComponent.generated.h"

class ANBCar;

/**
 * A breakable bit of the car that is also its own repair spot. While failed, the
 * interactable is enabled and the subclass applies an effect to the car that escalates
 * with time; completing the interaction repairs it. Server-authoritative.
 */
UCLASS(Abstract)
class PROJECTI_API UNBCarPartComponent : public UNBInteractableComponent
{
	GENERATED_BODY()

public:
	UNBCarPartComponent();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/** Short name for the HUD, e.g. "ENGINE". */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Part")
	FText PartName;

	/** What's wrong while failed, e.g. "OVERHEATING". */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Part")
	FText FailureText;

	// --- Sound and VFX. All optional; played on every machine, at the part. ---

	/** One-shot when it breaks (bang, pop, hiss). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Part|Feedback")
	FNBFeedback BreakFeedback;

	/** Loops while it's broken (engine fire and smoke, flapping door creak). Use looping assets.
	 *  Its "Severity" parameter climbs from 0 to 1 over SeverityRampSeconds after the break. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Part|Feedback")
	FNBFeedback FailedLoopFeedback;

	/** One-shot when a squirrel fixes it. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Part|Feedback")
	FNBFeedback RepairFeedback;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Part|Feedback", meta = (ClampMin = "0.1"))
	float SeverityRampSeconds = 10.f;

	/** Parameter on FailedLoopFeedback's sound and effect that gets the severity. */
	static const FName SeverityParam;

	UFUNCTION(BlueprintPure, Category = "Part")
	bool IsFailed() const { return bFailed; }

	/** Server world time of the last repair, or a very negative number if never repaired. */
	float GetLastRepairTime() const { return LastRepairTime; }

	/** Server only. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Part")
	void Fail();

	/** Server only. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Part")
	void Repair();

protected:
	virtual void BeginPlay() override;

	/** Server, every tick while failed. FailedSeconds counts from the moment it broke. */
	virtual void ApplyFailedEffect(float FailedSeconds) {}

	/** Server, on repair: put the car back to healthy. */
	virtual void ClearEffect() {}

	/** All machines, whenever the failed state changes (cosmetics). */
	virtual void OnFailedChanged() {}

	ANBCar* GetCar() const;

private:
	UFUNCTION()
	void OnRep_Failed();

	/** All machines: cosmetics hook plus feedback. One-shots only for a change seen live,
	 *  not for the state a late joiner or newly relevant client receives. */
	void HandleFailedChanged(bool bPlayOneShots);
	void RefreshFailedLoop();

	UFUNCTION()
	void HandleRepairCompleted(UNBInteractableComponent* Interactable);

	UPROPERTY(ReplicatedUsing = OnRep_Failed)
	bool bFailed = false;

	float TimeSinceFailure = 0.f;
	float LastRepairTime = -1000000.f;

	/** Local world time this machine saw the break, for the loop's severity. */
	float LocalFailTime = 0.f;
	FNBFeedbackLoop FailedLoop;
};
