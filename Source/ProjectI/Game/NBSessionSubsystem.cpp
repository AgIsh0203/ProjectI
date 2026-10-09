// Squirrel Wheels prototype.

#include "Game/NBSessionSubsystem.h"

#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Interfaces/OnlineExternalUIInterface.h"
#include "Online/OnlineSessionNames.h"
#include "OnlineSessionSettings.h"
#include "OnlineSubsystem.h"

namespace
{
	/** Tags our lobbies so a search on the shared dev AppID 480 only finds Squirrel Wheels. */
	const FName GameKey(TEXT("NBGAME"));
	const FString GameValue(TEXT("SquirrelWheels"));
	const TCHAR* RunMap = TEXT("/Game/Maps/L_TestTrack");
	constexpr int32 MaxPlayers = 4;
}

void UNBSessionSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	if (const IOnlineSessionPtr Sessions = GetSessions())
	{
		CreateHandle = Sessions->AddOnCreateSessionCompleteDelegate_Handle(FOnCreateSessionCompleteDelegate::CreateUObject(this, &UNBSessionSubsystem::HandleCreateComplete));
		DestroyHandle = Sessions->AddOnDestroySessionCompleteDelegate_Handle(FOnDestroySessionCompleteDelegate::CreateUObject(this, &UNBSessionSubsystem::HandleDestroyComplete));
		FindHandle = Sessions->AddOnFindSessionsCompleteDelegate_Handle(FOnFindSessionsCompleteDelegate::CreateUObject(this, &UNBSessionSubsystem::HandleFindComplete));
		JoinHandle = Sessions->AddOnJoinSessionCompleteDelegate_Handle(FOnJoinSessionCompleteDelegate::CreateUObject(this, &UNBSessionSubsystem::HandleJoinComplete));
		InviteHandle = Sessions->AddOnSessionUserInviteAcceptedDelegate_Handle(FOnSessionUserInviteAcceptedDelegate::CreateUObject(this, &UNBSessionSubsystem::HandleInviteAccepted));
	}
	if (GEngine)
	{
		NetworkFailureHandle = GEngine->OnNetworkFailure().AddUObject(this, &UNBSessionSubsystem::HandleNetworkFailure);
	}
	UE_LOG(LogTemp, Log, TEXT("NBSession: online subsystem %s"), *GetSubsystemName());
}

void UNBSessionSubsystem::Deinitialize()
{
	if (const IOnlineSessionPtr Sessions = GetSessions())
	{
		Sessions->ClearOnCreateSessionCompleteDelegate_Handle(CreateHandle);
		Sessions->ClearOnDestroySessionCompleteDelegate_Handle(DestroyHandle);
		Sessions->ClearOnFindSessionsCompleteDelegate_Handle(FindHandle);
		Sessions->ClearOnJoinSessionCompleteDelegate_Handle(JoinHandle);
		Sessions->ClearOnSessionUserInviteAcceptedDelegate_Handle(InviteHandle);
	}
	if (GEngine)
	{
		GEngine->OnNetworkFailure().Remove(NetworkFailureHandle);
	}
	Super::Deinitialize();
}

IOnlineSessionPtr UNBSessionSubsystem::GetSessions() const
{
	const IOnlineSubsystem* Online = IOnlineSubsystem::Get();
	return Online ? Online->GetSessionInterface() : nullptr;
}

FString UNBSessionSubsystem::GetSubsystemName() const
{
	const IOnlineSubsystem* Online = IOnlineSubsystem::Get();
	return Online ? Online->GetSubsystemName().ToString() : TEXT("none");
}

FString UNBSessionSubsystem::GetStatusText() const
{
	return Status;
}

void UNBSessionSubsystem::Host()
{
	// Re-hosting would reopen the map while this world still holds the listen socket.
	const UWorld* World = GetGameInstance()->GetWorld();
	const IOnlineSessionPtr Sessions = GetSessions();
	if (World && World->GetNetMode() == NM_ListenServer && Sessions && Sessions->GetNamedSession(NAME_GameSession))
	{
		Status = FString::Printf(TEXT("Already hosting (%s). Shift+Tab to invite friends"), *GetSubsystemName());
		return;
	}
	DestroyThen(EPending::Host);
}

