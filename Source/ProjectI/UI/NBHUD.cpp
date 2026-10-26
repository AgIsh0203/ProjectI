// Squirrel Wheels prototype.

#include "UI/NBHUD.h"

#include "Audio/NBAudioSettings.h"
#include "Blueprint/UserWidget.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"
#include "CanvasItem.h"
#include "Car/NBCar.h"
#include "Car/NBSeatComponent.h"
#include "Engine/Canvas.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerController.h"
#include "Parts/NBCarPartComponent.h"
#include "Engine/Engine.h"
#include "Engine/Font.h"
#include "Engine/FontFace.h"
#include "Engine/Texture2D.h"
#include "GlobalRenderResources.h"
#include "Engine/GameInstance.h"
#include "Game/NBSessionSubsystem.h"
#include "GameFramework/GameStateBase.h"
#include "Interaction/NBInteractableComponent.h"
#include "Game/NBDog.h"
#include "Game/NBRoute.h"
#include "Game/NBRunGameMode.h"
#include "Game/NBRunGameState.h"
#include "Player/NBControlSettings.h"
#include "Player/NBPlayerState.h"
#include "Player/NBSquirrel.h"
#include "UI/NBLobbyMenu.h"
#include "UObject/ConstructorHelpers.h"

namespace
{
	const FLinearColor PanelColor(0.f, 0.f, 0.f, 0.55f);
	const FLinearColor ProgressColor(1.f, 0.75f, 0.1f);
	const FLinearColor GoodColor(0.3f, 1.f, 0.3f);
	const FLinearColor BadColor(1.f, 0.25f, 0.2f);

	/** How long the NICE/MISS flash stays up after a timing-ring press. */
	constexpr double RingFeedbackSeconds = 0.45;

