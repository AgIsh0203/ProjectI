// Squirrel Wheels prototype.

#include "UI/NBLobbyMenu.h"

#include "Audio/NBAudioSettings.h"
#include "AudioMixerBlueprintLibrary.h"
#include "Blueprint/WidgetTree.h"
#include "Components/CheckBox.h"
#include "Components/ComboBoxString.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Engine/Engine.h"
#include "GameFramework/GameUserSettings.h"
#include "Components/Slider.h"
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
#include "Player/NBControlSettings.h"
#include "UI/NBHUD.h"

namespace
{
	const FLinearColor TitleColor(1.f, 0.75f, 0.1f);
	const FLinearColor DimColor(1.f, 1.f, 1.f, 0.75f);
	const FLinearColor ButtonColor(0.95f, 0.5f, 0.08f);
	constexpr int32 MaxSquirrels = 4;
	const TCHAR* const WindowModeNames[] = {TEXT("Fullscreen"), TEXT("Borderless window"), TEXT("Window")};
	const TCHAR* const QualityNames[] = {TEXT("Low"), TEXT("Medium"), TEXT("High"), TEXT("Epic")};
	const TCHAR* const FrameLimitNames[] = {TEXT("30"), TEXT("60"), TEXT("120"), TEXT("144"), TEXT("Unlimited")};
	const float FrameLimitValues[] = {30.f, 60.f, 120.f, 144.f, 0.f};
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
	// Fixed, so the panel doesn't jump in width between the Options tabs.
	Size->SetWidthOverride(560.f);
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
	GearsButton = AddButton(Box, TEXT("Manual gears: OFF"));
	GearsButton->OnClicked.AddDynamic(this, &UNBLobbyMenu::HandleManualGears);
	InviteButton = AddButton(Box, TEXT("Invite friends"));
	InviteButton->OnClicked.AddDynamic(this, &UNBLobbyMenu::HandleInvite);
	RestartButton = AddButton(Box, TEXT("Restart the run"));
	RestartButton->OnClicked.AddDynamic(this, &UNBLobbyMenu::HandleRestart);
	ResumeButton = AddButton(Box, TEXT("Back to the car"));
	ResumeButton->OnClicked.AddDynamic(this, &UNBLobbyMenu::HandleResume);
	LeaveButton = AddButton(Box, TEXT("Leave the game"));
	LeaveButton->OnClicked.AddDynamic(this, &UNBLobbyMenu::HandleLeave);
	SettingsButton = AddButton(Box, TEXT("Options"));
	SettingsButton->OnClicked.AddDynamic(this, &UNBLobbyMenu::HandleSettings);
	QuitButton = AddButton(Box, TEXT("Quit to desktop"));
	QuitButton->OnClicked.AddDynamic(this, &UNBLobbyMenu::HandleQuit);

	BuildSettings(Box);
}

void UNBLobbyMenu::BuildSettings(UVerticalBox* Column)
{
	SettingsBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("Settings"));
	Column->AddChildToVerticalBox(SettingsBox);
	SettingsBox->SetVisibility(ESlateVisibility::Collapsed);

	// The tab row: one button per page, the open one highlighted.
	UHorizontalBox* Tabs = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("Tabs"));
	UVerticalBoxSlot* TabsSlot = SettingsBox->AddChildToVerticalBox(Tabs);
	TabsSlot->SetPadding(FMargin(0.f, 4.f, 0.f, 10.f));
	TabsSlot->SetHorizontalAlignment(HAlign_Fill);
	for (const TCHAR* Label : {TEXT("Sound"), TEXT("Controls"), TEXT("Display")})
	{
		UButton* Tab = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass());
		UTextBlock* Text = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
		FSlateFontInfo Font = Text->GetFont();
		Font.Size = 17;
		Text->SetFont(Font);
		Text->SetText(FText::FromString(Label));
		Text->SetJustification(ETextJustify::Center);
		if (UButtonSlot* TextSlot = Cast<UButtonSlot>(Tab->AddChild(Text)))
		{
			TextSlot->SetPadding(FMargin(8.f, 4.f));
		}
		if (UHorizontalBoxSlot* TabSlot = Tabs->AddChildToHorizontalBox(Tab))
		{
			TabSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
			TabSlot->SetPadding(FMargin(3.f, 0.f));
		}
		TabButtons.Add(Tab);
	}
	TabButtons[0]->OnClicked.AddDynamic(this, &UNBLobbyMenu::HandleSoundTab);
	TabButtons[1]->OnClicked.AddDynamic(this, &UNBLobbyMenu::HandleControlsTab);
	TabButtons[2]->OnClicked.AddDynamic(this, &UNBLobbyMenu::HandleDisplayTab);

	for (int32 i = 0; i < 3; ++i)
	{
		UVerticalBox* Page = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass());
		SettingsBox->AddChildToVerticalBox(Page)->SetHorizontalAlignment(HAlign_Fill);
		TabPages.Add(Page);
	}
	BuildSoundPage(TabPages[0]);
	BuildControlsPage(TabPages[1]);
	BuildDisplayPage(TabPages[2]);

	UButton* Back = AddButton(SettingsBox, TEXT("Back"));
	Back->OnClicked.AddDynamic(this, &UNBLobbyMenu::HandleSettingsBack);
	RefreshSliderTexts();
	ShowTab(EOptionsTab::Sound);
}

