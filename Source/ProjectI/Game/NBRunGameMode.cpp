// Squirrel Wheels prototype.

#include "Game/NBRunGameMode.h"

#include "Car/NBCar.h"
#include "EngineUtils.h"
#include "Game/NBFailureDirector.h"
#include "Game/NBFinishZone.h"
#include "Game/NBRunGameState.h"
#include "Car/NBSeatComponent.h"
#include "Parts/NBCarPartComponent.h"
#include "Player/NBPlayerState.h"
#include "Player/NBSquirrel.h"
#include "UI/NBHUD.h"
#include "UObject/ConstructorHelpers.h"

ANBRunGameMode::ANBRunGameMode()
{
	PrimaryActorTick.bCanEverTick = true;

	DefaultPawnClass = ANBSquirrel::StaticClass();
	HUDClass = ANBHUD::StaticClass();
	PlayerStateClass = ANBPlayerState::StaticClass();
	GameStateClass = ANBRunGameState::StaticClass();

	Director = CreateDefaultSubobject<UNBFailureDirector>(TEXT("FailureDirector"));

	// The Blueprint child carries the mesh, tire sockets and torque/steering curves.
	static ConstructorHelpers::FClassFinder<ANBCar> CarBP(TEXT("/Game/VehicleTemplate/Blueprints/OffroadCar/BP_OffroadCar_Pawn"));
	CarClass = CarBP.Succeeded() ? CarBP.Class : TSubclassOf<ANBCar>(ANBCar::StaticClass());
}

double ANBRunGameMode::Now() const
{
	const AGameStateBase* State = GameState;
	return State ? State->GetServerWorldTimeSeconds() : GetWorld()->GetTimeSeconds();
}

ANBRunGameState* ANBRunGameMode::GetRunState() const
{
	return GetGameState<ANBRunGameState>();
}

void ANBRunGameMode::StartPlay()
{
	Super::StartPlay();

	FindOrSpawnCar();
	if (Car)
	{
		Car->SetRunLocked(true);
		FindOrSpawnFinishZone();
	}
	if (ANBRunGameState* State = GetRunState())
	{
		State->SetPhase(ENBRunPhase::Waiting, 0.0);
	}
}

void ANBRunGameMode::FindOrSpawnCar()
{
	for (TActorIterator<ANBCar> It(GetWorld()); It; ++It)
	{
		Car = *It;
		return;
	}

	FTransform SpawnTransform = FTransform::Identity;
	if (const AActor* Start = FindPlayerStart(nullptr))
	{
		const FRotator Yaw(0.f, Start->GetActorRotation().Yaw, 0.f);
		SpawnTransform = FTransform(Yaw, Start->GetActorLocation() + Yaw.RotateVector(CarSpawnOffset));
	}

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	Car = GetWorld()->SpawnActor<ANBCar>(CarClass, SpawnTransform, Params);
}

void ANBRunGameMode::FindOrSpawnFinishZone()
{
	for (TActorIterator<ANBFinishZone> It(GetWorld()); It; ++It)
	{
		FinishZone = *It;
		return;
	}

	const FVector Forward = FRotator(0.f, Car->GetActorRotation().Yaw, 0.f).Vector();
	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	FinishZone = GetWorld()->SpawnActor<ANBFinishZone>(ANBFinishZone::StaticClass(),
		Car->GetActorLocation() + Forward * FallbackFinishDistance, FRotator(0.f, Car->GetActorRotation().Yaw, 0.f), Params);
}

void ANBRunGameMode::DevStartRun()
{
	bForceStart = true;
}

void ANBRunGameMode::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	ANBRunGameState* State = GetRunState();
	if (!State || !Car)
	{
		return;
	}

	switch (State->GetPhase())
	{
	case ENBRunPhase::Waiting:
		TickWaiting();
		break;
	case ENBRunPhase::Countdown:
		if (State->GetSecondsLeft() <= 0.f)
		{
			BeginDriving();
		}
		break;
	case ENBRunPhase::Driving:
		TickDriving(DeltaSeconds);
		break;
	case ENBRunPhase::Finished:
		if (State->GetSecondsLeft() <= 0.f)
		{
			// Fresh map, fresh car. (In PIE this does nothing; stop and replay.)
			GetWorld()->ServerTravel(TEXT("?Restart"));
		}
		break;
	}
}

void ANBRunGameMode::TickWaiting()
{
	if (bForceStart || GameState->PlayerArray.Num() >= MinPlayers)
	{
		BeginCountdown();
	}
}

void ANBRunGameMode::BeginCountdown()
{
	GetRunState()->SetPhase(ENBRunPhase::Countdown, Now() + CountdownSeconds);
}

