// Squirrel Wheels prototype.

#include "Game/NBRunGameMode.h"

#include "Car/NBCar.h"
#include "EngineUtils.h"
#include "Game/NBFailureDirector.h"
#include "Game/NBFinishZone.h"
#include "Game/NBRoute.h"
#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"
#include "Game/NBRunGameState.h"
#include "Game/NBSessionSubsystem.h"
#include "Engine/GameInstance.h"
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
	RouteClass = ANBRoute::StaticClass();

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

void ANBRunGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);
	if (UGameplayStatics::HasOption(Options, TEXT("Route")))
	{
		bUseRoute = UGameplayStatics::GetIntOption(Options, TEXT("Route"), 1) != 0;
	}
}

ANBRoute* ANBRunGameMode::EnsureRoute()
{
	if (!bUseRoute || Route)
	{
		return Route;
	}

	for (TActorIterator<ANBRoute> It(GetWorld()); It; ++It)
	{
		Route = *It;
		break;
	}
	if (!Route)
	{
		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		Route = GetWorld()->SpawnActor<ANBRoute>(RouteClass ? *RouteClass : ANBRoute::StaticClass(), RouteOrigin, FRotator::ZeroRotator, Params);
	}
	if (!Route)
	{
		return nullptr;
	}

	// Squirrels start either side of where the car will be.
	const FTransform Start = Route->GetStartTransform();
	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	for (const FVector& Offset : {FVector(400.f, -350.f, 100.f), FVector(400.f, 350.f, 100.f), FVector(800.f, -350.f, 100.f), FVector(800.f, 350.f, 100.f)})
	{
		if (APlayerStart* Spot = GetWorld()->SpawnActor<APlayerStart>(APlayerStart::StaticClass(), Start.TransformPosition(Offset), Start.Rotator(), Params))
		{
			RouteStarts.Add(Spot);
		}
	}
	return Route;
}

AActor* ANBRunGameMode::ChoosePlayerStart_Implementation(AController* Player)
{
	if (EnsureRoute() && !RouteStarts.IsEmpty())
	{
		return RouteStarts[NextRouteStart++ % RouteStarts.Num()];
	}
	return Super::ChoosePlayerStart_Implementation(Player);
}

void ANBRunGameMode::StartPlay()
{
	Super::StartPlay();

	UNBSessionSubsystem* Sessions = GetGameInstance()->GetSubsystem<UNBSessionSubsystem>();
	const bool bRestarted = Sessions && Sessions->ConsumeRunRestart();
	bAutoStart = !bHostStartsRun || bRestarted || GetWorld()->IsPlayInEditor();

	EnsureRoute();
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
	FTransform SpawnTransform = FTransform::Identity;
	if (Route)
	{
		const FTransform Start = Route->GetStartTransform();
		SpawnTransform = FTransform(Start.Rotator(), Start.TransformPosition(CarSpawnOffset));
	}

	for (TActorIterator<ANBCar> It(GetWorld()); It; ++It)
	{
		Car = *It;
		if (Route)
		{
			Car->SetActorTransform(SpawnTransform, false, nullptr, ETeleportType::TeleportPhysics);
		}
		return;
	}

	if (!Route)
	{
		if (const AActor* Start = FindPlayerStart(nullptr))
		{
			const FRotator Yaw(0.f, Start->GetActorRotation().Yaw, 0.f);
			SpawnTransform = FTransform(Yaw, Start->GetActorLocation() + Yaw.RotateVector(CarSpawnOffset));
		}
	}

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	Car = GetWorld()->SpawnActor<ANBCar>(CarClass, SpawnTransform, Params);
}

