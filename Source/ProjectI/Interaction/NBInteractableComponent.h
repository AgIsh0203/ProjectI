// Squirrel Wheels prototype.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "NBInteractableComponent.generated.h"

class ANBSquirrel;
class UNBInteractableComponent;

/** How the action button drives an interactable's progress. */
UENUM(BlueprintType)
enum class ENBInteractMode : uint8
{
	/** Keep the button held for HoldSeconds. Progress drains while nobody holds. */
	Hold,
	/** Tap MashPresses times. Progress drains constantly. */
	Mash,
	/** Press while the sweeping marker is inside the sweet spot, RingHits times. */
	TimingRing,
	/** Like Hold, but only advances while RequiredUsers squirrels hold at once. */
	Push2,
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FNBInteractCompletedSignature, UNBInteractableComponent*, Interactable);

/**
 * A spot a squirrel can work on with the action button: repairs, the door, flipping the car.
 * Server-authoritative: clients only send press/release; progress, users and the ring's
 * sweet spot replicate back for the HUD. A squirrel uses the nearest enabled interactable
 * within Range of itself (seated squirrels use their seat's location).
 */
UCLASS(ClassGroup = (NB), meta = (BlueprintSpawnableComponent))
class PROJECTI_API UNBInteractableComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	UNBInteractableComponent();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/** The nearest enabled interactable Squirrel can reach, or nullptr. Valid on server and clients. */
	static UNBInteractableComponent* FindBestFor(const ANBSquirrel* Squirrel);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interact")
	ENBInteractMode Mode = ENBInteractMode::Hold;

	/** Shown on the HUD, e.g. "Cool engine". */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interact")
	FText Prompt;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interact")
	float Range = 160.f;

	/** Off after completing, until the owner re-enables it (e.g. when the part fails again). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interact")
	bool bDisableOnComplete = true;

	/** Seated squirrels can't use it; they have to get out (e.g. tires). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interact")
	bool bRequiresOnFoot = false;

	/** Hold / Push2: users cling to this component while holding, and get carried with it. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interact")
	bool bAttachUser = false;

	// --- Hold / Push2 ---

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interact|Hold", meta = (ClampMin = "0.1"))
	float HoldSeconds = 2.5f;

	/** Progress lost per second while not enough squirrels are holding. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interact|Hold", meta = (ClampMin = "0"))
	float HoldDrainPerSecond = 0.35f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interact|Hold", meta = (ClampMin = "2"))
	int32 RequiredUsers = 2;

	// --- Mash ---

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interact|Mash", meta = (ClampMin = "1"))
	int32 MashPresses = 14;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interact|Mash", meta = (ClampMin = "0"))
	float MashDrainPerSecond = 0.2f;

	// --- TimingRing ---

	/** Seconds for the marker to sweep once round the ring. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interact|TimingRing", meta = (ClampMin = "0.2"))
	float RingPeriod = 1.3f;

	/** Sweet spot width as a fraction of the ring. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interact|TimingRing", meta = (ClampMin = "0.02", ClampMax = "1"))
	float SweetSpotSize = 0.16f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interact|TimingRing", meta = (ClampMin = "1"))
	int32 RingHits = 3;

	/** Progress lost on a miss. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interact|TimingRing", meta = (ClampMin = "0"))
	float RingMissPenalty = 0.34f;

	/** Server only. Fires when progress reaches 1. */
	UPROPERTY(BlueprintAssignable, Category = "Interact")
	FNBInteractCompletedSignature OnCompleted;

	UFUNCTION(BlueprintPure, Category = "Interact")
	bool IsInteractEnabled() const { return bInteractEnabled; }

	/** Server only. Disabling also drops users and resets progress. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Interact")
	void SetInteractEnabled(bool bEnabled);

	UFUNCTION(BlueprintPure, Category = "Interact")
	float GetProgress() const { return Progress; }

	/** Squirrels currently holding (Hold / Push2). */
	UFUNCTION(BlueprintPure, Category = "Interact")
	int32 GetNumUsers() const { return Users.Num(); }

	UFUNCTION(BlueprintPure, Category = "Interact")
	bool IsUsedBy(const ANBSquirrel* Squirrel) const { return Users.Contains(Squirrel); }

	UFUNCTION(BlueprintPure, Category = "Interact")
	bool IsInRangeOf(const ANBSquirrel* Squirrel) const;

	/** Enabled, in range, and the squirrel is in a state that may use it. */
	UFUNCTION(BlueprintPure, Category = "Interact")
	bool CanBeUsedBy(const ANBSquirrel* Squirrel) const;

	/** Where the ring marker is now, 0..1. */
	UFUNCTION(BlueprintPure, Category = "Interact|TimingRing")
	float GetRingPhase() const;

	float GetRingPhaseAt(double ServerTime) const;

	UFUNCTION(BlueprintPure, Category = "Interact|TimingRing")
	float GetSweetSpotStart() const { return SweetSpotStart; }

	/** True if Phase (0..1) is inside the sweet spot, which may wrap past 1. */
	UFUNCTION(BlueprintPure, Category = "Interact|TimingRing")
	bool IsInSweetSpot(float Phase) const;

	/**
	 * Server only. Squirrel pressed the action button. PressServerTime is when the press
	 * happened in server world time (the timing ring is judged at that moment).
	 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Interact")
	void PressBy(ANBSquirrel* Squirrel, double PressServerTime);

	/** Server only. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Interact")
	void ReleaseBy(ANBSquirrel* Squirrel);

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	/** Server. Drops a holder and tells it so (e.g. to let go of a tire). */
	void RemoveUser(ANBSquirrel* Squirrel);
	void RemoveAllUsers();
	void AddProgress(float Delta);
	void Complete();
	void PickNewSweetSpot();
	double GetServerTime() const;

	UPROPERTY(Replicated)
	bool bInteractEnabled = true;

	UPROPERTY(Replicated)
	float Progress = 0.f;

	UPROPERTY(Replicated)
	TArray<TObjectPtr<ANBSquirrel>> Users;

	UPROPERTY(Replicated)
	float SweetSpotStart = 0.7f;
};
