// Squirrel Wheels prototype.

#include "Game/NBRunGameMode.h"

#include "Car/NBCar.h"
#include "EngineUtils.h"
#include "Game/NBFailureDirector.h"
#include "Game/NBFinishZone.h"
#include "Game/NBRunGameState.h"
#include "Game/NBSessionSubsystem.h"
#include "Engine/GameInstance.h"
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

	// Restarts keep everyone connected (and in the Steam lobby and voice) instead of
	// making clients reconnect. With no transition map set, the engine uses an empty one.
	bUseSeamlessTravel = true;

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

	UNBSessionSubsystem* Sessions = GetGameInstance()->GetSubsystem<UNBSessionSubsystem>();
	const bool bRestarted = Sessions && Sessions->ConsumeRunRestart();
	bAutoStart = !bHostStartsRun || bRestarted || GetWorld()->IsPlayInEditor();

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

bool ANBRunGameMode::CanStartRun() const
{
	const ANBRunGameState* State = GetRunState();
	return State && State->GetPhase() == ENBRunPhase::Waiting && State->PlayerArray.Num() >= MinPlayers;
}

void ANBRunGameMode::StartRunFromLobby()
{
	if (CanStartRun())
	{
		bForceStart = true;
	}
}

void ANBRunGameMode::RestartRun()
{
	if (bRestarting)
	{
		return;
	}
	// Set even if the travel is refused, so the end screen doesn't retry every frame.
	bRestarting = true;

	// Fresh map, fresh car. PIE refuses seamless travel, so there this does nothing; stop and replay.
	if (!GetWorld()->ServerTravel(TEXT("?Restart")))
	{
		UE_LOG(LogTemp, Warning, TEXT("NBRun: restart travel refused (seamless travel doesn't run in PIE)"));
		return;
	}
	if (UNBSessionSubsystem* Sessions = GetGameInstance()->GetSubsystem<UNBSessionSubsystem>())
	{
		Sessions->NoteRunRestart();
	}
	// The old world keeps ticking while the map loads; keep the car still and quiet.
	Director->End();
	if (Car)
	{
		Car->SetRunLocked(true);
	}
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
			RestartRun();
		}
		break;
	}
}

void ANBRunGameMode::TickWaiting()
{
	if (bForceStart || (bAutoStart && GameState->PlayerArray.Num() >= MinPlayers))
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
	Car->SetRunLocked(true);
	State->SetWreckSeconds(0.f);
	State->Finish(Result, Delivered, TimeBonus, Penalty, Score, Now() + EndScreenSeconds);
}
