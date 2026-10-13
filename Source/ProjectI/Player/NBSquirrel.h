// Squirrel Wheels prototype.

#pragma once

#include "CoreMinimal.h"
#include "FX/NBFeedback.h"
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
	virtual void OnRep_PlayerState() override;

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

	/** Local only: jolt this player's camera (0..1), e.g. on a crash. Decays by itself. */
	void AddCameraPunch(float Intensity);

	/** Dev: fail a car part. Engine, Brakes, Door, TireFL/FR/BL/BR, Tire (random) or All. */
	UFUNCTION(Exec)
	void NBFail(const FString& PartName);

	/** Dev: toggle random part failures every few seconds. */
	UFUNCTION(Exec)
	void NBChaos();

	/** Dev: roll the car onto its roof. */
	UFUNCTION(Exec)
	void NBFlip();

	/** Dev: play every in-game sound once, in turn, labelled on screen (host). */
	UFUNCTION(Exec)
	void NBSoundTour();

	/** Dev: end the run now: Delivered, TimeUp or Wrecked (host). */
	UFUNCTION(Exec)
	void NBEnd(const FString& Result);

	/** Server only. Dev sound tour: the grab chatter, which is otherwise private to the squirrel. */
	void DevPlayGrabFeedback() { Multicast_Grabbed(); }

	/** Dev: start the countdown without waiting for a second squirrel. */
	UFUNCTION(Exec)
	void NBStart();

	/** Dev: move the car to a route section, by number (1-based) or part of its name, e.g. NBWarp 7 or NBWarp jump. */
	UFUNCTION(Exec)
	void NBWarp(const FString& Section);

	/** Dev: host a lobby friends can join (the menu's "Host a game"). */
	UFUNCTION(Exec)
	void NBHost();

	/** Dev: find and join a lobby (the menu's "Join a game"). Steam invites join automatically. */
	UFUNCTION(Exec)
	void NBJoin();

	/** Open or close the lobby menu (Esc / gamepad Start; Esc stops PIE, so use this there). */
	UFUNCTION(Exec)
	void NBMenu();

