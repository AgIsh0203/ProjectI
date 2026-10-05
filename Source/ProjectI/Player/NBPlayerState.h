// Squirrel Wheels prototype.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "NBPlayerState.generated.h"

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

private:
	UPROPERTY(Replicated)
	int32 Falls = 0;

	UPROPERTY(Replicated)
	int32 Respawns = 0;
};
