// Squirrel Wheels prototype.

#pragma once

#include "CoreMinimal.h"
#include "Game/NBRunGameState.h"
#include "GameFramework/GameModeBase.h"
#include "NBRunGameMode.generated.h"

class ANBCar;
class ANBFinishZone;
class UNBFailureDirector;

/**
 * One run: squirrels spawn near the shared car, wait for a crew, count down, then drive the
 * acorns to the drop-off before the deadline while the failure director breaks things.
 * Ends on delivery, time up, or a total wreck, then restarts the map.
 */
UCLASS()
class PROJECTI_API ANBRunGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ANBRunGameMode();

	virtual void StartPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	UFUNCTION(BlueprintPure, Category = "Run")
	ANBCar* GetCar() const { return Car; }

	/** Dev: skip waiting for a crew and start the countdown now (NBStart). */
	void DevStartRun();

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Run")
	TSubclassOf<ANBCar> CarClass;

	/** Where the fallback car spawns, relative to the first player start. */
	UPROPERTY(EditDefaultsOnly, Category = "Run")
	FVector CarSpawnOffset = FVector(600.f, 0.f, 120.f);

	/** Squirrels needed before the countdown starts. */
	UPROPERTY(EditDefaultsOnly, Category = "Run")
	int32 MinPlayers = 2;

	UPROPERTY(EditDefaultsOnly, Category = "Run")
	float CountdownSeconds = 5.f;

	/** Time to reach the drop-off. */
	UPROPERTY(EditDefaultsOnly, Category = "Run")
	float RunSeconds = 150.f;

	/** At least this many parts failed for WreckSeconds in a row ends the run. */
	UPROPERTY(EditDefaultsOnly, Category = "Run")
	int32 WreckPartCount = 3;

	UPROPERTY(EditDefaultsOnly, Category = "Run")
	float WreckSeconds = 10.f;

	/** How long the end screen stays up before the map restarts. */
	UPROPERTY(EditDefaultsOnly, Category = "Run")
	float EndScreenSeconds = 12.f;

	UPROPERTY(EditDefaultsOnly, Category = "Run|Score")
	int32 PointsPerAcorn = 10;

	UPROPERTY(EditDefaultsOnly, Category = "Run|Score")
	int32 PointsPerSecondLeft = 2;

	/** Taken off for every squirrel that had to be respawned instead of climbing back in. */
	UPROPERTY(EditDefaultsOnly, Category = "Run|Score")
	int32 RespawnPenalty = 25;

	/** If the level has no ANBFinishZone, one is spawned this far ahead of the car's start. */
	UPROPERTY(EditDefaultsOnly, Category = "Run")
	float FallbackFinishDistance = 8000.f;

private:
	void FindOrSpawnCar();
	void FindOrSpawnFinishZone();
	void BeginCountdown();
	void BeginDriving();
	void EndRun(ENBRunResult Result);
	void TickWaiting();
	void TickDriving(float DeltaSeconds);

	UPROPERTY()
	TObjectPtr<ANBCar> Car;

	UPROPERTY()
	TObjectPtr<ANBFinishZone> FinishZone;

	UPROPERTY()
	TObjectPtr<UNBFailureDirector> Director;

	float WreckTimer = 0.f;
	bool bForceStart = false;

	double Now() const;
	ANBRunGameState* GetRunState() const;
};
