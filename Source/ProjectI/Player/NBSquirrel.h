// Squirrel Wheels prototype.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "NBSquirrel.generated.h"

class ANBCar;
class UCameraComponent;
class UInputAction;
class UInputMappingContext;
class UNBInteractableComponent;
class UNBSeatComponent;
class USpringArmComponent;
class UStaticMeshComponent;
struct FInputActionValue;

/**
 * A tiny squirrel. On foot it is a normal character; when seated it is attached to a
 * car seat with movement and collision off, and its move input is sent to the server
 * as that seat's control axis (steer for the wheel, gas/brake for the pedals).
 */
UCLASS()
class PROJECTI_API ANBSquirrel : public ACharacter
{
	GENERATED_BODY()

public:
	ANBSquirrel();

	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	virtual void PawnClientRestart() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PossessedBy(AController* NewController) override;

	/** Which entry of FurColors this squirrel wears; unique per player. */
	UFUNCTION(BlueprintPure, Category = "Squirrel")
	int32 GetColorIndex() const { return ColorIndex; }

	UFUNCTION(BlueprintPure, Category = "Squirrel")
	UNBSeatComponent* GetCurrentSeat() const { return CurrentSeat; }

	UFUNCTION(BlueprintPure, Category = "Squirrel")
	bool IsSeated() const { return CurrentSeat != nullptr; }

	/** Server only. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Squirrel")
	void EnterSeat(UNBSeatComponent* Seat);

	/** Server only. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Squirrel")
	void LeaveSeat();

	/** Server only. Seated: jump straight to the next free seat on the same car. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Squirrel")
	void HopToNextSeat();

	/** What the action button would work on right now (nearest usable interactable). */
	UFUNCTION(BlueprintPure, Category = "Squirrel")
	UNBInteractableComponent* GetFocusedInteractable() const;

	/** Local feedback for the HUD: did this player's last timing-ring press land, and when. */
	bool GetLastRingPress(bool& bOutHit, double& OutWorldTime) const;

	/** Holding on to an interactable that carries the squirrel (e.g. a tire). */
	UFUNCTION(BlueprintPure, Category = "Squirrel")
	bool IsClinging() const { return ClingTarget != nullptr; }

	/** Server only. Called by an interactable when it drops this squirrel as a user. */
	void HandleInteractionEnded(UNBInteractableComponent* Interactable);

	/** Dev: fail a car part. Engine, Brakes, Door, TireFL/FR/BL/BR, Tire (random) or All. */
	UFUNCTION(Exec)
	void NBFail(const FString& PartName);

	/** Dev: toggle random part failures every few seconds. */
	UFUNCTION(Exec)
	void NBChaos();

protected:
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<USpringArmComponent> CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<UCameraComponent> FollowCamera;

	/** Greybox body parts. Replaced by a CC0 squirrel mesh later. */
	UPROPERTY(VisibleAnywhere, Category = "Squirrel")
	TObjectPtr<UStaticMeshComponent> BodyVisual;

	UPROPERTY(VisibleAnywhere, Category = "Squirrel")
	TObjectPtr<UStaticMeshComponent> TailVisual;

	UPROPERTY(EditAnywhere, Category = "Camera")
	float OnFootArmLength = 320.f;

	/** Pulled back while seated so the whole car is framed. */
	UPROPERTY(EditAnywhere, Category = "Camera")
	float SeatedArmLength = 1100.f;

	/** Bright, distinct colors so viewers can tell the tiny squirrels apart. */
	UPROPERTY(EditAnywhere, Category = "Squirrel")
	TArray<FLinearColor> FurColors;

	UPROPERTY(EditAnywhere, Category = "Interaction")
	float InteractRange = 300.f;

	UPROPERTY(EditAnywhere, Category = "Interaction")
	float ExitSideOffset = 190.f;

private:
	void BuildInputAssets();
	void Move(const FInputActionValue& Value);
	void MoveCompleted(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void JumpPressed();
	void Interact();
	void ActionPressed();
	void ActionReleased();
	void ReleaseActiveInteractable();
	void StartClinging(UNBInteractableComponent* Target);
	void StopClinging();
	void RefreshAttachedState();

	UFUNCTION(Server, Reliable)
	void Server_DevFail(const FString& PartName);

	UFUNCTION(Server, Reliable)
	void Server_DevChaos();

	UFUNCTION()
	void OnRep_ClingTarget();
	void SendSeatInput(float Value);
	void ApplySeatedState(bool bSeated);

	/** On foot: take the nearest free seat. Seated: hop to the next free seat. */
	UFUNCTION(Server, Reliable)
	void Server_Interact();

	UFUNCTION(Server, Reliable)
	void Server_LeaveSeat();

	UFUNCTION(Server, Unreliable)
	void Server_SetSeatInput(float Value);

	/** PressServerTime: the client's estimate of server world time at the press. */
	UFUNCTION(Server, Reliable)
	void Server_ActionPressed(double PressServerTime);

	UFUNCTION(Server, Reliable)
	void Server_ActionReleased();

	UFUNCTION()
	void OnRep_CurrentSeat();

	UFUNCTION()
	void OnRep_ColorIndex();

	void ApplyFurColor();

	ANBCar* GetSeatCar() const;

	UPROPERTY(ReplicatedUsing = OnRep_CurrentSeat)
	TObjectPtr<UNBSeatComponent> CurrentSeat;

	/** What the squirrel is clinging to, if anything. Replicated so clients stop simulating movement. */
	UPROPERTY(ReplicatedUsing = OnRep_ClingTarget)
	TObjectPtr<UNBInteractableComponent> ClingTarget;

	/** INDEX_NONE until the server assigns one on possession. */
	UPROPERTY(ReplicatedUsing = OnRep_ColorIndex)
	int32 ColorIndex = INDEX_NONE;

	// Input assets are built in code so the prototype needs no input .uassets.
	UPROPERTY(Transient)
	TObjectPtr<UInputMappingContext> InputContext;

	UPROPERTY(Transient)
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(Transient)
	TObjectPtr<UInputAction> LookAction;

	UPROPERTY(Transient)
	TObjectPtr<UInputAction> JumpAction;

	UPROPERTY(Transient)
	TObjectPtr<UInputAction> InteractAction;

	UPROPERTY(Transient)
	TObjectPtr<UInputAction> ActionAction;

	/** Server only. The interactable this squirrel is holding the action button on. */
	UPROPERTY(Transient)
	TObjectPtr<UNBInteractableComponent> ActiveInteractable;

	float LastSentSeatInput = 0.f;

	bool bHasRingPress = false;
	bool bLastRingPressHit = false;
	double LastRingPressTime = 0.0;
};
