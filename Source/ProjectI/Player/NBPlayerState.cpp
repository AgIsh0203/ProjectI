// Squirrel Wheels prototype.

#include "Player/NBPlayerState.h"

#include "GameFramework/GameStateBase.h"
#include "Net/UnrealNetwork.h"

void ANBPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ANBPlayerState, Falls);
	DOREPLIFETIME(ANBPlayerState, Respawns);
	DOREPLIFETIME(ANBPlayerState, FiresPutOut);
	DOREPLIFETIME(ANBPlayerState, AcornsSpilled);
	DOREPLIFETIME(ANBPlayerState, DriveSeconds);
	DOREPLIFETIME(ANBPlayerState, MuteEndTime);
	DOREPLIFETIME(ANBPlayerState, PingIndex);
	DOREPLIFETIME(ANBPlayerState, PingTime);
}

void ANBPlayerState::AddDriveTime(float Seconds)
{
	DriveTime += Seconds;
	// Only dirty the replicated value once a second.
	DriveSeconds = FMath::FloorToInt(DriveTime);
}

float ANBPlayerState::GetHelpfulness() const
{
	constexpr float PerFire = 3.f;
	constexpr float PerDriveSecond = 1.f / 30.f;
	constexpr float PerFall = 1.f;
	constexpr float PerRespawn = 2.f;
	constexpr float PerAcornSpilled = 0.5f;
	return FiresPutOut * PerFire + DriveSeconds * PerDriveSecond
		- Falls * PerFall - Respawns * PerRespawn - AcornsSpilled * PerAcornSpilled;
}

double ANBPlayerState::ServerNow() const
{
	const AGameStateBase* State = GetWorld()->GetGameState();
	return State ? State->GetServerWorldTimeSeconds() : GetWorld()->GetTimeSeconds();
}

bool ANBPlayerState::IsMuted() const
{
	return MuteEndTime > ServerNow();
}

float ANBPlayerState::GetMuteSecondsLeft() const
{
	return FMath::Max(0.f, static_cast<float>(MuteEndTime - ServerNow()));
}

void ANBPlayerState::StartMute(float Seconds)
{
	MuteEndTime = ServerNow() + Seconds;
}

FText ANBPlayerState::GetPingText(int32 Index)
{
	static const FText Texts[NumPings] = {
		NSLOCTEXT("NB", "Ping1", "LEFT!"),
		NSLOCTEXT("NB", "Ping2", "RIGHT!"),
		NSLOCTEXT("NB", "Ping3", "GAS!"),
		NSLOCTEXT("NB", "Ping4", "BRAKE!"),
		NSLOCTEXT("NB", "Ping5", "HELP!"),
		NSLOCTEXT("NB", "Ping6", "FIX IT!"),
		NSLOCTEXT("NB", "Ping7", "JUMP OUT!"),
		NSLOCTEXT("NB", "Ping8", "NICE!"),
	};
	return Texts[FMath::Clamp(Index, 0, NumPings - 1)];
}

int32 ANBPlayerState::GetActivePing() const
{
	return ServerNow() - PingTime < PingShowSeconds ? PingIndex : INDEX_NONE;
}

bool ANBPlayerState::SendPing(int32 Index)
{
	const double Now = ServerNow();
	if (Index < 0 || Index >= NumPings || Now - PingTime < PingCooldownSeconds)
	{
		return false;
	}
	PingIndex = Index;
	PingTime = Now;
	return true;
}
