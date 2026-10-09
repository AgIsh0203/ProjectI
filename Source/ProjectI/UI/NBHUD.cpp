// Squirrel Wheels prototype.

#include "UI/NBHUD.h"

#include "Blueprint/UserWidget.h"
#include "Car/NBCar.h"
#include "Engine/Canvas.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerController.h"
#include "Parts/NBCarPartComponent.h"
#include "Engine/Engine.h"
#include "Engine/Font.h"
#include "Engine/GameInstance.h"
#include "Game/NBSessionSubsystem.h"
#include "GameFramework/GameStateBase.h"
#include "Interaction/NBInteractableComponent.h"
#include "Game/NBRunGameState.h"
#include "Player/NBPlayerState.h"
#include "Player/NBSquirrel.h"
#include "UI/NBLobbyMenu.h"

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

void ANBHUD::BeginPlay()
{
	Super::BeginPlay();
	if (!PlayerOwner || !PlayerOwner->IsLocalController())
	{
		return;
	}
	if (GetNetMode() == NM_Standalone)
	{
		OpenMenu();
		return;
	}
	// Arriving from the menu (host / join / restart): the viewport may still be in UI-only input.
	PlayerOwner->SetInputMode(FInputModeGameOnly());
	PlayerOwner->SetShowMouseCursor(false);
}

void ANBHUD::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (Menu)
	{
		Menu->RemoveFromParent();
		Menu = nullptr;
		bMenuOpen = false;
	}
	Super::EndPlay(EndPlayReason);
}

void ANBHUD::OpenMenu()
{
	if (!PlayerOwner || IsMenuOpen())
	{
		return;
	}
	if (!Menu)
	{
		Menu = CreateWidget<UNBLobbyMenu>(PlayerOwner.Get(), UNBLobbyMenu::StaticClass());
	}
	Menu->AddToViewport(10);
	bMenuOpen = true;

	FInputModeUIOnly Input;
	Input.SetWidgetToFocus(Menu->TakeWidget());
	Input.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	PlayerOwner->SetInputMode(Input);
	PlayerOwner->SetShowMouseCursor(true);
	Menu->FocusFirstButton();
}

void ANBHUD::CloseMenu()
{
	if (!IsMenuOpen())
	{
		return;
	}
	Menu->RemoveFromParent();
	bMenuOpen = false;
	if (PlayerOwner)
	{
		PlayerOwner->SetInputMode(FInputModeGameOnly());
		PlayerOwner->SetShowMouseCursor(false);
	}
}

void ANBHUD::ToggleMenu()
{
	if (IsMenuOpen())
	{
		CloseMenu();
	}
	else
	{
		OpenMenu();
	}
}

bool ANBHUD::IsMenuOpen() const
{
	return Menu && bMenuOpen;
}

void ANBHUD::DrawHUD()
{
	Super::DrawHUD();

	const ANBSquirrel* Squirrel = Cast<ANBSquirrel>(GetOwningPawn());
	if (!Squirrel || !Canvas)
	{
		return;
	}

	DrawCarStatus();
	DrawNetStatus();
	DrawRunStatus();

	if (Squirrel->IsRagdolled())
	{
		DrawCenteredText(TEXT("WHEEEEE!"), Canvas->ClipY * 0.3f, ProgressColor, 3.f);
	}
	const float RespawnLeft = Squirrel->GetRespawnSecondsLeft();
	if (RespawnLeft >= 0.f)
	{
		const FString Countdown = FString::Printf(TEXT("Get back in the car!  %d"), FMath::CeilToInt(RespawnLeft));
		DrawCenteredText(Countdown, Canvas->ClipY * 0.3f + 70.f, BadColor, 1.8f);
	}

	if (const UNBInteractableComponent* Interactable = Squirrel->GetFocusedInteractable())
	{
		DrawInteractable(*Interactable, Interactable->IsUsedBy(Squirrel));
	}

	const FString Hint = Squirrel->IsClinging() ? TEXT("Keep holding LMB    Space: let go")
		: Squirrel->IsSeated() ? TEXT("E: hop seat    Space: jump out    LMB: work")
		: TEXT("E: get in    Space: jump    LMB: work");
	DrawCenteredText(Hint, Canvas->ClipY - 36.f, FLinearColor(1.f, 1.f, 1.f, 0.7f), 1.f);
}