protected:
	virtual void BeginPlay() override;

	/** Plays this squirrel's voice from its body, fading with distance. */
	UPROPERTY(VisibleAnywhere, Category = "Voice")
	TObjectPtr<class UVOIPTalker> VoiceTalker;

	/** Voice carries this far at full volume, then fades out over VoiceFalloff (cm). */
	UPROPERTY(EditAnywhere, Category = "Voice")
	float VoiceFullRadius = 800.f;

	UPROPERTY(EditAnywhere, Category = "Voice")
	float VoiceFalloff = 2500.f;
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

	/** Cream patch on the front of the body. */
	UPROPERTY(VisibleAnywhere, Category = "Squirrel")
	TObjectPtr<UStaticMeshComponent> BellyVisual;

	/** Unscaled pivot for the head, so the eyes and ears aren't stretched by the body's scale. */
	UPROPERTY(VisibleAnywhere, Category = "Squirrel")
	TObjectPtr<USceneComponent> HeadRoot;

	UPROPERTY(VisibleAnywhere, Category = "Squirrel")
	TObjectPtr<UStaticMeshComponent> HeadVisual;

	UPROPERTY(VisibleAnywhere, Category = "Squirrel")
	TObjectPtr<UStaticMeshComponent> NoseVisual;

	/** Left, right. */
	UPROPERTY(VisibleAnywhere, Category = "Squirrel")
	TObjectPtr<UStaticMeshComponent> EarVisuals[2];

	UPROPERTY(VisibleAnywhere, Category = "Squirrel")
	TObjectPtr<UStaticMeshComponent> EyeVisuals[2];

	/** Googly: they slosh around inside the eyes with the squirrel's acceleration. */
	UPROPERTY(VisibleAnywhere, Category = "Squirrel")
	TObjectPtr<UStaticMeshComponent> PupilVisuals[2];

	UPROPERTY(EditAnywhere, Category = "Camera")
	float OnFootArmLength = 320.f;

	/** Pulled back while seated so the whole car is framed. */
	UPROPERTY(EditAnywhere, Category = "Camera")
	float SeatedArmLength = 1100.f;

	/** Seated, the camera swings back behind the car once the mouse/stick has been idle this long (s). */
	UPROPERTY(EditAnywhere, Category = "Camera")
	float SeatedFollowDelay = 1.f;

	/** How fast the seated camera swings toward the car's heading (RInterpTo speed). */
	UPROPERTY(EditAnywhere, Category = "Camera")
	float SeatedFollowSpeed = 2.5f;

	/** Below this car speed (cm/s) the seated camera stays put, so a car spinning in place doesn't drag it around. */
	UPROPERTY(EditAnywhere, Category = "Camera")
	float SeatedFollowMinSpeed = 150.f;

	/** In the pedal seat the tail sticks straight up like a flag, so the footwell squirrel shows over the dashboard. */
	UPROPERTY(EditAnywhere, Category = "Squirrel")
	FTransform PedalTailTransform = FTransform(FRotator::ZeroRotator, FVector(-6.f, 20.f, 55.f), FVector(0.16f, 0.16f, 1.f));

	/** Tail spring: how hard it swings back to its pose (1/s^2), how fast the swing dies (1/s), and
	 *  how far it leans with acceleration (degrees of lean = TailGain * cm/s^2 / TailStiffness). */
	UPROPERTY(EditAnywhere, Category = "Squirrel|Charm")
	float TailStiffness = 120.f;

	UPROPERTY(EditAnywhere, Category = "Squirrel|Charm")
	float TailDamping = 7.f;

	UPROPERTY(EditAnywhere, Category = "Squirrel|Charm")
	float TailGain = 1.8f;

	/** Most the tail leans away from its pose (degrees). */
	UPROPERTY(EditAnywhere, Category = "Squirrel|Charm")
	float TailMaxLean = 40.f;

	/** Idle sway on top of the spring (degrees), so a parked squirrel still looks alive. */
	UPROPERTY(EditAnywhere, Category = "Squirrel|Charm")
	float TailIdleSway = 5.f;

	/** Pupil spring, like the tail's but in cm. Low damping keeps them googly; gravity pulls them low. */
	UPROPERTY(EditAnywhere, Category = "Squirrel|Charm")
	float PupilStiffness = 60.f;

	UPROPERTY(EditAnywhere, Category = "Squirrel|Charm")
	float PupilDamping = 2.5f;

	UPROPERTY(EditAnywhere, Category = "Squirrel|Charm")
	float PupilGain = 0.075f;

	/** How far a pupil can roll off the eye's centre (cm) before it bounces off the rim. */
	UPROPERTY(EditAnywhere, Category = "Squirrel|Charm")
	float PupilTravel = 2.3f;

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

	/** Camera offset (cm) at a full-strength punch; it shakes randomly within this and decays. */
	UPROPERTY(EditAnywhere, Category = "Camera")
	float CameraPunchDistance = 35.f;

	/** Camera roll (degrees) at a full-strength punch. */
	UPROPERTY(EditAnywhere, Category = "Camera")
	float CameraPunchRoll = 4.f;

	/** How fast a punch dies away (strength per second). */
	UPROPERTY(EditAnywhere, Category = "Camera", meta = (ClampMin = "0.1"))
	float CameraPunchDecay = 3.f;

	// --- Sound and VFX. All optional: assign assets in BP_Squirrel. Played on every machine. ---

	/** Thrown out of the car (or bailing at speed). */
	UPROPERTY(EditAnywhere, Category = "Squirrel|Feedback")
	FNBFeedback ScreamFeedback;

	/** The tumbling body hits something. Intensity runs from LandMinSpeed (0) to LandMaxSpeed (1). */
	UPROPERTY(EditAnywhere, Category = "Squirrel|Feedback")
	FNBFeedback LandFeedback;

	/** Impact speed (cm/s) below which a bounce makes no thud. */
	UPROPERTY(EditAnywhere, Category = "Squirrel|Feedback", meta = (ClampMin = "0"))
	float LandMinSpeed = 250.f;

	UPROPERTY(EditAnywhere, Category = "Squirrel|Feedback", meta = (ClampMin = "0"))
	float LandMaxSpeed = 1500.f;

	/** Minimum seconds between thuds, so a rolling body doesn't buzz. */
	UPROPERTY(EditAnywhere, Category = "Squirrel|Feedback", meta = (ClampMin = "0"))
	float LandCooldown = 0.25f;

	/** Grabs a part to work on it. */
	UPROPERTY(EditAnywhere, Category = "Squirrel|Feedback")
	FNBFeedback GrabFeedback;

