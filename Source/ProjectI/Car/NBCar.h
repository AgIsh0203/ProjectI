// Squirrel Wheels prototype.

#pragma once

#include "CoreMinimal.h"
#include "Car/NBSeatComponent.h"
#include "FX/NBFeedback.h"
#include "TP_VehicleAdvOffroadCar.h"
#include "NBCar.generated.h"

class UNBBrakePart;
class UNBCarPartComponent;
class UNBDoorPart;
class UNBEnginePart;
class UNBInteractableComponent;
class UNBTirePart;
class UAudioComponent;
class UInstancedStaticMeshComponent;
class USoundBase;
class UStaticMeshComponent;

/** Car-wide moments that get a sound/effect on every machine. */
UENUM()
enum class ENBCarFeedback : uint8
{
	Crash,
	Spill,
	Flipped,
	Righted,
	Horn,
};

/**
 * The one car everyone shares: a Chaos wheeled vehicle built on the template offroad car.
 * Mesh, tire sockets and curves come from the Blueprint child (BP_OffroadCar_Pawn).
 *
 * Nobody possesses it. Seated squirrels send their axis to the server, which feeds
 * Chaos directly; clients follow via Predictive Interpolation physics replication.
 */
UCLASS()
class PROJECTI_API ANBCar : public ATP_VehicleAdvOffroadCar
{
	GENERATED_BODY()

public:
	ANBCar();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void Tick(float DeltaSeconds) override;

	/** Server only. Routes a seated squirrel's axis to the control that seat owns. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Car")
	void SetSeatInput(ENBSeatRole SeatRole, float Value);

	/** Server only. A squirrel left the seat: its control goes back to neutral (and the Gears seat lets go of the handbrake). */
	void ClearSeatInput(ENBSeatRole SeatRole);

	/** The seat that owns a control (Wheel, Pedals, Gears); null for Rider. */
	UFUNCTION(BlueprintPure, Category = "Car")
	UNBSeatComponent* GetSeat(ENBSeatRole SeatRole) const;

	// --- Gears seat (G5). ---

	/** Server only. The host's Manual gears option: W/S on the Gears seat shift, and nobody there leaves the car in its gear. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Car|Gears")
	void SetManualGears(bool bManual);

	UFUNCTION(BlueprintPure, Category = "Car|Gears")
	bool IsManualGears() const { return bManualGears; }

	/** Server only. The Gears squirrel holds (or lets go of) the handbrake: the rear wheels lock. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Car|Gears")
	void SetHandbrake(bool bOn);

	UFUNCTION(BlueprintPure, Category = "Car|Gears")
	bool IsHandbrakeOn() const { return bHandbrake; }

	/** The gear the Gears squirrel picked (1..number of forward gears), manual gears only. */
	UFUNCTION(BlueprintPure, Category = "Car|Gears")
	int32 GetManualGear() const { return ManualGear; }

	int32 GetNumForwardGears() const;

	/** 0..1 on the RPM gauge where over-revving starts wearing the engine (the gauge's red zone). */
	float GetOverRevAlpha() const { return OverRevStart; }

	/** Closest unoccupied seat within MaxDistance of Location, or nullptr. */
	UNBSeatComponent* FindNearestFreeSeat(const FVector& Location, float MaxDistance) const;

	/** The next free seat after From in hop order (wrapping), or nullptr if every other seat is taken. */
	UNBSeatComponent* FindNextFreeSeat(const UNBSeatComponent* From) const;

	/** Every seat, in hop order. */
	const TArray<TObjectPtr<UNBSeatComponent>>& GetSeats() const { return Seats; }

	/** Every breakable part. */
	const TArray<TObjectPtr<UNBCarPartComponent>>& GetParts() const { return Parts; }

	// --- Dev failure triggers, until the failure director (M2) exists. Server only. ---

	/** Engine, Brakes, Door, TireFL/FR/BL/BR, Tire (random tire) or All. */
	void DevFail(const FString& PartName);

	/** Dev (NBSoundTour): plays every car, part and squirrel sound in turn, 4 s apart, naming each
	 *  one on screen, so the sounds can be judged in the real mix. Server only. */
	void DevSoundTour();

	/** Dev: roll the car onto its roof so the flip-up can be tested. */
	void DevFlip();

