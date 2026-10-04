// Squirrel Wheels prototype.

#include "Game/NBRunGameMode.h"

#include "Car/NBCar.h"
#include "EngineUtils.h"
#include "Player/NBSquirrel.h"
#include "UObject/ConstructorHelpers.h"

ANBRunGameMode::ANBRunGameMode()
{
	DefaultPawnClass = ANBSquirrel::StaticClass();

	// The Blueprint child carries the mesh, tire sockets and torque/steering curves.
	static ConstructorHelpers::FClassFinder<ANBCar> CarBP(TEXT("/Game/VehicleTemplate/Blueprints/OffroadCar/BP_OffroadCar_Pawn"));
	CarClass = CarBP.Succeeded() ? CarBP.Class : TSubclassOf<ANBCar>(ANBCar::StaticClass());
}

void ANBRunGameMode::StartPlay()
{
	Super::StartPlay();

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
