// Squirrel Wheels prototype.

#include "Interaction/NBInteractableComponent.h"

#include "Engine/World.h"
#include "GameFramework/GameStateBase.h"
#include "Interaction/NBInteractionSubsystem.h"
#include "Net/UnrealNetwork.h"
#include "Player/NBSquirrel.h"

UNBInteractableComponent::UNBInteractableComponent()
{
	SetIsReplicatedByDefault(true);
	PrimaryComponentTick.bCanEverTick = true;
}

void UNBInteractableComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UNBInteractableComponent, bInteractEnabled);
	DOREPLIFETIME(UNBInteractableComponent, Progress);
	DOREPLIFETIME(UNBInteractableComponent, Users);
	DOREPLIFETIME(UNBInteractableComponent, SweetSpotStart);
}

void UNBInteractableComponent::BeginPlay()
{
	Super::BeginPlay();
	if (UNBInteractionSubsystem* Subsystem = GetWorld()->GetSubsystem<UNBInteractionSubsystem>())
	{
		Subsystem->Register(this);
	}
	// Progress only moves on the server; clients just read the replicated values.
	SetComponentTickEnabled(GetOwner()->HasAuthority());
}

void UNBInteractableComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UNBInteractionSubsystem* Subsystem = GetWorld()->GetSubsystem<UNBInteractionSubsystem>())
	{
		Subsystem->Unregister(this);
	}
	Super::EndPlay(EndPlayReason);
}

UNBInteractableComponent* UNBInteractableComponent::FindBestFor(const ANBSquirrel* Squirrel)
{
	const UNBInteractionSubsystem* Subsystem = Squirrel ? Squirrel->GetWorld()->GetSubsystem<UNBInteractionSubsystem>() : nullptr;
	if (!Subsystem)
	{
		return nullptr;
	}

	UNBInteractableComponent* Best = nullptr;
	float BestDistSq = TNumericLimits<float>::Max();
	for (UNBInteractableComponent* Interactable : Subsystem->GetInteractables())
	{
		if (!Interactable || !Interactable->CanBeUsedBy(Squirrel))
		{
			continue;
		}
		const float DistSq = FVector::DistSquared(Interactable->GetComponentLocation(), Squirrel->GetActorLocation());
		if (DistSq < BestDistSq)
		{
			BestDistSq = DistSq;
			Best = Interactable;
		}
	}
	return Best;
}

bool UNBInteractableComponent::IsInRangeOf(const ANBSquirrel* Squirrel) const
{
	return Squirrel && FVector::DistSquared(GetComponentLocation(), Squirrel->GetActorLocation()) <= FMath::Square(Range);
}

bool UNBInteractableComponent::CanBeUsedBy(const ANBSquirrel* Squirrel) const
{
	return bInteractEnabled && IsInRangeOf(Squirrel) && !(bRequiresOnFoot && Squirrel->IsSeated());
}

void UNBInteractableComponent::SetInteractEnabled(bool bEnabled)
{
	check(GetOwner()->HasAuthority());
	bInteractEnabled = bEnabled;
	if (!bEnabled)
	{
		RemoveAllUsers();
		Progress = 0.f;
		Contributors.Reset();
	}
}

void UNBInteractableComponent::RemoveUser(ANBSquirrel* Squirrel)
{
	if (Users.Remove(Squirrel) > 0 && IsValid(Squirrel))
	{
		Squirrel->HandleInteractionEnded(this);
	}
}

void UNBInteractableComponent::RemoveAllUsers()
{
	const TArray<TObjectPtr<ANBSquirrel>> OldUsers = MoveTemp(Users);
	Users.Reset();
	for (ANBSquirrel* User : OldUsers)
	{
		if (IsValid(User))
		{
			User->HandleInteractionEnded(this);
		}
	}
}

double UNBInteractableComponent::GetServerTime() const
{
	const AGameStateBase* GameState = GetWorld()->GetGameState();
	return GameState ? GameState->GetServerWorldTimeSeconds() : GetWorld()->GetTimeSeconds();
}

float UNBInteractableComponent::GetRingPhase() const
{
	return GetRingPhaseAt(GetServerTime());
}