	/** Random failures every DevChaosInterval seconds. */
	void SetDevChaos(bool bEnable);
	bool IsDevChaos() const { return DevChaosTimer.IsValid(); }

	/**
	 * Upside down or on its side and nearly stopped. Riders are thrown out, the seats are
	 * closed, and two squirrels on foot have to push together at FlipSpot to right it.
	 */
	UFUNCTION(BlueprintPure, Category = "Car|Flip")
	bool IsFlipped() const { return bFlipped; }

	/**
	 * Server: put the car back on the trail (a squirrel held the reset key), at the nearest point of the
	 * route, upright, facing along it, riders aboard. Squirrels on foot come back to it without a respawn
	 * penalty. Costs AcornsLostOnReset and then waits ResetCooldown. False if refused.
	 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Car|Reset")
	bool ResetToTrail();

	/** Seconds until the car can be reset again (0 = now), on any machine. */
	float GetResetCooldownLeft() const;

	/** Local time this machine saw the last reset (for the HUD's "CAR RESET!"). */
	double GetLastResetSeenTime() const { return LastResetSeenTime; }
	int32 GetAcornsLostOnReset() const { return AcornsLostOnReset; }

	/** cm/s along the car's forward axis. */
	UFUNCTION(BlueprintPure, Category = "Car")
	float GetForwardSpeed() const;

	UFUNCTION(BlueprintPure, Category = "Car")
	float GetSteerInput() const { return SteerInput; }

	UFUNCTION(BlueprintPure, Category = "Car")
	float GetThrottleInput() const { return ThrottleInput; }

	// --- Engine state. Chaos doesn't replicate it to an unpossessed car, so the car does. ---

	/** Engine speed on this machine: live on the server, smoothed from the replicated value on clients. */
	UFUNCTION(BlueprintPure, Category = "Car|Engine")
	float GetEngineRPM() const { return EngineRPM; }

	/** 0 at idle, 1 at the redline. */
	UFUNCTION(BlueprintPure, Category = "Car|Engine")
	float GetEngineRPMAlpha() const;

	UFUNCTION(BlueprintPure, Category = "Car|Engine")
	int32 GetEngineGear() const { return RepGear; }

	// --- Acorn cargo. The car starts full; bumps and flips spill it. ---

	UFUNCTION(BlueprintPure, Category = "Car|Cargo")
	int32 GetAcorns() const { return Acorns; }

	UFUNCTION(BlueprintPure, Category = "Car|Cargo")
	int32 GetMaxAcorns() const { return MaxAcorns; }

	/** 0..1: how far the off-trail grip loss has built up (replicated coarsely, for the HUD). */
	UFUNCTION(BlueprintPure, Category = "Car|Offroad")
	float GetOffroadLevel() const { return HasAuthority() ? OffroadLevel : RepOffroad / 255.f; }
	float GetOffroadMinGrip() const { return OffroadMinGrip; }

	/** Air points banked this run from clean landings (server; added to the score on delivery). */
	int32 GetAirPoints() const { return AirPoints; }

	/** The last clean landing, for the HUD's "AIR TIME! +N": its points, and the local time this machine saw it. */
	int32 GetLastJumpPoints() const { return LastJumpPoints; }
	double GetLastJumpSeenTime() const { return LastJumpSeenTime; }

	/** Server only. Throws up to Count acorns out of the bed as physics props. Returns how many spilled. */
	int32 SpillAcorns(int32 Count);

	/** Server only. A seated squirrel honks; ignored inside HornCooldown. */
	void Honk();

	/** Local world time this machine last heard the horn, for the HUD's HONK! bubble. */
	double GetLastHornTime() const { return LastHornTime; }

	/** Server only. The run isn't live (waiting, countdown, over): brakes held, no spilling. */
	void SetRunLocked(bool bLocked);

	/** Server only. A squirrel hopped seats: the door rattles looser. */
	void NoteSeatHop();

	// --- Hooks for car parts. 1 = healthy. Server only. ---

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Car|Damage")
	void SetEnginePowerScale(float Scale);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Car|Damage")
	void SetBrakePowerScale(float Scale);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Car|Damage")
	void SetWheelGripScale(int32 WheelIndex, float Scale);

