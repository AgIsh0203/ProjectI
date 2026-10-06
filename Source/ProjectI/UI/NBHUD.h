// Squirrel Wheels prototype.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "NBHUD.generated.h"

class UNBInteractableComponent;

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

private:
	/** Failure list at the top plus a marker over each broken part, so viewers can follow along. */
	void DrawCarStatus();
	/** Top-left: offline / hosting / connected, the online subsystem and the lobby keys. */
	void DrawNetStatus();
	void DrawInteractable(const UNBInteractableComponent& Interactable, bool bUsing);
	void DrawRing(const UNBInteractableComponent& Interactable, const FVector2D& Center, float Radius);
	void DrawBar(float X, float Y, float Width, float Height, float Fill, const FLinearColor& Color);
	void DrawCenteredText(const FString& Text, float Y, const FLinearColor& Color, float Scale);
};
