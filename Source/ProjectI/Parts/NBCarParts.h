// Squirrel Wheels prototype.

#pragma once

#include "CoreMinimal.h"
#include "Parts/NBCarPartComponent.h"
#include "NBCarParts.generated.h"

/** Overheats: engine power fades, then it stalls. Mash to cool. */
UCLASS(ClassGroup = (NB), meta = (BlueprintSpawnableComponent))
class PROJECTI_API UNBEnginePart : public UNBCarPartComponent
{
	GENERATED_BODY()

public:
	UNBEnginePart();

	/** Power left just before stalling. */
	UPROPERTY(EditAnywhere, Category = "Part|Engine", meta = (ClampMin = "0", ClampMax = "1"))
	float MinPowerBeforeStall = 0.35f;

	/** Seconds to fade from full power to MinPowerBeforeStall. */
	UPROPERTY(EditAnywhere, Category = "Part|Engine", meta = (ClampMin = "0.1"))
	float FadeSeconds = 10.f;

	/** Seconds after failing that the engine stalls completely. */
	UPROPERTY(EditAnywhere, Category = "Part|Engine", meta = (ClampMin = "0.1"))
	float StallSeconds = 16.f;

protected:
	virtual void ApplyFailedEffect(float FailedSeconds) override;
	virtual void ClearEffect() override;
};

/** Brake fade: braking power drops, then goes. Pump with the timing ring. */
UCLASS(ClassGroup = (NB), meta = (BlueprintSpawnableComponent))
class PROJECTI_API UNBBrakePart : public UNBCarPartComponent
{
	GENERATED_BODY()

public:
	UNBBrakePart();

	UPROPERTY(EditAnywhere, Category = "Part|Brakes", meta = (ClampMin = "0", ClampMax = "1"))
	float FadedBrakePower = 0.25f;

	/** Seconds after failing that the brakes are gone completely. */
	UPROPERTY(EditAnywhere, Category = "Part|Brakes", meta = (ClampMin = "0.1"))
	float GoneSeconds = 12.f;

protected:
	virtual void ApplyFailedEffect(float FailedSeconds) override;
	virtual void ClearEffect() override;
};

/** Flat tire: that wheel loses grip. Patch it from outside the car by clinging and holding. */
UCLASS(ClassGroup = (NB), meta = (BlueprintSpawnableComponent))
class PROJECTI_API UNBTirePart : public UNBCarPartComponent
{
	GENERATED_BODY()

public:
	UNBTirePart();

	/** Chaos wheel this tire belongs to (matches the car's WheelSetups bone). */
	UPROPERTY(EditAnywhere, Category = "Part|Tire")
	FName WheelBone;

	UPROPERTY(EditAnywhere, Category = "Part|Tire", meta = (ClampMin = "0", ClampMax = "1"))
	float FlatGrip = 0.3f;

protected:
	virtual void BeginPlay() override;
	virtual void ApplyFailedEffect(float FailedSeconds) override;
	virtual void ClearEffect() override;

private:
	int32 WheelIndex = INDEX_NONE;
};

/** The door swings open and squirrels on its side fall out. Slam it shut. */
UCLASS(ClassGroup = (NB), meta = (BlueprintSpawnableComponent))
class PROJECTI_API UNBDoorPart : public UNBCarPartComponent
{
	GENERATED_BODY()

public:
	UNBDoorPart();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/** The hinge the door panel hangs from; rotated open/closed on every machine. */
	UPROPERTY(Transient)
	TObjectPtr<USceneComponent> Hinge;

	/** Hinge yaw when open (negative swings outwards on the car's right side). */
	UPROPERTY(EditAnywhere, Category = "Part|Door")
	float OpenYaw = -85.f;

	/** How often a squirrel on the door's side gets thrown out while it's open. */
	UPROPERTY(EditAnywhere, Category = "Part|Door", meta = (ClampMin = "0.1"))
	float EjectIntervalSeconds = 3.f;

	/** The car must be going at least this fast (cm/s) to throw anyone out. */
	UPROPERTY(EditAnywhere, Category = "Part|Door", meta = (ClampMin = "0"))
	float EjectMinSpeed = 300.f;

protected:
	virtual void BeginPlay() override;
	virtual void ApplyFailedEffect(float FailedSeconds) override;
	virtual void OnFailedChanged() override;

private:
	void EjectSquirrelOnDoorSide();

	float NextEjectTime = 0.f;
};