	/** A flat tire pulls the car towards its side, wobbles the wheel and caps the top speed. */
	void SetWheelFlat(const class UNBTirePart* Tire, bool bFlat);

	/** Failed brakes seizing: forces this much brake (0..1) over whatever the pedals say. */
	void SetBrakeGrab(float Strength);

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Car")
	TObjectPtr<UNBSeatComponent> WheelSeat;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Car")
	TObjectPtr<UNBSeatComponent> PedalSeat;

	/** Beside the wheel: the Gears seat (shifting with Manual gears on, and the handbrake). */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Car")
	TObjectPtr<UNBSeatComponent> PassengerSeat;

	/** Rider spots for squirrels who aren't driving. */

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Car")
	TObjectPtr<UNBSeatComponent> DeckSeat;

	/** On the front hood, next to the brakes. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Car")
	TObjectPtr<UNBSeatComponent> HoodSeat;

	/** Every seat, in hop order. Filled in BeginPlay. */
	UPROPERTY(Transient)
	TArray<TObjectPtr<UNBSeatComponent>> Seats;

	// --- Parts. Each is also the spot you repair it from. ---

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Car|Parts")
	TObjectPtr<UNBEnginePart> EnginePart;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Car|Parts")
	TObjectPtr<UNBBrakePart> BrakePart;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Car|Parts")
	TObjectPtr<UNBTirePart> TireFL;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Car|Parts")
	TObjectPtr<UNBTirePart> TireFR;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Car|Parts")
	TObjectPtr<UNBTirePart> TireBL;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Car|Parts")
	TObjectPtr<UNBTirePart> TireBR;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Car|Parts")
	TObjectPtr<UNBDoorPart> DoorPart;

	/** Greybox door on the passenger side: the hinge swings, the panel hangs off it. */
	UPROPERTY(VisibleAnywhere, Category = "Car|Parts")
	TObjectPtr<USceneComponent> DoorHinge;

	UPROPERTY(VisibleAnywhere, Category = "Car|Parts")
	TObjectPtr<UStaticMeshComponent> DoorPanel;

	/** Where squirrels push to right a flipped car. Only enabled while flipped. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Car|Flip")
	TObjectPtr<UNBInteractableComponent> FlipSpot;

	/** Counts as flipped while the car's up axis is below this (0 = on its side). */
	UPROPERTY(EditAnywhere, Category = "Car|Flip")
	float FlipUpDot = 0.35f;

	/** ...and it is slower than this (cm/s)... */
	UPROPERTY(EditAnywhere, Category = "Car|Flip")
	float FlipMaxSpeed = 200.f;

	/** ...for this long, so a roll that lands on its wheels doesn't count. */
	UPROPERTY(EditAnywhere, Category = "Car|Flip")
	float FlipConfirmSeconds = 1.5f;

	/** Righted cars are lifted this far before being dropped back on their wheels. */
	UPROPERTY(EditAnywhere, Category = "Car|Flip")
	float RightingLift = 120.f;

	// --- Reset to the trail (18 Oct: a car down a cliff couldn't get back up). ---

	/** Acorns the reset spills (the price of the tow). */
	UPROPERTY(EditAnywhere, Category = "Car|Reset", meta = (ClampMin = "0"))
	int32 AcornsLostOnReset = 3;

	/** Seconds before the car can be reset again. */
	UPROPERTY(EditAnywhere, Category = "Car|Reset", meta = (ClampMin = "0"))
	float ResetCooldown = 15.f;

	/** The reset lands this far (cm) back along the trail from where the car last was on it, clear of the edge it went off. */
	UPROPERTY(EditAnywhere, Category = "Car|Reset", meta = (ClampMin = "0"))
	float ResetBackOff = 800.f;

	/** Every part. Filled in BeginPlay. */
	UPROPERTY(Transient)
	TArray<TObjectPtr<UNBCarPartComponent>> Parts;

	UPROPERTY(EditAnywhere, Category = "Car|Cargo")
	int32 MaxAcorns = 40;

	/** Velocity change over one CheckSpill step (cm/s) below which a bump spills nothing. */
	UPROPERTY(EditAnywhere, Category = "Car|Cargo")
	float SpillMinDeltaV = 350.f;

	/** One acorn per this much velocity change beyond the minimum (cm/s). */
	UPROPERTY(EditAnywhere, Category = "Car|Cargo")
	float SpillDeltaVPerAcorn = 120.f;

