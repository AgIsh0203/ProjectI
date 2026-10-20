// Squirrel Wheels prototype.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "NBHUD.generated.h"

class UNBInteractableComponent;
class UNBLobbyMenu;
class UTexture2D;
enum class ENBBinding : uint8;

/** The game's buttons, as the HUD names them: a key or a gamepad button depending on what was used last. */
enum class ENBPrompt : uint8
{
	Interact, // E / X: get in, hop seat
	Jump,     // Space / A: jump, jump out, let go
	Work,     // LMB / B: repair, push
	Horn,     // H / Y
	Menu,     // Esc / Menu (Start)
	Count
};
static_assert(static_cast<int32>(ENBPrompt::Count) == 5, "KeyIcons / PadIcons in ANBHUD are sized 5");

/**
 * Greybox HUD drawn on the canvas: the focused interactable's prompt and progress
 * (with a timing ring when needed) plus a controls hint. Replaced by UMG in M3.
 */
UCLASS()
class PROJECTI_API ANBHUD : public AHUD
{
	GENERATED_BODY()

public:
	ANBHUD();

	virtual void DrawHUD() override;

	/** The lobby menu: shows the cursor and takes input until closed. */
	void OpenMenu();
	void CloseMenu();
	void ToggleMenu();
	bool IsMenuOpen() const;

protected:
	/** Offline there is nothing to do but host or join, so the menu opens by itself. */
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	/** Failure list at the top plus a marker over each broken part, so viewers can follow along. */
	void DrawCarStatus();
	/** Top-right timer and acorn count, the waiting/countdown banners, the wreck warning and the end screen. */
	void DrawRunStatus();
	/** Right, under the acorns: the car's health and a wear bar per part (green to red, BROKEN flashing). */
	void DrawDamageMeter(const class ANBCar& Car, float Y);

	/** Bottom left, for everyone: the gear and an RPM needle (red zone = over-revving, with Manual gears on). */
	void DrawGearGauge(const class ANBCar& Car);
	/** Bottom: how far along the route the car is, and a banner when it enters a new section. */
	void DrawRouteProgress(const class ANBCar& Car);
	/** Acorn in Mouth banners, the chat wheel legend and recent pings. */
	void DrawVoiceStatus();
	void DrawEndScreen(const class ANBRunGameState& State);
	/** Top-left: offline / hosting / connected, the online subsystem and the menu key. */
	void DrawNetStatus();
	void DrawInteractable(const UNBInteractableComponent& Interactable, bool bUsing);
	void DrawRing(const UNBInteractableComponent& Interactable, const FVector2D& Center, float Radius);
	void DrawBar(float X, float Y, float Width, float Height, float Fill, const FLinearColor& Color);
	/** Centred on CenterX, or on the screen when it is negative. */
	void DrawCenteredText(const FString& Text, float Y, const FLinearColor& Color, float Scale, float CenterX = -1.f);

	// --- Input prompts (Kenney Input Prompts, CC0, /Game/ThirdParty/Kenney/InputPrompts). ---

	/** Switches the prompts to gamepad buttons on any pad input, back to keys on any key or mouse input. */
	void UpdateInputDevice();
	UTexture2D* PromptIcon(ENBPrompt Prompt) const;
	/** For text-only places: "LMB" or "B"; follows the player's key bindings. */
	FString PromptName(ENBPrompt Prompt) const;
	/** The key bound to a rebindable action, e.g. "W" or "LMB". */
	FString KeyLabel(ENBBinding Binding) const;
	/** A centred row of [icon] label pairs. */
	void DrawPromptRow(const TArray<TPair<ENBPrompt, FString>>& Items, float Y, float Scale, const FLinearColor& Color, float CenterX = -1.f);

	UPROPERTY()
	TObjectPtr<UTexture2D> KeyIcons[5];

	UPROPERTY()
	TObjectPtr<UTexture2D> PadIcons[5];

	bool bUsingGamepad = false;

	/** Queues a callout for each part that broke since the last frame. */
	void UpdateCallouts();
	/** The big "ENGINE ON FIRE!" lines: they slam in, shake, and fade. Readable with the sound off. */
	void DrawCallouts();
	/** Onboarding: WHEEL / PEDALS over the two driving seats until someone first sits in each. */
	void DrawSeatLabels();
	/** "HONK!" over the car for a moment after each honk. */
	void DrawHornBubble();

	/** "HANDBRAKE!" over the car while the Gears squirrel holds it. */
	void DrawHandbrakeBubble();

	/** The WIGGLY ROAD dog (#13): WOOF! on each bark, CHOMP! on a tyre, YIPE! running off, zzz asleep nearby. */
	void DrawDogBubbles();

	/** "AIR TIME! +N" over the car for a moment after each clean jump landing. */
	void DrawJumpBubble();
	/** "OFF THE TRAIL!" above the hints while the car's off-road grip loss builds up. */
	void DrawOffroadWarning();
	/** The car reset: "STUCK? Hold R" once the car has sat stuck for a few seconds, the hold bar,
	 *  and "CAR RESET!" afterwards. */
	void DrawCarReset(const class ANBSquirrel& Squirrel);
	/** Seconds the car has been stuck (slow and off the trail, or flipped), measured locally. */
	float StuckSeconds = 0.f;

