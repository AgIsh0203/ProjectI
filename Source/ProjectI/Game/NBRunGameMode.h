// Squirrel Wheels prototype.

#pragma once

#include "CoreMinimal.h"
#include "Game/NBRunGameState.h"
#include "GameFramework/GameModeBase.h"
#include "NBRunGameMode.generated.h"

class ANBCar;
class ANBFinishZone;
class ANBRoute;
class APlayerStart;
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

	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;
	virtual void StartPlay() override;
	virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;
	virtual void Tick(float DeltaSeconds) override;

	UFUNCTION(BlueprintPure, Category = "Run")
	ANBCar* GetCar() const { return Car; }

	/** The long course, or null when playing on the test track (?Route=0). */
	UFUNCTION(BlueprintPure, Category = "Run")
	ANBRoute* GetRoute() const { return Route; }

	/** Dev: skip waiting for a crew and start the countdown now (NBStart). */
	void DevStartRun();

	/** Dev: move the car to the start of a route section, by number (1-based) or name (NBWarp). */
	void DevWarp(const FString& Section);

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

	/** Time to reach the drop-off on the test track. On the route, the route's own deadline is used. */
	UPROPERTY(EditDefaultsOnly, Category = "Run")
	float RunSeconds = 150.f;

	/**
	 * Run on the long route (built in code, away from the test track's floor) instead of the
	 * test track. The URL option ?Route=0 turns it off for one session.
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Run|Route")
	bool bUseRoute = true;

	UPROPERTY(EditDefaultsOnly, Category = "Run|Route")
	TSubclassOf<ANBRoute> RouteClass;

	/** Where the route's start sits in the world: far enough out to clear L_TestTrack's floor. */
	UPROPERTY(EditDefaultsOnly, Category = "Run|Route")
	FVector RouteOrigin = FVector(0.f, 300000.f, 0.f);

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

	/** Acorn in Mouth: someone gets muted every MuteMinInterval..MuteMaxInterval seconds, for MuteSeconds. */
	UPROPERTY(EditDefaultsOnly, Category = "Run|Acorn")
	float MuteMinInterval = 75.f;

	UPROPERTY(EditDefaultsOnly, Category = "Run|Acorn")
	float MuteMaxInterval = 105.f;

	UPROPERTY(EditDefaultsOnly, Category = "Run|Acorn")
	float MuteSeconds = 20.f;

	/** If the level has no ANBFinishZone, one is spawned this far ahead of the car's start. */
	UPROPERTY(EditDefaultsOnly, Category = "Run")
	float FallbackFinishDistance = 8000.f;

private:
	/** Server: the route, spawned on first use (squirrels can spawn before StartPlay). */
	ANBRoute* EnsureRoute();
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
	TObjectPtr<ANBRoute> Route;

	/** Spawn spots beside the car at the route's start, handed out in turn. */
	UPROPERTY()
	TArray<TObjectPtr<APlayerStart>> RouteStarts;
	int32 NextRouteStart = 0;

	UPROPERTY()
	TObjectPtr<UNBFailureDirector> Director;

	/** Weighted pick: the Wheel seat and squirrels at a failing part are likelier to need to talk. */
	void GiveAcornInMouth();

	double NextMuteTime = 0.0;
	float WreckTimer = 0.f;
	bool bForceStart = false;

	double Now() const;
	ANBRunGameState* GetRunState() const;
};