void UNBLobbyMenu::BuildSoundPage(UVerticalBox* Page)
{
	const UNBAudioSettings* Audio = UNBAudioSettings::Get(this);
	GameVolumeSlider = AddSliderRow(Page, TEXT("Game volume"), 0.f, UNBAudioSettings::MaxGameVolume, 0.05f, Audio ? Audio->GetGameVolume() : 1.f, GameVolumeText);
	GameVolumeSlider->OnValueChanged.AddDynamic(this, &UNBLobbyMenu::HandleGameVolume);
	AddSliderRow(Page, TEXT("Voices"), 0.f, UNBAudioSettings::MaxVoiceVolume, 0.05f, Audio ? Audio->GetVoiceVolume() : 1.f, VoiceVolumeText)
		->OnValueChanged.AddDynamic(this, &UNBLobbyMenu::HandleVoiceVolume);
	AddSliderRow(Page, TEXT("Mic boost"), 0.f, UNBAudioSettings::MaxMicBoost, 0.05f, Audio ? Audio->GetMicBoost() : 1.f, MicBoostText)
		->OnValueChanged.AddDynamic(this, &UNBLobbyMenu::HandleMicBoost);
	AddCheckRow(Page, FString::Printf(TEXT("Push to talk (hold %s)"), *GetPushToTalkKeyLabel()), Audio && Audio->IsPushToTalk())
		->OnCheckStateChanged.AddDynamic(this, &UNBLobbyMenu::HandlePushToTalk);

	OutputDeviceCombo = AddComboRow(Page, TEXT("Speakers / headphones"), TEXT("OutputDevice"));
	OutputDeviceCombo->OnSelectionChanged.AddDynamic(this, &UNBLobbyMenu::HandleOutputDevice);
	FOnAudioOutputDevicesObtained OnDevices;
	OnDevices.BindUFunction(this, GET_FUNCTION_NAME_CHECKED(UNBLobbyMenu, HandleDevicesObtained));
	UAudioMixerBlueprintLibrary::GetAvailableAudioOutputDevices(this, OnDevices);

	MicDeviceCombo = AddComboRow(Page, TEXT("Microphone"), TEXT("MicDevice"));
	MicDeviceCombo->OnSelectionChanged.AddDynamic(this, &UNBLobbyMenu::HandleMicDevice);
	RefreshMicDevices();

	UTextBlock* MicNote = AddText(Page, 13, DimColor);
	MicNote->SetAutoWrapText(true);
	MicNote->SetText(FText::FromString(TEXT("Nobody hears you? Pick your mic above, then raise Mic boost. "
		"Others too quiet? Raise Voices.")));
	TabFirstControls.Add(GameVolumeSlider);
}

void UNBLobbyMenu::BuildControlsPage(UVerticalBox* Page)
{
	const UNBControlSettings* Controls = UNBControlSettings::Get(this);
	USlider* Sensitivity = AddSliderRow(Page, TEXT("Look sensitivity"), UNBControlSettings::MinLookSensitivity, UNBControlSettings::MaxLookSensitivity, 0.05f,
		Controls ? Controls->GetLookSensitivity() : 1.f, LookSensitivityText);
	Sensitivity->OnValueChanged.AddDynamic(this, &UNBLobbyMenu::HandleLookSensitivity);
	AddCheckRow(Page, TEXT("Invert look up / down"), Controls && Controls->IsLookInverted())
		->OnCheckStateChanged.AddDynamic(this, &UNBLobbyMenu::HandleInvertLook);
	AddSliderRow(Page, TEXT("Crash camera shake"), 0.f, 1.f, 0.05f, Controls ? Controls->GetCameraShake() : 1.f, CameraShakeText)
		->OnValueChanged.AddDynamic(this, &UNBLobbyMenu::HandleCameraShake);

	BuildKeyList(Page);
	TabFirstControls.Add(Sensitivity);
}

void UNBBindingButtonProxy::HandleClicked()
{
	if (Menu)
	{
		Menu->BeginRebind(Index, bPad);
	}
}

