// Squirrel Wheels prototype.

#include "Game/NBFailureDirector.h"

#include "Car/NBCar.h"
#include "GameFramework/GameStateBase.h"
#include "Parts/NBCarPartComponent.h"

UNBFailureDirector::UNBFailureDirector()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

void UNBFailureDirector::Begin(ANBCar* InCar, float RunSeconds)
{
	Car = InCar;
	RunLength = FMath::Max(RunSeconds, 1.f);
	Elapsed = 0.f;
	NextFailureAt = FirstFailureDelay;
	bActive = Car != nullptr;
	SetComponentTickEnabled(bActive);
}

void UNBFailureDirector::End()
{
	bActive = false;
	SetComponentTickEnabled(false);
}

int32 UNBFailureDirector::MaxConcurrent(float Progress) const
{
	const AGameStateBase* GameState = GetWorld()->GetGameState();
	const int32 Players = FMath::Clamp(GameState ? GameState->PlayerArray.Num() : 2, 2, 4);
	// One at first, then up to one more than the squirrels can fix at once, so a total wreck stays possible.
	return FMath::Clamp(1 + FMath::FloorToInt(Progress * (Players + 1)), 1, Players + 1);
}

float UNBFailureDirector::NextInterval(float Progress) const
{
	const float Base = FMath::Lerp(StartInterval, EndInterval, Progress);
	return Base * FMath::FRandRange(0.8f, 1.2f);
}

void UNBFailureDirector::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	if (!bActive || !Car)
	{
		return;
	}

	Elapsed += DeltaTime;
	if (Elapsed < NextFailureAt)
	{
		return;
	}

	const float Progress = FMath::Clamp(Elapsed / RunLength, 0.f, 1.f);
	const float Now = GetWorld()->GetTimeSeconds();

	int32 NumFailed = 0;
	TArray<UNBCarPartComponent*, TInlineAllocator<8>> Candidates;
	for (UNBCarPartComponent* Part : Car->GetParts())
	{
		if (!Part)
		{
			continue;
		}
		if (Part->IsFailed())
		{
			++NumFailed;
		}
		else if (Now - Part->GetLastRepairTime() >= RepairCooldown)
		{
			Candidates.Add(Part);
		}
	}

	if (NumFailed >= MaxConcurrent(Progress) || Candidates.IsEmpty())
	{
		// Try again shortly rather than waiting a whole interval.
		NextFailureAt = Elapsed + 1.5f;
		return;
	}

	Candidates[FMath::RandRange(0, Candidates.Num() - 1)]->Fail();
	NextFailureAt = Elapsed + NextInterval(Progress);
}