	/** How long a failure callout stays on screen, and how many show at once. */
	constexpr float CalloutSeconds = 2.4f;
	constexpr int32 MaxCallouts = 3;

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

ANBHUD::ANBHUD()
{
	// /Game/ThirdParty is always cooked; only this constructor references the face.
	static ConstructorHelpers::FObjectFinder<UFontFace> ShoutFace(TEXT("/Game/ThirdParty/Fonts/LilitaOne-Regular.LilitaOne-Regular"));
	ShoutFontFace = ShoutFace.Object;

	// The run's sounds, all CC0 (CREDITS.md). Like the font, only this constructor references them.
	static ConstructorHelpers::FObjectFinder<USoundBase> MenuMusicFinder(TEXT("/Game/ThirdParty/OpenGameArt/Music/SW_Music_Menu.SW_Music_Menu"));
	static ConstructorHelpers::FObjectFinder<USoundBase> RunMusicFinder(TEXT("/Game/ThirdParty/OpenGameArt/Music/SW_Music_Run.SW_Music_Run"));
	static ConstructorHelpers::FObjectFinder<USoundBase> BeepFinder(TEXT("/Game/ThirdParty/Kenney/Sounds/SW_Countdown_Beep.SW_Countdown_Beep"));
	static ConstructorHelpers::FObjectFinder<USoundBase> GoFinder(TEXT("/Game/ThirdParty/Kenney/Sounds/SW_Countdown_Go.SW_Countdown_Go"));
	static ConstructorHelpers::FObjectFinder<USoundBase> DeliveredFinder(TEXT("/Game/ThirdParty/Kenney/Sounds/SW_Run_Delivered.SW_Run_Delivered"));
	static ConstructorHelpers::FObjectFinder<USoundBase> FailedFinder(TEXT("/Game/ThirdParty/Kenney/Sounds/SW_Run_Failed.SW_Run_Failed"));
	MenuMusic = MenuMusicFinder.Object;
	RunMusic = RunMusicFinder.Object;
	CountdownBeep = BeepFinder.Object;
	CountdownGo = GoFinder.Object;
	DeliveredSound = DeliveredFinder.Object;
	FailedSound = FailedFinder.Object;

	// Button icons in ENBPrompt order: keyboard/mouse, then Xbox (the common pad layout on PC).
	static const TCHAR* const KeyNames[] = {TEXT("keyboard_e"), TEXT("keyboard_space"), TEXT("mouse_left"), TEXT("keyboard_h"), TEXT("keyboard_escape")};
	static const TCHAR* const PadNames[] = {TEXT("xbox_button_color_x"), TEXT("xbox_button_color_a"), TEXT("xbox_button_color_b"), TEXT("xbox_button_color_y"), TEXT("xbox_button_menu")};
	for (int32 i = 0; i < UE_ARRAY_COUNT(KeyNames); ++i)
	{
		ConstructorHelpers::FObjectFinder<UTexture2D> Key(*FString::Printf(TEXT("/Game/ThirdParty/Kenney/InputPrompts/T_%s.T_%s"), KeyNames[i], KeyNames[i]));
		ConstructorHelpers::FObjectFinder<UTexture2D> Pad(*FString::Printf(TEXT("/Game/ThirdParty/Kenney/InputPrompts/T_%s.T_%s"), PadNames[i], PadNames[i]));
		KeyIcons[i] = Key.Object;
		PadIcons[i] = Pad.Object;
	}
}

void ANBHUD::UpdateInputDevice()
{
	if (!PlayerOwner)
	{
		return;
	}
	static const FKey PadButtons[] = {EKeys::Gamepad_FaceButton_Bottom, EKeys::Gamepad_FaceButton_Right, EKeys::Gamepad_FaceButton_Left,
		EKeys::Gamepad_FaceButton_Top, EKeys::Gamepad_Special_Right, EKeys::Gamepad_Special_Left, EKeys::Gamepad_LeftShoulder,
		EKeys::Gamepad_RightShoulder, EKeys::Gamepad_LeftTrigger, EKeys::Gamepad_RightTrigger, EKeys::Gamepad_DPad_Up,
		EKeys::Gamepad_DPad_Down, EKeys::Gamepad_DPad_Left, EKeys::Gamepad_DPad_Right, EKeys::Gamepad_LeftThumbstick, EKeys::Gamepad_RightThumbstick};
	static const FKey PadSticks[] = {EKeys::Gamepad_LeftX, EKeys::Gamepad_LeftY, EKeys::Gamepad_RightX, EKeys::Gamepad_RightY};
	static const FKey KeyboardKeys[] = {EKeys::W, EKeys::A, EKeys::S, EKeys::D, EKeys::E, EKeys::SpaceBar, EKeys::H, EKeys::Escape,
		EKeys::LeftMouseButton, EKeys::RightMouseButton, EKeys::One, EKeys::Two, EKeys::Three, EKeys::Four};

	for (const FKey& Key : PadButtons)
	{
		if (PlayerOwner->IsInputKeyDown(Key))
		{
			bUsingGamepad = true;
			return;
		}
	}
	for (const FKey& Stick : PadSticks)
	{
		// Past the dead zone, so a drifting stick doesn't steal the prompts back.
		if (FMath::Abs(PlayerOwner->GetInputAnalogKeyState(Stick)) > 0.35f)
		{
			bUsingGamepad = true;
			return;
		}
	}
	float MouseX = 0.f, MouseY = 0.f;
	PlayerOwner->GetInputMouseDelta(MouseX, MouseY);
	if (FMath::Abs(MouseX) + FMath::Abs(MouseY) > 0.5f)
	{
		bUsingGamepad = false;
		return;
	}
	for (const FKey& Key : KeyboardKeys)
	{
		if (PlayerOwner->IsInputKeyDown(Key))
		{
			bUsingGamepad = false;
			return;
		}
	}
}

namespace
{
	/** The rebindable action behind a prompt (Menu has none: Esc / Tab are fixed). */
	ENBBinding PromptBinding(ENBPrompt Prompt)
	{
		switch (Prompt)
		{
		case ENBPrompt::Interact: return ENBBinding::Interact;
		case ENBPrompt::Jump: return ENBBinding::Jump;
		case ENBPrompt::Work: return ENBBinding::Work;
		case ENBPrompt::Horn: return ENBBinding::Horn;
		default: return ENBBinding::Count;
		}
	}
}

UTexture2D* ANBHUD::PromptIcon(ENBPrompt Prompt) const
{
	const int32 Index = static_cast<int32>(Prompt);
	if (Index >= 5)
	{
		return nullptr;
	}
	// The icons show the default keys and buttons; a rebound one falls back to its name.
	const ENBBinding Binding = PromptBinding(Prompt);
	const UNBControlSettings* Controls = UNBControlSettings::Get(this);
	if (bUsingGamepad)
	{
		if (Binding != ENBBinding::Count && Controls && Controls->GetPadKey(Binding) != UNBControlSettings::GetDefaultPadKey(Binding))
		{
			return nullptr;
		}
		return PadIcons[Index].Get();
	}
	if (Binding != ENBBinding::Count && Controls && Controls->GetKey(Binding) != UNBControlSettings::GetDefaultKey(Binding))
	{
		return nullptr;
	}
	return KeyIcons[Index].Get();
}

FString ANBHUD::PromptName(ENBPrompt Prompt) const
{
	const int32 Index = static_cast<int32>(Prompt);
	if (Index >= 5)
	{
		return TEXT("?");
	}
	if (Prompt == ENBPrompt::Menu)
	{
		if (bUsingGamepad)
		{
			return TEXT("Menu");
		}
		// In the editor Esc stops Play In Editor, so point at Tab there.
		return GetWorld() && GetWorld()->IsPlayInEditor() ? TEXT("Tab") : TEXT("Esc");
	}
	return KeyLabel(PromptBinding(Prompt));
}

FString ANBHUD::KeyLabel(ENBBinding Binding) const
{
	const UNBControlSettings* Controls = UNBControlSettings::Get(this);
	if (bUsingGamepad && UNBControlSettings::HasPadButton(Binding))
	{
		return Controls ? Controls->GetPadKeyLabel(Binding) : UNBControlSettings::GetPadKeyLabel(UNBControlSettings::GetDefaultPadKey(Binding));
	}
	return Controls ? Controls->GetKeyLabel(Binding) : UNBControlSettings::GetKeyLabel(UNBControlSettings::GetDefaultKey(Binding));
}

void ANBHUD::DrawPromptRow(const TArray<TPair<ENBPrompt, FString>>& Items, float Y, float Scale, const FLinearColor& Color, float CenterX)
{
	UFont* Font = GEngine->GetMediumFont();
	const float IconSize = 30.f * Scale;
	const float IconGap = 6.f * Scale;
	const float ItemGap = 26.f * Scale;

	// Measure first so the row centres.
	TArray<float> TextWidths;
	TArray<float> KeyWidths;
	float Total = 0.f;
	for (const TPair<ENBPrompt, FString>& Item : Items)
	{
		float W = 0.f, H = 0.f;
		GetTextSize(Item.Value, W, H, Font, Scale);
		TextWidths.Add(W);
		// A rebound key has no icon: its "[name]" may be wider than an icon.
		float KeyW = IconSize;
		if (!PromptIcon(Item.Key))
		{
			GetTextSize(FString::Printf(TEXT("[%s]"), *PromptName(Item.Key)), KeyW, H, Font, Scale);
			KeyW = FMath::Max(KeyW, IconSize);
		}
		KeyWidths.Add(KeyW);
		Total += KeyW + IconGap + W;
	}
	Total += ItemGap * FMath::Max(Items.Num() - 1, 0);

	float X = (CenterX < 0.f ? Canvas->ClipX * 0.5f : CenterX) - Total * 0.5f;
	for (int32 i = 0; i < Items.Num(); ++i)
	{
		if (UTexture2D* Icon = PromptIcon(Items[i].Key))
		{
			DrawTextureSimple(Icon, X, Y - IconSize * 0.5f, IconSize / FMath::Max(Icon->GetSizeX(), 1));
		}
		else
		{
			// No icon asset: fall back to the key's name in brackets.
			DrawText(FString::Printf(TEXT("[%s]"), *PromptName(Items[i].Key)), Color, X, Y - 9.f * Scale, Font, Scale);
		}
		X += KeyWidths[i] + IconGap;
		DrawText(Items[i].Value, Color, X, Y - 9.f * Scale, Font, Scale);
		X += TextWidths[i] + ItemGap;
	}
}

void ANBHUD::PlayMusic(USoundBase* Music)
{
	if (Music == PlayingMusic)
	{
		return;
	}
	PlayingMusic = Music;
	if (MusicComponent)
	{
		// Let the old track die away while the new one comes in.
		MusicComponent->FadeOut(1.f, 0.f);
		MusicComponent = nullptr;
	}
	if (Music)
	{
		// 2D; the music assets loop. Auto-destroyed once faded out.
		MusicComponent = UGameplayStatics::CreateSound2D(this, Music, MusicVolume, 1.f, 0.f, nullptr, false, true);
		if (MusicComponent)
		{
			MusicComponent->FadeIn(1.f, MusicVolume);
		}
	}
}

void ANBHUD::UpdateRunAudio()
{
	const ANBRunGameState* State = GetWorld()->GetGameState<ANBRunGameState>();
	if (!State)
	{
		PlayMusic(MenuMusic);
		return;
	}
	const ENBRunPhase Phase = State->GetPhase();
	const bool bPhaseChanged = LastPhase != static_cast<uint8>(Phase);
	// A late joiner (or the first frame) only takes the current state: no jingle for an old ending.
	const bool bFirstFrame = LastPhase == 0xFF;
	LastPhase = static_cast<uint8>(Phase);

	switch (Phase)
	{
	case ENBRunPhase::Waiting:
		PlayMusic(MenuMusic);
		break;

	case ENBRunPhase::Countdown:
	{
		// Music drops out so the beeps carry the tension.
		PlayMusic(nullptr);
		const int32 Second = FMath::CeilToInt(State->GetSecondsLeft());
		if (Second != LastCountdownSecond && Second > 0)
		{
			UGameplayStatics::PlaySound2D(this, CountdownBeep, 0.7f);
		}
		LastCountdownSecond = Second;
		break;
	}

	case ENBRunPhase::Driving:
		if (bPhaseChanged && !bFirstFrame)
		{
			UGameplayStatics::PlaySound2D(this, CountdownGo, 0.8f);
		}
		PlayMusic(RunMusic);
		break;

	case ENBRunPhase::Finished:
		if (bPhaseChanged && !bFirstFrame)
		{
			const bool bDelivered = State->GetResult() == ENBRunResult::Delivered;
			UGameplayStatics::PlaySound2D(this, bDelivered ? DeliveredSound.Get() : FailedSound.Get(), 0.9f);
		}
		PlayMusic(nullptr);
		break;
	}
	if (Phase != ENBRunPhase::Countdown)
	{
		LastCountdownSecond = -1;
	}
}

UFont* ANBHUD::GetShoutFont() const
{
	return ShoutFont ? ShoutFont.Get() : GEngine->GetLargeFont();
}

void ANBHUD::BeginPlay()
{
	Super::BeginPlay();
	if (ShoutFontFace && !ShoutFont)
	{
		// A runtime (composite) font draws through the Slate font cache at any size. LegacyFontSize
		// matches the engine's large font, so the shout scales stay the same.
		ShoutFont = NewObject<UFont>(this, TEXT("ShoutFont"), RF_Transient);
		ShoutFont->FontCacheType = EFontCacheType::Runtime;
		ShoutFont->LegacyFontSize = 24;
		FTypefaceEntry& Regular = ShoutFont->GetMutableInternalCompositeFont().DefaultTypeface.Fonts.Emplace_GetRef(TEXT("Regular"));
		Regular.Font = FFontData(ShoutFontFace);
	}
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
	if (MusicComponent)
	{
		MusicComponent->Stop();
		MusicComponent = nullptr;
	}
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
	// Before the pawn check: the menu music also plays while no squirrel is possessed yet.
	UpdateRunAudio();
	UpdateInputDevice();

	const ANBSquirrel* Squirrel = Cast<ANBSquirrel>(GetOwningPawn());
	if (!Squirrel || !Canvas)
	{
		return;
	}

	DrawSeatLabels();
	DrawNameTags();
	DrawCarStatus();
	DrawNetStatus();
	DrawRunStatus();
	DrawVoiceStatus();
	UpdateCallouts();
	DrawCallouts();
	DrawHornBubble();
	DrawHandbrakeBubble();
	DrawDogBubbles();
	DrawJumpBubble();
	DrawOffroadWarning();
	DrawCarReset(*Squirrel);

	DrawEjectReason(*Squirrel);
	const float RespawnLeft = Squirrel->GetRespawnSecondsLeft();
	if (RespawnLeft >= 0.f)
	{
		// Say what the countdown is for (friend test 2: "I don't see the point in needing to return the car quickly").
		const ANBRunGameState* RunState = GetWorld()->GetGameState<ANBRunGameState>();
		const int32 Penalty = RunState ? RunState->GetRespawnPenalty() : 0;
		const FString Countdown = FString::Printf(TEXT("Get back in a seat!  %d"), FMath::CeilToInt(RespawnLeft));
		DrawShoutText(Countdown, Canvas->ClipX * 0.5f, Canvas->ClipY * 0.3f + 100.f, BadColor, 0.9f);
		DrawShoutText(Penalty > 0 ? FString::Printf(TEXT("or you respawn at the car: -%d points"), Penalty) : FString(TEXT("or you respawn at the car")),
			Canvas->ClipX * 0.5f, Canvas->ClipY * 0.3f + 138.f, FLinearColor::White, 0.5f);
	}
	DrawSeatHint(*Squirrel);

	const UNBInteractableComponent* Interactable = Squirrel->GetFocusedInteractable();
	if (Interactable)
	{
		DrawInteractable(*Interactable, Interactable->IsUsedBy(Squirrel));
	}
	DrawRoles();

	TArray<TPair<ENBPrompt, FString>> Hint;
	if (Squirrel->IsClinging())
	{
		Hint = {{ENBPrompt::Work, TEXT("keep holding")}, {ENBPrompt::Jump, TEXT("let go")}};
	}
	else if (Squirrel->IsSeated())
	{
		Hint = {{ENBPrompt::Interact, TEXT("hop seat")}, {ENBPrompt::Jump, TEXT("jump out")}, {ENBPrompt::Work, TEXT("work")}, {ENBPrompt::Horn, TEXT("horn")}};
	}
	else
	{
		Hint = {{ENBPrompt::Interact, TEXT("get in")}, {ENBPrompt::Jump, TEXT("jump")}, {ENBPrompt::Work, TEXT("work")}};
	}
	DrawPromptRow(Hint, Canvas->ClipY - 30.f, 1.f, FLinearColor(1.f, 1.f, 1.f, 0.8f));
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
			DrawCenteredText(FString::Printf(TEXT("CAR FLIPPED!  Two squirrels: hold %s next to it"), *PromptName(ENBPrompt::Work)), ListY, Alarm, 1.7f);
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
	AlarmStackBottom = ListY;
}

void ANBHUD::UpdateCallouts()
{
	const double Now = GetWorld()->GetTimeSeconds();
	for (TActorIterator<ANBCar> It(GetWorld()); It; ++It)
	{
		for (const UNBCarPartComponent* Part : It->GetParts())
		{
			if (!Part)
			{
				continue;
			}
			const bool bFailed = Part->IsFailed();
			const bool* Before = SeenFailed.Find(Part);
			if (Before && !*Before && bFailed)
			{
				const FText& Text = Part->CalloutText.IsEmpty() ? Part->PartName : Part->CalloutText;
				Callouts.Add({Text.ToString(), Now});
			}
			SeenFailed.Add(Part, bFailed);
		}
	}
	for (TActorIterator<ANBDog> It(GetWorld()); It; ++It)
	{
		const uint8 DogState = static_cast<uint8>(It->GetState());
		const uint8* Before = SeenDogStates.Find(*It);
		if (Before && *Before != DogState && It->GetState() == ENBDogState::Biting)
		{
			Callouts.Add({FString::Printf(TEXT("THE DOG'S GOT A TYRE!  HONK! (%s)"), *PromptName(ENBPrompt::Horn)), Now});
		}
		SeenDogStates.Add(*It, DogState);
	}
	Callouts.RemoveAll([Now](const FCallout& Callout) { return Now - Callout.StartTime > CalloutSeconds; });
	// A burst of failures shows the newest few, not a wall of text.
	if (Callouts.Num() > MaxCallouts)
	{
		Callouts.RemoveAt(0, Callouts.Num() - MaxCallouts);
	}
}

void ANBHUD::DrawCallouts()
{
	const double Now = GetWorld()->GetTimeSeconds();
	// Newest just below the alarms at the top, older ones pushed down and shrunk.
	float Y = FMath::Max(Canvas->ClipY * 0.36f, AlarmStackBottom + 60.f);
	for (int32 i = Callouts.Num() - 1; i >= 0; --i)
	{
		const float Age = static_cast<float>(Now - Callouts[i].StartTime);
		// Slams in from twice the size, shakes for half a second, fades over the last 0.4 s.
		const float Slam = 1.f - FMath::SmoothStep(0.f, 0.18f, Age);
		const float Scale = (i == Callouts.Num() - 1 ? 3.2f : 2.f) * (1.f + Slam);
		const float Shake = FMath::Max(0.f, 1.f - Age / 0.5f) * FMath::Sin(Age * 45.f) * 10.f;
		const float Alpha = FMath::Clamp((CalloutSeconds - Age) / 0.4f, 0.f, 1.f);
		DrawShoutText(Callouts[i].Text, Canvas->ClipX * 0.5f + Shake, Y, FLinearColor(BadColor.R, BadColor.G, BadColor.B, Alpha), Scale);
		Y += i == Callouts.Num() - 1 ? 70.f : 48.f;
	}
}

void ANBHUD::DrawHornBubble()
{
	const double Now = GetWorld()->GetTimeSeconds();
	for (TActorIterator<ANBCar> It(GetWorld()); It; ++It)
	{
		const float Age = static_cast<float>(Now - It->GetLastHornTime());
		FVector2D Screen;
		if (Age > 0.6f || !PlayerOwner || !PlayerOwner->ProjectWorldLocationToScreen(It->GetActorLocation() + FVector(0.f, 0.f, 260.f), Screen))
		{
			continue;
		}
		const float Pop = 1.f + 0.6f * (1.f - FMath::SmoothStep(0.f, 0.12f, Age));
		DrawShoutText(TEXT("HONK!"), Screen.X, Screen.Y - 30.f, FLinearColor(ProgressColor.R, ProgressColor.G, ProgressColor.B, FMath::Clamp((0.6f - Age) / 0.2f, 0.f, 1.f)), 2.f * Pop);
	}
}

void ANBHUD::DrawHandbrakeBubble()
{
	const float Pulse = 0.7f + 0.3f * FMath::Sin(GetWorld()->GetTimeSeconds() * 12.f);
	for (TActorIterator<ANBCar> It(GetWorld()); It; ++It)
	{
		FVector2D Screen;
		if (It->IsHandbrakeOn() && PlayerOwner && PlayerOwner->ProjectWorldLocationToScreen(It->GetActorLocation() + FVector(0.f, 0.f, 260.f), Screen))
		{
			DrawShoutText(TEXT("HANDBRAKE!"), Screen.X, Screen.Y, FLinearColor(BadColor.R, BadColor.G, BadColor.B, Pulse), 1.4f);
		}
	}
}

void ANBHUD::DrawDogBubbles()
{
	const double Now = GetWorld()->GetTimeSeconds();
	const APawn* Viewer = PlayerOwner ? PlayerOwner->GetPawn() : nullptr;
	for (TActorIterator<ANBDog> It(GetWorld()); It; ++It)
	{
		FVector2D Screen;
		if (!PlayerOwner || !PlayerOwner->ProjectWorldLocationToScreen(It->GetBubbleLocation(), Screen))
		{
			continue;
		}
		const float BarkAge = static_cast<float>(Now - It->GetLastBarkTime());
		switch (It->GetState())
		{
		case ENBDogState::Biting:
		{
			const float Pulse = 1.f + 0.15f * FMath::Sin(Now * 20.f);
			DrawShoutText(TEXT("CHOMP!"), Screen.X, Screen.Y, BadColor, 1.6f * Pulse);
			break;
		}
		case ENBDogState::Scared:
			DrawShoutText(TEXT("YIPE!"), Screen.X, Screen.Y, GoodColor, 1.2f);
			break;
		case ENBDogState::Sleeping:
			if (Viewer && FVector::Dist(Viewer->GetActorLocation(), It->GetActorLocation()) < 4000.f)
			{
				DrawShoutText(TEXT("zzz"), Screen.X + 20.f * FMath::Sin(Now * 1.5f), Screen.Y, FLinearColor(1.f, 1.f, 1.f, 0.8f), 0.8f);
			}
			break;
		default:
			if (BarkAge < 0.6f)
			{
				const float Pop = 1.f + 0.6f * (1.f - FMath::SmoothStep(0.f, 0.12f, BarkAge));
				DrawShoutText(TEXT("WOOF!"), Screen.X, Screen.Y, FLinearColor(ProgressColor.R, ProgressColor.G, ProgressColor.B, FMath::Clamp((0.6f - BarkAge) / 0.2f, 0.f, 1.f)), 1.4f * Pop);
			}
			break;
		}
	}
}

void ANBHUD::DrawCarReset(const ANBSquirrel& Squirrel)
{
	const ANBRunGameState* State = GetWorld()->GetGameState<ANBRunGameState>();
	const ANBCar* Car = nullptr;
	for (TActorIterator<ANBCar> It(GetWorld()); It; ++It)
	{
		Car = *It;
		break;
	}
	if (!Car || !State || State->GetPhase() != ENBRunPhase::Driving)
	{
		StuckSeconds = 0.f;
		return;
	}
	const FString Key = KeyLabel(ENBBinding::ResetCar);
	const float Now = GetWorld()->GetTimeSeconds();

	// After a reset.
	if (Now - Car->GetLastResetSeenTime() < 2.5f)
	{
		DrawShoutText(FString::Printf(TEXT("CAR RESET!  -%d acorns"), Car->GetAcornsLostOnReset()), Canvas->ClipX * 0.5f, Canvas->ClipY * 0.42f, ProgressColor, 1.2f);
	}

	// Holding the key: a bar that fills.
	const float Hold = Squirrel.GetResetHoldProgress();
	if (Hold > 0.f)
	{
		const float Cooldown = Car->GetResetCooldownLeft();
		const float W = 320.f, X = (Canvas->ClipX - W) * 0.5f, Y = Canvas->ClipY * 0.5f + 60.f;
		DrawShoutText(Cooldown > 0.f ? FString::Printf(TEXT("Reset ready in %d s"), FMath::CeilToInt(Cooldown)) : FString(TEXT("Back to the trail...")),
			Canvas->ClipX * 0.5f, Y - 22.f, Cooldown > 0.f ? BadColor : FLinearColor::White, 0.6f);
		DrawBar(X, Y, W, 14.f, Cooldown > 0.f ? 0.f : Hold, ProgressColor);
	}

	// Stuck: slow and off the trail, or upside down, for a few seconds.
	const bool bStuck = Car->GetVelocity().Size() < 150.f && (Car->GetOffroadLevel() > 0.3f || Car->IsFlipped());
	StuckSeconds = bStuck ? StuckSeconds + GetWorld()->GetDeltaSeconds() : 0.f;
	if (StuckSeconds > 3.f && Hold <= 0.f)
	{
		const float Cooldown = Car->GetResetCooldownLeft();
		const FString Text = Cooldown > 0.f ? FString::Printf(TEXT("STUCK?  Reset ready in %d s"), FMath::CeilToInt(Cooldown))
			: FString::Printf(TEXT("STUCK?  Hold %s: back on the trail (-%d acorns)"), *Key, Car->GetAcornsLostOnReset());
		const float Bob = 4.f * FMath::Sin(Now * 3.f);
		DrawShoutText(Text, Canvas->ClipX * 0.5f, Canvas->ClipY - 230.f + Bob, FLinearColor(1.f, 0.85f, 0.3f), 0.7f);
	}
}

void ANBHUD::DrawOffroadWarning()
{
	for (TActorIterator<ANBCar> It(GetWorld()); It; ++It)
	{
		const float Level = It->GetOffroadLevel();
		if (Level < 0.15f)
		{
			continue;
		}
		// Brighter and shakier as the grip goes, so the driver knows to get back on the trail.
		const float Pulse = 0.75f + 0.25f * FMath::Sin(GetWorld()->GetTimeSeconds() * (4.f + 8.f * Level));
		const FString Text = FString::Printf(TEXT("OFF THE TRAIL!  grip -%d%%"), FMath::RoundToInt(Level * (1.f - It->GetOffroadMinGrip()) * 100.f));
		DrawShoutText(Text, Canvas->ClipX * 0.5f, Canvas->ClipY - 185.f, FLinearColor(1.f, 0.55f, 0.15f, Pulse), 0.6f + 0.25f * Level);
		return;
	}
}

void ANBHUD::DrawJumpBubble()
{
	const double Now = GetWorld()->GetTimeSeconds();
	for (TActorIterator<ANBCar> It(GetWorld()); It; ++It)
	{
		const float Age = static_cast<float>(Now - It->GetLastJumpSeenTime());
		FVector2D Screen;
		if (Age > 1.6f || It->GetLastJumpPoints() <= 0 || !PlayerOwner || !PlayerOwner->ProjectWorldLocationToScreen(It->GetActorLocation() + FVector(0.f, 0.f, 300.f), Screen))
		{
			continue;
		}
		const float Pop = 1.f + 0.6f * (1.f - FMath::SmoothStep(0.f, 0.15f, Age));
		const FLinearColor Color(GoodColor.R, GoodColor.G, GoodColor.B, FMath::Clamp((1.6f - Age) / 0.4f, 0.f, 1.f));
		DrawShoutText(FString::Printf(TEXT("AIR TIME! +%d"), It->GetLastJumpPoints()), Screen.X, Screen.Y - 30.f - 40.f * Age, Color, 1.6f * Pop);
	}
}

void ANBHUD::DrawSeatLabels()
{
	const float Bob = 6.f * FMath::Sin(GetWorld()->GetTimeSeconds() * 4.f);
	// They're for squirrels still choosing a seat; once seated they'd only cover the repair panel.
	const ANBSquirrel* Squirrel = Cast<ANBSquirrel>(GetOwningPawn());
	const bool bShow = !Squirrel || !Squirrel->IsSeated();
	TArray<TPair<ENBSeatRole, FVector2D>, TInlineAllocator<3>> Labels;
	bool bManualGears = false;
	for (TActorIterator<ANBCar> It(GetWorld()); It; ++It)
	{
		for (const UNBSeatComponent* Seat : It->GetSeats())
		{
			if (!Seat || Seat->Role == ENBSeatRole::Rider || UsedSeats.Contains(Seat))
			{
				continue;
			}
			if (!Seat->IsFree())
			{
				UsedSeats.Add(Seat);
				continue;
			}
			FVector2D Screen;
			if (!bShow || !PlayerOwner || !PlayerOwner->ProjectWorldLocationToScreen(Seat->GetComponentLocation() + FVector(0.f, 0.f, 45.f), Screen))
			{
				continue;
			}
			Labels.Add({Seat->Role, Screen});
		}
		bManualGears = It->IsManualGears();
	}

	// The seats are close together, so from most angles their labels landed on top of each other
	// (friend test 1). If any two overlap, spread them all sideways in seat order: wheel, pedals, gears.
	bool bOverlap = false;
	for (int32 i = 0; i < Labels.Num(); ++i)
	{
		for (int32 j = i + 1; j < Labels.Num(); ++j)
		{
			bOverlap |= FMath::Abs(Labels[i].Value.X - Labels[j].Value.X) < 220.f && FMath::Abs(Labels[i].Value.Y - Labels[j].Value.Y) < 90.f;
		}
	}
	if (bOverlap)
	{
		auto Order = [](ENBSeatRole SeatRole) { return SeatRole == ENBSeatRole::Wheel ? 0 : SeatRole == ENBSeatRole::Pedals ? 1 : 2; };
		Labels.Sort([&Order](const TPair<ENBSeatRole, FVector2D>& A, const TPair<ENBSeatRole, FVector2D>& B) { return Order(A.Key) < Order(B.Key); });
		float MidX = 0.f;
		for (const TPair<ENBSeatRole, FVector2D>& Label : Labels)
		{
			MidX += Label.Value.X / Labels.Num();
		}
		for (int32 i = 0; i < Labels.Num(); ++i)
		{
			Labels[i].Value.X = MidX + (i - (Labels.Num() - 1) * 0.5f) * 230.f;
		}
	}
	for (const TPair<ENBSeatRole, FVector2D>& Label : Labels)
	{
		const TCHAR* Title = Label.Key == ENBSeatRole::Wheel ? TEXT("WHEEL") : Label.Key == ENBSeatRole::Pedals ? TEXT("PEDALS") : TEXT("GEARS");
		const TCHAR* Job = Label.Key == ENBSeatRole::Wheel ? TEXT("steers") : Label.Key == ENBSeatRole::Pedals ? TEXT("gas + brake")
			: bManualGears ? TEXT("shifts + handbrake") : TEXT("handbrake");
		DrawShoutText(Title, Label.Value.X, Label.Value.Y - 40.f + Bob, GoodColor, 1.5f);
		DrawShoutText(Job, Label.Value.X, Label.Value.Y - 12.f + Bob, FLinearColor::White, 0.8f);
	}
}

namespace
{
	const TCHAR* RoleName(ENBSeatRole Role)
	{
		switch (Role)
		{
		case ENBSeatRole::Wheel: return TEXT("WHEEL");
		case ENBSeatRole::Pedals: return TEXT("PEDALS");
		case ENBSeatRole::Gears: return TEXT("GEARS");
		default: return TEXT("RIDER");
		}
	}
}

void ANBHUD::DrawNameTags()
{
	const ANBSquirrel* Me = Cast<ANBSquirrel>(GetOwningPawn());
	const float Time = GetWorld()->GetTimeSeconds();
	// Working a part, the tags of everyone on the car would sit over your squirrel's animation.
	const bool bMeWorking = Me && Me->GetWorkTarget();
	for (TActorIterator<ANBSquirrel> It(GetWorld()); It; ++It)
	{
		const ANBSquirrel* Squirrel = *It;
		const APlayerState* Player = Squirrel->GetPlayerState();
		FVector2D Screen;
		if (!Player || !PlayerOwner || !PlayerOwner->ProjectWorldLocationToScreen(Squirrel->GetActorLocation() + FVector(0.f, 0.f, 70.f), Screen))
		{
			continue;
		}
		FLinearColor Fur = Squirrel->GetFurColor();
		// Faint over a squirrel working a part, so its animation shows through (18 Oct: TailFan buried under UI).
		if (bMeWorking || Squirrel->GetWorkTarget())
		{
			Fur.A = 0.3f;
		}
		if (Squirrel == Me)
		{
			// A bobbing arrow: findable at a glance, even in a pile of squirrels.
			const float Y = Screen.Y - 34.f + 5.f * FMath::Sin(Time * 5.f);
			DrawShoutText(TEXT("YOU"), Screen.X, Y - 22.f, Fur, 0.9f);
			FCanvasTriangleItem Arrow(FVector2D(Screen.X - 11.f, Y - 6.f), FVector2D(Screen.X + 11.f, Y - 6.f), FVector2D(Screen.X, Y + 10.f), GWhiteTexture);
			Arrow.SetColor(Fur);
			Canvas->DrawItem(Arrow);
			continue;
		}
		FString Tag = Player->GetPlayerName();
		if (const UNBSeatComponent* Seat = Squirrel->GetCurrentSeat())
		{
			if (Seat->Role != ENBSeatRole::Rider)
			{
				Tag += FString::Printf(TEXT("  %s"), RoleName(Seat->Role));
			}
		}
		DrawShoutText(Tag, Screen.X, Screen.Y - 30.f, Fur, 0.7f);
	}
}

void ANBHUD::DrawRoles()
{
	const ANBSquirrel* Me = Cast<ANBSquirrel>(GetOwningPawn());
	const ANBRunGameState* State = GetWorld()->GetGameState<ANBRunGameState>();
	TActorIterator<ANBCar> CarIt(GetWorld());
	if (!Me || !CarIt)
	{
		return;
	}
	const ANBCar& Car = **CarIt;

	// Top left, under the net status: who has the wheel and the pedals.
	const bool bLive = State && State->GetPhase() == ENBRunPhase::Driving;
	const float Pulse = 0.6f + 0.4f * FMath::Sin(GetWorld()->GetTimeSeconds() * 8.f);
	float Y = 58.f;
	if (ANBRunGameMode::IsSoloDrive(GetWorld()))
	{
		// Dev solo drive: one squirrel is every seat, so the empty ones aren't news.
		DrawLeftShout(TEXT("SOLO DRIVE (dev): WASD drive, Shift handbrake, F / Q or mouse wheel shift (switches to manual), E hops seats"), Y, ProgressColor);
		return;
	}
	for (const ENBSeatRole SeatRole : {ENBSeatRole::Wheel, ENBSeatRole::Pedals, ENBSeatRole::Gears})
	{
		const UNBSeatComponent* Seat = Car.GetSeat(SeatRole);
		// With automatic gears the Gears seat only holds the handbrake: nobody there is no emergency.
		const bool bNeeded = SeatRole != ENBSeatRole::Gears || Car.IsManualGears();
		const ANBSquirrel* Driver = Seat ? Seat->GetOccupant() : nullptr;
		const APlayerState* Player = Driver ? Driver->GetPlayerState() : nullptr;
		FString Line = FString::Printf(TEXT("%s: "), RoleName(SeatRole));
		FLinearColor Color = FLinearColor(1.f, 1.f, 1.f, 0.85f);
		if (Player && Driver == Me)
		{
			// Your own job carries its controls, so nobody has to ask what the keys do.
			switch (SeatRole)
			{
			case ENBSeatRole::Wheel:
				Line += bUsingGamepad ? FString(TEXT("YOU  (left stick)"))
					: FString::Printf(TEXT("YOU  (%s / %s)"), *KeyLabel(ENBBinding::Left), *KeyLabel(ENBBinding::Right));
				break;
			case ENBSeatRole::Pedals:
				Line += bUsingGamepad ? FString(TEXT("YOU  (left stick up / down)"))
					: FString::Printf(TEXT("YOU  (%s gas / %s brake)"), *KeyLabel(ENBBinding::Forward), *KeyLabel(ENBBinding::Back));
				break;
			default:
			{
				const FString Handbrake = FString::Printf(TEXT("hold %s handbrake"), *KeyLabel(ENBBinding::Handbrake));
				const FString Shift = !Car.IsManualGears() ? FString()
					: bUsingGamepad ? FString(TEXT("stick up / down shift, "))
					: FString::Printf(TEXT("%s / %s shift, "), *KeyLabel(ENBBinding::Forward), *KeyLabel(ENBBinding::Back));
				Line += FString::Printf(TEXT("YOU  (%s%s)"), *Shift, *Handbrake);
				break;
			}
			}
			Color = Driver->GetFurColor();
		}
		else if (Player)
		{
			Line += Player->GetPlayerName();
			Color = Driver->GetFurColor();
		}
		else
		{
			Line += TEXT("nobody!");
			Color = bLive && bNeeded ? FLinearColor(BadColor.R, BadColor.G, BadColor.B, Pulse) : FLinearColor(1.f, 1.f, 1.f, 0.6f);
		}
		DrawLeftShout(Line, Y, Color);
		Y += 28.f;
	}
	const UNBSeatComponent* MySeat = Me->GetCurrentSeat();
	if (MySeat && MySeat->Role == ENBSeatRole::Rider)
	{
		DrawLeftShout(TEXT("YOU RIDE: fix what breaks near you"), Y, Me->GetFurColor());
	}
}

void ANBHUD::DrawLeftShout(const FString& Text, float Y, const FLinearColor& Color)
{
	float W = 0.f, H = 0.f;
	GetTextSize(Text, W, H, GetShoutFont(), 0.8f);
	DrawShoutText(Text, 16.f + W * 0.5f, Y, Color, 0.8f);
}

void ANBHUD::DrawShoutText(const FString& Text, float CenterX, float Y, const FLinearColor& Color, float Scale)
{
	UFont* Font = GetShoutFont();
	float W = 0.f, H = 0.f;
	GetTextSize(Text, W, H, Font, Scale);
	FCanvasTextItem Item(FVector2D(CenterX - W * 0.5f, Y - H * 0.5f), FText::FromString(Text), Font, Color);
	Item.Scale = FVector2D(Scale);
	Item.bOutlined = true;
	Item.OutlineColor = FLinearColor(0.f, 0.f, 0.f, Color.A);
	Item.EnableShadow(FLinearColor(0.f, 0.f, 0.f, 0.6f * Color.A), FVector2D(3.f, 3.f));
	Canvas->DrawItem(Item);
}

void ANBHUD::DrawEjectReason(const ANBSquirrel& Squirrel)
{
	const float Age = GetWorld()->GetTimeSeconds() - Squirrel.GetLastEjectTime();
	if (!Squirrel.IsRagdolled() && Age > 3.f)
	{
		return;
	}
	const FString Work = PromptName(ENBPrompt::Work);
	FString Title = TEXT("WHEEEEE!");
	FString Help;
	switch (Squirrel.GetLastEjectReason())
	{
	case ENBEjectReason::Door:
		Title = TEXT("THE DOOR FLUNG YOU OUT!");
		Help = FString::Printf(TEXT("The door is broken: slam it shut (mash %s at the door)"), *Work);
		break;
	case ENBEjectReason::Flipped:
		Title = TEXT("THE CAR FLIPPED!");
		Help = FString::Printf(TEXT("Push it back up together (%s by the car)"), *Work);
		break;
	case ENBEjectReason::Bailed:
		Title = TEXT("YOU BAILED!");
		Help = TEXT("Jumping out of a fast car throws you");
		break;
	case ENBEjectReason::River:
		Title = TEXT("SWEPT AWAY!");
		Help = TEXT("Deep water: stay in the car at the fords");
		break;
	case ENBEjectReason::Dog:
		Title = TEXT("THE DOG GOT YOU!");
		Help = TEXT("Stay in the car on WIGGLY ROAD, and honk to scare it off");
		break;
	case ENBEjectReason::Ball:
		Title = TEXT("WRECKING BALL!");
		Help = TEXT("Wait for it to swing away, then go");
		break;
	default:
		break;
	}
	const float Alpha = FMath::Clamp(3.f - Age, 0.f, 1.f);
	const float Y = Canvas->ClipY * 0.3f;
	DrawShoutText(Title, Canvas->ClipX * 0.5f, Y, FLinearColor(ProgressColor.R, ProgressColor.G, ProgressColor.B, Squirrel.IsRagdolled() ? 1.f : Alpha), 1.6f);
	if (!Help.IsEmpty())
	{
		DrawShoutText(Help, Canvas->ClipX * 0.5f, Y + 48.f, FLinearColor(1.f, 1.f, 1.f, Squirrel.IsRagdolled() ? 1.f : Alpha), 0.55f);
	}
}

void ANBHUD::DrawSeatHint(const ANBSquirrel& Squirrel)
{
	const float Age = GetWorld()->GetTimeSeconds() - Squirrel.GetSeatHintTime();
	if (!Squirrel.IsSeated() || Age > 2.5f)
	{
		return;
	}
	FString Text;
	switch (Squirrel.GetSeatHint())
	{
	case ENBSeatHint::PedalsCantSteer:
		Text = bUsingGamepad ? TEXT("You're on the PEDALS: stick up / down. The WHEEL squirrel steers!")
			: FString::Printf(TEXT("You're on the PEDALS: %s / %s. The WHEEL squirrel steers!"), *KeyLabel(ENBBinding::Forward), *KeyLabel(ENBBinding::Back));
		break;
	case ENBSeatHint::WheelCantDrive:
		Text = bUsingGamepad ? TEXT("You're on the WHEEL: stick left / right. The PEDALS squirrel drives!")
			: FString::Printf(TEXT("You're on the WHEEL: %s / %s. The PEDALS squirrel drives!"), *KeyLabel(ENBBinding::Left), *KeyLabel(ENBBinding::Right));
		break;
	case ENBSeatHint::RiderCantDrive:
		Text = FString::Printf(TEXT("You're riding. %s hops seats: WHEEL steers, PEDALS drive, GEARS shift"), *PromptName(ENBPrompt::Interact));
		break;
	case ENBSeatHint::GearsCantSteer:
		Text = TEXT("You're on the GEARS. The WHEEL squirrel steers!");
		break;
	case ENBSeatHint::GearsAutomatic:
		Text = FString::Printf(TEXT("Gears are automatic this run: hold %s for the handbrake"), *KeyLabel(ENBBinding::Handbrake));
		break;
	case ENBSeatHint::HandbrakeNotHere:
		Text = TEXT("The handbrake is the GEARS squirrel's job (beside the wheel)");
		break;
	default:
		return;
	}
	const float Alpha = FMath::Clamp((2.5f - Age) * 2.f, 0.f, 1.f);
	DrawShoutText(Text, Canvas->ClipX * 0.5f, Canvas->ClipY * 0.68f, FLinearColor(1.f, 0.85f, 0.3f, Alpha), 0.75f);
}

void ANBHUD::DrawSoundHint()
{
	// Above the route bar, clear of the centre where falls and countdowns stack up.
	const UNBAudioSettings* Audio = UNBAudioSettings::Get(this);
	const FString Talk = Audio && Audio->IsPushToTalk()
		? FString::Printf(TEXT("Hold %s to talk.  "), *KeyLabel(ENBBinding::PushToTalk))
		: FString();
	DrawShoutText(FString::Printf(TEXT("%sCan't hear your crew?  %s: Options"), *Talk, *PromptName(ENBPrompt::Menu)),
		Canvas->ClipX * 0.5f, Canvas->ClipY - 140.f, FLinearColor(0.85f, 0.95f, 1.f), 0.6f);
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
			: FString::Printf(TEXT("Everyone here? %s: start the run"), *PromptName(ENBPrompt::Menu));
		DrawCenteredText(Waiting, Canvas->ClipY * 0.25f, FLinearColor::White, 2.f);
		DrawSoundHint();
		break;
	}
	case ENBRunPhase::Countdown:
		DrawCenteredText(SecondsLeft > 0.f ? FString::FromInt(FMath::CeilToInt(SecondsLeft)) : TEXT("GO!"), Canvas->ClipY * 0.25f, ProgressColor, 4.f);
		DrawCenteredText(TEXT("Everyone get in the car!"), Canvas->ClipY * 0.25f + 90.f, FLinearColor::White, 1.5f);
		DrawSoundHint();
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
			const float WreckY = FMath::Max(Canvas->ClipY * 0.2f, AlarmStackBottom + 10.f);
			DrawCenteredText(TEXT("CAR FALLING APART!  Fix something!"), WreckY, BadColor, 1.6f);
			DrawBar((Canvas->ClipX - 360.f) * 0.5f, WreckY + 36.f, 360.f, 14.f, Fill, BadColor);
			AlarmStackBottom = WreckY + 60.f;
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
		DrawDamageMeter(*Car, 112.f);
		DrawRouteProgress(*Car);
		DrawGearGauge(*Car);
	}
}

