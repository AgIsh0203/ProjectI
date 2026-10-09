// Squirrel Wheels prototype.

#include "UI/NBLobbyMenu.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/ButtonSlot.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "Game/NBRunGameMode.h"
#include "Game/NBRunGameState.h"
#include "Game/NBSessionSubsystem.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "InputCoreTypes.h"
#include "Kismet/KismetSystemLibrary.h"
#include "UI/NBHUD.h"

namespace
{
	const FLinearColor TitleColor(1.f, 0.75f, 0.1f);
	const FLinearColor DimColor(1.f, 1.f, 1.f, 0.75f);
	const FLinearColor ButtonColor(0.95f, 0.5f, 0.08f);
	constexpr int32 MaxSquirrels = 4;
}

void UNBLobbyMenu::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	SetIsFocusable(true);
	BuildTree();
	Refresh();
}

void UNBLobbyMenu::BuildTree()
{
	UBorder* Backdrop = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("Backdrop"));
	Backdrop->SetBrushColor(FLinearColor(0.f, 0.f, 0.f, 0.6f));
	Backdrop->SetHorizontalAlignment(HAlign_Center);
	Backdrop->SetVerticalAlignment(VAlign_Center);
	WidgetTree->RootWidget = Backdrop;

	USizeBox* Size = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("PanelSize"));
	Size->SetMinDesiredWidth(460.f);
	Backdrop->SetContent(Size);

	UBorder* Panel = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("Panel"));
	Panel->SetBrushColor(FLinearColor(0.05f, 0.04f, 0.03f, 0.92f));
	Panel->SetPadding(FMargin(32.f, 24.f));
	Size->SetContent(Panel);

	UVerticalBox* Box = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("Column"));
	Panel->SetContent(Box);

	UTextBlock* Title = AddText(Box, 34, TitleColor);
	Title->SetText(FText::FromString(TEXT("SQUIRREL WHEELS")));
	ModeText = AddText(Box, 16, DimColor);
	CrewText = AddText(Box, 16, FLinearColor::White);
	StatusText = AddText(Box, 14, TitleColor);
	StatusText->SetAutoWrapText(true);

	HostButton = AddButton(Box, TEXT("Host a game"));
	HostButton->OnClicked.AddDynamic(this, &UNBLobbyMenu::HandleHost);
	JoinButton = AddButton(Box, TEXT("Join a game"));
	JoinButton->OnClicked.AddDynamic(this, &UNBLobbyMenu::HandleJoin);
	PracticeButton = AddButton(Box, TEXT("Practice alone"));
	PracticeButton->OnClicked.AddDynamic(this, &UNBLobbyMenu::HandlePractice);
	StartButton = AddButton(Box, TEXT("Start the run"));
	StartButton->OnClicked.AddDynamic(this, &UNBLobbyMenu::HandleStart);
	InviteButton = AddButton(Box, TEXT("Invite friends"));
	InviteButton->OnClicked.AddDynamic(this, &UNBLobbyMenu::HandleInvite);
	RestartButton = AddButton(Box, TEXT("Restart the run"));
	RestartButton->OnClicked.AddDynamic(this, &UNBLobbyMenu::HandleRestart);
	ResumeButton = AddButton(Box, TEXT("Back to the car"));
	ResumeButton->OnClicked.AddDynamic(this, &UNBLobbyMenu::HandleResume);
	LeaveButton = AddButton(Box, TEXT("Leave the game"));
	LeaveButton->OnClicked.AddDynamic(this, &UNBLobbyMenu::HandleLeave);
	QuitButton = AddButton(Box, TEXT("Quit to desktop"));
	QuitButton->OnClicked.AddDynamic(this, &UNBLobbyMenu::HandleQuit);
}

UTextBlock* UNBLobbyMenu::AddText(UVerticalBox* Box, int32 Size, const FLinearColor& Color)
{
	UTextBlock* Text = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
	FSlateFontInfo Font = Text->GetFont();
	Font.Size = Size;
	Text->SetFont(Font);
	Text->SetColorAndOpacity(FSlateColor(Color));
	Text->SetJustification(ETextJustify::Center);
	UVerticalBoxSlot* BoxSlot = Box->AddChildToVerticalBox(Text);
	BoxSlot->SetPadding(FMargin(0.f, 4.f));
	BoxSlot->SetHorizontalAlignment(HAlign_Fill);
	return Text;
}

UButton* UNBLobbyMenu::AddButton(UVerticalBox* Box, const FString& Label)
{
	UButton* Button = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass());
	Button->SetBackgroundColor(ButtonColor);

	UTextBlock* Text = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
	FSlateFontInfo Font = Text->GetFont();
	Font.Size = 20;
	Text->SetFont(Font);
	Text->SetText(FText::FromString(Label));
	Text->SetColorAndOpacity(FSlateColor(FLinearColor::White));
	Text->SetJustification(ETextJustify::Center);
	if (UButtonSlot* TextSlot = Cast<UButtonSlot>(Button->AddChild(Text)))
	{
		TextSlot->SetPadding(FMargin(12.f, 6.f));
	}

	UVerticalBoxSlot* BoxSlot = Box->AddChildToVerticalBox(Button);
	BoxSlot->SetPadding(FMargin(0.f, 5.f));
	BoxSlot->SetHorizontalAlignment(HAlign_Fill);
	Buttons.Add(Button);
	return Button;
}

void UNBLobbyMenu::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	Refresh();
}

