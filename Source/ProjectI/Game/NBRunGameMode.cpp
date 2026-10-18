// Squirrel Wheels prototype.

#include "Game/NBRunGameMode.h"

#include "Car/NBCar.h"
#include "HAL/IConsoleManager.h"
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

namespace
{
	TAutoConsoleVariable<int32> CVarSoloDrive(TEXT("nb.SoloDrive"), 0,
		TEXT("Dev, PIE only: 1 = one squirrel drives alone (WASD steer + gas/brake from any seat, Shift handbrake, mouse wheel shifts)."));
}

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

	// The squirrel's sound/VFX slots are filled in its Blueprint child (always cooked, see DefaultGame.ini).
	static ConstructorHelpers::FClassFinder<ANBSquirrel> SquirrelBP(TEXT("/Game/Blueprints/BP_Squirrel"));
	if (SquirrelBP.Succeeded())
	{
		DefaultPawnClass = SquirrelBP.Class;
	}
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
	if (UGameplayStatics::HasOption(Options, TEXT("Autopilot")))
	{
		Auto.Speed = UGameplayStatics::GetIntOption(Options, TEXT("Autopilot"), 1600);
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
	// ?AutoStart in the URL: start at MinPlayers without the host's button (unattended 2-instance tests).
	bAutoStart = !bHostStartsRun || bRestarted || GetWorld()->IsPlayInEditor() || UGameplayStatics::HasOption(OptionsString, TEXT("AutoStart"));
	// ?Autopilot=<cm/s> drives alone, like NBAuto: start without waiting for a crew (one -game window can time the route).
	bForceStart = Auto.Speed > 0.f;

	EnsureRoute();
	FindOrSpawnCar();
	// The host's last choice (kept by the game instance across restarts), or ?ManualGears=0/1 in the URL.
	bool bManual = Sessions && Sessions->IsManualGears();
	if (UGameplayStatics::HasOption(OptionsString, TEXT("ManualGears")))
	{
		bManual = UGameplayStatics::GetIntOption(OptionsString, TEXT("ManualGears"), 1) != 0;
	}
	if (Car)
	{
		Car->SetRunLocked(true);
		FindOrSpawnFinishZone();
	}
	if (ANBRunGameState* State = GetRunState())
	{
		State->SetPhase(ENBRunPhase::Waiting, 0.0);
	}
	SetManualGears(bManual);
}

bool ANBRunGameMode::CanChangeManualGears() const
{
	// Solo drive (dev) can flip it any time.
	const ANBRunGameState* State = GetRunState();
	return State && (State->GetPhase() == ENBRunPhase::Waiting || IsSoloDrive(GetWorld()));
}