	// --- Jumps (friend test 1: "what's the use of the jump if we lose the nuts anyway?"). ---

	/** All four wheels off the ground at least this long (s) counts as a jump... */
	UPROPERTY(EditAnywhere, Category = "Car|Jumps")
	float MinJumpAirtime = 0.5f;

	/** ...landing at this horizontal speed (cm/s) or faster... */
	UPROPERTY(EditAnywhere, Category = "Car|Jumps")
	float MinJumpSpeed = 600.f;

	/** ...and landing it upright earns this many points per second in the air... */
	UPROPERTY(EditAnywhere, Category = "Car|Jumps")
	float AirPointsPerSecond = 25.f;

	/** ...and for this long after touchdown only sideways jolts spill acorns, not the landing thump. */
	UPROPERTY(EditAnywhere, Category = "Car|Jumps")
	float CleanLandingGrace = 0.3f;

	/** Share of the remaining cargo dumped when the car flips. */
	UPROPERTY(EditAnywhere, Category = "Car|Cargo", meta = (ClampMin = "0", ClampMax = "1"))
	float FlipSpillFraction = 0.25f;

	/** The cargo you can see: one instance per acorn, piled on the engine deck. Shrinks as acorns spill. */
	UPROPERTY(VisibleAnywhere, Category = "Car|Cargo")
	TObjectPtr<UInstancedStaticMeshComponent> AcornPile;

	UPROPERTY(EditAnywhere, Category = "Car|Dev")
	float DevChaosInterval = 12.f;

	// --- Wear: how the car is driven decides which parts the failure director picks. Rates are
	// wear per second (1 = worn out) unless noted. ---

	/** Speed (cm/s) where flooring it starts wearing the engine and rear tires... */
	UPROPERTY(EditAnywhere, Category = "Car|Wear")
	float WearSpeedStart = 1000.f;

	/** ...and where it wears them fastest. */
	UPROPERTY(EditAnywhere, Category = "Car|Wear")
	float WearSpeedFull = 2000.f;

	/** At full speed and full throttle (half that coasting). */
	UPROPERTY(EditAnywhere, Category = "Car|Wear")
	float EngineWearRate = 0.03f;

	/** Each rear tire, at full speed. */
	UPROPERTY(EditAnywhere, Category = "Car|Wear")
	float RearTireWearRate = 0.02f;

	/** Each front tire, at full lock above FrontTireWearSpeed. */
	UPROPERTY(EditAnywhere, Category = "Car|Wear")
	float FrontTireWearRate = 0.035f;

	/** Below this speed (cm/s) steering wears the front tires proportionally less. */
	UPROPERTY(EditAnywhere, Category = "Car|Wear")
	float FrontTireWearSpeed = 800.f;

	/** Braking hard at BrakeWearSpeed or faster. */
	UPROPERTY(EditAnywhere, Category = "Car|Wear")
	float BrakeHeavyWearRate = 0.08f;

	UPROPERTY(EditAnywhere, Category = "Car|Wear")
	float BrakeWearSpeed = 1500.f;

	/** Never braking also hurts them: after BrakeIdleSeconds above BrakeIdleMinSpeed without a touch
	 *  of the brakes, they wear at this rate until someone brakes. */
	UPROPERTY(EditAnywhere, Category = "Car|Wear")
	float BrakeIdleWearRate = 0.02f;

	UPROPERTY(EditAnywhere, Category = "Car|Wear")
	float BrakeIdleSeconds = 30.f;

	UPROPERTY(EditAnywhere, Category = "Car|Wear")
	float BrakeIdleMinSpeed = 600.f;

	/** Added to the door each time a squirrel hops seats (not per second). */
	UPROPERTY(EditAnywhere, Category = "Car|Wear")
	float DoorWearPerHop = 0.15f;

	// --- Flat tires (friend test 1: "what difference does a flat tyre make?"). ---

	/** Steering added towards the side of a flat front tire (full lock = 1)... */
	UPROPERTY(EditAnywhere, Category = "Car|Damage")
	float FlatFrontPull = 0.3f;

	/** ...and of a flat rear tire. */
	UPROPERTY(EditAnywhere, Category = "Car|Damage")
	float FlatRearPull = 0.15f;

