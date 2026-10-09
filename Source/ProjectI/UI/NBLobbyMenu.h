// Squirrel Wheels prototype.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "NBLobbyMenu.generated.h"

class UButton;
class UTextBlock;
class UVerticalBox;

/**
 * The lobby menu, built in code so it needs no widget asset. Offline it offers host, join,
 * practice and quit; in a game it lists the crew, and the host can start, invite, restart
 * or leave. Opened and closed by ANBHUD (Esc / gamepad Start toggles it).
 */
UCLASS()
class PROJECTI_API UNBLobbyMenu : public UUserWidget
{
	GENERATED_BODY()

public:
	/** Give keyboard/gamepad focus to the first button that is showing. */
	void FocusFirstButton();

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

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
