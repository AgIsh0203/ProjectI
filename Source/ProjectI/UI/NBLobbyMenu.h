// Squirrel Wheels prototype.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "NBLobbyMenu.generated.h"

class UButton;
class UCheckBox;
class UComboBoxString;
class USlider;
class UTextBlock;
class UVerticalBox;
struct FAudioOutputDeviceInfo;
class UNBLobbyMenu;

/** One key-binding button on the Options page: dynamic OnClicked has no payload, so this carries the index. */
UCLASS()
class PROJECTI_API UNBBindingButtonProxy : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY()
	TObjectPtr<UNBLobbyMenu> Menu;

	int32 Index = 0;

	/** The Gamepad column rather than the keyboard one. */
	bool bPad = false;

	UFUNCTION()
	void HandleClicked();
};

/**
 * The lobby menu, built in code so it needs no widget asset. Offline it offers host, join,
 * practice and quit; in a game it lists the crew, and the host can start, invite, restart
 * or leave; Options holds sound, controls and display settings. Opened and closed by ANBHUD
 * (Esc, Tab or gamepad Start toggles it; Tab because Esc stops Play In Editor).
 */
UCLASS()
class PROJECTI_API UNBLobbyMenu : public UUserWidget
{
	GENERATED_BODY()

public:
	/** Give keyboard/gamepad focus to the first button that is showing. */
	void FocusFirstButton();

	/** Options > Controls: wait for the next key or mouse button (bPad: gamepad button) for this ENBBinding. */
	void BeginRebind(int32 BindingIndex, bool bPad);

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;
	/** Previews, so a key being bound (Space, Enter) isn't eaten by the focused button first. */
	virtual FReply NativeOnPreviewKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;
	virtual FReply NativeOnPreviewMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

private:
	void BuildTree();
	UTextBlock* AddText(UVerticalBox* Box, int32 Size, const FLinearColor& Color);
	UButton* AddButton(UVerticalBox* Box, const FString& Label);
	/** Show the buttons and lines that fit the net mode and run phase. */
	void Refresh();
	void CloseMenu();

	UFUNCTION()
	void HandleHost();

	UFUNCTION()
	void HandleJoin();

	UFUNCTION()
	void HandlePractice();

	UFUNCTION()
	void HandleStart();

	/** The host's Manual gears ON / OFF (G5), before the run starts. */
	UFUNCTION()
	void HandleManualGears();

	UFUNCTION()
	void HandleInvite();

	UFUNCTION()
	void HandleRestart();

	UFUNCTION()
	void HandleResume();

	UFUNCTION()
	void HandleLeave();

	UFUNCTION()
	void HandleQuit();

	// --- Options page: replaces the buttons while open, with Sound / Controls / Display tabs. ---

	enum class EOptionsTab : uint8 { Sound, Controls, Display };

	void BuildSettings(UVerticalBox* Column);
	void BuildSoundPage(UVerticalBox* Page);
	void BuildControlsPage(UVerticalBox* Page);
	void BuildDisplayPage(UVerticalBox* Page);
	/** A labelled slider; the label text is refreshed by RefreshSliderTexts. */
	USlider* AddSliderRow(UVerticalBox* Box, const FString& Label, float Min, float Max, float Step, float Value, TObjectPtr<UTextBlock>& OutValueText);
	/** A label over a full-width combo box. */
	UComboBoxString* AddComboRow(UVerticalBox* Box, const FString& Label, const TCHAR* Name);
	/** A check box with a label beside it. */
	UCheckBox* AddCheckRow(UVerticalBox* Box, const FString& Label, bool bChecked);
	void RefreshSliderTexts();
	/** Lists the mics again (Options opened) and selects the saved one. */
	void RefreshMicDevices();
	void ShowTab(EOptionsTab Tab);
	/** Fill the display combos from the current UGameUserSettings. */
	void RefreshDisplayOptions();
	/** Apply and save UGameUserSettings after a display change. */
	void ApplyDisplay();

	UFUNCTION()
	void HandleSettings();

	UFUNCTION()
	void HandleSettingsBack();

	UFUNCTION()
	void HandleSoundTab();

	UFUNCTION()
	void HandleControlsTab();

	UFUNCTION()
	void HandleDisplayTab();

	UFUNCTION()
	void HandleLookSensitivity(float Value);

	UFUNCTION()
	void HandleInvertLook(bool bChecked);

	UFUNCTION()
	void HandlePushToTalk(bool bChecked);

	/** The push-to-talk key's short name, for the Sound tab's check box. */
	FString GetPushToTalkKeyLabel() const;

	UFUNCTION()
	void HandleCameraShake(float Value);

	UFUNCTION()
	void HandleWindowMode(FString SelectedItem, ESelectInfo::Type SelectionType);