	/** Steering shake per flat tire, so the car visibly wobbles. */
	UPROPERTY(EditAnywhere, Category = "Car|Damage")
	float FlatWobble = 0.08f;

	/** With one flat the engine cuts out above this speed (cm/s)... */
	UPROPERTY(EditAnywhere, Category = "Car|Damage")
	float FlatTopSpeed = 1400.f;

	/** ...and each further flat lowers it by this much. */
	UPROPERTY(EditAnywhere, Category = "Car|Damage")
	float FlatTopSpeedPerExtra = 300.f;

	/** Manual gears: above this share of the rev range (0 idle .. 1 redline) the engine over-revs and wears. 0.65 is ~4900 rpm:
	 *  on the flat 1st gear tops out at ~5100 rpm and 2200 cm/s (18 Oct, PIE), so staying in 1st wears the engine... */
	UPROPERTY(EditAnywhere, Category = "Car|Wear", meta = (ClampMin = "0", ClampMax = "1"))
	float OverRevStart = 0.65f;

	/** ...up to this much per second at the redline (past 0.6 wear a part can break early, see the failure director)... */
	UPROPERTY(EditAnywhere, Category = "Car|Wear")
	float OverRevWearRate = 0.25f;

	/** ...times this in top gear, where there's no shifting up: only easing off saves it (the HUD says so). */
	UPROPERTY(EditAnywhere, Category = "Car|Wear", meta = (ClampMin = "0"))
	float TopGearOverRevScale = 2.f;

	// --- Steering at speed (18 Oct, the user: "at top speed you can take the turns easily"). ---

	/** Full steering up to this speed (cm/s)... */
	UPROPERTY(EditAnywhere, Category = "Car|Handling", meta = (ClampMin = "0"))
	float SteerFadeStart = 500.f;

	/** ...fading to SteerAtSpeed of it by this speed and above. */
	UPROPERTY(EditAnywhere, Category = "Car|Handling", meta = (ClampMin = "0"))
	float SteerFadeFull = 1500.f;

	UPROPERTY(EditAnywhere, Category = "Car|Handling", meta = (ClampMin = "0", ClampMax = "1"))
	float SteerAtSpeed = 0.4f;

	/** Every part heals this much per second, so careful stretches cool things down. */
	UPROPERTY(EditAnywhere, Category = "Car|Wear")
	float WearRecoveryRate = 0.006f;

	/** Fording a river: every tyre wears this much per second on the hidden stones (at full depth and speed)... */
	UPROPERTY(EditAnywhere, Category = "Car|Wear")
	float RiverTireWearRate = 0.05f;

	/** ...and the water slows the car: velocity lost per second at full depth (0..1). */
	UPROPERTY(EditAnywhere, Category = "Car|Wear")
	float RiverDrag = 0.5f;

	/** Water this deep (cm) over the ground under the car is "full depth". */
	UPROPERTY(EditAnywhere, Category = "Car|Wear")
	float RiverFullDepth = 60.f;

	/** Rough ground: each tyre wears this much per second while its suspension pumps at RoughJoltFull... */
	UPROPERTY(EditAnywhere, Category = "Car|Wear")
	float RoughTireWearRate = 0.05f;

	/** ...measured in suspension travels per second (smoothed): below RoughJoltStart the ride counts as smooth. The
	 *  autopilot on L_Route measured 0.1-0.6 on the trail and 2-4 on rough patches at 16 m/s. */
	UPROPERTY(EditAnywhere, Category = "Car|Wear")
	float RoughJoltStart = 1.f;

	UPROPERTY(EditAnywhere, Category = "Car|Wear")
	float RoughJoltFull = 4.f;

	// --- Off the trail (18 Oct: "you can take the path but not for long"). The trail is the route's dirt
	// track (ANBRoute::GetTrailWeight); grass, hillsides and shortcuts are off it. ---

	/** Grip of a wheel off the trail once the off-road level has built up fully... */
	UPROPERTY(EditAnywhere, Category = "Car|Offroad", meta = (ClampMin = "0", ClampMax = "1"))
	float OffroadMinGrip = 0.45f;

	/** ...which takes this long with the whole car off the trail... */
	UPROPERTY(EditAnywhere, Category = "Car|Offroad", meta = (ClampMin = "0.1"))
	float OffroadRampSeconds = 5.f;

