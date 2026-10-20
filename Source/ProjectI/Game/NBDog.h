// Squirrel Wheels prototype.

#pragma once

#include "CoreMinimal.h"
#include "FX/NBFeedback.h"
#include "GameFramework/Actor.h"
#include "NBDog.generated.h"

class ANBCar;
class ANBRoute;
class ANBSquirrel;
class UNBTirePart;
class UStaticMeshComponent;

UENUM()
enum class ENBDogState : uint8
{
	/** Curled up at its kennel: no run, or the car isn't on its stretch. */
	Sleeping,
	/** After the car: runs at the nearest tyre, or at a squirrel on foot nearby. */
	Chasing,
	/** Hanging off a tyre: wears it, and may leave it flat. */
	Biting,
	/** A honk: runs away from the car for ScaredSeconds. */
	Scared,
	/** The car left its stretch: trots back to the kennel. */
	GoingHome,
};

/**
 * Roadmap #13, "one hazard with a face": a dog that lives beside WIGGLY ROAD and chases the car
 * while it's on that stretch. It bites a tyre (wear, sometimes a flat) or grabs a squirrel on foot
 * and flings it; a honk nearby scares it off for a while, so the horn gets a job.
 *
 * Server-authoritative and kinematic (no physics, no collision): the server moves the actor along
 * the ground with a downward trace and replicates movement; every other machine eases a visual
 * root after it. The body is greybox basic shapes, animated in code, until a model is imported.
 */
UCLASS()
class PROJECTI_API ANBDog : public AActor
{
	GENERATED_BODY()

public:
	ANBDog();

	virtual void Tick(float DeltaSeconds) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/** Server. Where the dog lives, and the route distances of the stretch it guards. */
	void SetHome(const FTransform& Home, float ZoneStart, float ZoneEnd);

	ENBDogState GetState() const { return State; }

	/** The tyre it's biting (replicated), or null. */
	const UNBTirePart* GetBiteTire() const { return BiteTire; }

	/** Local world time this machine last heard it bark, for the HUD's WOOF! bubble. */
	double GetLastBarkTime() const { return LastBarkTime; }

	/** Where the HUD hangs its bubble (above the head, smoothed on clients). */
	FVector GetBubbleLocation() const;

	/** Dev (server): wake up and chase now, wherever the car is. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Dog")
	void DevChase();

	// --- Tuning ---

	/** Chasing speed (cm/s). The car's top speed is ~1650, so a car flat out on a straight escapes. */
	UPROPERTY(EditAnywhere, Category = "Dog")
	float RunSpeed = 1450.f;

	/** Trotting home (cm/s). */
	UPROPERTY(EditAnywhere, Category = "Dog")
	float TrotSpeed = 500.f;

	/** The stretch it guards starts this far before the zone start given by the route (cm). */
	UPROPERTY(EditAnywhere, Category = "Dog")
	float ZoneMargin = 5000.f;

	/** Horizontal distance to a tyre that starts a bite (cm). */
	UPROPERTY(EditAnywhere, Category = "Dog")
	float BiteRange = 140.f;

	/** How long it hangs off a tyre (s). */
	UPROPERTY(EditAnywhere, Category = "Dog")
	float BiteSeconds = 2.5f;

	/** Tyre wear per second while biting (wear is 0..1). */
	UPROPERTY(EditAnywhere, Category = "Dog")
	float BiteWearRate = 0.15f;

	/** Chance a bite leaves the tyre flat when it lets go. */
	UPROPERTY(EditAnywhere, Category = "Dog")
	float FlatChance = 0.35f;

	/** No new bite for this long after letting go (s). */
	UPROPERTY(EditAnywhere, Category = "Dog")
	float BiteCooldown = 5.f;

	/** A squirrel on foot within this distance of the dog draws it off the car (cm). */
	UPROPERTY(EditAnywhere, Category = "Dog")
	float SquirrelInterest = 900.f;

	/** Grabbing distance for a squirrel on foot (cm). */
	UPROPERTY(EditAnywhere, Category = "Dog")
	float GrabRange = 110.f;

	/** The fling: speed along the dog's heading, and up (cm/s). */
	UPROPERTY(EditAnywhere, Category = "Dog")
	FVector2D FlingVelocity = FVector2D(700.f, 550.f);