void ANBHUD::DrawCarStatus()
{
	// Pulse so failures catch the eye on stream.
	const float Pulse = 0.65f + 0.35f * FMath::Sin(GetWorld()->GetTimeSeconds() * 8.f);
	const FLinearColor Alarm(BadColor.R, BadColor.G * Pulse, BadColor.B * Pulse, 1.f);

	float ListY = 20.f;
	for (TActorIterator<ANBCar> It(GetWorld()); It; ++It)
	{
		if (It->IsFlipped())
		{
			DrawCenteredText(TEXT("CAR FLIPPED!  Two squirrels: hold LMB next to it"), ListY, Alarm, 1.7f);
			ListY += 36.f;
		}
		for (const UNBCarPartComponent* Part : It->GetParts())
		{
			if (!Part || !Part->IsFailed())
			{
				continue;
			}
			const FString Line = FString::Printf(TEXT("%s %s!"), *Part->PartName.ToString(), *Part->FailureText.ToString());
			DrawCenteredText(Line, ListY, Alarm, 1.4f);
			ListY += 30.f;

			FVector2D Screen;
			if (PlayerOwner && PlayerOwner->ProjectWorldLocationToScreen(Part->GetComponentLocation(), Screen))
			{
				const float Size = 26.f;
				DrawRect(Alarm, Screen.X - Size * 0.5f, Screen.Y - Size * 0.5f, Size, Size);
				DrawText(TEXT("!"), FLinearColor::White, Screen.X - 4.f, Screen.Y - 12.f, GEngine->GetMediumFont(), 1.3f);
				DrawText(Part->PartName.ToString(), Alarm, Screen.X + Size * 0.7f, Screen.Y - 10.f, GEngine->GetMediumFont(), 1.f);
			}
		}
	}
}

void ANBHUD::DrawRunStatus()
{
	const ANBRunGameState* State = GetWorld()->GetGameState<ANBRunGameState>();
	if (!State)
	{
		return;
	}
	const ANBCar* Car = nullptr;
	for (TActorIterator<ANBCar> It(GetWorld()); It; ++It)
	{
		Car = *It;
		break;
	}

	UFont* Font = GEngine->GetMediumFont();
	const float Right = Canvas->ClipX - 24.f;
	auto DrawRight = [&](const FString& Text, float Y, const FLinearColor& Color, float Scale)
	{
		float W = 0.f, H = 0.f;
		GetTextSize(Text, W, H, Font, Scale);
		DrawText(Text, Color, Right - W, Y, Font, Scale);
	};

	const float SecondsLeft = State->GetSecondsLeft();
	switch (State->GetPhase())
	{
	case ENBRunPhase::Waiting:
	{
		const int32 Players = State->PlayerArray.Num();
		const FString Waiting = Players < 2 ? TEXT("Waiting for another squirrel...")
			: GetNetMode() == NM_Client ? TEXT("Waiting for the host to start...")
			: TEXT("Everyone here? Esc: start the run");
		DrawCenteredText(Waiting, Canvas->ClipY * 0.25f, FLinearColor::White, 2.f);
		break;
	}
	case ENBRunPhase::Countdown:
		DrawCenteredText(SecondsLeft > 0.f ? FString::FromInt(FMath::CeilToInt(SecondsLeft)) : TEXT("GO!"), Canvas->ClipY * 0.25f, ProgressColor, 4.f);
		DrawCenteredText(TEXT("Everyone get in the car!"), Canvas->ClipY * 0.25f + 90.f, FLinearColor::White, 1.5f);
		break;
	case ENBRunPhase::Driving:
	{
		const int32 Total = FMath::CeilToInt(SecondsLeft);
		const bool bLow = SecondsLeft < 20.f;
		const float Blink = bLow ? 0.65f + 0.35f * FMath::Sin(GetWorld()->GetTimeSeconds() * 10.f) : 1.f;
		const FLinearColor TimeColor = bLow ? FLinearColor(BadColor.R, BadColor.G * Blink, BadColor.B * Blink) : FLinearColor::White;
		DrawRight(FString::Printf(TEXT("%d:%02d"), Total / 60, Total % 60), 14.f, TimeColor, 2.2f);

		if (State->GetWreckSeconds() > 0.f)
		{
			const float Fill = State->GetWreckSeconds() / 10.f;
			DrawCenteredText(TEXT("CAR FALLING APART!  Fix something!"), Canvas->ClipY * 0.2f, BadColor, 1.6f);
			DrawBar((Canvas->ClipX - 360.f) * 0.5f, Canvas->ClipY * 0.2f + 36.f, 360.f, 14.f, Fill, BadColor);
		}
		break;
	}
	case ENBRunPhase::Finished:
		DrawEndScreen(*State);
		break;
	}

	if (Car && State->GetPhase() != ENBRunPhase::Finished)
	{
		DrawRight(FString::Printf(TEXT("ACORNS  %d / %d"), Car->GetAcorns(), Car->GetMaxAcorns()), 70.f, ProgressColor, 1.5f);
	}
}