void ANBHUD::DrawDamageMeter(const ANBCar& Car, float Y)
{
	// Friend test 1 asked to see how damaged the car is. Wear is the "about to go" signal the
	// failure director leans on, so this shows the cause before the effect.
	const TArray<TObjectPtr<UNBCarPartComponent>>& Parts = Car.GetParts();
	if (Parts.IsEmpty())
	{
		return;
	}
	float Health = 0.f;
	for (const UNBCarPartComponent* Part : Parts)
	{
		Health += Part ? (Part->IsFailed() ? 0.f : 1.f - Part->GetWear()) : 1.f;
	}
	Health /= Parts.Num();

	UFont* Font = GEngine->GetSmallFont();
	const float Right = Canvas->ClipX - 24.f;
	const float BarW = 90.f;
	const float Flash = 0.5f + 0.5f * FMath::Sin(GetWorld()->GetTimeSeconds() * 10.f);
	auto WearColor = [](float Wear)
	{
		// Green, through yellow, to red.
		return Wear < 0.5f ? FLinearColor::LerpUsingHSV(GoodColor, ProgressColor, Wear * 2.f)
			: FLinearColor::LerpUsingHSV(ProgressColor, BadColor, (Wear - 0.5f) * 2.f);
	};

	const FString Title = FString::Printf(TEXT("CAR  %d%%"), FMath::RoundToInt(Health * 100.f));
	float W = 0.f, H = 0.f;
	GetTextSize(Title, W, H, Font, 1.4f);
	DrawText(Title, WearColor(1.f - Health), Right - W, Y, Font, 1.4f);
	Y += H + 6.f;

	for (const UNBCarPartComponent* Part : Parts)
	{
		if (!Part)
		{
			continue;
		}
		const bool bBroken = Part->IsFailed();
		const float Wear = bBroken ? 1.f : Part->GetWear();
		const FString Label = bBroken ? FString::Printf(TEXT("%s  BROKEN"), *Part->PartName.ToString()) : Part->PartName.ToString();
		const FLinearColor Color = bBroken ? FLinearColor(BadColor.R, BadColor.G, BadColor.B, 0.4f + 0.6f * Flash) : WearColor(Wear);
		GetTextSize(Label, W, H, Font, 1.f);
		DrawText(Label, bBroken ? Color : FLinearColor(1.f, 1.f, 1.f, 0.85f), Right - BarW - 10.f - W, Y, Font, 1.f);
		DrawBar(Right - BarW, Y + 3.f, BarW, 10.f, FMath::Max(Wear, 0.04f), Color);
		Y += H + 3.f;
	}
}