void UNBLobbyMenu::Refresh()
{
	const UWorld* World = GetWorld();
	if (!World || !StatusText)
	{
		return;
	}
	const ENetMode NetMode = World->GetNetMode();
	const bool bOnline = NetMode != NM_Standalone;
	const bool bAuthority = NetMode != NM_Client;
	const ANBRunGameState* State = World->GetGameState<ANBRunGameState>();
	const ENBRunPhase Phase = State ? State->GetPhase() : ENBRunPhase::Waiting;
	const ANBRunGameMode* Mode = World->GetAuthGameMode<ANBRunGameMode>();
	const UNBSessionSubsystem* Sessions = GetGameInstance() ? GetGameInstance()->GetSubsystem<UNBSessionSubsystem>() : nullptr;

	auto Show = [](UWidget* Widget, bool bVisible)
	{
		Widget->SetVisibility(bVisible ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	};

	const FString Online = Sessions ? Sessions->GetSubsystemName() : TEXT("none");
	ModeText->SetText(FText::FromString(
		!bOnline ? FString::Printf(TEXT("Offline  [%s]"), *Online)
		: bAuthority ? FString::Printf(TEXT("You're hosting  [%s]"), *Online)
		: FString::Printf(TEXT("Connected  [%s]"), *Online)));

	// The crew, so everyone can see who made it in before the host starts.
	FString Crew;
	if (bOnline && State)
	{
		Crew = FString::Printf(TEXT("Squirrels  %d / %d"), State->PlayerArray.Num(), MaxSquirrels);
		for (const APlayerState* Player : State->PlayerArray)
		{
			if (Player)
			{
				Crew += TEXT("\n") + Player->GetPlayerName();
			}
		}
	}
	Show(CrewText, !Crew.IsEmpty());
	CrewText->SetText(FText::FromString(Crew));

	FString Status = Sessions ? Sessions->GetStatusText() : FString();
	FString Hint;
	if (bOnline && Phase == ENBRunPhase::Waiting)
	{
		if (!bAuthority)
		{
			Hint = TEXT("Waiting for the host to start the run");
		}
		else if (Mode && !Mode->CanStartRun())
		{
			Hint = FString::Printf(TEXT("Invite friends: a run needs %d squirrels"), Mode->GetMinPlayers());
		}
	}
	if (!Hint.IsEmpty())
	{
		Status += (Status.IsEmpty() ? TEXT("") : TEXT("\n")) + Hint;
	}
	Show(StatusText, !Status.IsEmpty());
	StatusText->SetText(FText::FromString(Status));

	Show(HostButton, !bOnline);
	Show(JoinButton, !bOnline);
	Show(PracticeButton, !bOnline && Phase == ENBRunPhase::Waiting);
	Show(StartButton, bOnline && bAuthority && Phase == ENBRunPhase::Waiting);
	StartButton->SetIsEnabled(Mode && Mode->CanStartRun());
	Show(InviteButton, bOnline);
	Show(RestartButton, bAuthority && Phase != ENBRunPhase::Waiting);
	Show(LeaveButton, bOnline);
}

void UNBLobbyMenu::FocusFirstButton()
{
	Refresh();
	for (UButton* Button : Buttons)
	{
		if (Button && Button->GetVisibility() == ESlateVisibility::Visible && Button->GetIsEnabled())
		{
			Button->SetKeyboardFocus();
			return;
		}
	}
}

FReply UNBLobbyMenu::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	const FKey Key = InKeyEvent.GetKey();
	if (Key == EKeys::Escape || Key == EKeys::Gamepad_Special_Right || Key == EKeys::Gamepad_FaceButton_Right)
	{
		CloseMenu();
		return FReply::Handled();
	}
	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

void UNBLobbyMenu::CloseMenu()
{
	const APlayerController* PC = GetOwningPlayer();
	if (ANBHUD* HUD = PC ? PC->GetHUD<ANBHUD>() : nullptr)
	{
		HUD->CloseMenu();
	}
}

void UNBLobbyMenu::HandleHost()
{
	if (UNBSessionSubsystem* Sessions = GetGameInstance()->GetSubsystem<UNBSessionSubsystem>())
	{
		Sessions->Host();
	}
}

void UNBLobbyMenu::HandleJoin()
{
	if (UNBSessionSubsystem* Sessions = GetGameInstance()->GetSubsystem<UNBSessionSubsystem>())
	{
		Sessions->FindAndJoin();
	}
}

void UNBLobbyMenu::HandlePractice()
{
	if (ANBRunGameMode* Mode = GetWorld()->GetAuthGameMode<ANBRunGameMode>())
	{
		Mode->DevStartRun();
	}
	CloseMenu();
}

void UNBLobbyMenu::HandleStart()
{
	if (ANBRunGameMode* Mode = GetWorld()->GetAuthGameMode<ANBRunGameMode>())
	{
		Mode->StartRunFromLobby();
	}
	CloseMenu();
}

void UNBLobbyMenu::HandleInvite()
{
	if (UNBSessionSubsystem* Sessions = GetGameInstance()->GetSubsystem<UNBSessionSubsystem>())
	{
		Sessions->ShowInviteUI();
	}
}

void UNBLobbyMenu::HandleRestart()
{
	// Close first: the travel keeps this player controller, and it shouldn't arrive in UI-only input.
	CloseMenu();
	if (ANBRunGameMode* Mode = GetWorld()->GetAuthGameMode<ANBRunGameMode>())
	{
		Mode->RestartRun();
	}
}

void UNBLobbyMenu::HandleResume()
{
	CloseMenu();
}

void UNBLobbyMenu::HandleLeave()
{
	if (UNBSessionSubsystem* Sessions = GetGameInstance()->GetSubsystem<UNBSessionSubsystem>())
	{
		Sessions->Leave();
	}
}

void UNBLobbyMenu::HandleQuit()
{
	UKismetSystemLibrary::QuitGame(this, GetOwningPlayer(), EQuitPreference::Quit, false);
}