float UNBInteractableComponent::GetRingPhaseAt(double ServerTime) const
{
	return static_cast<float>(FMath::Frac(ServerTime / RingPeriod));
}

bool UNBInteractableComponent::IsInSweetSpot(float Phase) const
{
	// Distance travelled past the start of the sweet spot, wrapped to 0..1.
	const float PastStart = FMath::Frac(Phase - SweetSpotStart + 1.f);
	return PastStart <= SweetSpotSize;
}

void UNBInteractableComponent::PressBy(ANBSquirrel* Squirrel, double PressServerTime)
{
	check(GetOwner()->HasAuthority());
	if (!CanBeUsedBy(Squirrel))
	{
		return;
	}

	switch (Mode)
	{
	case ENBInteractMode::Hold:
	case ENBInteractMode::Push2:
		Users.AddUnique(Squirrel);
		break;

	case ENBInteractMode::Mash:
		Contributors.AddUnique(Squirrel);
		AddProgress(1.f / MashPresses);
		break;

	case ENBInteractMode::TimingRing:
	{
		// Trust the client's timestamp only within a latency window, so a laggy
		// client isn't punished but nobody can claim a press from long ago.
		constexpr double MaxClockSkew = 0.3;
		const double Now = GetServerTime();
		const double JudgedTime = FMath::Clamp(PressServerTime, Now - MaxClockSkew, Now);
		if (IsInSweetSpot(GetRingPhaseAt(JudgedTime)))
		{
			PickNewSweetSpot();
			Contributors.AddUnique(Squirrel);
			AddProgress(1.f / RingHits);
		}
		else
		{
			AddProgress(-RingMissPenalty);
		}
		break;
	}
	}
}

void UNBInteractableComponent::ReleaseBy(ANBSquirrel* Squirrel)
{
	check(GetOwner()->HasAuthority());
	RemoveUser(Squirrel);
}

void UNBInteractableComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	// Subclasses may tick on clients for cosmetics; progress is server-only.
	if (!bInteractEnabled || !GetOwner()->HasAuthority())
	{
		return;
	}

	// Drop holders who left, died, sat down somewhere or wandered off.
	for (int32 i = Users.Num() - 1; i >= 0; --i)
	{
		ANBSquirrel* User = Users[i];
		if (!IsValid(User))
		{
			Users.RemoveAt(i);
		}
		else if (!CanBeUsedBy(User))
		{
			RemoveUser(User);
		}
	}

	switch (Mode)
	{
	case ENBInteractMode::Hold:
	case ENBInteractMode::Push2:
	{
		const int32 Needed = Mode == ENBInteractMode::Push2 ? RequiredUsers : 1;
		if (Users.Num() >= Needed)
		{
			for (ANBSquirrel* User : Users)
			{
				Contributors.AddUnique(User);
			}
			AddProgress(DeltaTime / HoldSeconds);
		}
		else if (Progress > 0.f)
		{
			AddProgress(-HoldDrainPerSecond * DeltaTime);
		}
		break;
	}

	case ENBInteractMode::Mash:
		if (Progress > 0.f)
		{
			AddProgress(-MashDrainPerSecond * DeltaTime);
		}
		break;

	case ENBInteractMode::TimingRing:
		break;
	}
}

void UNBInteractableComponent::AddProgress(float Delta)
{
	Progress = FMath::Clamp(Progress + Delta, 0.f, 1.f);
	if (Progress >= 1.f)
	{
		Complete();
	}
	else if (Progress <= 0.f)
	{
		// Gave up: whoever started this attempt doesn't get credit for the next one.
		Contributors.Reset();
	}
}

void UNBInteractableComponent::Complete()
{
	Progress = 0.f;
	CompletedBy = MoveTemp(Contributors);
	Contributors.Reset();
	RemoveAllUsers();
	if (bDisableOnComplete)
	{
		bInteractEnabled = false;
	}
	OnCompleted.Broadcast(this);
}

void UNBInteractableComponent::PickNewSweetSpot()
{
	// Jump the sweet spot so a hit isn't followed by the same rhythm forever.
	SweetSpotStart = FMath::Frac(SweetSpotStart + FMath::FRandRange(0.3f, 0.7f));
}
