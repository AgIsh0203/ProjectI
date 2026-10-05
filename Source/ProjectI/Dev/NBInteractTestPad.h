// Squirrel Wheels prototype.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NBInteractTestPad.generated.h"

class UNBInteractableComponent;
class UStaticMeshComponent;
class UTextRenderComponent;

/**
 * Dev-only: a labelled block with one interactable, for trying interaction modes on the
 * test track. Re-arms itself shortly after each completion.
 */
UCLASS()
class PROJECTI_API ANBInteractTestPad : public AActor
{
	GENERATED_BODY()

public:
	ANBInteractTestPad();

	virtual void OnConstruction(const FTransform& Transform) override;

	/** How many times it has been completed (server). Handy for automated checks. */
	UFUNCTION(BlueprintPure, Category = "Dev")
	int32 GetCompletions() const { return Completions; }

	UFUNCTION(BlueprintPure, Category = "Dev")
	UNBInteractableComponent* GetInteractable() const { return Interactable; }

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, Category = "Dev")
	TObjectPtr<UStaticMeshComponent> Block;

	UPROPERTY(VisibleAnywhere, Category = "Dev")
	TObjectPtr<UTextRenderComponent> Label;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dev")
	TObjectPtr<UNBInteractableComponent> Interactable;

	UPROPERTY(EditAnywhere, Category = "Dev")
	float RearmSeconds = 1.5f;

private:
	UFUNCTION()
	void HandleCompleted(UNBInteractableComponent* Completed);

	void Rearm();

	int32 Completions = 0;
	FTimerHandle RearmTimer;
};
