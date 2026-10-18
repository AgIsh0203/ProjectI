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

/** ANBRunGameMode::DevAutopilot's state. */
struct FNBAutopilot
{
	float Speed = 0.f;
	int32 Section = INDEX_NONE;
	double RunStart = 0.0, SectionStart = 0.0;
	float MinSpeed = 0.f, MaxSpeed = 0.f, ClimbMinSpeed = 0.f, MaxTilt = 0.f;
	int32 Acorns = 0, AirPoints = 0, Stucks = 0, TotalStucks = 0;
	float SlowSeconds = 0.f, ReverseSeconds = 0.f;
	float LastAlong = 0.f;
};

/**
 * One run: squirrels spawn near the shared car, wait in the lobby until the host starts,
 * count down, then drive the acorns to the drop-off before the deadline while the failure
 * director breaks things. Ends on delivery, time up, or a total wreck, then reloads the map
 * with seamless travel so the crew stays connected, and the next run starts by itself.
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

	/** Dev: skip waiting for a crew and start the countdown now (NBStart, the menu's "Practice alone"). */
	void DevStartRun();

	/** Dev (NBSoundTour): stop random failures, then play every in-game sound in turn. */
	void DevSoundTour();

	/** Dev (NBEnd): end the run now with this result, for the end screen and its jingle. */
	void DevEndRun(ENBRunResult Result);

	/** The host has a crew of at least MinPlayers and the run hasn't started. */
	bool CanStartRun() const;

	/** The host pressed "Start the run" in the lobby menu. Ignored until CanStartRun. */
	void StartRunFromLobby();

	/** Reload the map for a fresh run with the same crew (end of a run, or the host's menu). */
	void RestartRun();

	int32 GetMinPlayers() const { return MinPlayers; }

	/** The host's Manual gears option (lobby menu, ?ManualGears=1). Only while waiting: it locks at the countdown. */
	UFUNCTION(BlueprintCallable, Category = "Run")
	void SetManualGears(bool bManual);
	bool CanChangeManualGears() const;

	/** Dev: move the car to the start of a route section, by number (1-based) or name (NBWarp). */
	void DevWarp(const FString& Section);

	/**
	 * Dev (NBAuto, ?Autopilot=<cm/s> in the URL): the server drives the car down the route's centreline at
	 * up to Speed cm/s, with no random failures, logging each section ("NBAuto: ..."). Starts the run if it's
	 * waiting. 0 stops it. For checking the whole route drives with no stuck points.
	 */
	UFUNCTION(BlueprintCallable, Category = "Run|Dev")
	void DevAutopilot(float Speed);

	/**
	 * Dev, PIE only, never shipped: solo drive (console nb.SoloDrive 1, or NBSolo). One squirrel drives alone: WASD steer
	 * and gas/brake from any seat, Shift handbrake, mouse wheel shifts. The run starts with one squirrel, the host's squirrel
	 * is put in the WHEEL seat, and nothing breaks by itself (NBFail / NBChaos still work).
	 */
	static bool IsSoloDrive(const UWorld* World);

	/** Dev (NBSolo): turn solo drive on for this squirrel: seat it at the wheel, stop random failures, start the run. */
	void DevSolo(class ANBSquirrel* Driver);

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Run")
	TSubclassOf<ANBCar> CarClass;

	/** Where the fallback car spawns, relative to the first player start. */
	UPROPERTY(EditDefaultsOnly, Category = "Run")
	FVector CarSpawnOffset = FVector(600.f, 0.f, 120.f);

	/** Squirrels needed before the countdown can start. */
	UPROPERTY(EditDefaultsOnly, Category = "Run")
	int32 MinPlayers = 2;

	/**
	 * The first run waits for the host to press Start, so a crew of 3-4 can gather. Runs after
	 * a restart, and every run in PIE (so scripted tests need no click), start at MinPlayers.
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Run")
	bool bHostStartsRun = true;

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

	/** A delivered run with Manual gears on scores this many times as much (bad endings still score 0). */
	UPROPERTY(EditDefaultsOnly, Category = "Run|Score")
	float ManualGearsScoreScale = 1.25f;

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

	/** Server. Picks the MVP and "Most useless" from the player stats for the end screen. */
	void PickAwards();
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

	/** Solo drive: put the host's squirrel at the wheel if it isn't seated. */
	void SeatSoloDriver();

	/** Weighted pick: the Wheel seat and squirrels at a failing part are likelier to need to talk. */
	void GiveAcornInMouth();

	double NextMuteTime = 0.0;
	float WreckTimer = 0.f;
	bool bForceStart = false;
	/** Start the countdown as soon as MinPlayers are in, without the host's click. */
	bool bAutoStart = false;
	bool bRestarting = false;

	/** RestartRun removes the car, then travels on this timer (see RestartRun for why). */
	void TravelToRestart();
	FTimerHandle RestartTravelTimer;

	double Now() const;
	ANBRunGameState* GetRunState() const;

	/** DevAutopilot: steer toward the centreline ahead, slow for bends, back out when stuck, log per section. */
	void TickAutopilot(float DeltaSeconds);
	/** Logs the section the autopilot just left (or the run's end). */
	void LogAutopilotSection(const TCHAR* Why);

	FNBAutopilot Auto;

};