	UFUNCTION()
	void HandleResolution(FString SelectedItem, ESelectInfo::Type SelectionType);

	UFUNCTION()
	void HandleQuality(FString SelectedItem, ESelectInfo::Type SelectionType);

	UFUNCTION()
	void HandleFrameLimit(FString SelectedItem, ESelectInfo::Type SelectionType);

	UFUNCTION()
	void HandleVSync(bool bChecked);

	/** The key list on the Controls tab: one row per ENBBinding, plus Reset. */
	void BuildKeyList(UVerticalBox* Page);
	void RefreshKeyTexts();
	/** Bind the waiting action to Key if it can be bound; true if the event was used. */
	bool TryFinishRebind(const FKey& Key);

	UFUNCTION()
	void HandleResetKeys();

	/** ENBBinding being rebound, or INDEX_NONE. */
	int32 WaitingBinding = INDEX_NONE;

	/** WaitingBinding wants a gamepad button. */
	bool bWaitingPad = false;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UTextBlock>> BindingKeyTexts;

	/** The Gamepad column per ENBBinding; null for the moves (the left stick). */
	UPROPERTY(Transient)
	TArray<TObjectPtr<UTextBlock>> PadKeyTexts;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UNBBindingButtonProxy>> BindingProxies;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> BindingNote;

	UFUNCTION()
	void HandleGameVolume(float Value);

	UFUNCTION()
	void HandleVoiceVolume(float Value);

	UFUNCTION()
	void HandleMicBoost(float Value);

	UFUNCTION()
	void HandleOutputDevice(FString SelectedItem, ESelectInfo::Type SelectionType);

	UFUNCTION()
	void HandleDevicesObtained(const TArray<FAudioOutputDeviceInfo>& Devices);

	UFUNCTION()
	void HandleMicDevice(FString SelectedItem, ESelectInfo::Type SelectionType);

	/** The first entry of both device lists ("" = the system default). Not "Windows default": there's a Linux build. */
	inline static const FString SystemDefaultEntry = TEXT("System default");

	bool bShowSettings = false;
	EOptionsTab CurrentTab = EOptionsTab::Sound;

	/** One per tab, in EOptionsTab order. */
	UPROPERTY(Transient)
	TArray<TObjectPtr<UButton>> TabButtons;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UVerticalBox>> TabPages;

	/** The control each tab focuses first, for keyboard and gamepad. */
	UPROPERTY(Transient)
	TArray<TObjectPtr<UWidget>> TabFirstControls;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> LookSensitivityText;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> CameraShakeText;

	UPROPERTY(Transient)
	TObjectPtr<UComboBoxString> WindowModeCombo;

	UPROPERTY(Transient)
	TObjectPtr<UComboBoxString> ResolutionCombo;

	UPROPERTY(Transient)
	TObjectPtr<UComboBoxString> QualityCombo;

	UPROPERTY(Transient)
	TObjectPtr<UComboBoxString> FrameLimitCombo;

	UPROPERTY(Transient)
	TObjectPtr<UCheckBox> VSyncCheck;

	UPROPERTY(Transient)
	TObjectPtr<UButton> SettingsButton;

	UPROPERTY(Transient)
	TObjectPtr<UVerticalBox> SettingsBox;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> GameVolumeText;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> VoiceVolumeText;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> MicBoostText;

	UPROPERTY(Transient)
	TObjectPtr<USlider> GameVolumeSlider;

	UPROPERTY(Transient)
	TObjectPtr<UComboBoxString> OutputDeviceCombo;

	/** Combo entry -> device id ("" = the system default). */
	TMap<FString, FString> DeviceIds;

	/** Entries are the mics' names; SystemDefaultEntry = "". */
	UPROPERTY(Transient)
	TObjectPtr<UComboBoxString> MicDeviceCombo;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> ModeText;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> CrewText;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> StatusText;

	UPROPERTY(Transient)
	TObjectPtr<UButton> HostButton;

	UPROPERTY(Transient)
	TObjectPtr<UButton> JoinButton;

	UPROPERTY(Transient)
	TObjectPtr<UButton> PracticeButton;

	UPROPERTY(Transient)
	TObjectPtr<UButton> StartButton;

	UPROPERTY(Transient)
	TObjectPtr<UButton> GearsButton;

	UPROPERTY(Transient)
	TObjectPtr<UButton> InviteButton;

	UPROPERTY(Transient)
	TObjectPtr<UButton> RestartButton;

	UPROPERTY(Transient)
	TObjectPtr<UButton> ResumeButton;

	UPROPERTY(Transient)
	TObjectPtr<UButton> LeaveButton;

	UPROPERTY(Transient)
	TObjectPtr<UButton> QuitButton;

	/** In display order, for focusing the first visible one. */
	UPROPERTY(Transient)
	TArray<TObjectPtr<UButton>> Buttons;
};
