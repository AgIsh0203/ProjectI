// Squirrel Wheels prototype.

#pragma once

#include "CoreMinimal.h"
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

	/** Short name for the HUD, e.g. "ENGINE". */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Part")
	FText PartName;

	/** What's wrong while failed, e.g. "OVERHEATING". */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Part")
	FText FailureText;

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

	UFUNCTION()
	void HandleRepairCompleted(UNBInteractableComponent* Interactable);

	UPROPERTY(ReplicatedUsing = OnRep_Failed)
	bool bFailed = false;

	float TimeSinceFailure = 0.f;
	float LastRepairTime = -1000000.f;
};
