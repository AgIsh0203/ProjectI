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

	/** Shouted across the middle of the screen when it breaks, e.g. "ENGINE ON FIRE!". */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Part")
	FText CalloutText;

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

	/** While a squirrel works on the part (fanning the engine fire), the failed loop's puffs shrink to this share of
	 *  their rate and size, easing over WorkedDampSeconds: the fire visibly dies down under the fanning and stops
	 *  hiding the squirrel (18 Oct: "can't see the tail fan"). */
	UPROPERTY(EditAnywhere, Category = "Part|Feedback")
	float WorkedLoopDamp = 0.25f;

	UPROPERTY(EditAnywhere, Category = "Part|Feedback")
	float WorkedDampSeconds = 0.4f;

	/** Parameter on FailedLoopFeedback's sound and effect that gets the severity. */
	static const FName SeverityParam;

	UFUNCTION(BlueprintPure, Category = "Part")
	bool IsFailed() const { return bFailed; }

	/** Server world time of the last repair, or a very negative number if never repaired. */
	float GetLastRepairTime() const { return LastRepairTime; }

	/** Server only: seconds since it broke, 0 while healthy. */
	float GetFailedSeconds() const { return bFailed ? TimeSinceFailure : 0.f; }

	/** How worn out it is, 0 (fresh) to 1 (about to go). The car's driving wears it; the failure
	 *  director picks worn parts more often. Replicated coarsely, for a damage meter. */
	UFUNCTION(BlueprintPure, Category = "Part")
	float GetWear() const { return HasAuthority() ? Wear : RepWear / 255.f; }

	/** Server only. Negative heals. Ignored while broken: a broken part starts fresh once fixed. */
	void AddWear(float Amount);

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

	bool HasAuthority() const { return GetOwner() && GetOwner()->HasAuthority(); }
	void SetWear(float NewWear);

	float TimeSinceFailure = 0.f;
	float LastRepairTime = -1000000.f;

	float Wear = 0.f;

	UPROPERTY(Replicated)
	uint8 RepWear = 0;

	/** Local world time this machine saw the break, for the loop's severity. */
	float LocalFailTime = 0.f;

	/** The failed loop's current damping (1 = full fire), eased toward WorkedLoopDamp while worked. */
	float LoopDamp = 1.f;
	FNBFeedbackLoop FailedLoop;
};
