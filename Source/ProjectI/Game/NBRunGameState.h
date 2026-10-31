// Squirrel Wheels prototype.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "NBRunGameState.generated.h"

class APlayerState;

UENUM(BlueprintType)
enum class ENBRunPhase : uint8
{
	/** Not enough squirrels yet. */
	Waiting,
	Countdown,
	Driving,
	/** Ended; the end screen is up until the restart. */
	Finished,
};

UENUM(BlueprintType)
enum class ENBRunResult : uint8
{
	None,
	Delivered,
	Wrecked,
};

/** Replicated state of the current run: phase, clocks and the final score. Written by ANBRunGameMode. */
UCLASS()
class PROJECTI_API ANBRunGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintPure, Category = "Run")
	ENBRunPhase GetPhase() const { return Phase; }

	UFUNCTION(BlueprintPure, Category = "Run")
	ENBRunResult GetResult() const { return Result; }

	/** Seconds left on the countdown or before the restart. Never negative; 0 while waiting or driving. */
	UFUNCTION(BlueprintPure, Category = "Run")
	float GetSecondsLeft() const;

	/** The stopwatch: seconds since the run started driving, frozen when it ends (31 Oct: no deadline). */
	UFUNCTION(BlueprintPure, Category = "Run")
	float GetRunSeconds() const;

	/** The route's par time: a delivered run scores a bonus per second under it. */
	UFUNCTION(BlueprintPure, Category = "Run")
	float GetParSeconds() const { return ParSeconds; }

	/** Seconds the total-wreck clock has run (0 while fewer than 3 parts are failed). */
	UFUNCTION(BlueprintPure, Category = "Run")
	float GetWreckSeconds() const { return WreckSeconds; }

	UFUNCTION(BlueprintPure, Category = "Run")
	int32 GetAcornsDelivered() const { return AcornsDelivered; }

	UFUNCTION(BlueprintPure, Category = "Run")
	int32 GetTimeBonus() const { return TimeBonus; }

	/** Points from clean jump landings (ANBCar::GetAirPoints). */
	UFUNCTION(BlueprintPure, Category = "Run")
	int32 GetAirBonus() const { return AirBonus; }

	UFUNCTION(BlueprintPure, Category = "Run")
	int32 GetRespawnPenalty() const { return RespawnPenalty; }

	UFUNCTION(BlueprintPure, Category = "Run")
	int32 GetFinalScore() const { return FinalScore; }

	/** End-screen badge; null with fewer than 2 squirrels. */
	UFUNCTION(BlueprintPure, Category = "Run")
	APlayerState* GetMvp() const { return Mvp; }

	/** End-screen badge; null with fewer than 2 squirrels. */
	UFUNCTION(BlueprintPure, Category = "Run")
	APlayerState* GetMostUseless() const { return MostUseless; }

	/** The host's Manual gears option (G5): the Gears seat shifts, and a delivered run scores x GetManualGearsScale. */
	UFUNCTION(BlueprintPure, Category = "Run")
	bool IsManualGears() const { return bManualGears; }

	float GetManualGearsScale() const { return ManualGearsScale; }

	// --- Server only: written by the game mode. ---
	void SetManualGears(bool bManual, float ScoreScale) { bManualGears = bManual; ManualGearsScale = ScoreScale; }
	void SetPhase(ENBRunPhase NewPhase, double NewEndTime);
	void StartDriving(float NewParSeconds);
	void SetWreckSeconds(float Seconds) { WreckSeconds = Seconds; }
	void SetAwards(APlayerState* NewMvp, APlayerState* NewMostUseless) { Mvp = NewMvp; MostUseless = NewMostUseless; }
	void Finish(ENBRunResult NewResult, int32 Delivered, int32 NewTimeBonus, int32 NewAirBonus, int32 NewPenalty, int32 NewScore, double RestartTime);

private:
	UPROPERTY(Replicated)
	ENBRunPhase Phase = ENBRunPhase::Waiting;

	UPROPERTY(Replicated)
	ENBRunResult Result = ENBRunResult::None;

	/** Server world time the countdown or restart clock hits zero. */
	UPROPERTY(Replicated)
	double PhaseEndTime = 0.0;

	/** Server world time the run started driving. */
	UPROPERTY(Replicated)
	double DriveStartTime = 0.0;

	/** The stopwatch at the end of the run. */
	UPROPERTY(Replicated)
	float FinalRunSeconds = 0.f;

	UPROPERTY(Replicated)
	float ParSeconds = 0.f;

	UPROPERTY(Replicated)
	float WreckSeconds = 0.f;

	UPROPERTY(Replicated)
	int32 AcornsDelivered = 0;

	UPROPERTY(Replicated)
	int32 TimeBonus = 0;

	UPROPERTY(Replicated)
	int32 AirBonus = 0;

	UPROPERTY(Replicated)
	int32 RespawnPenalty = 0;

	UPROPERTY(Replicated)
	int32 FinalScore = 0;

	UPROPERTY(Replicated)
	TObjectPtr<APlayerState> Mvp;

	UPROPERTY(Replicated)
	TObjectPtr<APlayerState> MostUseless;

	UPROPERTY(Replicated)
	bool bManualGears = false;

	UPROPERTY(Replicated)
	float ManualGearsScale = 1.f;
};