void UNBLobbyMenu::BuildKeyList(UVerticalBox* Page)
{
	auto MakeText = [this](const FString& Text, int32 Size, const FLinearColor& Color)
	{
		UTextBlock* Block = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
		FSlateFontInfo Font = Block->GetFont();
		Font.Size = Size;
		Block->SetFont(Font);
		Block->SetText(FText::FromString(Text));
		Block->SetColorAndOpacity(FSlateColor(Color));
		Block->SetJustification(ETextJustify::Center);
		return Block;
	};
	// A key column: a fixed-width box holding a button, or plain text where there is none.
	auto AddColumn = [this](UHorizontalBox* Row, float Width, UWidget* Content)
	{
		USizeBox* Size = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass());
		Size->SetWidthOverride(Width);
		Size->SetContent(Content);
		if (UHorizontalBoxSlot* SizeSlot = Row->AddChildToHorizontalBox(Size))
		{
			SizeSlot->SetPadding(FMargin(4.f, 0.f, 0.f, 0.f));
			SizeSlot->SetVerticalAlignment(VAlign_Center);
		}
	};
	auto AddKeyButton = [&](UHorizontalBox* Row, float Width, int32 Index, bool bPad)
	{
		UButton* KeyButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass());
		KeyButton->SetBackgroundColor(bPad ? FLinearColor(0.25f, 0.3f, 0.35f) : FLinearColor(0.35f, 0.3f, 0.25f));
		UTextBlock* KeyText = MakeText(FString(), 15, FLinearColor::White);
		if (UButtonSlot* TextSlot = Cast<UButtonSlot>(KeyButton->AddChild(KeyText)))
		{
			TextSlot->SetPadding(FMargin(4.f, 1.f));
		}
		AddColumn(Row, Width, KeyButton);

		UNBBindingButtonProxy* Proxy = NewObject<UNBBindingButtonProxy>(this);
		Proxy->Menu = this;
		Proxy->Index = Index;
		Proxy->bPad = bPad;
		KeyButton->OnClicked.AddDynamic(Proxy, &UNBBindingButtonProxy::HandleClicked);
		BindingProxies.Add(Proxy);
		return KeyText;
	};
	constexpr float KeyWidth = 130.f;
	constexpr float PadWidth = 90.f;

	// Column titles.
	UHorizontalBox* Header = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass());
	Page->AddChildToVerticalBox(Header)->SetHorizontalAlignment(HAlign_Fill);
	if (UHorizontalBoxSlot* SpacerSlot = Header->AddChildToHorizontalBox(MakeText(FString(), 13, DimColor)))
	{
		SpacerSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
	}
	AddColumn(Header, KeyWidth, MakeText(TEXT("Keyboard / mouse"), 13, DimColor));
	AddColumn(Header, PadWidth, MakeText(TEXT("Gamepad"), 13, DimColor));

	for (int32 i = 0; i < static_cast<int32>(ENBBinding::Count); ++i)
	{
		const ENBBinding Binding = static_cast<ENBBinding>(i);
		// One row: the action on the left, its key and its gamepad button on the right.
		UHorizontalBox* Row = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass());
		UVerticalBoxSlot* RowSlot = Page->AddChildToVerticalBox(Row);
		RowSlot->SetPadding(FMargin(0.f, 1.f));
		RowSlot->SetHorizontalAlignment(HAlign_Fill);

		UTextBlock* Label = MakeText(UNBControlSettings::GetBindingLabel(Binding), 15, FLinearColor::White);
		Label->SetJustification(ETextJustify::Left);
		if (UHorizontalBoxSlot* LabelSlot = Row->AddChildToHorizontalBox(Label))
		{
			LabelSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
			LabelSlot->SetVerticalAlignment(VAlign_Center);
		}

		BindingKeyTexts.Add(AddKeyButton(Row, KeyWidth, i, false));
		if (UNBControlSettings::HasPadButton(Binding))
		{
			PadKeyTexts.Add(AddKeyButton(Row, PadWidth, i, true));
		}
		else
		{
			AddColumn(Row, PadWidth, MakeText(TEXT("left stick"), 13, DimColor));
			PadKeyTexts.Add(nullptr);
		}
	}

	BindingNote = AddText(Page, 13, DimColor);
	BindingNote->SetAutoWrapText(true);
	UButton* Reset = AddButton(Page, TEXT("Reset keys"));
	Reset->OnClicked.AddDynamic(this, &UNBLobbyMenu::HandleResetKeys);
	// Not one of the main menu's buttons, which Refresh hides while Options is open.
	Buttons.Remove(Reset);
	RefreshKeyTexts();
}

void UNBLobbyMenu::RefreshKeyTexts()
{
	const UNBControlSettings* Controls = UNBControlSettings::Get(this);
	auto Show = [this](UTextBlock* Text, bool bWaiting, const FString& Label, const TCHAR* Prompt)
	{
		if (Text)
		{
			Text->SetText(FText::FromString(bWaiting ? FString(Prompt) : Label));
			Text->SetColorAndOpacity(FSlateColor(bWaiting ? TitleColor : FLinearColor::White));
		}
	};
	for (int32 i = 0; i < BindingKeyTexts.Num(); ++i)
	{
		const ENBBinding Binding = static_cast<ENBBinding>(i);
		const bool bWaitingHere = i == WaitingBinding;
		Show(BindingKeyTexts[i], bWaitingHere && !bWaitingPad,
			Controls ? Controls->GetKeyLabel(Binding) : UNBControlSettings::GetKeyLabel(UNBControlSettings::GetDefaultKey(Binding)), TEXT("press a key..."));
		Show(PadKeyTexts.IsValidIndex(i) ? PadKeyTexts[i].Get() : nullptr, bWaitingHere && bWaitingPad,
			Controls ? Controls->GetPadKeyLabel(Binding) : UNBControlSettings::GetPadKeyLabel(UNBControlSettings::GetDefaultPadKey(Binding)), TEXT("press..."));
	}
}

