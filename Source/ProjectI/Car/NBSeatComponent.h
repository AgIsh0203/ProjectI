// Squirrel Wheels prototype.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "NBSeatComponent.generated.h"

class ANBSquirrel;

/** What a seated squirrel controls. */
UENUM(BlueprintType)
enum class ENBSeatRole : uint8
{
	Wheel  UMETA(DisplayName = "Wheel (steer)"),
	Pedals UMETA(DisplayName = "Pedals (gas/brake)"),
	Rider  UMETA(DisplayName = "Rider (no control)"),
};

/**
 * A spot on the car a squirrel can occupy. The squirrel attaches to this component,
 * so its transform is where the squirrel sits. Occupancy is server-authoritative.
 */
UCLASS(ClassGroup = (NB), meta = (BlueprintSpawnableComponent))
class PROJECTI_API UNBSeatComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	UNBSeatComponent();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Seat")
	ENBSeatRole Role = ENBSeatRole::Wheel;

	UFUNCTION(BlueprintPure, Category = "Seat")
	ANBSquirrel* GetOccupant() const { return Occupant; }

	UFUNCTION(BlueprintPure, Category = "Seat")
	bool IsFree() const { return Occupant == nullptr; }

	/** Server only. Pass nullptr to vacate. */
	void SetOccupant(ANBSquirrel* NewOccupant);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
	UPROPERTY(Replicated)
	TObjectPtr<ANBSquirrel> Occupant;
};