void ANBHUD::DrawGearGauge(const ANBCar& Car)
{
	// A half dial, bottom left: the needle sweeps from idle (left) to the redline (right).
	const FVector2D Center(110.f, Canvas->ClipY - 100.f);
	const float Radius = 70.f;
	const bool bManual = Car.IsManualGears();
	const float RedStart = bManual ? Car.GetOverRevAlpha() : 1.f;
	auto Point = [&](float Alpha, float R)
	{
		const float Angle = PI * (1.f - Alpha);
		return Center + FVector2D(FMath::Cos(Angle), -FMath::Sin(Angle)) * R;
	};
	constexpr int32 Ticks = 14;
	for (int32 i = 0; i <= Ticks; ++i)
	{
		const float Alpha = static_cast<float>(i) / Ticks;
		const bool bRedTick = Alpha >= RedStart;
		FCanvasLineItem Tick(Point(Alpha, Radius * (i % 2 ? 0.88f : 0.8f)), Point(Alpha, Radius));
		Tick.SetColor(bRedTick ? BadColor : FLinearColor(1.f, 1.f, 1.f, 0.7f));
		Tick.LineThickness = bRedTick ? 4.f : 2.f;
		Canvas->DrawItem(Tick);
	}
	const float Rpm = Car.GetEngineRPMAlpha();
	const bool bRed = Rpm >= RedStart;
	const float Flash = 0.6f + 0.4f * FMath::Sin(GetWorld()->GetTimeSeconds() * 14.f);
	FCanvasLineItem Needle(Center, Point(Rpm, Radius * 0.92f));
	Needle.SetColor(bRed ? FLinearColor(BadColor.R, BadColor.G, BadColor.B, Flash) : ProgressColor);
	Needle.LineThickness = 4.f;
	Canvas->DrawItem(Needle);

	const int32 Gear = Car.GetEngineGear();
	const FString GearText = Gear < 0 ? FString(TEXT("R")) : Gear == 0 ? FString(TEXT("N")) : FString::FromInt(Gear);
	DrawShoutText(GearText, Center.X, Center.Y + 24.f, bRed ? BadColor : FLinearColor::White, 1.4f);
	DrawShoutText(bManual ? TEXT("MANUAL") : TEXT("AUTO"), Center.X, Center.Y + 58.f, FLinearColor(1.f, 1.f, 1.f, 0.7f), 0.55f);
	if (bManual && bRed)
	{
		// In top gear there's nothing to shift to: easing off is the only way out of the red (and it overheats faster).
		const bool bTopGear = Gear >= Car.GetNumForwardGears();
		DrawShoutText(bTopGear ? TEXT("EASE OFF! OVERHEATING") : TEXT("SHIFT UP!"), Center.X + (bTopGear ? 40.f : 0.f), Center.Y - Radius - 22.f,
			FLinearColor(BadColor.R, BadColor.G, BadColor.B, Flash), 0.8f);
	}
}