void UNBLobbyMenu::BeginRebind(int32 BindingIndex, bool bPad)
{
	WaitingBinding = BindingIndex;
	bWaitingPad = bPad;
	if (BindingNote)
	{
		BindingNote->SetText(FText::FromString(bPad
			? TEXT("Press a gamepad button. Start, Esc or a mouse click cancels.")
			: TEXT("Press a key or click a mouse button. Esc cancels.")));
	}
	RefreshKeyTexts();
}

bool UNBLobbyMenu::TryFinishRebind(const FKey& Key)
{
	if (WaitingBinding == INDEX_NONE)
	{
		return false;
	}
	const ENBBinding Binding = static_cast<ENBBinding>(WaitingBinding);
	UNBControlSettings* Controls = UNBControlSettings::Get(this);
	FString Note;
	if (bWaitingPad)
	{
		if (Key == EKeys::Escape || Key == EKeys::Gamepad_Special_Right || Key.IsMouseButton())
		{
			Note = TEXT("Cancelled.");
		}
		else if (!UNBControlSettings::IsPadBindable(Key))
		{
			// Wait for another button: Start, LB (the ping shift), the D-pad (pings) and the sticks are fixed.
			BindingNote->SetText(FText::FromString(FString::Printf(TEXT("%s can't be used (Start, LB, the D-pad and the sticks are fixed). Try another button."),
				*UNBControlSettings::GetPadKeyLabel(Key))));
			return true;
		}
		else if (Controls)
		{
			const ENBBinding Swapped = Controls->SetPadKey(Binding, Key);
			Note = Swapped == ENBBinding::Count
				? FString::Printf(TEXT("%s is now %s on the gamepad."), UNBControlSettings::GetBindingLabel(Binding), *UNBControlSettings::GetPadKeyLabel(Key))
				: FString::Printf(TEXT("%s is now %s on the gamepad; %s took %s."), UNBControlSettings::GetBindingLabel(Binding),
					*UNBControlSettings::GetPadKeyLabel(Key), UNBControlSettings::GetBindingLabel(Swapped), *Controls->GetPadKeyLabel(Swapped));
		}
	}
	else if (Key == EKeys::Escape || Key == EKeys::Gamepad_FaceButton_Right || Key == EKeys::Gamepad_Special_Right)
	{
		Note = TEXT("Cancelled.");
	}
	else if (!UNBControlSettings::IsBindable(Key))
	{
		// Wait for another key: Tab and 1-8 are fixed, and gamepad buttons go in the Gamepad column.
		BindingNote->SetText(FText::FromString(Key.IsGamepadKey()
			? FString(TEXT("That's a gamepad button: click the action's Gamepad box for that. Press a key, or Esc to cancel."))
			: FString::Printf(TEXT("%s can't be used (Esc, Tab and 1-8 are fixed). Try another key."), *UNBControlSettings::GetKeyLabel(Key))));
		return true;
	}
	else if (Controls)
	{
		const ENBBinding Swapped = Controls->SetKey(Binding, Key);
		Note = Swapped == ENBBinding::Count
			? FString::Printf(TEXT("%s is now %s."), UNBControlSettings::GetBindingLabel(Binding), *UNBControlSettings::GetKeyLabel(Key))
			: FString::Printf(TEXT("%s is now %s; %s took %s."), UNBControlSettings::GetBindingLabel(Binding), *UNBControlSettings::GetKeyLabel(Key),
				UNBControlSettings::GetBindingLabel(Swapped), *Controls->GetKeyLabel(Swapped));
	}
	WaitingBinding = INDEX_NONE;
	BindingNote->SetText(FText::FromString(Note));
	RefreshKeyTexts();
	return true;
}

void UNBLobbyMenu::HandleResetKeys()
{
	if (UNBControlSettings* Controls = UNBControlSettings::Get(this))
	{
		Controls->ResetKeys();
	}
	WaitingBinding = INDEX_NONE;
	BindingNote->SetText(FText::FromString(TEXT("Keys reset to the defaults.")));
	RefreshKeyTexts();
}

FReply UNBLobbyMenu::NativeOnPreviewKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	if (TryFinishRebind(InKeyEvent.GetKey()))
	{
		return FReply::Handled();
	}
	return Super::NativeOnPreviewKeyDown(InGeometry, InKeyEvent);
}

FReply UNBLobbyMenu::NativeOnPreviewMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (TryFinishRebind(InMouseEvent.GetEffectingButton()))
	{
		return FReply::Handled();
	}
	return Super::NativeOnPreviewMouseButtonDown(InGeometry, InMouseEvent);
}

