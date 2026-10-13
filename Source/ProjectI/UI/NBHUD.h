// Squirrel Wheels prototype.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "NBHUD.generated.h"

class UNBInteractableComponent;
class UNBLobbyMenu;

/**
 * Greybox HUD drawn on the canvas: the focused interactable's prompt and progress
 * (with a timing ring when needed) plus a controls hint. Replaced by UMG in M3.
 */
UCLASS()
class PROJECTI_API ANBHUD : public AHUD
{
	GENERATED_BODY()

public:
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
	void DrawCenteredText(const FString& Text, float Y, const FLinearColor& Color, float Scale);

	/** Queues a callout for each part that broke since the last frame. */
	void UpdateCallouts();
	/** The big "ENGINE ON FIRE!" lines: they slam in, shake, and fade. Readable with the sound off. */
	void DrawCallouts();
	/** Onboarding: WHEEL / PEDALS over the two driving seats until someone first sits in each. */
	void DrawSeatLabels();
	/** "HONK!" over the car for a moment after each honk. */
	void DrawHornBubble();
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

	/** Driving seats someone has sat in; their labels are gone for good (until the map reloads). */
	TSet<TWeakObjectPtr<const class UNBSeatComponent>> UsedSeats;

	UPROPERTY(Transient)
	TObjectPtr<UNBLobbyMenu> Menu;

	bool bMenuOpen = false;

	TWeakObjectPtr<class ANBRoute> Route;
	int32 ShownSection = INDEX_NONE;
	double SectionShownAt = -100.0;
};
