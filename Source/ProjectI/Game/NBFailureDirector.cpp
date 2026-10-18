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
	QuietUntil = 0.f;
	bWasFull = false;
	bStorm = true;
	PhaseEndsAt = StormSeconds;
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
	const float Progress = FMath::Clamp(Elapsed / RunLength, 0.f, 1.f);
	const float Now = GetWorld()->GetTimeSeconds();

	if (Elapsed >= PhaseEndsAt)
	{
		bStorm = !bStorm;
		PhaseEndsAt = Elapsed + (bStorm ? StormSeconds : FMath::Lerp(CalmSecondsStart, CalmSecondsEnd, Progress));
		if (bStorm)
		{
			// The storm hits straight away rather than an interval later.
			NextFailureAt = FMath::Min(NextFailureAt, Elapsed + 2.f);
		}
		UE_LOG(LogTemp, Log, TEXT("NBDirector: %s for %.0f s"), bStorm ? TEXT("storm") : TEXT("calm"), PhaseEndsAt - Elapsed);
	}

	int32 NumBlocking = 0;
	TArray<UNBCarPartComponent*, TInlineAllocator<8>> Candidates;
	for (UNBCarPartComponent* Part : Car->GetParts())
	{
		if (!Part)
		{
			continue;
		}
		if (Part->IsFailed())
		{
			NumBlocking += Part->GetFailedSeconds() < IgnoredFailureSeconds ? 1 : 0;
		}
		else if (Now - Part->GetLastRepairTime() >= RepairCooldown)
		{
			Candidates.Add(Part);
		}
	}

	if (NumBlocking >= MaxConcurrent(Progress))
	{
		bWasFull = true;
		return;
	}
	if (bWasFull)
	{
		// A repair freed a slot: a breather before the next one, not an instant replacement.
		bWasFull = false;
		NextFailureAt = FMath::Max(NextFailureAt, Elapsed + NextInterval(Progress));
	}
	if (Car->GetVelocity().Size() < MinMovingSpeed)
	{
		// Standing still: nothing wears out by itself, and the timer pauses rather than firing on the first metre.
		NextFailureAt += DeltaTime;
		return;
	}
	if (Elapsed < FMath::Max(QuietUntil, FirstFailureDelay) || Candidates.IsEmpty())
	{
		return;
	}

	// A badly worn part can go before the timer says so.
	for (UNBCarPartComponent* Part : Candidates)
	{
		const float HazardStart = bStorm ? WearHazardStart : CalmHazardWear;
		const float Over = (Part->GetWear() - HazardStart) / (1.f - HazardStart);
		if (Over > 0.f && FMath::FRand() < WearHazardPerSecond * Over * Over * DeltaTime)
		{
			FailPart(Part, Progress, TEXT("worn out"));
			return;
		}
	}

	if (!bStorm || Elapsed < NextFailureAt)
	{
		return;
	}

	// Timed failure: random, weighted towards worn parts.
	float TotalWeight = 0.f;
	TArray<float, TInlineAllocator<8>> Weights;
	for (const UNBCarPartComponent* Part : Candidates)
	{
		Weights.Add(FreshWeight + WearBias * FMath::Square(Part->GetWear()));
		TotalWeight += Weights.Last();
	}
	float Pick = FMath::FRandRange(0.f, TotalWeight);
	int32 Index = 0;
	while (Index < Candidates.Num() - 1 && Pick > Weights[Index])
	{
		Pick -= Weights[Index++];
	}
	FailPart(Candidates[Index], Progress, TEXT("timer"));
}

void UNBFailureDirector::FailPart(UNBCarPartComponent* Part, float Progress, const TCHAR* Why)
{
	UE_LOG(LogTemp, Log, TEXT("NBDirector: %s fails at %.0f s (%s, wear %.2f)"), *Part->PartName.ToString(), Elapsed, Why, Part->GetWear());
	Part->Fail();
	NextFailureAt = Elapsed + NextInterval(Progress) * (bStorm ? StormIntervalScale : 1.f);
	QuietUntil = Elapsed + MinGap;
}