void UNBSessionSubsystem::FindAndJoin()
{
	DestroyThen(EPending::Join);
}

void UNBSessionSubsystem::Leave()
{
	DestroyThen(EPending::Leave);
}

void UNBSessionSubsystem::ShowInviteUI()
{
	const IOnlineSubsystem* Online = IOnlineSubsystem::Get();
	const IOnlineExternalUIPtr ExternalUI = Online ? Online->GetExternalUIInterface() : nullptr;
	if (!ExternalUI.IsValid() || !ExternalUI->ShowInviteUI(0, NAME_GameSession))
	{
		Status = TEXT("Can't open the invite dialog here. Shift+Tab opens the Steam overlay");
	}
}

bool UNBSessionSubsystem::ConsumeRunRestart()
{
	const bool bWas = bRunRestarting;
	bRunRestarting = false;
	return bWas;
}

void UNBSessionSubsystem::DestroyThen(EPending Next)
{
	// A stale session (e.g. after a disconnect) blocks creating or joining a new one.
	const IOnlineSessionPtr Sessions = GetSessions();
	if (Sessions && Sessions->GetNamedSession(NAME_GameSession))
	{
		AfterDestroy = Next;
		Status = TEXT("Leaving old session...");
		Sessions->DestroySession(NAME_GameSession);
		return;
	}

	if (Next == EPending::Host)
	{
		CreateLobby();
	}
	else if (Next == EPending::Join)
	{
		if (!Sessions)
		{
			Status = TEXT("Can't search: no online subsystem");
			return;
		}
		Search = MakeShared<FOnlineSessionSearch>();
		Search->MaxSearchResults = 100;
		Search->bIsLanQuery = GetSubsystemName() == TEXT("NULL");
		Search->QuerySettings.Set(SEARCH_LOBBIES, true, EOnlineComparisonOp::Equals);
		Search->QuerySettings.Set(GameKey, GameValue, EOnlineComparisonOp::Equals);
		Status = TEXT("Searching for a game...");
		Sessions->FindSessions(0, Search.ToSharedRef());
	}
	else if (Next == EPending::Leave)
	{
		// Without "listen" the map opens standalone, which drops any connection and shows the menu.
		Status = TEXT("Left the game");
		UGameplayStatics::OpenLevel(GetGameInstance(), FName(RunMap), true);
	}
}

void UNBSessionSubsystem::CreateLobby()
{
	const IOnlineSessionPtr Sessions = GetSessions();
	if (!Sessions)
	{
		// No online subsystem at all: still let LAN / direct-IP tests host.
		Status = TEXT("Hosting (no online subsystem)");
		UGameplayStatics::OpenLevel(GetGameInstance(), FName(RunMap), true, TEXT("listen"));
		return;
	}

	FOnlineSessionSettings Settings;
	Settings.NumPublicConnections = MaxPlayers;
	Settings.bIsLANMatch = GetSubsystemName() == TEXT("NULL");
	Settings.bShouldAdvertise = true;
	Settings.bUsesPresence = true;
	Settings.bUseLobbiesIfAvailable = true;
	Settings.bAllowJoinInProgress = true;
	Settings.bAllowJoinViaPresence = true;
	Settings.bAllowInvites = true;
	Settings.Set(GameKey, GameValue, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);

	Status = TEXT("Creating lobby...");
	if (!Sessions->CreateSession(0, NAME_GameSession, Settings))
	{
		Status = TEXT("Couldn't create a lobby");
	}
}

void UNBSessionSubsystem::HandleCreateComplete(FName SessionName, bool bSuccess)
{
	if (!bSuccess)
	{
		Status = TEXT("Couldn't create a lobby");
		return;
	}
	Status = FString::Printf(TEXT("Hosting (%s). Shift+Tab to invite friends"), *GetSubsystemName());
	UGameplayStatics::OpenLevel(GetGameInstance(), FName(RunMap), true, TEXT("listen"));
}