void ANBRunGameMode::FindOrSpawnFinishZone()
{
	if (Route)
	{
		FinishZone = Route->GetFinishZone();
		return;
	}

	for (TActorIterator<ANBFinishZone> It(GetWorld()); It; ++It)
	{
		FinishZone = *It;
		return;
	}

	const FVector Forward = FRotator(0.f, Car->GetActorRotation().Yaw, 0.f).Vector();
	FVector Where = Car->GetActorLocation() + Forward * FallbackFinishDistance;
	// Sit the gate on the ground there.
	FHitResult Ground;
	if (GetWorld()->LineTraceSingleByChannel(Ground, Where + FVector(0.f, 0.f, 2000.f), Where - FVector(0.f, 0.f, 2000.f), ECC_Visibility))
	{
		Where.Z = Ground.ImpactPoint.Z;
	}
	Where.Z += 10.f;
	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	FinishZone = GetWorld()->SpawnActor<ANBFinishZone>(ANBFinishZone::StaticClass(),
		Where, FRotator(0.f, Car->GetActorRotation().Yaw, 0.f), Params);
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

void ANBRunGameMode::DevWarp(const FString& Section)
{
	if (!Route || !Car || Route->GetNumSections() == 0)
	{
		return;
	}

	int32 Index = INDEX_NONE;
	if (Section.IsNumeric())
	{
		Index = FCString::Atoi(*Section) - 1;
	}
	else
	{
		for (int32 i = 0; i < Route->GetNumSections(); ++i)
		{
			if (Route->GetSectionName(i).Contains(Section))
			{
				Index = i;
				break;
			}
		}
	}
	if (Index < 0 || Index >= Route->GetNumSections())
	{
		UE_LOG(LogTemp, Warning, TEXT("NBWarp: no section '%s' (1-%d)"), *Section, Route->GetNumSections());
		return;
	}

	// Riders come along; anyone on foot is put back at the car.
	const FTransform Spot = Route->GetTransformAt(Route->GetSectionStart(Index) + 500.f, 150.f);
	Car->SetActorTransform(Spot, false, nullptr, ETeleportType::TeleportPhysics);
	Car->GetMesh()->SetPhysicsLinearVelocity(FVector::ZeroVector);
	Car->GetMesh()->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
	for (TActorIterator<ANBSquirrel> It(GetWorld()); It; ++It)
	{
		if (!It->IsSeated())
		{
			It->RespawnAtCar();
		}
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
	const float Deadline = Route ? Route->GetDeadlineSeconds() : RunSeconds;
	GetRunState()->SetPhase(ENBRunPhase::Driving, Now() + Deadline);
	WreckTimer = 0.f;
	NextMuteTime = Now() + FMath::FRandRange(MuteMinInterval, MuteMaxInterval);
	Car->SetRunLocked(false);
	Director->Begin(Car, Deadline);
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

	// Seconds at the controls count toward the MVP pick.
	for (const ENBSeatRole SeatRole : { ENBSeatRole::Wheel, ENBSeatRole::Pedals })
	{
		const UNBSeatComponent* Seat = Car->GetSeat(SeatRole);
		if (ANBPlayerState* Stats = (Seat && Seat->GetOccupant()) ? Seat->GetOccupant()->GetPlayerState<ANBPlayerState>() : nullptr)
		{
			Stats->AddDriveTime(DeltaSeconds);
		}
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
	PickAwards();
	State->Finish(Result, Delivered, TimeBonus, Penalty, Score, Now() + EndScreenSeconds);
}

void ANBRunGameMode::PickAwards()
{
	TArray<ANBPlayerState*> Players;
	for (APlayerState* Player : GameState->PlayerArray)
	{
		if (ANBPlayerState* Stats = Cast<ANBPlayerState>(Player))
		{
			Players.Add(Stats);
		}
	}
	// One squirrel can't be both, and a solo practice run has nobody to compare with.
	if (Players.Num() < 2)
	{
		GetRunState()->SetAwards(nullptr, nullptr);
		return;
	}

	// Ties: the MVP goes to more repairs, "Most useless" to more falls.
	Players.Sort([](const ANBPlayerState& A, const ANBPlayerState& B)
	{
		const float HelpA = A.GetHelpfulness();
		const float HelpB = B.GetHelpfulness();
		if (HelpA != HelpB)
		{
			return HelpA > HelpB;
		}
		if (A.GetFiresPutOut() != B.GetFiresPutOut())
		{
			return A.GetFiresPutOut() > B.GetFiresPutOut();
		}
		return A.GetFalls() < B.GetFalls();
	});
	GetRunState()->SetAwards(Players[0], Players.Last());
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