void UNBLobbyMenu::BuildDisplayPage(UVerticalBox* Page)
{
	WindowModeCombo = AddComboRow(Page, TEXT("Window"), TEXT("WindowMode"));
	for (const TCHAR* Mode : WindowModeNames)
	{
		WindowModeCombo->AddOption(Mode);
	}
	WindowModeCombo->OnSelectionChanged.AddDynamic(this, &UNBLobbyMenu::HandleWindowMode);

	ResolutionCombo = AddComboRow(Page, TEXT("Resolution"), TEXT("Resolution"));
	ResolutionCombo->OnSelectionChanged.AddDynamic(this, &UNBLobbyMenu::HandleResolution);

	QualityCombo = AddComboRow(Page, TEXT("Graphics quality"), TEXT("Quality"));
	for (const TCHAR* Level : QualityNames)
	{
		QualityCombo->AddOption(Level);
	}
	QualityCombo->OnSelectionChanged.AddDynamic(this, &UNBLobbyMenu::HandleQuality);

	FrameLimitCombo = AddComboRow(Page, TEXT("Frame rate cap"), TEXT("FrameLimit"));
	for (const TCHAR* Limit : FrameLimitNames)
	{
		FrameLimitCombo->AddOption(Limit);
	}
	FrameLimitCombo->OnSelectionChanged.AddDynamic(this, &UNBLobbyMenu::HandleFrameLimit);

	VSyncCheck = AddCheckRow(Page, TEXT("VSync"), false);
	VSyncCheck->OnCheckStateChanged.AddDynamic(this, &UNBLobbyMenu::HandleVSync);
	TabFirstControls.Add(WindowModeCombo);
	RefreshDisplayOptions();
}

USlider* UNBLobbyMenu::AddSliderRow(UVerticalBox* Box, const FString& Label, float Min, float Max, float Step, float Value, TObjectPtr<UTextBlock>& OutValueText)
{
	OutValueText = AddText(Box, 16, FLinearColor::White);
	OutValueText->SetToolTipText(FText::FromString(Label));
	USlider* Slider = WidgetTree->ConstructWidget<USlider>(USlider::StaticClass());
	Slider->SetMinValue(Min);
	Slider->SetMaxValue(Max);
	Slider->SetStepSize(Step);
	Slider->SetValue(Value);
	Slider->SetSliderBarColor(FLinearColor(1.f, 1.f, 1.f, 0.4f));
	Slider->SetSliderHandleColor(ButtonColor);
	UVerticalBoxSlot* BoxSlot = Box->AddChildToVerticalBox(Slider);
	BoxSlot->SetPadding(FMargin(0.f, 0.f, 0.f, 8.f));
	BoxSlot->SetHorizontalAlignment(HAlign_Fill);
	return Slider;
}

UComboBoxString* UNBLobbyMenu::AddComboRow(UVerticalBox* Box, const FString& Label, const TCHAR* Name)
{
	AddText(Box, 16, FLinearColor::White)->SetText(FText::FromString(Label));
	UComboBoxString* Combo = WidgetTree->ConstructWidget<UComboBoxString>(UComboBoxString::StaticClass(), Name);
	UVerticalBoxSlot* ComboSlot = Box->AddChildToVerticalBox(Combo);
	ComboSlot->SetPadding(FMargin(0.f, 2.f, 0.f, 8.f));
	ComboSlot->SetHorizontalAlignment(HAlign_Fill);
	return Combo;
}

UCheckBox* UNBLobbyMenu::AddCheckRow(UVerticalBox* Box, const FString& Label, bool bChecked)
{
	UHorizontalBox* Row = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass());
	UVerticalBoxSlot* RowSlot = Box->AddChildToVerticalBox(Row);
	RowSlot->SetPadding(FMargin(0.f, 4.f, 0.f, 8.f));
	RowSlot->SetHorizontalAlignment(HAlign_Center);

	UCheckBox* Check = WidgetTree->ConstructWidget<UCheckBox>(UCheckBox::StaticClass());
	Check->SetIsChecked(bChecked);
	Row->AddChildToHorizontalBox(Check)->SetVerticalAlignment(VAlign_Center);

	UTextBlock* Text = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
	FSlateFontInfo Font = Text->GetFont();
	Font.Size = 16;
	Text->SetFont(Font);
	Text->SetText(FText::FromString(Label));
	if (UHorizontalBoxSlot* TextSlot = Row->AddChildToHorizontalBox(Text))
	{
		TextSlot->SetPadding(FMargin(10.f, 0.f, 0.f, 0.f));
		TextSlot->SetVerticalAlignment(VAlign_Center);
	}
	return Check;
}

void UNBLobbyMenu::RefreshSliderTexts()
{
	auto Percent = [](const TCHAR* Label, float Value)
	{
		return FText::FromString(FString::Printf(TEXT("%s  %d%%"), Label, FMath::RoundToInt(Value * 100.f)));
	};
	const UNBAudioSettings* Audio = UNBAudioSettings::Get(this);
	if (Audio && GameVolumeText)
	{
		GameVolumeText->SetText(Percent(TEXT("Game volume"), Audio->GetGameVolume()));
		VoiceVolumeText->SetText(Percent(TEXT("Voices"), Audio->GetVoiceVolume()));
		MicBoostText->SetText(Percent(TEXT("Mic boost"), Audio->GetMicBoost()));
	}
	const UNBControlSettings* Controls = UNBControlSettings::Get(this);
	if (Controls && LookSensitivityText)
	{
		LookSensitivityText->SetText(FText::FromString(FString::Printf(TEXT("Look sensitivity  %.2fx"), Controls->GetLookSensitivity())));
		CameraShakeText->SetText(Percent(TEXT("Crash camera shake"), Controls->GetCameraShake()));
	}
}