void UNBSessionSubsystem::HandleDestroyComplete(FName SessionName, bool bSuccess)
{
	const EPending Next = AfterDestroy;
	AfterDestroy = EPending::None;
	Status.Reset();
	if (Next == EPending::Join && PendingInvite.IsValid())
	{
		JoinResult(PendingInvite);
		PendingInvite = FOnlineSessionSearchResult();
		return;
	}
	DestroyThen(Next);
}

void UNBSessionSubsystem::HandleFindComplete(bool bSuccess)
{
	if (!Search.IsValid())
	{
		return;
	}
	for (const FOnlineSessionSearchResult& Result : Search->SearchResults)
	{
		// The lobby filter isn't guaranteed on every subsystem; check the tag ourselves too.
		FString Value;
		if (Result.IsValid() && Result.Session.SessionSettings.Get(GameKey, Value) && Value == GameValue)
		{
			JoinResult(Result);
			return;
		}
	}
	Status = FString::Printf(TEXT("No game found (%d lobbies seen). Ask the host for an invite"), Search->SearchResults.Num());
}

void UNBSessionSubsystem::JoinResult(const FOnlineSessionSearchResult& Result)
{
	const IOnlineSessionPtr Sessions = GetSessions();
	if (!Sessions)
	{
		return;
	}
	Status = FString::Printf(TEXT("Joining %s..."), *Result.Session.OwningUserName);
	Sessions->JoinSession(0, NAME_GameSession, Result);
}

void UNBSessionSubsystem::HandleJoinComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	const IOnlineSessionPtr Sessions = GetSessions();
	FString Connect;
	if (Result != EOnJoinSessionCompleteResult::Success || !Sessions || !Sessions->GetResolvedConnectString(SessionName, Connect))
	{
		Status = FString::Printf(TEXT("Join failed (%s)"), LexToString(Result));
		return;
	}
	APlayerController* PC = GetGameInstance()->GetFirstLocalPlayerController();
	if (!PC)
	{
		Status = TEXT("Join failed (no local player)");
		return;
	}
	Status = TEXT("Connected");
	UE_LOG(LogTemp, Log, TEXT("NBSession: travelling to %s"), *Connect);
	PC->ClientTravel(Connect, TRAVEL_Absolute);
}

void UNBSessionSubsystem::HandleInviteAccepted(bool bSuccess, int32 ControllerId, FUniqueNetIdPtr UserId, const FOnlineSessionSearchResult& Invite)
{
	if (!bSuccess || !Invite.IsValid())
	{
		Status = TEXT("Invite failed");
		return;
	}
	// Leave whatever we're in (we may be hosting our own lobby) and join the friend's.
	const IOnlineSessionPtr Sessions = GetSessions();
	if (Sessions && Sessions->GetNamedSession(NAME_GameSession))
	{
		PendingInvite = Invite;
		AfterDestroy = EPending::Join;
		Status = TEXT("Leaving old session...");
		Sessions->DestroySession(NAME_GameSession);
		return;
	}
	JoinResult(Invite);
}

void UNBSessionSubsystem::HandleNetworkFailure(UWorld* World, UNetDriver* NetDriver, ENetworkFailure::Type FailureType, const FString& Error)
{
	if (World && World->GetGameInstance() != GetGameInstance())
	{
		return;
	}
	// The engine sends us back to the default map offline; say why on the menu.
	Status = World && World->GetNetMode() == NM_Client
		? FString::Printf(TEXT("Lost the connection to the host (%s)"), ENetworkFailure::ToString(FailureType))
		: FString::Printf(TEXT("Network error (%s)"), ENetworkFailure::ToString(FailureType));
	UE_LOG(LogTemp, Warning, TEXT("NBSession: network failure %s: %s"), ENetworkFailure::ToString(FailureType), *Error);
}
