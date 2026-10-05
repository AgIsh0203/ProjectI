// Squirrel Wheels prototype.

#include "UI/NBHUD.h"

#include "Engine/Canvas.h"
#include "Engine/Engine.h"
#include "Engine/Font.h"
#include "Interaction/NBInteractableComponent.h"
#include "Player/NBSquirrel.h"

namespace
{
	const FLinearColor PanelColor(0.f, 0.f, 0.f, 0.55f);
	const FLinearColor ProgressColor(1.f, 0.75f, 0.1f);
	const FLinearColor GoodColor(0.3f, 1.f, 0.3f);
	const FLinearColor BadColor(1.f, 0.25f, 0.2f);

	/** How long the NICE/MISS flash stays up after a timing-ring press. */
	constexpr double RingFeedbackSeconds = 0.45;

	const TCHAR* VerbFor(ENBInteractMode Mode)
	{
		switch (Mode)
		{
		case ENBInteractMode::Hold:       return TEXT("HOLD");
		case ENBInteractMode::Mash:       return TEXT("MASH");
		case ENBInteractMode::TimingRing: return TEXT("TIME IT");
		case ENBInteractMode::Push2:      return TEXT("PUSH TOGETHER");
		}
		return TEXT("");
	}
}

void ANBHUD::DrawHUD()
{
	Super::DrawHUD();

	const ANBSquirrel* Squirrel = Cast<ANBSquirrel>(GetOwningPawn());
	if (!Squirrel || !Canvas)
	{
		return;
	}

	if (const UNBInteractableComponent* Interactable = Squirrel->GetFocusedInteractable())
	{
		DrawInteractable(*Interactable, Interactable->IsUsedBy(Squirrel));
	}

	const FString Hint = Squirrel->IsSeated()
		? TEXT("E: hop seat    Space: jump out    LMB: work")
		: TEXT("E: get in    Space: jump    LMB: work");
	DrawCenteredText(Hint, Canvas->ClipY - 36.f, FLinearColor(1.f, 1.f, 1.f, 0.7f), 1.f);
}

void ANBHUD::DrawInteractable(const UNBInteractableComponent& Interactable, bool bUsing)
{
	const float PanelW = 420.f;
	const float PanelH = Interactable.Mode == ENBInteractMode::TimingRing ? 250.f : 110.f;
	const float PanelX = (Canvas->ClipX - PanelW) * 0.5f;
	const float PanelY = Canvas->ClipY - 70.f - PanelH;
	DrawRect(PanelColor, PanelX, PanelY, PanelW, PanelH);

	const FString Title = FString::Printf(TEXT("[LMB] %s  %s"), VerbFor(Interactable.Mode), *Interactable.Prompt.ToString());
	DrawCenteredText(Title, PanelY + 10.f, bUsing ? ProgressColor : FLinearColor::White, 1.4f);

	float Y = PanelY + 50.f;
	if (Interactable.Mode == ENBInteractMode::Push2)
	{
		const FString Count = FString::Printf(TEXT("Pushing: %d / %d"), Interactable.GetNumUsers(), Interactable.RequiredUsers);
		const bool bEnough = Interactable.GetNumUsers() >= Interactable.RequiredUsers;
		DrawCenteredText(Count, Y - 4.f, bEnough ? GoodColor : BadColor, 1.1f);
		Y += 24.f;
	}
	else if (Interactable.Mode == ENBInteractMode::TimingRing)
	{
		DrawRing(Interactable, FVector2D(Canvas->ClipX * 0.5f, Y + 70.f), 60.f);
		Y += 150.f;
	}

	DrawBar(PanelX + 20.f, Y, PanelW - 40.f, 18.f, Interactable.GetProgress(), ProgressColor);
}

void ANBHUD::DrawRing(const UNBInteractableComponent& Interactable, const FVector2D& Center, float Radius)
{
	// Phase 0 is at 12 o'clock and the marker runs clockwise.
	auto PointAt = [&Center](float Phase, float R)
	{
		const float Angle = Phase * 2.f * PI - HALF_PI;
		return Center + FVector2D(FMath::Cos(Angle), FMath::Sin(Angle)) * R;
	};

	constexpr int32 Segments = 64;
	for (int32 i = 0; i < Segments; ++i)
	{
		const float A = static_cast<float>(i) / Segments;
		const float B = static_cast<float>(i + 1) / Segments;
		const bool bSweet = Interactable.IsInSweetSpot((A + B) * 0.5f);
		Canvas->K2_DrawLine(PointAt(A, Radius), PointAt(B, Radius), bSweet ? 10.f : 3.f, bSweet ? GoodColor : FLinearColor(1.f, 1.f, 1.f, 0.6f));
	}

	const float Phase = Interactable.GetRingPhase();
	Canvas->K2_DrawLine(PointAt(Phase, Radius * 0.55f), PointAt(Phase, Radius * 1.25f), 5.f, ProgressColor);

	const ANBSquirrel* Squirrel = Cast<ANBSquirrel>(GetOwningPawn());
	bool bHit = false;
	double PressTime = 0.0;
	if (Squirrel && Squirrel->GetLastRingPress(bHit, PressTime) && GetWorld()->GetTimeSeconds() - PressTime < RingFeedbackSeconds)
	{
		const FString Feedback = bHit ? TEXT("NICE!") : TEXT("MISS");
		DrawCenteredText(Feedback, Center.Y - 14.f, bHit ? GoodColor : BadColor, 1.5f);
	}
}

void ANBHUD::DrawBar(float X, float Y, float Width, float Height, float Fill, const FLinearColor& Color)
{
	DrawRect(FLinearColor(1.f, 1.f, 1.f, 0.15f), X, Y, Width, Height);
	DrawRect(Color, X, Y, Width * FMath::Clamp(Fill, 0.f, 1.f), Height);
}

void ANBHUD::DrawCenteredText(const FString& Text, float Y, const FLinearColor& Color, float Scale)
{
	UFont* Font = GEngine->GetMediumFont();
	float W = 0.f, H = 0.f;
	GetTextSize(Text, W, H, Font, Scale);
	DrawText(Text, Color, (Canvas->ClipX - W) * 0.5f, Y, Font, Scale);
}