void UNBLobbyMenu::ShowTab(EOptionsTab Tab)
{
	CurrentTab = Tab;
	WaitingBinding = INDEX_NONE;
	RefreshKeyTexts();
	const int32 Index = static_cast<int32>(Tab);
	for (int32 i = 0; i < TabPages.Num(); ++i)
	{
		TabPages[i]->SetVisibility(i == Index ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
		TabButtons[i]->SetBackgroundColor(i == Index ? ButtonColor : FLinearColor(0.35f, 0.3f, 0.25f));
	}
	if (Tab == EOptionsTab::Display)
	{
		RefreshDisplayOptions();
	}
	if (bShowSettings && TabFirstControls.IsValidIndex(Index))
	{
		TabFirstControls[Index]->SetKeyboardFocus();
	}
}

void UNBLobbyMenu::HandleSettings()
{
	bShowSettings = true;
	// So a mic plugged in after launch shows up.
	RefreshMicDevices();
	Refresh();
	ShowTab(CurrentTab);
}

void UNBLobbyMenu::HandleSettingsBack()
{
	bShowSettings = false;
	FocusFirstButton();
}

void UNBLobbyMenu::HandleSoundTab()
{
	ShowTab(EOptionsTab::Sound);
}

void UNBLobbyMenu::HandleControlsTab()
{
	ShowTab(EOptionsTab::Controls);
}

void UNBLobbyMenu::HandleDisplayTab()
{
	ShowTab(EOptionsTab::Display);
}

void UNBLobbyMenu::HandleLookSensitivity(float Value)
{
	if (UNBControlSettings* Controls = UNBControlSettings::Get(this))
	{
		Controls->SetLookSensitivity(Value);
	}
	RefreshSliderTexts();
}

FString UNBLobbyMenu::GetPushToTalkKeyLabel() const
{
	const UNBControlSettings* Controls = UNBControlSettings::Get(this);
	const FString Key = Controls ? Controls->GetKeyLabel(ENBBinding::PushToTalk) : UNBControlSettings::GetKeyLabel(UNBControlSettings::GetDefaultKey(ENBBinding::PushToTalk));
	const FString Pad = Controls ? Controls->GetPadKeyLabel(ENBBinding::PushToTalk) : UNBControlSettings::GetPadKeyLabel(UNBControlSettings::GetDefaultPadKey(ENBBinding::PushToTalk));
	return FString::Printf(TEXT("%s, gamepad %s"), *Key, *Pad);
}

void UNBLobbyMenu::HandlePushToTalk(bool bChecked)
{
	if (UNBAudioSettings* Audio = UNBAudioSettings::Get(this))
	{
		Audio->SetPushToTalk(bChecked);
	}
}

void UNBLobbyMenu::HandleInvertLook(bool bChecked)
{
	if (UNBControlSettings* Controls = UNBControlSettings::Get(this))
	{
		Controls->SetLookInverted(bChecked);
	}
}

void UNBLobbyMenu::HandleCameraShake(float Value)
{
	if (UNBControlSettings* Controls = UNBControlSettings::Get(this))
	{
		Controls->SetCameraShake(Value);
	}
	RefreshSliderTexts();
}

void UNBLobbyMenu::RefreshDisplayOptions()
{
	const UGameUserSettings* Settings = GEngine ? GEngine->GetGameUserSettings() : nullptr;
	if (!Settings || !WindowModeCombo)
	{
		return;
	}
	// The handlers ignore ESelectInfo::Direct, so filling the combos here applies nothing.
	const EWindowMode::Type Mode = Settings->GetFullscreenMode();
	WindowModeCombo->SetSelectedOption(WindowModeNames[Mode == EWindowMode::Fullscreen ? 0 : Mode == EWindowMode::WindowedFullscreen ? 1 : 2]);

	ResolutionCombo->ClearOptions();
	TArray<FIntPoint> Resolutions;
	UKismetSystemLibrary::GetSupportedFullscreenResolutions(Resolutions);
	const FIntPoint Current = Settings->GetScreenResolution();
	Resolutions.AddUnique(Current);
	Resolutions.Sort([](const FIntPoint& A, const FIntPoint& B) { return A.X != B.X ? A.X > B.X : A.Y > B.Y; });
	for (const FIntPoint& Resolution : Resolutions)
	{
		ResolutionCombo->AddOption(FString::Printf(TEXT("%d x %d"), Resolution.X, Resolution.Y));
	}
	ResolutionCombo->SetSelectedOption(FString::Printf(TEXT("%d x %d"), Current.X, Current.Y));
	// Borderless always fills the monitor, so the resolution only matters for the other two.
	ResolutionCombo->SetIsEnabled(Mode != EWindowMode::WindowedFullscreen);

	const int32 Quality = Settings->GetOverallScalabilityLevel();
	const bool bPreset = Quality >= 0 && Quality < UE_ARRAY_COUNT(QualityNames);
	if (!bPreset && QualityCombo->FindOptionIndex(TEXT("Custom")) == INDEX_NONE)
	{
		QualityCombo->AddOption(TEXT("Custom"));
	}
	QualityCombo->SetSelectedOption(bPreset ? QualityNames[Quality] : TEXT("Custom"));

	const float Limit = Settings->GetFrameRateLimit();
	int32 LimitIndex = UE_ARRAY_COUNT(FrameLimitValues) - 1;
	for (int32 i = 0; i < UE_ARRAY_COUNT(FrameLimitValues); ++i)
	{
		if (FMath::IsNearlyEqual(Limit, FrameLimitValues[i]))
		{
			LimitIndex = i;
		}
	}
	FrameLimitCombo->SetSelectedOption(FrameLimitNames[LimitIndex]);
	VSyncCheck->SetIsChecked(Settings->IsVSyncEnabled());
}

void UNBLobbyMenu::ApplyDisplay()
{
	if (UGameUserSettings* Settings = GEngine ? GEngine->GetGameUserSettings() : nullptr)
	{
		// Applies resolution, window mode, scalability and the frame limit, then saves GameUserSettings.ini.
		Settings->ApplySettings(false);
	}
	RefreshDisplayOptions();
}

void UNBLobbyMenu::HandleWindowMode(FString SelectedItem, ESelectInfo::Type SelectionType)
{
	UGameUserSettings* Settings = GEngine ? GEngine->GetGameUserSettings() : nullptr;
	if (!Settings || SelectionType == ESelectInfo::Direct)
	{
		return;
	}
	const int32 Index = WindowModeCombo->GetSelectedIndex();
	const EWindowMode::Type Mode = Index == 0 ? EWindowMode::Fullscreen : Index == 1 ? EWindowMode::WindowedFullscreen : EWindowMode::Windowed;
	Settings->SetFullscreenMode(Mode);
	if (Mode == EWindowMode::WindowedFullscreen)
	{
		Settings->SetScreenResolution(Settings->GetDesktopResolution());
	}
	ApplyDisplay();
}

void UNBLobbyMenu::HandleResolution(FString SelectedItem, ESelectInfo::Type SelectionType)
{
	UGameUserSettings* Settings = GEngine ? GEngine->GetGameUserSettings() : nullptr;
	FString Width, Height;
	if (!Settings || SelectionType == ESelectInfo::Direct || !SelectedItem.Split(TEXT(" x "), &Width, &Height))
	{
		return;
	}
	Settings->SetScreenResolution(FIntPoint(FCString::Atoi(*Width), FCString::Atoi(*Height)));
	ApplyDisplay();
}

void UNBLobbyMenu::HandleQuality(FString SelectedItem, ESelectInfo::Type SelectionType)
{
	UGameUserSettings* Settings = GEngine ? GEngine->GetGameUserSettings() : nullptr;
	const int32 Index = QualityCombo->GetSelectedIndex();
	if (!Settings || SelectionType == ESelectInfo::Direct || Index < 0 || Index >= UE_ARRAY_COUNT(QualityNames))
	{
		return;
	}
	Settings->SetOverallScalabilityLevel(Index);
	ApplyDisplay();
}

void UNBLobbyMenu::HandleFrameLimit(FString SelectedItem, ESelectInfo::Type SelectionType)
{
	UGameUserSettings* Settings = GEngine ? GEngine->GetGameUserSettings() : nullptr;
	const int32 Index = FrameLimitCombo->GetSelectedIndex();
	if (!Settings || SelectionType == ESelectInfo::Direct || Index < 0 || Index >= UE_ARRAY_COUNT(FrameLimitValues))
	{
		return;
	}
	Settings->SetFrameRateLimit(FrameLimitValues[Index]);
	ApplyDisplay();
}

void UNBLobbyMenu::HandleVSync(bool bChecked)
{
	if (UGameUserSettings* Settings = GEngine ? GEngine->GetGameUserSettings() : nullptr)
	{
		Settings->SetVSyncEnabled(bChecked);
		ApplyDisplay();
	}
}

void UNBLobbyMenu::HandleGameVolume(float Value)
{
	if (UNBAudioSettings* Audio = UNBAudioSettings::Get(this))
	{
		Audio->SetGameVolume(Value);
	}
	RefreshSliderTexts();
}

void UNBLobbyMenu::HandleVoiceVolume(float Value)
{
	if (UNBAudioSettings* Audio = UNBAudioSettings::Get(this))
	{
		Audio->SetVoiceVolume(Value);
	}
	RefreshSliderTexts();
}

void UNBLobbyMenu::HandleMicBoost(float Value)
{
	if (UNBAudioSettings* Audio = UNBAudioSettings::Get(this))
	{
		Audio->SetMicBoost(Value);
	}
	RefreshSliderTexts();
}

void UNBLobbyMenu::HandleOutputDevice(FString SelectedItem, ESelectInfo::Type SelectionType)
{
	// Direct is our own SetSelectedOption while filling the list.
	UNBAudioSettings* Audio = UNBAudioSettings::Get(this);
	if (Audio && SelectionType != ESelectInfo::Direct)
	{
		if (const FString* Id = DeviceIds.Find(SelectedItem))
		{
			Audio->SetOutputDevice(*Id);
		}
	}
}

void UNBLobbyMenu::RefreshMicDevices()
{
	if (!MicDeviceCombo)
	{
		return;
	}
	const UNBAudioSettings* Audio = UNBAudioSettings::Get(this);
	const FString Current = Audio && !Audio->GetMicDevice().IsEmpty() ? Audio->GetMicDevice() : SystemDefaultEntry;
	TArray<FString> Wanted = UNBAudioSettings::GetMicDevices();
	Wanted.Insert(SystemDefaultEntry, 0);
	// A saved mic that isn't plugged in stays chosen: voice uses the default until it's back.
	Wanted.AddUnique(Current);
	// Updated in place, not cleared: ClearOptions keeps the old selection's name, so selecting it
	// again afterwards does nothing and the box shows blank.
	for (int32 Index = MicDeviceCombo->GetOptionCount() - 1; Index >= 0; --Index)
	{
		const FString Option = MicDeviceCombo->GetOptionAtIndex(Index);
		if (!Wanted.Contains(Option))
		{
			MicDeviceCombo->RemoveOption(Option);
		}
	}
	for (const FString& Mic : Wanted)
	{
		if (MicDeviceCombo->FindOptionIndex(Mic) == INDEX_NONE)
		{
			MicDeviceCombo->AddOption(Mic);
		}
	}
	MicDeviceCombo->SetSelectedOption(Current);
}

void UNBLobbyMenu::HandleMicDevice(FString SelectedItem, ESelectInfo::Type SelectionType)
{
	// Direct is our own SetSelectedOption while filling the list.
	UNBAudioSettings* Audio = UNBAudioSettings::Get(this);
	if (Audio && SelectionType != ESelectInfo::Direct)
	{
		Audio->SetMicDevice(SelectedItem == SystemDefaultEntry ? FString() : SelectedItem);
	}
}

void UNBLobbyMenu::HandleDevicesObtained(const TArray<FAudioOutputDeviceInfo>& Devices)
{
	if (!OutputDeviceCombo)
	{
		return;
	}
	const UNBAudioSettings* Audio = UNBAudioSettings::Get(this);
	const FString Current = Audio ? Audio->GetOutputDeviceId() : FString();
	OutputDeviceCombo->ClearOptions();
	DeviceIds.Reset();
	OutputDeviceCombo->AddOption(SystemDefaultEntry);
	DeviceIds.Add(SystemDefaultEntry, FString());
	FString Selected = SystemDefaultEntry;
	for (const FAudioOutputDeviceInfo& Device : Devices)
	{
		FString Entry = Device.Name;
		while (DeviceIds.Contains(Entry))
		{
			Entry += TEXT(" ");
		}
		OutputDeviceCombo->AddOption(Entry);
		DeviceIds.Add(Entry, Device.DeviceId);
		if (!Current.IsEmpty() && Device.DeviceId == Current)
		{
			Selected = Entry;
		}
	}
	OutputDeviceCombo->SetSelectedOption(Selected);
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
			if (State && State->IsManualGears())
			{
				Hint += FString::Printf(TEXT("\nManual gears ON: the GEARS seat shifts, score x%.2f"), State->GetManualGearsScale());
			}
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

	Show(ResumeButton, true);
	Show(SettingsButton, true);
	Show(QuitButton, true);
	Show(HostButton, !bOnline);
	Show(JoinButton, !bOnline);
	Show(PracticeButton, !bOnline && Phase == ENBRunPhase::Waiting);
	Show(StartButton, bOnline && bAuthority && Phase == ENBRunPhase::Waiting);
	StartButton->SetIsEnabled(Mode && Mode->CanStartRun());
	Show(GearsButton, bAuthority && Mode && Mode->CanChangeManualGears());
	if (UTextBlock* GearsLabel = Cast<UTextBlock>(GearsButton->GetChildAt(0)))
	{
		const bool bManual = State && State->IsManualGears();
		GearsLabel->SetText(FText::FromString(bManual
			? FString::Printf(TEXT("Manual gears: ON  (score x%.2f)"), State->GetManualGearsScale())
			: FString(TEXT("Manual gears: OFF"))));
	}
	Show(InviteButton, bOnline);
	Show(RestartButton, bAuthority && Phase != ENBRunPhase::Waiting);
	Show(LeaveButton, bOnline);

	// The sound page takes over the panel: hide every other button and the status lines.
	if (bShowSettings)
	{
		for (UButton* Button : Buttons)
		{
			if (Button && Button->GetParent() != SettingsBox)
			{
				Show(Button, false);
			}
		}
		Show(CrewText, false);
		Show(StatusText, false);
	}
	Show(SettingsBox, bShowSettings);
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
	if (Key == EKeys::Escape || Key == EKeys::Tab || Key == EKeys::Gamepad_Special_Right || Key == EKeys::Gamepad_FaceButton_Right)
	{
		if (bShowSettings)
		{
			HandleSettingsBack();
			return FReply::Handled();
		}
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

void UNBLobbyMenu::HandleManualGears()
{
	if (ANBRunGameMode* Mode = GetWorld()->GetAuthGameMode<ANBRunGameMode>())
	{
		const ANBRunGameState* State = GetWorld()->GetGameState<ANBRunGameState>();
		Mode->SetManualGears(!(State && State->IsManualGears()));
	}
	Refresh();
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
