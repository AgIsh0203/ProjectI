// Squirrel Wheels prototype.

#include "Game/NBRunGameState.h"

#include "Net/UnrealNetwork.h"

void ANBRunGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ANBRunGameState, Phase);
	DOREPLIFETIME(ANBRunGameState, Result);
	DOREPLIFETIME(ANBRunGameState, PhaseEndTime);
	DOREPLIFETIME(ANBRunGameState, WreckSeconds);
	DOREPLIFETIME(ANBRunGameState, AcornsDelivered);
	DOREPLIFETIME(ANBRunGameState, DriveStartTime);
	DOREPLIFETIME(ANBRunGameState, FinalRunSeconds);
	DOREPLIFETIME(ANBRunGameState, ParSeconds);
	DOREPLIFETIME(ANBRunGameState, TimeBonus);
	DOREPLIFETIME(ANBRunGameState, AirBonus);
	DOREPLIFETIME(ANBRunGameState, RespawnPenalty);
	DOREPLIFETIME(ANBRunGameState, FinalScore);
	DOREPLIFETIME(ANBRunGameState, Mvp);
	DOREPLIFETIME(ANBRunGameState, MostUseless);
	DOREPLIFETIME(ANBRunGameState, bManualGears);
	DOREPLIFETIME(ANBRunGameState, ManualGearsScale);
}

float ANBRunGameState::GetSecondsLeft() const
{
	if (Phase == ENBRunPhase::Waiting || Phase == ENBRunPhase::Driving)
	{
		return 0.f;
	}
	return FMath::Max(0.f, static_cast<float>(PhaseEndTime - GetServerWorldTimeSeconds()));
}

float ANBRunGameState::GetRunSeconds() const
{
	switch (Phase)
	{
	case ENBRunPhase::Driving:  return FMath::Max(0.f, static_cast<float>(GetServerWorldTimeSeconds() - DriveStartTime));
	case ENBRunPhase::Finished: return FinalRunSeconds;
	default:                    return 0.f;
	}
}

void ANBRunGameState::StartDriving(float NewParSeconds)
{
	Phase = ENBRunPhase::Driving;
	DriveStartTime = GetServerWorldTimeSeconds();
	PhaseEndTime = 0.0;
	ParSeconds = NewParSeconds;
}

void ANBRunGameState::SetPhase(ENBRunPhase NewPhase, double NewEndTime)
{
	Phase = NewPhase;
	PhaseEndTime = NewEndTime;
}

void ANBRunGameState::Finish(ENBRunResult NewResult, int32 Delivered, int32 NewTimeBonus, int32 NewAirBonus, int32 NewPenalty, int32 NewScore, double RestartTime)
{
	FinalRunSeconds = GetRunSeconds();
	Phase = ENBRunPhase::Finished;
	Result = NewResult;
	AcornsDelivered = Delivered;
	TimeBonus = NewTimeBonus;
	AirBonus = NewAirBonus;
	RespawnPenalty = NewPenalty;
	FinalScore = NewScore;
	PhaseEndTime = RestartTime;
}