	/** ...and this long to recover once back on it. */
	UPROPERTY(EditAnywhere, Category = "Car|Offroad", meta = (ClampMin = "0.1"))
	float OffroadRecoverSeconds = 2.f;

	/** Wear per second on each tyre off the trail at full off-road level, while moving. */
	UPROPERTY(EditAnywhere, Category = "Car|Offroad")
	float OffroadTireWearRate = 0.05f;

	/** Engine wear per second with the whole car off the trail at full level (it labours on grass). */
	UPROPERTY(EditAnywhere, Category = "Car|Offroad")
	float OffroadEngineWearRate = 0.015f;

	/** Velocity lost per second with the whole car off the trail at full level (0..1). */
	UPROPERTY(EditAnywhere, Category = "Car|Offroad", meta = (ClampMin = "0", ClampMax = "1"))
	float OffroadDrag = 0.2f;

	// --- Crash damage (friend test 2: "random damage ... should be a punishment for hitting stuff"). ---

	/** A sideways or head-on hit (horizontal velocity change over 0.1 s, cm/s) at least this hard damages the part
	 *  on the side that hit: wear from 0 here up to CrashWear at CrashMaxDeltaV. Straight-down landings never count. */
	UPROPERTY(EditAnywhere, Category = "Car|Damage", meta = (ClampMin = "0"))
	float CrashDamageMinDeltaV = 600.f;

	UPROPERTY(EditAnywhere, Category = "Car|Damage", meta = (ClampMin = "0", ClampMax = "1"))
	float CrashWear = 0.7f;

	/** A hit at least this hard (cm/s) breaks that part outright, unless it was repaired in the last CrashRepairGrace s. */
	UPROPERTY(EditAnywhere, Category = "Car|Damage", meta = (ClampMin = "0"))
	float CrashBreakDeltaV = 1200.f;

	UPROPERTY(EditAnywhere, Category = "Car|Damage", meta = (ClampMin = "0"))
	float CrashRepairGrace = 4.f;

	/** One damaging hit per this many seconds, so a scrape along a wall is one hit, not ten. */
	UPROPERTY(EditAnywhere, Category = "Car|Damage", meta = (ClampMin = "0"))
	float CrashDamageCooldown = 1.f;

	// --- Sound and VFX. All optional: assign assets in BP_OffroadCar_Pawn. ---

	/** Plays the engine loop at the engine deck. Add attenuation on this component in the Blueprint. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Car|Feedback")
	TObjectPtr<UAudioComponent> EngineAudio;

	/** Looping engine sound. A MetaSound can read the RPM, Throttle and EngineFailed float inputs;
	 *  a plain looping wave works too, with bPitchFromRPM. */
	UPROPERTY(EditAnywhere, Category = "Car|Feedback")
	TObjectPtr<USoundBase> EngineSound;

	/** Raw RPM (EngineIdleRPM..MaxRPM). */
	UPROPERTY(EditAnywhere, Category = "Car|Feedback")
	FName EngineRPMParam = TEXT("RPM");

	/** -1 (brake) to 1 (full gas). */
	UPROPERTY(EditAnywhere, Category = "Car|Feedback")
	FName EngineThrottleParam = TEXT("Throttle");

	/** 1 while the engine part is broken (sputter, cough), else 0. */
	UPROPERTY(EditAnywhere, Category = "Car|Feedback")
	FName EngineFailedParam = TEXT("EngineFailed");

	/** Drive the engine sound's pitch from RPM, for a plain looping wave. Turn off if a MetaSound does it. */
	UPROPERTY(EditAnywhere, Category = "Car|Feedback")
	bool bPitchFromRPM = true;

	UPROPERTY(EditAnywhere, Category = "Car|Feedback", meta = (EditCondition = "bPitchFromRPM", ClampMin = "0.1"))
	float EnginePitchAtIdle = 0.8f;

	UPROPERTY(EditAnywhere, Category = "Car|Feedback", meta = (EditCondition = "bPitchFromRPM", ClampMin = "0.1"))
	float EnginePitchAtRedline = 2.f;

