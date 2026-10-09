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
	TimeUp,
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

	/** Seconds left on whichever clock the phase uses: countdown, deadline or restart. Never negative. */
	UFUNCTION(BlueprintPure, Category = "Run")
	float GetSecondsLeft() const;

	/** Seconds the total-wreck clock has run (0 while fewer than 3 parts are failed). */
	UFUNCTION(BlueprintPure, Category = "Run")
	float GetWreckSeconds() const { return WreckSeconds; }

	UFUNCTION(BlueprintPure, Category = "Run")
	int32 GetAcornsDelivered() const { return AcornsDelivered; }

	UFUNCTION(BlueprintPure, Category = "Run")
	int32 GetTimeBonus() const { return TimeBonus; }

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

	// --- Server only: written by the game mode. ---
	void SetPhase(ENBRunPhase NewPhase, double NewEndTime);
	void SetWreckSeconds(float Seconds) { WreckSeconds = Seconds; }
	void SetAwards(APlayerState* NewMvp, APlayerState* NewMostUseless) { Mvp = NewMvp; MostUseless = NewMostUseless; }
	void Finish(ENBRunResult NewResult, int32 Delivered, int32 NewTimeBonus, int32 NewPenalty, int32 NewScore, double RestartTime);

private:
	UPROPERTY(Replicated)
	ENBRunPhase Phase = ENBRunPhase::Waiting;

	UPROPERTY(Replicated)
	ENBRunResult Result = ENBRunResult::None;

	/** Server world time the current clock hits zero. */
	UPROPERTY(Replicated)
	double PhaseEndTime = 0.0;

	UPROPERTY(Replicated)
	float WreckSeconds = 0.f;

	UPROPERTY(Replicated)
	int32 AcornsDelivered = 0;

	UPROPERTY(Replicated)
	int32 TimeBonus = 0;

	UPROPERTY(Replicated)
	int32 RespawnPenalty = 0;

	UPROPERTY(Replicated)
	int32 FinalScore = 0;

	UPROPERTY(Replicated)
	TObjectPtr<APlayerState> Mvp;

	UPROPERTY(Replicated)
	TObjectPtr<APlayerState> MostUseless;
};