void ANBRunGameMode::BeginDriving()
{
	GetRunState()->SetPhase(ENBRunPhase::Driving, Now() + RunSeconds);
	WreckTimer = 0.f;
	NextMuteTime = Now() + FMath::FRandRange(MuteMinInterval, MuteMaxInterval);
	Car->SetRunLocked(false);
	Director->Begin(Car, RunSeconds);
}

void ANBRunGameMode::TickDriving(float DeltaSeconds)
{
	ANBRunGameState* State = GetRunState();

	if (FinishZone && !Car->IsFlipped() && FinishZone->Contains(*Car))
	{
		EndRun(ENBRunResult::Delivered);
		return;
	}
	if (State->GetSecondsLeft() <= 0.f)
	{
		EndRun(ENBRunResult::TimeUp);
		return;
	}

	if (Now() >= NextMuteTime)
	{
		if (State->GetSecondsLeft() > MuteSeconds)
		{
			GiveAcornInMouth();
		}
		NextMuteTime = Now() + FMath::FRandRange(MuteMinInterval, MuteMaxInterval);
	}

	int32 NumFailed = 0;
	for (const UNBCarPartComponent* Part : Car->GetParts())
	{
		NumFailed += (Part && Part->IsFailed()) ? 1 : 0;
	}
	const float PrevWreck = WreckTimer;
	WreckTimer = NumFailed >= WreckPartCount ? WreckTimer + DeltaSeconds : 0.f;
	// Replicate in 0.1 s steps rather than every frame.
	if (FMath::FloorToInt(WreckTimer * 10.f) != FMath::FloorToInt(PrevWreck * 10.f))
	{
		State->SetWreckSeconds(WreckTimer);
	}
	if (WreckTimer >= WreckSeconds)
	{
		EndRun(ENBRunResult::Wrecked);
	}
}

void ANBRunGameMode::EndRun(ENBRunResult Result)
{
	ANBRunGameState* State = GetRunState();
	const float SecondsLeft = State->GetSecondsLeft();

	int32 Delivered = 0;
	int32 TimeBonus = 0;
	int32 Penalty = 0;
	int32 Score = 0;
	if (Result == ENBRunResult::Delivered)
	{
		Delivered = Car->GetAcorns();
		TimeBonus = FMath::FloorToInt(SecondsLeft) * PointsPerSecondLeft;
		for (const APlayerState* Player : GameState->PlayerArray)
		{
			if (const ANBPlayerState* Stats = Cast<ANBPlayerState>(Player))
			{
				Penalty += Stats->GetRespawns() * RespawnPenalty;
			}
		}
		Score = FMath::Max(0, Delivered * PointsPerAcorn + TimeBonus - Penalty);
	}

	Director->End();
	for (APlayerState* Player : GameState->PlayerArray)
	{
		if (ANBPlayerState* Stats = Cast<ANBPlayerState>(Player))
		{
			Stats->ClearMute();
		}
	}
	Car->SetRunLocked(true);
	State->SetWreckSeconds(0.f);
	State->Finish(Result, Delivered, TimeBonus, Penalty, Score, Now() + EndScreenSeconds);
}

void ANBRunGameMode::GiveAcornInMouth()
{
	TArray<ANBPlayerState*> Candidates;
	TArray<float> Weights;
	float TotalWeight = 0.f;
	for (TActorIterator<ANBSquirrel> It(GetWorld()); It; ++It)
	{
		ANBPlayerState* Stats = It->GetPlayerState<ANBPlayerState>();
		if (!Stats || Stats->IsMuted())
		{
			continue;
		}
		float Weight = 1.f;
		if (const UNBSeatComponent* Seat = It->GetCurrentSeat(); Seat && Seat->Role == ENBSeatRole::Wheel)
		{
			Weight += 2.f;
		}
		for (const UNBCarPartComponent* Part : Car->GetParts())
		{
			if (Part && Part->IsFailed() && Part->IsInRangeOf(*It))
			{
				Weight += 2.f;
				break;
			}
		}
		Candidates.Add(Stats);
		Weights.Add(Weight);
		TotalWeight += Weight;
	}
	if (Candidates.IsEmpty())
	{
		return;
	}

	float Roll = FMath::FRandRange(0.f, TotalWeight);
	for (int32 i = 0; i < Candidates.Num(); ++i)
	{
		Roll -= Weights[i];
		if (Roll <= 0.f || i == Candidates.Num() - 1)
		{
			Candidates[i]->StartMute(MuteSeconds);
			return;
		}
	}
}