	/** Engine volume with no throttle; full throttle plays at 1. */
	UPROPERTY(EditAnywhere, Category = "Car|Feedback", meta = (ClampMin = "0", ClampMax = "1"))
	float EngineIdleVolume = 0.6f;

	/** How fast clients' RPM follows the replicated value (higher is snappier). */
	UPROPERTY(EditAnywhere, Category = "Car|Feedback", meta = (ClampMin = "0.5"))
	float RPMSmoothingSpeed = 8.f;

	/** A hard hit, at the car. Intensity runs from CrashMinDeltaV (0) to CrashMaxDeltaV (1). */
	UPROPERTY(EditAnywhere, Category = "Car|Feedback")
	FNBFeedback CrashFeedback;

	/** Velocity change over 0.1 s (cm/s) that counts as a crash. */
	UPROPERTY(EditAnywhere, Category = "Car|Feedback", meta = (ClampMin = "0"))
	float CrashMinDeltaV = 500.f;

	UPROPERTY(EditAnywhere, Category = "Car|Feedback", meta = (ClampMin = "0"))
	float CrashMaxDeltaV = 1600.f;

	/** Minimum seconds between crash sounds, so one long scrape doesn't machine-gun them. */
	UPROPERTY(EditAnywhere, Category = "Car|Feedback", meta = (ClampMin = "0"))
	float CrashCooldown = 0.3f;

	/** Acorns flying out of the bed. Intensity is the number spilled / SpillFeedbackFullCount. */
	UPROPERTY(EditAnywhere, Category = "Car|Feedback")
	FNBFeedback SpillFeedback;

	UPROPERTY(EditAnywhere, Category = "Car|Feedback", meta = (ClampMin = "1"))
	int32 SpillFeedbackFullCount = 6;

	/** The car ends up on its roof or side. */
	UPROPERTY(EditAnywhere, Category = "Car|Feedback")
	FNBFeedback FlippedFeedback;

	/** Two squirrels push it back onto its wheels. */
	UPROPERTY(EditAnywhere, Category = "Car|Feedback")
	FNBFeedback RightedFeedback;

	/** A seated squirrel honks (H / gamepad Y). */
	UPROPERTY(EditAnywhere, Category = "Car|Feedback")
	FNBFeedback HornFeedback;

	/** Minimum seconds between honks, so spamming H gives a rhythm instead of a drone. */
	UPROPERTY(EditAnywhere, Category = "Car|Feedback", meta = (ClampMin = "0"))
	float HornCooldown = 0.25f;

	/** Riders' cameras shake on a crash; squirrels on foot feel it fading out to this distance (cm). */
	UPROPERTY(EditAnywhere, Category = "Car|Feedback", meta = (ClampMin = "0"))
	float CrashShakeRadius = 2500.f;

private:
	void ApplyInputsToVehicle();
	void UpdateEngineState(float DeltaSeconds);

	/** Server: wears the parts from speed, steering and braking. */
	void UpdateWear(float DeltaSeconds);

	/** Seconds driven above BrakeIdleMinSpeed since the brakes were last used. */
	float SecondsWithoutBraking = 0.f;
	/** Server: the car was in a river last tick (for the NBRiver log). */
	bool bInRiver = false;

	/** Bit per flat tire: FL, FR, BL, BR. */
	uint8 FlatWheels = 0;
	float BrakeGrab = 0.f;
	UNBTirePart* CreateTire(FName Name, FName WheelBone, const FVector& Location, const FText& PartName);
	void DevFailRandomPart();
	void SoundTourStep();
	FTimerHandle SoundTourTimer;
	int32 SoundTourIndex = 0;
	void CheckFlipped();
	void CheckSpill();
	void SetFlipped(bool bNewFlipped);

	/** All machines: show exactly Acorns instances of the pile. */
	void RefreshAcornPile();

	/** Camera punch for this machine's squirrels in or near the car. */
	void ShakeLocalCameras(float Intensity) const;

	UFUNCTION()
	void OnRep_Acorns();

	UFUNCTION()
	void HandleFlipPushed(UNBInteractableComponent* Interactable);

	/** Cosmetic only, so unreliable: a dropped crash sound is fine. */
	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_Feedback(ENBCarFeedback Event, float Intensity);

	FTimerHandle FlipCheckTimerHandle;
	float FlippedSeconds = 0.f;

