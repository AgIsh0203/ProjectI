// Squirrel Wheels prototype.

#pragma once

#include "CoreMinimal.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSessionSettings.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "NBSessionSubsystem.generated.h"

/**
 * Steam lobby plumbing for the prototype: host a lobby and reopen the map as a listen
 * server, find and join one, and follow Steam invites / "Join Game" from the friends list.
 * Works with any online subsystem; with Steam off (PIE) hosting still opens a listen server.
 */
UCLASS()
class PROJECTI_API UNBSessionSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/** Create a lobby friends can join, then reload the map as a listen server. */
	void Host();

	/** Search for a Squirrel Wheels lobby and join the first one found. */
	void FindAndJoin();

	/** One line for the HUD, e.g. "Hosting (Steam)" or "Searching...". */
	FString GetStatusText() const;

	/** Which online subsystem is live: "STEAM", "NULL", ... */
	FString GetSubsystemName() const;

private:
	enum class EPending : uint8 { None, Host, Join };

	IOnlineSessionPtr GetSessions() const;
	void DestroyThen(EPending Next);
	void CreateLobby();
	void JoinResult(const FOnlineSessionSearchResult& Result);

	void HandleCreateComplete(FName SessionName, bool bSuccess);
	void HandleDestroyComplete(FName SessionName, bool bSuccess);
	void HandleFindComplete(bool bSuccess);
	void HandleJoinComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);
	void HandleInviteAccepted(bool bSuccess, int32 ControllerId, FUniqueNetIdPtr UserId, const FOnlineSessionSearchResult& Invite);

	TSharedPtr<FOnlineSessionSearch> Search;
	EPending AfterDestroy = EPending::None;

	/** An accepted invite waiting for the old session to be destroyed. */
	FOnlineSessionSearchResult PendingInvite;
	FString Status;

	FDelegateHandle CreateHandle;
	FDelegateHandle DestroyHandle;
	FDelegateHandle FindHandle;
	FDelegateHandle JoinHandle;
	FDelegateHandle InviteHandle;
};
