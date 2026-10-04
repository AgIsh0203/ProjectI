// Squirrel Wheels prototype.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "NBRunGameMode.generated.h"

class ANBCar;

/** One run: squirrels spawn near the shared car. Spawns a car if the level has none. */
UCLASS()
class PROJECTI_API ANBRunGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ANBRunGameMode();

	virtual void StartPlay() override;

	UFUNCTION(BlueprintPure, Category = "Run")
	ANBCar* GetCar() const { return Car; }

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Run")
	TSubclassOf<ANBCar> CarClass;

	/** Where the fallback car spawns, relative to the first player start. */
	UPROPERTY(EditDefaultsOnly, Category = "Run")
	FVector CarSpawnOffset = FVector(600.f, 0.f, 120.f);

private:
	UPROPERTY()
	TObjectPtr<ANBCar> Car;
};