void ANBRunGameMode::SetManualGears(bool bManual)
{
	if (!CanChangeManualGears())
	{
		return;
	}
	GetRunState()->SetManualGears(bManual, ManualGearsScoreScale);
	if (UNBSessionSubsystem* Sessions = GetGameInstance()->GetSubsystem<UNBSessionSubsystem>())
	{
		Sessions->SetManualGears(bManual);
	}
	if (Car)
	{
		Car->SetManualGears(bManual);
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

void ANBRunGameMode::DevSoundTour()
{
	// Random breakdowns would play over the tour and muddle which sound is which.
	Director->End();
	if (Car)
	{
		Car->DevSoundTour();
	}
}

void ANBRunGameMode::DevEndRun(ENBRunResult Result)
{
	const ANBRunGameState* State = GetRunState();
	if (State && State->GetPhase() != ENBRunPhase::Finished)
	{
		EndRun(Result);
	}
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

	// PIE refuses seamless travel, so there this does nothing; stop and replay.
	if (GetWorld()->IsPlayInEditor())
	{
		UE_LOG(LogTemp, Warning, TEXT("NBRun: restart travel refused (seamless travel doesn't run in PIE)"));
		return;
	}
	Director->End();

	// Get rid of the car before travelling. A client that only hears the car is gone after it has
	// moved to the new map destroys it there, and the Chaos vehicle's network physics component then
	// reads the old map's freed physics solver: a crash on every client at every restart (2026-10-17
	// friend test). A second's grace lets every client remove it while its old world is still alive.
	for (TActorIterator<ANBSquirrel> It(GetWorld()); It; ++It)
	{
		if (It->IsSeated())
		{
			It->LeaveSeat();
		}
	}
	if (Car)
	{
		Car->Destroy();
		Car = nullptr;
	}
	GetWorldTimerManager().SetTimer(RestartTravelTimer, this, &ANBRunGameMode::TravelToRestart, 1.f);
}

void ANBRunGameMode::TravelToRestart()
{
	// Fresh map, fresh car.
	if (!GetWorld()->ServerTravel(TEXT("?Restart")))
	{
		UE_LOG(LogTemp, Warning, TEXT("NBRun: restart travel refused"));
		return;
	}
	if (UNBSessionSubsystem* Sessions = GetGameInstance()->GetSubsystem<UNBSessionSubsystem>())
	{
		Sessions->NoteRunRestart();
	}
}

void ANBRunGameMode::DevAutopilot(float Speed)
{
	Auto = FNBAutopilot();
	Auto.Speed = FMath::Max(Speed, 0.f);
	if (Auto.Speed <= 0.f)
	{
		if (Car)
		{
			Car->SetSeatInput(ENBSeatRole::Wheel, 0.f);
			Car->SetSeatInput(ENBSeatRole::Pedals, 0.f);
		}
		UE_LOG(LogTemp, Log, TEXT("NBAuto: off"));
		return;
	}
	Director->End();
	const ANBRunGameState* State = GetRunState();
	if (State && State->GetPhase() == ENBRunPhase::Waiting)
	{
		bForceStart = true;
	}
	UE_LOG(LogTemp, Log, TEXT("NBAuto: on at %.0f cm/s"), Auto.Speed);
}

bool ANBRunGameMode::IsSoloDrive(const UWorld* World)
{
	return World && World->IsPlayInEditor() && CVarSoloDrive.GetValueOnGameThread() != 0;
}

void ANBRunGameMode::DevSolo(ANBSquirrel* Driver)
{
	if (!GetWorld()->IsPlayInEditor())
	{
		UE_LOG(LogTemp, Warning, TEXT("NBSolo: PIE only"));
		return;
	}
	CVarSoloDrive->Set(1);
	Director->End();
	if (Car && Driver && !Driver->IsSeated())
	{
		Driver->EnterSeat(Car->GetSeat(ENBSeatRole::Wheel));
	}
	const ANBRunGameState* State = GetRunState();
	if (State && State->GetPhase() == ENBRunPhase::Waiting)
	{
		bForceStart = true;
	}
	UE_LOG(LogTemp, Log, TEXT("NBSolo: on (nb.SoloDrive 0 turns it off)"));
}

void ANBRunGameMode::SeatSoloDriver()
{
	if (!Car)
	{
		return;
	}
	for (TActorIterator<ANBSquirrel> It(GetWorld()); It; ++It)
	{
		const AController* Controller = It->GetController();
		if (Controller && Controller->IsLocalController())
		{
			if (!It->IsSeated())
			{
				It->EnterSeat(Car->GetSeat(ENBSeatRole::Wheel));
			}
			return;
		}
	}
}

void ANBRunGameMode::LogAutopilotSection(const TCHAR* Why)
{
	if (!Route || Auto.Section == INDEX_NONE)
	{
		return;
	}
	UE_LOG(LogTemp, Log, TEXT("NBAuto: %-16s %5.1f s  speed %4.0f-%4.0f cm/s  slowest climbing %4.0f  max tilt %4.1f deg  acorns lost %d  air +%d  stuck %d  (%s)"),
		*Route->GetSectionName(Auto.Section), Now() - Auto.SectionStart, Auto.MinSpeed, Auto.MaxSpeed,
		Auto.ClimbMinSpeed < TNumericLimits<float>::Max() ? Auto.ClimbMinSpeed : -1.f, Auto.MaxTilt,
		Auto.Acorns - Car->GetAcorns(), Car->GetAirPoints() - Auto.AirPoints, Auto.Stucks, Why);
}

void ANBRunGameMode::TickAutopilot(float DeltaSeconds)
{
	if (!Route || !Car)
	{
		return;
	}
	const FVector Location = Car->GetActorLocation();
	const float Along = Route->GetDistanceAlong(Location);
	const float Speed = Car->GetForwardSpeed();

	const int32 Section = Route->GetSectionAt(Along);
	if (Section != Auto.Section)
	{
		LogAutopilotSection(TEXT("next"));
		if (Auto.Section == INDEX_NONE)
		{
			Auto.RunStart = Now();
		}
		const int32 TotalStucks = Auto.TotalStucks + Auto.Stucks;
		const float AutoSpeed = Auto.Speed;
		const double RunStart = Auto.RunStart;
		Auto = FNBAutopilot();
		Auto.Speed = AutoSpeed;
		Auto.RunStart = RunStart;
		Auto.TotalStucks = TotalStucks;
		Auto.Section = Section;
		Auto.SectionStart = Now();
		Auto.MinSpeed = Auto.ClimbMinSpeed = TNumericLimits<float>::Max();
		Auto.Acorns = Car->GetAcorns();
		Auto.AirPoints = Car->GetAirPoints();
	}
	Auto.LastAlong = Along;
	if (Now() - Auto.SectionStart > 1.0)
	{
		Auto.MinSpeed = FMath::Min(Auto.MinSpeed, Speed);
	}
	Auto.MaxSpeed = FMath::Max(Auto.MaxSpeed, Speed);
	if (Route->GetRoadGrade(Along) > 0.04f)
	{
		Auto.ClimbMinSpeed = FMath::Min(Auto.ClimbMinSpeed, Speed);
	}
	Auto.MaxTilt = FMath::Max(Auto.MaxTilt, FMath::RadiansToDegrees(FMath::Acos(FMath::Clamp(Car->GetActorUpVector().Z, -1.f, 1.f))));

	if (Car->IsFlipped())
	{
		// Put it back on the road a little further on, like a crew pushing it upright.
		UE_LOG(LogTemp, Warning, TEXT("NBAuto: FLIPPED at %.0f m (%s)"), Along / 100.f, *Route->GetSectionName(Section));
		++Auto.Stucks;
		Car->SetActorTransform(Route->GetTransformAt(Along + 1000.f, 150.f), false, nullptr, ETeleportType::TeleportPhysics);
		Car->GetMesh()->SetPhysicsLinearVelocity(FVector::ZeroVector);
		Car->GetMesh()->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
		return;
	}

	// Aim at the centreline further ahead the faster we go; slow down for the bends coming up.
	const float LookAhead = FMath::Clamp(FMath::Abs(Speed) * 1.2f, 1200.f, 3000.f);
	const FVector Target = Route->GetTransformAt(Along + LookAhead).GetLocation();
	const float AimYaw = (Target - Location).Rotation().Yaw;
	const float Error = FMath::FindDeltaAngleDegrees(Car->GetActorRotation().Yaw, AimYaw);
	const float Bend = FMath::Abs(FMath::FindDeltaAngleDegrees(Route->GetTransformAt(Along).Rotator().Yaw, Route->GetTransformAt(Along + 5000.f).Rotator().Yaw));
	const float Desired = Auto.Speed * (Bend > 90.f ? 0.4f : Bend > 45.f ? 0.6f : Bend > 25.f ? 0.8f : 1.f);

	float Steer = FMath::Clamp(Error / 25.f, -1.f, 1.f);
	float Throttle = FMath::Clamp((Desired - Speed) / 400.f, -1.f, 1.f);

	// Stuck: under 1.5 m/s for 4 s. Back out with the wheel the other way for 1.5 s, then carry on.
	if (Auto.ReverseSeconds > 0.f)
	{
		Auto.ReverseSeconds -= DeltaSeconds;
		Steer = -Steer;
		Throttle = -1.f;
	}
	else if (FMath::Abs(Speed) < 150.f)
	{
		Auto.SlowSeconds += DeltaSeconds;
		if (Auto.SlowSeconds > 4.f)
		{
			UE_LOG(LogTemp, Warning, TEXT("NBAuto: STUCK at %.0f m (%s), z %.0f, grade %.1f %%"),
				Along / 100.f, *Route->GetSectionName(Section), Location.Z, Route->GetRoadGrade(Along) * 100.f);
			++Auto.Stucks;
			Auto.SlowSeconds = 0.f;
			Auto.ReverseSeconds = 1.5f;
		}
	}
	else
	{
		Auto.SlowSeconds = 0.f;
	}
	Car->SetSeatInput(ENBSeatRole::Wheel, Steer);
	Car->SetSeatInput(ENBSeatRole::Pedals, Throttle);
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
		if (Auto.Speed > 0.f && GetRunState()->GetPhase() == ENBRunPhase::Driving)
		{
			TickAutopilot(DeltaSeconds);
		}
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
	const bool bSolo = IsSoloDrive(GetWorld()) && GameState->PlayerArray.Num() >= 1;
	if (bForceStart || bSolo || (bAutoStart && GameState->PlayerArray.Num() >= MinPlayers))
	{
		BeginCountdown();
	}
}

void ANBRunGameMode::BeginCountdown()
{
	GetRunState()->SetPhase(ENBRunPhase::Countdown, Now() + CountdownSeconds);
	if (IsSoloDrive(GetWorld()))
	{
		SeatSoloDriver();
	}
}

void ANBRunGameMode::BeginDriving()
{
	const float Deadline = Route ? Route->GetDeadlineSeconds() : RunSeconds;
	GetRunState()->SetPhase(ENBRunPhase::Driving, Now() + Deadline);
	WreckTimer = 0.f;
	NextMuteTime = Now() + FMath::FRandRange(MuteMinInterval, MuteMaxInterval);
	Car->SetRunLocked(false);
	// The autopilot and solo drive have no breakdowns: they're for the road, not the director.
	const bool bSolo = IsSoloDrive(GetWorld());
	if (bSolo)
	{
		SeatSoloDriver();
	}
	if (Auto.Speed <= 0.f && !bSolo)
	{
		Director->Begin(Car, Deadline);
	}
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
	for (const ENBSeatRole SeatRole : { ENBSeatRole::Wheel, ENBSeatRole::Pedals, ENBSeatRole::Gears })
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

	if (Auto.Speed > 0.f && Auto.Section != INDEX_NONE)
	{
		LogAutopilotSection(TEXT("end"));
		UE_LOG(LogTemp, Log, TEXT("NBAuto: run over: %s after %.1f s at %.0f m of %.0f, %d acorns left, %d stuck/flipped"),
			*UEnum::GetValueAsString(Result), Now() - Auto.RunStart, Auto.LastAlong / 100.f, Route ? Route->GetLength() / 100.f : 0.f,
			Car ? Car->GetAcorns() : 0, Auto.TotalStucks + Auto.Stucks);
		Car->SetSeatInput(ENBSeatRole::Wheel, 0.f);
		Car->SetSeatInput(ENBSeatRole::Pedals, 0.f);
		Auto.Section = INDEX_NONE;
	}

	int32 Delivered = 0;
	int32 TimeBonus = 0;
	int32 AirBonus = 0;
	int32 Penalty = 0;
	int32 Score = 0;
	if (Result == ENBRunResult::Delivered)
	{
		Delivered = Car->GetAcorns();
		TimeBonus = FMath::FloorToInt(SecondsLeft) * PointsPerSecondLeft;
		AirBonus = Car->GetAirPoints();
		for (const APlayerState* Player : GameState->PlayerArray)
		{
			if (const ANBPlayerState* Stats = Cast<ANBPlayerState>(Player))
			{
				Penalty += Stats->GetRespawns() * RespawnPenalty;
			}
		}
		Score = FMath::Max(0, Delivered * PointsPerAcorn + TimeBonus + AirBonus - Penalty);
		if (State->IsManualGears())
		{
			Score = FMath::RoundToInt(Score * State->GetManualGearsScale());
		}
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
	State->Finish(Result, Delivered, TimeBonus, AirBonus, Penalty, Score, Now() + EndScreenSeconds);
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
