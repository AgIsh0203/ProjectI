// Squirrel Wheels prototype.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "NBSquirrel.generated.h"

class ANBCar;
class UCameraComponent;
class UInputAction;
class UInputMappingContext;
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

	UPROPERTY(EditAnywhere, Category = "Interaction")
	float InteractRange = 300.f;

	UPROPERTY(EditAnywhere, Category = "Interaction")
	float ExitSideOffset = 190.f;

private:
	void BuildInputAssets();
	void Move(const FInputActionValue& Value);
	void MoveCompleted(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void Interact();
	void SendSeatInput(float Value);
	void ApplySeatedState(bool bSeated);

	UFUNCTION(Server, Reliable)
	void Server_Interact();

	UFUNCTION(Server, Unreliable)
	void Server_SetSeatInput(float Value);

	UFUNCTION()
	void OnRep_CurrentSeat();

	ANBCar* GetSeatCar() const;

	UPROPERTY(ReplicatedUsing = OnRep_CurrentSeat)
	TObjectPtr<UNBSeatComponent> CurrentSeat;

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

	float LastSentSeatInput = 0.f;
};