void ANBHUD::DrawRouteProgress(const ANBCar& Car)
{
	if (!Route.IsValid())
	{
		for (TActorIterator<ANBRoute> It(GetWorld()); It; ++It)
		{
			Route = *It;
			break;
		}
	}
	const ANBRoute* Course = Route.Get();
	if (!Course || Course->GetLength() <= 0.f || Course->GetNumSections() == 0)
	{
		return;
	}

	const float Along = Course->GetDistanceAlong(Car.GetActorLocation());
	const int32 Section = Course->GetSectionAt(Along);
	const double Now = GetWorld()->GetTimeSeconds();
	if (Section != ShownSection)
	{
		// No banner for the first section on joining; the sign at the start covers it.
		SectionShownAt = ShownSection == INDEX_NONE ? -100.0 : Now;
		ShownSection = Section;
	}

	// Progress bar with the section name and the distance left.
	const float Width = FMath::Min(520.f, Canvas->ClipX * 0.5f);
	const float X = (Canvas->ClipX - Width) * 0.5f;
	const float Y = Canvas->ClipY - 78.f;
	DrawBar(X, Y, Width, 10.f, Along / Course->GetLength(), ProgressColor);
	const float MetresLeft = (Course->GetLength() - Along) / 100.f;
	const FString Left = MetresLeft >= 1000.f ? FString::Printf(TEXT("%.1f km"), MetresLeft / 1000.f) : FString::Printf(TEXT("%d m"), FMath::RoundToInt(MetresLeft));
	DrawCenteredText(FString::Printf(TEXT("%s   -   depot in %s"), *Course->GetSectionName(Section), *Left), Y - 24.f, FLinearColor(1.f, 1.f, 1.f, 0.85f), 1.1f);

	const double Since = Now - SectionShownAt;
	if (Since < 3.0)
	{
		const float Alpha = Since < 2.0 ? 1.f : static_cast<float>(3.0 - Since);
		DrawCenteredText(Course->GetSectionName(Section), Canvas->ClipY * 0.4f, FLinearColor(ProgressColor.R, ProgressColor.G, ProgressColor.B, Alpha), 2.6f);
	}
}

