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

USTRUCT()
struct FNBRagdollState
{
	GENERATED_BODY()

	UPROPERTY()
	bool bActive = false;

	UPROPERTY()
	FVector_NetQuantize Start;

	UPROPERTY()
	FVector_NetQuantize Velocity;
};

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

	/**
	 * Server only. Throw the squirrel out: it leaves its seat, tumbles as a physics body
	 * launched at LaunchVelocity, gets up after RagdollSeconds, and is respawned at the car
	 * if it isn't back in a seat RespawnDelay seconds after the fall.
	 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Squirrel")
	void Eject(FVector LaunchVelocity);

	/** Server only. Eject with the car's momentum plus a kick out of the seat's side. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Squirrel")
	void EjectFromCar();

	/** Server only. Put the squirrel back at the car now (a free seat, else beside it). */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Squirrel")
	void RespawnAtCar();

	UFUNCTION(BlueprintPure, Category = "Squirrel")
	bool IsRagdolled() const { return Ragdoll.bActive; }

	/** Seconds until the automatic respawn after a fall, or a negative number if none is pending. */
	UFUNCTION(BlueprintPure, Category = "Squirrel")
	float GetRespawnSecondsLeft() const;

	virtual void Tick(float DeltaSeconds) override;
	virtual void FellOutOfWorld(const UDamageType& DamageType) override;

	/** Dev: fail a car part. Engine, Brakes, Door, TireFL/FR/BL/BR, Tire (random) or All. */
	UFUNCTION(Exec)
	void NBFail(const FString& PartName);

	/** Dev: toggle random part failures every few seconds. */
	UFUNCTION(Exec)
	void NBChaos();

	/** Dev: roll the car onto its roof. */
	UFUNCTION(Exec)
	void NBFlip();

	/** Host a lobby friends can join (H). */
	UFUNCTION(Exec)
	void NBHost();

	/** Find and join a lobby (J). Steam invites join automatically. */
	UFUNCTION(Exec)
	void NBJoin();

protected:
	virtual void BeginPlay() override;
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

	/** How long a thrown-out squirrel tumbles before getting up. */
	UPROPERTY(EditAnywhere, Category = "Falling")
	float RagdollSeconds = 2.5f;

	/** Seconds after a fall to get back in a seat before being respawned (with a penalty). */
	UPROPERTY(EditAnywhere, Category = "Falling")
	float RespawnDelay = 8.f;

	/** Jumping out of a car going faster than this (cm/s) throws you out instead of hopping. */
	UPROPERTY(EditAnywhere, Category = "Falling")
	float BailSpeed = 600.f;

	/** Extra kick on top of the car's velocity when thrown out: sideways and up (cm/s). */
	UPROPERTY(EditAnywhere, Category = "Falling")
	FVector2D EjectKick = FVector2D(350.f, 450.f);

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

	UFUNCTION(Server, Reliable)
	void Server_DevFlip();

	UFUNCTION()
	void OnRep_ClingTarget();
	void SendSeatInput(float Value);
	void StartLocalRagdoll();
	void StopLocalRagdoll();
	void Recover();
	void ClearRespawn();
	void SnapTo(const FVector& Location, const FRotator& Rotation);

	UFUNCTION(Client, Reliable)
	void Client_SnapTo(FVector_NetQuantize Location, FRotator Rotation);

	UFUNCTION()
	void OnRep_Ragdoll();

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

	/** Replicated launch so every machine tumbles the same body from the same start. */
	UPROPERTY(ReplicatedUsing = OnRep_Ragdoll)
	FNBRagdollState Ragdoll;

	/** Server world time of the pending respawn; 0 when none. */
	UPROPERTY(Replicated)
	double RespawnServerTime = 0.0;

	/** Whether this machine is currently simulating the tumbling body. */
	bool bLocalRagdoll = false;

	FTransform DefaultBodyTransform;
	FTransform DefaultTailTransform;
	FTimerHandle RecoverTimer;
	FTimerHandle RespawnTimer;

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

	UPROPERTY(Transient)
	TObjectPtr<UInputAction> HostAction;

	UPROPERTY(Transient)
	TObjectPtr<UInputAction> JoinAction;

	/** Server only. The interactable this squirrel is holding the action button on. */
	UPROPERTY(Transient)
	TObjectPtr<UNBInteractableComponent> ActiveInteractable;

	float LastSentSeatInput = 0.f;

	bool bHasRingPress = false;
	bool bLastRingPressHit = false;
	double LastRingPressTime = 0.0;
};