	/** A honk from a car within this distance scares it (cm). */
	UPROPERTY(EditAnywhere, Category = "Dog")
	float ScareRadius = 2500.f;

	/** How long a honk keeps it away (s). */
	UPROPERTY(EditAnywhere, Category = "Dog")
	float ScaredSeconds = 10.f;

	/** Seconds between barks while chasing (random in this range). */
	UPROPERTY(EditAnywhere, Category = "Dog")
	FVector2D BarkInterval = FVector2D(2.f, 3.5f);

	/** Fur colour of the greybox body. */
	UPROPERTY(EditAnywhere, Category = "Dog")
	FLinearColor FurColor = FLinearColor(0.42f, 0.27f, 0.13f);

	// --- Sound / VFX slots (empty until assets are assigned). ---

	UPROPERTY(EditAnywhere, Category = "Dog|Feedback")
	FNBFeedback BarkFeedback;

	UPROPERTY(EditAnywhere, Category = "Dog|Feedback")
	FNBFeedback BiteFeedback;

	UPROPERTY(EditAnywhere, Category = "Dog|Feedback")
	FNBFeedback YelpFeedback;

protected:
	virtual void BeginPlay() override;

private:
	enum class EDogSound : uint8 { Bark, Bite, Yelp };

	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_Sound(uint8 Sound);

	void SetState(ENBDogState NewState);
	ANBCar* FindCar();
	/** The run is on and the car is on the dog's stretch. */
	bool IsCarInZone(const ANBCar& Car) const;
	/** Steps towards Target at Speed over the ground, facing the way it moves. Returns the horizontal distance left. */
	float MoveTowards(const FVector& Target, float Speed, float DeltaSeconds);
	void SnapToGround(FVector& Location) const;
	UNBTirePart* PickTire(const ANBCar& Car) const;
	/** The spot beside the tyre the dog runs at / hangs on: just outside the wheel. */
	FVector TireSpot(const ANBCar& Car, const UNBTirePart& Tire) const;
	ANBSquirrel* FindSquirrelOnFoot() const;
	void UpdateServer(float DeltaSeconds);
	void UpdateVisuals(float DeltaSeconds);

	UStaticMeshComponent* MakePart(FName Name, USceneComponent* Parent, UStaticMesh* Mesh, const FVector& Location, const FVector& Scale);

	UPROPERTY(VisibleAnywhere, Category = "Dog")
	TObjectPtr<USceneComponent> Root;

	/** Everything visible hangs here; on clients it eases after the replicated actor. */
	UPROPERTY(VisibleAnywhere, Category = "Dog")
	TObjectPtr<USceneComponent> VisualRoot;

	UPROPERTY(VisibleAnywhere, Category = "Dog")
	TObjectPtr<UStaticMeshComponent> Body;

	UPROPERTY(VisibleAnywhere, Category = "Dog")
	TObjectPtr<USceneComponent> HeadRoot;

	UPROPERTY(VisibleAnywhere, Category = "Dog")
	TObjectPtr<UStaticMeshComponent> Tail;

	UPROPERTY(VisibleAnywhere, Category = "Dog")
	TArray<TObjectPtr<UStaticMeshComponent>> Legs;

	UPROPERTY(Replicated)
	ENBDogState State = ENBDogState::Sleeping;

	UPROPERTY(Replicated)
	TObjectPtr<UNBTirePart> BiteTire;

	// Server.
	TWeakObjectPtr<ANBCar> Car;
	FTransform HomeTransform = FTransform::Identity;
	float ZoneStartDistance = 0.f;
	float ZoneEndDistance = -1.f;
	bool bDevChase = false;
	double StateEndTime = 0.0;
	double NextBiteTime = 0.0;
	double NextBarkTime = 0.0;
	double LastHornSeen = 0.0;

	// Every machine (cosmetic).
	double LastBarkTime = -1000.0;
	FVector SmoothedLocation = FVector::ZeroVector;
	FRotator SmoothedRotation = FRotator::ZeroRotator;
	FVector LastVisualLocation = FVector::ZeroVector;
	float VisualSpeed = 0.f;
	float GaitPhase = 0.f;
	bool bVisualsPlaced = false;
	TArray<FTransform> LegRest;
};
