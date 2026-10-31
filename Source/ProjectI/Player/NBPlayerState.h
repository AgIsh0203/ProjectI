// Squirrel Wheels prototype.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "NBPlayerState.generated.h"

/** A funny end-screen award (#10); each squirrel gets at most one. */
UENUM(BlueprintType)
enum class ENBAward : uint8
{
	None,
	FrequentFlyer,   // most falls
	Butterfingers,   // most acorns spilled
	Firefighter,     // most fires put out
	BackseatDriver,  // most pings sent
	LongestFlight,   // farthest single throw
};

/** Per-player run stats, replicated for the HUD and the end screen. */
UCLASS()
class PROJECTI_API ANBPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/** Times this squirrel was thrown out of the car. */
	UFUNCTION(BlueprintPure, Category = "Stats")
	int32 GetFalls() const { return Falls; }

	/** Times it didn't make it back in time and was respawned (costs score in M2). */
	UFUNCTION(BlueprintPure, Category = "Stats")
	int32 GetRespawns() const { return Respawns; }

	/** Server only. */
	void AddFall() { ++Falls; }

	/** Server only. */
	void AddRespawn() { ++Respawns; }

	/** Broken parts this squirrel helped fix ("fires put out"). */
	UFUNCTION(BlueprintPure, Category = "Stats")
	int32 GetFiresPutOut() const { return FiresPutOut; }

	/** Acorns that flew out of the bed while this squirrel was driving. */
	UFUNCTION(BlueprintPure, Category = "Stats")
	int32 GetAcornsSpilled() const { return AcornsSpilled; }

	/** Whole seconds spent in the Wheel or Pedals seat during the run. */
	UFUNCTION(BlueprintPure, Category = "Stats")
	int32 GetDriveSeconds() const { return DriveSeconds; }

	/** Chat-wheel pings this squirrel sent. */
	UFUNCTION(BlueprintPure, Category = "Stats")
	int32 GetPingsSent() const { return PingsSent; }

	/** Farthest single throw out of the car, in cm (ground distance from launch to landing). */
	UFUNCTION(BlueprintPure, Category = "Stats")
	float GetLongestFlight() const { return LongestFlight; }

	/** Server only. Keeps the longest. */
	void NoteFlight(float Distance) { LongestFlight = FMath::Max(LongestFlight, Distance); }

	/** The end-screen award the game mode gave this squirrel, or None. */
	UFUNCTION(BlueprintPure, Category = "Stats")
	ENBAward GetAward() const { return Award; }

	/** The stat behind an award, for picking it and for the end screen. */
	float GetAwardStat(ENBAward Which) const;

	/** The award's name and the stat behind it, e.g. "FREQUENT FLYER  (5 falls)". */
	static FString DescribeAward(ENBAward Which, float Stat);

	/** Server only. */
	void SetAward(ENBAward NewAward) { Award = NewAward; }

	/** Server only. */
	void AddFirePutOut() { ++FiresPutOut; }

	/** Server only. */
	void AddAcornsSpilled(int32 Count) { AcornsSpilled += Count; }

	/** Server only. Accumulates exactly; replicates whole seconds. */
	void AddDriveTime(float Seconds);

	/**
	 * How much this squirrel helped, for picking the MVP (highest) and "Most useless"
	 * (lowest). Repairs and driving count up; falls, respawns and spilled acorns down.
	 */
	float GetHelpfulness() const;

	// --- Acorn in Mouth: the voice is muffled for a while (muted until 31 Oct); the chat wheel still works. ---

	/** True while this squirrel has the acorn: every listener muffles its voice (UNBVoiceTalker). */
	UFUNCTION(BlueprintPure, Category = "Stats")
	bool IsMuted() const;

	UFUNCTION(BlueprintPure, Category = "Stats")
	float GetMuteSecondsLeft() const;

	/** Server only. */
	void StartMute(float Seconds);

	/** Server only. */
	void ClearMute() { MuteEndTime = 0.0; }

	// --- Chat wheel: the last ping this squirrel sent. ---

	static constexpr int32 NumPings = 8;

	/** Text for ping slot 0..NumPings-1. */
	static FText GetPingText(int32 Index);

	/** Slot of the ping still on screen, or INDEX_NONE. */
	int32 GetActivePing() const;

	/** Server only. Returns false if the squirrel pinged too recently. */
	bool SendPing(int32 Index);

private:
	static constexpr float PingShowSeconds = 4.f;
	static constexpr float PingCooldownSeconds = 1.f;

	double ServerNow() const;

	UPROPERTY(Replicated)
	double MuteEndTime = 0.0;

	UPROPERTY(Replicated)
	int32 PingIndex = INDEX_NONE;

	UPROPERTY(Replicated)
	double PingTime = -1000.0;

	UPROPERTY(Replicated)
	int32 Falls = 0;

	UPROPERTY(Replicated)
	int32 Respawns = 0;

	UPROPERTY(Replicated)
	int32 FiresPutOut = 0;

	UPROPERTY(Replicated)
	int32 AcornsSpilled = 0;

	UPROPERTY(Replicated)
	int32 DriveSeconds = 0;

	UPROPERTY(Replicated)
	int32 PingsSent = 0;

	UPROPERTY(Replicated)
	float LongestFlight = 0.f;

	UPROPERTY(Replicated)
	ENBAward Award = ENBAward::None;

	/** Server. Exact drive time; DriveSeconds is its whole part. */
	float DriveTime = 0.f;
};
