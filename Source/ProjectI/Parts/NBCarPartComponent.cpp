// Squirrel Wheels prototype.

#include "Parts/NBCarPartComponent.h"

#include "Car/NBCar.h"
#include "Net/UnrealNetwork.h"
#include "Player/NBPlayerState.h"
#include "Player/NBSquirrel.h"

const FName UNBCarPartComponent::SeverityParam(TEXT("Severity"));

UNBCarPartComponent::UNBCarPartComponent()
{
	// Only usable while broken.
	bDisableOnComplete = true;
}

void UNBCarPartComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UNBCarPartComponent, bFailed);
}

void UNBCarPartComponent::BeginPlay()
{
	Super::BeginPlay();
	if (GetOwner()->HasAuthority())
	{
		SetInteractEnabled(false);
		OnCompleted.AddDynamic(this, &UNBCarPartComponent::HandleRepairCompleted);
	}
	// A client may have received bFailed before BeginPlay (joined mid-failure).
	LocalFailTime = GetWorld()->GetTimeSeconds();
	RefreshFailedLoop();
}

void UNBCarPartComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	FailedLoop.Stop();
	Super::EndPlay(EndPlayReason);
}

ANBCar* UNBCarPartComponent::GetCar() const
{
	return Cast<ANBCar>(GetOwner());
}

void UNBCarPartComponent::Fail()
{
	check(GetOwner()->HasAuthority());
	if (bFailed)
	{
		return;
	}
	bFailed = true;
	TimeSinceFailure = 0.f;
	SetInteractEnabled(true);
	ApplyFailedEffect(0.f);
	HandleFailedChanged(true);
}

void UNBCarPartComponent::Repair()
{
	check(GetOwner()->HasAuthority());
	if (!bFailed)
	{
		return;
	}
	bFailed = false;
	LastRepairTime = GetWorld()->GetTimeSeconds();
	SetInteractEnabled(false);
	ClearEffect();
	HandleFailedChanged(true);
}

void UNBCarPartComponent::HandleRepairCompleted(UNBInteractableComponent* Interactable)
{
	// Everyone who worked on it put the fire out.
	for (const TWeakObjectPtr<ANBSquirrel>& Fixer : GetCompletedBy())
	{
		if (ANBPlayerState* Stats = Fixer.IsValid() ? Fixer->GetPlayerState<ANBPlayerState>() : nullptr)
		{
			Stats->AddFirePutOut();
		}
	}
	Repair();
}

void UNBCarPartComponent::OnRep_Failed()
{
	// Initial replication arrives before BeginPlay; that's state, not a live event.
	HandleFailedChanged(HasBegunPlay());
}

void UNBCarPartComponent::HandleFailedChanged(bool bPlayOneShots)
{
	OnFailedChanged();
	if (!HasBegunPlay())
	{
		return;
	}
	LocalFailTime = GetWorld()->GetTimeSeconds();
	if (bPlayOneShots)
	{
		NBFeedback::PlayAttached(bFailed ? BreakFeedback : RepairFeedback, this);
	}
	RefreshFailedLoop();
}

void UNBCarPartComponent::RefreshFailedLoop()
{
	if (!bFailed)
	{
		FailedLoop.Stop();
		return;
	}
	if (!FailedLoop.IsPlaying())
	{
		FailedLoop = NBFeedback::StartLoop(FailedLoopFeedback, this);
		FailedLoop.SetFloat(SeverityParam, 0.f);
	}
	// The interactable only ticks on the server; clients need the tick to ramp the severity.
	if (FailedLoop.IsPlaying())
	{
		SetComponentTickEnabled(true);
	}
}

void UNBCarPartComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	if (bFailed && GetOwner()->HasAuthority())
	{
		TimeSinceFailure += DeltaTime;
		ApplyFailedEffect(TimeSinceFailure);
	}
	if (bFailed && FailedLoop.IsPlaying())
	{
		FailedLoop.SetFloat(SeverityParam, FMath::Clamp((GetWorld()->GetTimeSeconds() - LocalFailTime) / SeverityRampSeconds, 0.f, 1.f));
	}
}