void ANBHUD::DrawVoiceStatus()
{
	const AGameStateBase* State = GetWorld()->GetGameState();
	if (!State)
	{
		return;
	}
	const APlayerState* Local = PlayerOwner ? PlayerOwner->PlayerState.Get() : nullptr;
	UFont* Font = GEngine->GetMediumFont();
	float Y = Canvas->ClipY * 0.5f - 60.f;

	for (const APlayerState* Player : State->PlayerArray)
	{
		const ANBPlayerState* Stats = Cast<ANBPlayerState>(Player);
		if (!Stats)
		{
			continue;
		}
		if (Stats->IsMuted())
		{
			const bool bMe = Stats == Local;
			const FString Line = bMe
				? FString::Printf(TEXT("ACORN IN MOUTH!  Muted for %d s  -  use the chat wheel (%s)"), FMath::CeilToInt(Stats->GetMuteSecondsLeft()),
					bUsingGamepad ? TEXT("D-pad, LB + D-pad") : TEXT("1-8"))
				: FString::Printf(TEXT("%s has an acorn in their mouth!  (%d s)"), *Stats->GetPlayerName(), FMath::CeilToInt(Stats->GetMuteSecondsLeft()));
			DrawCenteredText(Line, Canvas->ClipY * 0.12f + (bMe ? 0.f : 28.f), bMe ? BadColor : ProgressColor, bMe ? 1.5f : 1.1f);
		}
		const int32 Ping = Stats->GetActivePing();
		if (Ping != INDEX_NONE)
		{
			DrawText(FString::Printf(TEXT("%s: %s"), *Stats->GetPlayerName(), *ANBPlayerState::GetPingText(Ping).ToString()), GoodColor, 24.f, Y, Font, 1.3f);
			Y += 30.f;
		}
	}

	// Chat wheel legend, only when it matters.
	if (const ANBPlayerState* Me = Cast<ANBPlayerState>(Local); Me && Me->IsMuted())
	{
		if (bUsingGamepad)
		{
			// Two rows: the D-pad alone, then with LB held.
			const int32 Buttons = ANBSquirrel::NumPadPingButtons;
			FString Rows[2] = {TEXT("D-pad    "), TEXT("LB + D-pad    ")};
			for (int32 i = 0; i < ANBPlayerState::NumPings; ++i)
			{
				Rows[i / Buttons] += FString::Printf(TEXT("%s: %s   "), *ANBSquirrel::GetPadPingButtonName(i % Buttons), *ANBPlayerState::GetPingText(i).ToString());
			}
			DrawCenteredText(Rows[0], Canvas->ClipY - 96.f, FLinearColor::White, 1.1f);
			DrawCenteredText(Rows[1], Canvas->ClipY - 70.f, FLinearColor::White, 1.1f);
		}
		else
		{
			FString Legend;
			for (int32 i = 0; i < ANBPlayerState::NumPings; ++i)
			{
				Legend += FString::Printf(TEXT("%d %s   "), i + 1, *ANBPlayerState::GetPingText(i).ToString());
			}
			DrawCenteredText(Legend, Canvas->ClipY - 70.f, FLinearColor::White, 1.1f);
		}
	}

	// Push to talk: show when the mic is live, above the ping list.
	const UNBAudioSettings* Audio = UNBAudioSettings::Get(this);
	const ANBSquirrel* Squirrel = PlayerOwner ? Cast<ANBSquirrel>(PlayerOwner->GetPawn()) : nullptr;
	if (Audio && Audio->IsPushToTalk() && Squirrel && Squirrel->IsVoiceSending())
	{
		DrawText(TEXT("MIC ON"), GoodColor, 24.f, Canvas->ClipY * 0.5f - 100.f, Font, 1.4f);
	}
}