	UPROPERTY(Replicated)
	bool bFlipped = false;

	FTimerHandle DevChaosTimer;
	FTimerHandle SpillTimerHandle;
	FVector LastSpillVelocity = FVector::ZeroVector;

	/** Server: wears (or breaks) the part on the side the car was hit from. DeltaVelocity is horizontal. */
	void ApplyCrashDamage(const FVector& DeltaVelocity);
	float NextCrashDamageTime = 0.f;

	/** Server: per wheel, last tick's suspension length and its smoothed rate of change (rough ground). */
	float LastSuspension[4] = {};
	float SmoothedJolt[4] = {};

	/** Server: off-trail state. Sampled every OffroadSampleInterval; grip = parts' scale x off-road scale. */
	void UpdateOffroad(float DeltaSeconds, float Speed);
	void ApplyWheelGrip(int32 WheelIndex);
	static constexpr float OffroadSampleInterval = 0.1f;
	float OffroadSampleTimer = 0.f;
	float OffroadLevel = 0.f;
	/** Per wheel, 0 = on the trail, 1 = fully off it (last sample). */
	float WheelOffroad[4] = {};
	/** Per wheel, the grip scale the parts set (a flat tyre). */
	float PartGrip[4] = {1.f, 1.f, 1.f, 1.f};
	bool bWasOffroad = false;

	/** Server: distance along the route at the last sample with the car on the trail (-1 = none yet). The reset
	 *  goes back here, not to the nearest trail anywhere, so a fall off a cliff can't skip down to a later leg. */
	float LastOnTrailDistance = -1.f;

	UPROPERTY(Replicated)
	uint8 RepOffroad = 0;

	/** Server: tracks airtime in CheckSpill; returns true while a clean landing's grace is running. */
	bool UpdateJump(double Now);
	double AirborneSince = -1.0;
	double CleanLandingUntil = 0.0;
	int32 AirPoints = 0;

	/** The last clean landing's points; JumpCount is bumped per jump so equal jumps still replicate. */
	UPROPERTY(Replicated)
	int32 LastJumpPoints = 0;

	UPROPERTY(ReplicatedUsing = OnRep_JumpCount)
	uint8 JumpCount = 0;

	UFUNCTION()
	void OnRep_JumpCount();

	/** Server world time the next reset is allowed. */
	UPROPERTY(Replicated)
	float ResetReadyTime = 0.f;

	/** Bumped per reset so every machine sees each one. */
	UPROPERTY(ReplicatedUsing = OnRep_ResetCount)
	uint8 ResetCount = 0;

	UFUNCTION()
	void OnRep_ResetCount();

	double LastResetSeenTime = -100.0;

	/** Local time this machine saw the last jump. */
	double LastJumpSeenTime = -100.0;
	bool bRunLocked = false;
	float NextCrashFeedbackTime = 0.f;

	float EngineRPM = 0.f;

	/** Server's engine RPM, rounded to 10 so idle wobble doesn't re-send it every frame. */
	UPROPERTY(Replicated)
	uint16 RepEngineRPM = 0;

	UPROPERTY(Replicated)
	int8 RepGear = 0;

	UPROPERTY(ReplicatedUsing = OnRep_Acorns)
	int32 Acorns = 0;

	/** Every acorn's spot in the pile, bottom layer first, so spills empty it from the top. */
	TArray<FTransform> AcornPileSlots;

	float NextHornTime = 0.f;
	double LastHornTime = -100.0;

	UPROPERTY(Replicated)
	float SteerInput = 0.f;

	/** >0 gas, <0 brake (Chaos reverses when braking from a stop). */
	UPROPERTY(Replicated)
	float ThrottleInput = 0.f;

	/** Server: in manual, keeps Chaos in ManualGear (its reverse-as-brake logic drops to reverse and back to 1st). */
	void HoldManualGear();

	UPROPERTY(Replicated)
	bool bManualGears = false;

	UPROPERTY(Replicated)
	bool bHandbrake = false;

	UPROPERTY(Replicated)
	int8 ManualGear = 1;

	/** Server: the Gears seat's last W/S axis; a push past half shifts once. */
	float GearStick = 0.f;

	float BaseMaxEngineTorque = 0.f;
	TArray<float> BaseBrakeTorque;
	TArray<float> BaseFriction;
};