private:
	void BuildInputAssets();
	void Move(const FInputActionValue& Value);
	void MoveCompleted(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void JumpPressed();
	void Interact();
	void ActionPressed();

	/** Chat wheel, keys 1-8. */
	void PingPressed(int32 Index);

	/** H / gamepad Y. Seated only. */
	void HornPressed();

	UFUNCTION(Server, Reliable)
	void Server_Honk();

	/** Cosmetic only, so unreliable. */
	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_Grabbed();

	/** Thuds for the tumbling body. Bound on every machine, since each simulates its own copy. */
	UFUNCTION()
	void OnBodyHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	/** Cosmetic, every machine: tail wobble and googly pupils, driven by the squirrel's acceleration. */
	void UpdateCharm(float DeltaSeconds);

	/** Body-relative acceleration from the smoothed movement of the actor (cm/s^2), clamped so teleports don't kick. */
	FVector SampleCharmAcceleration(float DeltaSeconds);

	FVector CharmLastLocation = FVector::ZeroVector;
	FVector CharmVelocity = FVector::ZeroVector;
	bool bHasCharmSample = false;
	FVector2D TailLean = FVector2D::ZeroVector;
	FVector2D TailLeanVelocity = FVector2D::ZeroVector;
	FVector2D PupilOffset = FVector2D::ZeroVector;
	FVector2D PupilVelocity = FVector2D::ZeroVector;

	/** The tail's pose before wobble: the default curl or the pedal flag. */
	FTransform TailBaseTransform;

	/** Local only: applies the decaying camera punch. */
	void UpdateCameraPunch(float DeltaSeconds);

	float CameraPunch = 0.f;
	FVector DefaultCameraLocation = FVector::ZeroVector;
	FRotator DefaultCameraRotation = FRotator::ZeroRotator;
	double NextLandFeedbackTime = 0.0;

	/** Server: mashing a part sends a press per click, but it should chatter once per grab. */
	double NextGrabFeedbackTime = 0.0;

	UFUNCTION(Server, Reliable)
	void Server_Ping(int32 Index);

	/** Local player only: mic off while Acorn in Mouth, back on afterwards. */
	void UpdateVoiceMute();

	UPROPERTY(Transient)
	TArray<TObjectPtr<UInputAction>> PingActions;

	bool bVoiceMuted = false;

	void RegisterVoiceTalker();
	void ActionReleased();
	void ReleaseActiveInteractable();
	void StartClinging(UNBInteractableComponent* Target);
	void StopClinging();
	void RefreshAttachedState();

	/** Tail pose for the current seat: the pedal flag, or the default curl. Skipped while tumbling. */
	void RefreshTailPose();

	/** Local only: swings a seated player's camera behind the car while they aren't looking around. */
	void UpdateSeatedCamera(float DeltaSeconds);

	UFUNCTION(Server, Reliable)
	void Server_DevFail(const FString& PartName);

	UFUNCTION(Server, Reliable)
	void Server_DevChaos();

	UFUNCTION(Server, Reliable)
	void Server_DevFlip();

	UFUNCTION(Server, Reliable)
	void Server_DevStart();

	UFUNCTION(Server, Reliable)
	void Server_DevSoundTour();

	UFUNCTION(Server, Reliable)
	void Server_DevEnd(const FString& Result);

	UFUNCTION(Server, Reliable)
	void Server_DevWarp(const FString& Section);

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

	/** Parts that ride on the body ball while tumbling (tail, belly, head), with their standing poses. */
	TArray<TPair<TObjectPtr<USceneComponent>, FTransform>> RideAlongParts;

	/** Tints a basic-shape part (BasicShapeMaterial's Color parameter). */
	static void SetPartColor(UStaticMeshComponent* Part, const FLinearColor& Color);
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
	TObjectPtr<UInputAction> MenuAction;

	UPROPERTY(Transient)
	TObjectPtr<UInputAction> HornAction;

	/** Server only. The interactable this squirrel is holding the action button on. */
	UPROPERTY(Transient)
	TObjectPtr<UNBInteractableComponent> ActiveInteractable;

	float LastSentSeatInput = 0.f;

	/** World time of the last non-zero look input (local only). */
	double LastLookInputTime = -1000.0;

	bool bHasRingPress = false;
	bool bLastRingPressHit = false;
	double LastRingPressTime = 0.0;
};