	/** Who is who: a bobbing YOU arrow over this player's squirrel, and every other squirrel's
	 *  name (plus its seat) in its fur colour. Friend test 1: nobody knew which squirrel they were. */
	void DrawNameTags();

	/** Who is driving, top left: the WHEEL / PEDALS crew (your own line carries its controls), with
	 *  "nobody!" flashing when a driving seat is empty during a run. */
	void DrawRoles();
	/** For 3 s after a fall: why ("THE DOOR FLUNG YOU OUT!") and what to do about it. Friend test 2:
	 *  "no idea why I get thrown out of the car". */
	void DrawEjectReason(const class ANBSquirrel& Squirrel);

	/** For 2.5 s after a key the seat ignores: whose job that is ("The WHEEL squirrel steers"). */
	void DrawSeatHint(const class ANBSquirrel& Squirrel);

	/** Before the run: where the voice and volume settings are. Friend test 2 couldn't find them. */
	void DrawSoundHint();

	/** DrawShoutText at 0.8, left-aligned to the screen's left margin. */
	void DrawLeftShout(const FString& Text, float Y, const FLinearColor& Color);
	/** Large outlined text centred on CenterX, for anything viewers must read at a glance. */
	void DrawShoutText(const FString& Text, float CenterX, float Y, const FLinearColor& Color, float Scale);

	struct FCallout
	{
		FString Text;
		double StartTime = 0.0;
	};
	TArray<FCallout> Callouts;

	/** Where the top-of-screen alarms (flip banner, failure list, wreck warning) end this frame,
	 *  so the next one stacks below instead of on top. */
	float AlarmStackBottom = 0.f;

	/** Each part's failed state last frame. A part seen for the first time only records its state,
	 *  so joining (or restarting) mid-failure doesn't shout about old breaks. */
	TMap<TWeakObjectPtr<const class UNBCarPartComponent>, bool> SeenFailed;

	/** Each dog's state last frame, for the callout when it gets hold of a tyre (first sight only records). */
	TMap<TWeakObjectPtr<const class ANBDog>, uint8> SeenDogStates;

	/** Driving seats someone has sat in; their labels are gone for good (until the map reloads). */
	TSet<TWeakObjectPtr<const class UNBSeatComponent>> UsedSeats;

	/** Lilita One (OFL, /Game/ThirdParty/Fonts) for DrawShoutText. An always-cooked FontFace asset;
	 *  BeginPlay wraps it in a runtime UFont, since Python can't author a Font asset's typefaces. */
	UPROPERTY()
	TObjectPtr<class UFontFace> ShoutFontFace;

	UPROPERTY(Transient)
	TObjectPtr<class UFont> ShoutFont;

	// --- Run audio: local to each player, so it lives on the HUD. Assets in /Game/ThirdParty (always cooked). ---

	/** Plays the menu music outside a run and the run music while driving, plus the countdown
	 *  beeps and the end jingle. Called every frame. */
	void UpdateRunAudio();
	void PlayMusic(class USoundBase* Music);

	UPROPERTY(EditAnywhere, Category = "Audio")
	TObjectPtr<class USoundBase> MenuMusic;

	UPROPERTY(EditAnywhere, Category = "Audio")
	TObjectPtr<class USoundBase> RunMusic;

	UPROPERTY(EditAnywhere, Category = "Audio")
	float MusicVolume = 0.35f;

	/** Each whole second of the countdown. */
	UPROPERTY(EditAnywhere, Category = "Audio")
	TObjectPtr<class USoundBase> CountdownBeep;

	/** The countdown hits zero and the car is free. */
	UPROPERTY(EditAnywhere, Category = "Audio")
	TObjectPtr<class USoundBase> CountdownGo;

	UPROPERTY(EditAnywhere, Category = "Audio")
	TObjectPtr<class USoundBase> DeliveredSound;

	/** Time up or a total wreck. */
	UPROPERTY(EditAnywhere, Category = "Audio")
	TObjectPtr<class USoundBase> FailedSound;

	UPROPERTY(Transient)
	TObjectPtr<class UAudioComponent> MusicComponent;

	UPROPERTY(Transient)
	TObjectPtr<class USoundBase> PlayingMusic;

	/** Last frame's phase and countdown second, to fire one-shots on the edges. */
	uint8 LastPhase = 0xFF;
	int32 LastCountdownSecond = -1;

	/** The chunky font if it loaded, else the engine's large font. */
	UFont* GetShoutFont() const;

	UPROPERTY(Transient)
	TObjectPtr<UNBLobbyMenu> Menu;

	bool bMenuOpen = false;

	TWeakObjectPtr<class ANBRoute> Route;
	int32 ShownSection = INDEX_NONE;
	double SectionShownAt = -100.0;
};