void ANBHUD::DrawEndScreen(const ANBRunGameState& State)
{
	const float PanelW = 560.f;
	const float PanelH = 400.f;
	const float PanelX = (Canvas->ClipX - PanelW) * 0.5f;
	const float PanelY = (Canvas->ClipY - PanelH) * 0.5f;
	DrawRect(FLinearColor(0.f, 0.f, 0.f, 0.75f), PanelX, PanelY, PanelW, PanelH);

	FString Title;
	FLinearColor TitleColor = GoodColor;
	switch (State.GetResult())
	{
	case ENBRunResult::Delivered: Title = TEXT("ACORNS DELIVERED!"); break;
	case ENBRunResult::TimeUp:    Title = TEXT("TIME'S UP"); TitleColor = BadColor; break;
	case ENBRunResult::Wrecked:   Title = TEXT("TOTAL WRECK"); TitleColor = BadColor; break;
	default: break;
	}
	float Y = PanelY + 20.f;
	DrawCenteredText(Title, Y, TitleColor, 2.4f);
	Y += 70.f;

	if (State.GetResult() == ENBRunResult::Delivered)
	{
		DrawCenteredText(FString::Printf(TEXT("Acorns delivered: %d"), State.GetAcornsDelivered()), Y, FLinearColor::White, 1.3f);
		DrawCenteredText(FString::Printf(TEXT("Time bonus: +%d"), State.GetTimeBonus()), Y + 30.f, FLinearColor::White, 1.3f);
		DrawCenteredText(FString::Printf(TEXT("Respawn penalty: -%d"), State.GetRespawnPenalty()), Y + 60.f, FLinearColor::White, 1.3f);
		DrawCenteredText(FString::Printf(TEXT("SCORE  %d"), State.GetFinalScore()), Y + 100.f, ProgressColor, 2.2f);
	}
	else
	{
		DrawCenteredText(TEXT("The acorns never made it."), Y + 20.f, FLinearColor::White, 1.4f);
	}
	Y += 170.f;

	for (const APlayerState* Player : State.PlayerArray)
	{
		if (const ANBPlayerState* Stats = Cast<ANBPlayerState>(Player))
		{
			const FString Line = FString::Printf(TEXT("%s    falls: %d    respawns: %d"), *Stats->GetPlayerName(), Stats->GetFalls(), Stats->GetRespawns());
			DrawCenteredText(Line, Y, FLinearColor(1.f, 1.f, 1.f, 0.8f), 1.f);
			Y += 24.f;
		}
	}
	DrawCenteredText(FString::Printf(TEXT("Restarting in %d"), FMath::CeilToInt(State.GetSecondsLeft())), PanelY + PanelH - 36.f, FLinearColor(1.f, 1.f, 1.f, 0.6f), 1.f);
}

void ANBHUD::DrawNetStatus()
{
	const UNBSessionSubsystem* Sessions = GetGameInstance()->GetSubsystem<UNBSessionSubsystem>();
	const AGameStateBase* GameState = GetWorld()->GetGameState();
	const int32 Players = GameState ? GameState->PlayerArray.Num() : 1;

	FString Line;
	switch (GetNetMode())
	{
	case NM_Standalone:
		Line = TEXT("OFFLINE    Esc: menu");
		break;
	case NM_ListenServer:
		Line = FString::Printf(TEXT("HOST    %d / 4 squirrels    Esc: menu"), Players);
		break;
	default:
		Line = FString::Printf(TEXT("CONNECTED    %d / 4 squirrels    Esc: menu"), Players);
		break;
	}
	if (Sessions)
	{
		Line += FString::Printf(TEXT("    [%s]"), *Sessions->GetSubsystemName());
	}
	UFont* Font = GEngine->GetSmallFont();
	DrawText(Line, FLinearColor(1.f, 1.f, 1.f, 0.8f), 16.f, 12.f, Font, 1.2f);
	if (Sessions && !Sessions->GetStatusText().IsEmpty())
	{
		DrawText(Sessions->GetStatusText(), ProgressColor, 16.f, 32.f, Font, 1.2f);
	}
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