void ANBHUD::DrawEndScreen(const ANBRunGameState& State)
{
	int32 AwardCount = 0;
	for (const APlayerState* Player : State.PlayerArray)
	{
		const ANBPlayerState* Stats = Cast<ANBPlayerState>(Player);
		AwardCount += Stats && Stats->GetAward() != ENBAward::None ? 1 : 0;
	}
	const float AwardLineH = 30.f;
	const float PanelW = 720.f;
	const float PanelH = (State.IsManualGears() ? 550.f : 520.f) + AwardCount * AwardLineH + (AwardCount > 0 ? 10.f : 0.f);
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
		DrawCenteredText(FString::Printf(TEXT("Air bonus: +%d"), State.GetAirBonus()), Y + 60.f, FLinearColor::White, 1.3f);
		DrawCenteredText(FString::Printf(TEXT("Respawn penalty: -%d"), State.GetRespawnPenalty()), Y + 90.f, FLinearColor::White, 1.3f);
		if (State.IsManualGears())
		{
			DrawCenteredText(FString::Printf(TEXT("Manual gears x%.2f"), State.GetManualGearsScale()), Y + 120.f, GoodColor, 1.3f);
			Y += 30.f;
		}
		DrawCenteredText(FString::Printf(TEXT("SCORE  %d"), State.GetFinalScore()), Y + 130.f, ProgressColor, 2.2f);
	}
	else
	{
		DrawCenteredText(TEXT("The acorns never made it."), Y + 20.f, FLinearColor::White, 1.4f);
	}
	Y += 190.f;

	// Badges, big enough to read in a clip.
	const APlayerState* Mvp = State.GetMvp();
	const APlayerState* Useless = State.GetMostUseless();
	if (Mvp && Useless)
	{
		DrawCenteredText(FString::Printf(TEXT("MVP: %s"), *Mvp->GetPlayerName()), Y, ProgressColor, 1.6f);
		DrawCenteredText(FString::Printf(TEXT("MOST USELESS: %s"), *Useless->GetPlayerName()), Y + 34.f, BadColor, 1.6f);
		Y += 80.f;
	}

	// One funny award per squirrel (#10), in its fur colour: made to be screenshotted.
	if (AwardCount > 0)
	{
		for (const APlayerState* Player : State.PlayerArray)
		{
			const ANBPlayerState* Stats = Cast<ANBPlayerState>(Player);
			if (!Stats || Stats->GetAward() == ENBAward::None)
			{
				continue;
			}
			FLinearColor Color = GoodColor;
			if (const ANBSquirrel* Squirrel = Cast<ANBSquirrel>(Stats->GetPawn()))
			{
				Color = Squirrel->GetFurColor();
			}
			const FString Line = FString::Printf(TEXT("%s:  %s"), *Stats->GetPlayerName(),
				*ANBPlayerState::DescribeAward(Stats->GetAward(), Stats->GetAwardStat(Stats->GetAward())));
			DrawCenteredText(Line, Y, Color, 1.3f);
			Y += AwardLineH;
		}
		Y += 10.f;
	}

	for (const APlayerState* Player : State.PlayerArray)
	{
		if (const ANBPlayerState* Stats = Cast<ANBPlayerState>(Player))
		{
			const int32 Drive = Stats->GetDriveSeconds();
			const FString Line = FString::Printf(TEXT("%s    fires out: %d    acorns spilled: %d    falls: %d    respawns: %d    drove %d:%02d"),
				*Stats->GetPlayerName(), Stats->GetFiresPutOut(), Stats->GetAcornsSpilled(), Stats->GetFalls(), Stats->GetRespawns(), Drive / 60, Drive % 60);
			const FLinearColor LineColor = Player == Mvp ? ProgressColor : Player == Useless ? BadColor : FLinearColor(1.f, 1.f, 1.f, 0.8f);
			DrawCenteredText(Line, Y, LineColor, 1.f);
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
		Line = FString::Printf(TEXT("OFFLINE    %s: menu"), *PromptName(ENBPrompt::Menu));
		break;
	case NM_ListenServer:
		Line = FString::Printf(TEXT("HOST    %d / 4 squirrels    %s: menu"), Players, *PromptName(ENBPrompt::Menu));
		break;
	default:
		Line = FString::Printf(TEXT("CONNECTED    %d / 4 squirrels    %s: menu"), Players, *PromptName(ENBPrompt::Menu));
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
	// Bottom right, off the car: the chase camera puts the car, and the squirrel doing the repair, bottom centre
	// (18 Oct: the panel and its cues hid the TailFan animation).
	const float PanelX = FMath::Max(Canvas->ClipX - PanelW - 30.f, 0.f);
	const float PanelCX = PanelX + PanelW * 0.5f;
	const float PanelY = Canvas->ClipY - 70.f - PanelH;
	DrawRect(PanelColor, PanelX, PanelY, PanelW, PanelH);

	// Someone else is on it (Hold / Push2 track their users): say so, dimmed, so the bar filling up
	// doesn't read as your own repair (friend test 1: "the widgets are getting shared").
	const ANBSquirrel* Other = nullptr;
	for (const ANBSquirrel* User : Interactable.GetUsers())
	{
		if (User && User != GetOwningPawn())
		{
			Other = User;
			break;
		}
	}
	const APlayerState* OtherPlayer = Other ? Other->GetPlayerState() : nullptr;
	if (!bUsing && OtherPlayer && Interactable.Mode != ENBInteractMode::Push2)
	{
		DrawCenteredText(FString::Printf(TEXT("%s IS ON IT"), *OtherPlayer->GetPlayerName().ToUpper()), PanelY + 12.f, Other->GetFurColor(), 1.4f, PanelCX);
		DrawBar(PanelX + 20.f, PanelY + 50.f, PanelW - 40.f, 18.f, Interactable.GetProgress(), FLinearColor(1.f, 1.f, 1.f, 0.35f));
		return;
	}

	const FString Title = FString::Printf(TEXT("%s  %s"), VerbFor(Interactable.Mode), *Interactable.Prompt.ToString());
	DrawPromptRow({{ENBPrompt::Work, Title}}, PanelY + 22.f, 1.4f, bUsing ? ProgressColor : FLinearColor::White, PanelCX);

	float Y = PanelY + 50.f;
	if (Interactable.Mode == ENBInteractMode::Push2)
	{
		const FString Count = FString::Printf(TEXT("Pushing: %d / %d"), Interactable.GetNumUsers(), Interactable.RequiredUsers);
		const bool bEnough = Interactable.GetNumUsers() >= Interactable.RequiredUsers;
		DrawCenteredText(Count, Y - 4.f, bEnough ? GoodColor : BadColor, 1.1f, PanelCX);
		Y += 24.f;
	}
	else if (Interactable.Mode == ENBInteractMode::TimingRing)
	{
		DrawRing(Interactable, FVector2D(PanelCX, Y + 70.f), 60.f);
		Y += 150.f;
	}
	else if (Interactable.Mode == ENBInteractMode::Mash)
	{
		// Friend test 1: nobody understood the engine wanted many fast presses. Count them down,
		// in the panel and bouncing on the part itself.
		const int32 Left = FMath::Max(1, FMath::CeilToInt((1.f - Interactable.GetProgress()) * Interactable.MashPresses));
		const float Time = GetWorld()->GetTimeSeconds();
		const float Bounce = FMath::Abs(FMath::Sin(Time * 10.f));
		// Inside the panel, not over the car, where it hid the squirrel doing the work.
		DrawShoutText(FString::Printf(TEXT("%d MORE!"), Left), PanelCX, PanelY + 84.f - 3.f * Bounce, ProgressColor, 1.f + 0.08f * Bounce);
		// On the part only until the first press: it says how, and then the squirrel's animation is the show.
		FVector2D Screen;
		if (Interactable.GetProgress() <= 0.f && PlayerOwner && PlayerOwner->ProjectWorldLocationToScreen(Interactable.GetComponentLocation() + FVector(0.f, 0.f, 40.f), Screen))
		{
			DrawShoutText(FString::Printf(TEXT("MASH %s!"), *PromptName(ENBPrompt::Work)), Screen.X, Screen.Y - 30.f - 10.f * Bounce, ProgressColor, 1.f);
		}
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

void ANBHUD::DrawCenteredText(const FString& Text, float Y, const FLinearColor& Color, float Scale, float CenterX)
{
	UFont* Font = GEngine->GetMediumFont();
	float W = 0.f, H = 0.f;
	GetTextSize(Text, W, H, Font, Scale);
	DrawText(Text, Color, (CenterX < 0.f ? Canvas->ClipX * 0.5f : CenterX) - W * 0.5f, Y, Font, Scale);
}
