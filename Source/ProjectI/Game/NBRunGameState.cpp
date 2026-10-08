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
	DOREPLIFETIME(ANBRunGameState, TimeBonus);
	DOREPLIFETIME(ANBRunGameState, RespawnPenalty);
	DOREPLIFETIME(ANBRunGameState, FinalScore);
}

float ANBRunGameState::GetSecondsLeft() const
{
	if (Phase == ENBRunPhase::Waiting)
	{
		return 0.f;
	}
	return FMath::Max(0.f, static_cast<float>(PhaseEndTime - GetServerWorldTimeSeconds()));
}

void ANBRunGameState::SetPhase(ENBRunPhase NewPhase, double NewEndTime)
{
	Phase = NewPhase;
	PhaseEndTime = NewEndTime;
}

void ANBRunGameState::Finish(ENBRunResult NewResult, int32 Delivered, int32 NewTimeBonus, int32 NewPenalty, int32 NewScore, double RestartTime)
{
	Phase = ENBRunPhase::Finished;
	Result = NewResult;
	AcornsDelivered = Delivered;
	TimeBonus = NewTimeBonus;
	RespawnPenalty = NewPenalty;
	FinalScore = NewScore;
	PhaseEndTime = RestartTime;
}
