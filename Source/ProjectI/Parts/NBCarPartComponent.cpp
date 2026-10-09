// Squirrel Wheels prototype.

#include "Parts/NBCarPartComponent.h"

#include "Car/NBCar.h"
#include "Net/UnrealNetwork.h"
#include "Player/NBPlayerState.h"
#include "Player/NBSquirrel.h"

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
	OnFailedChanged();
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
	OnFailedChanged();
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
	OnFailedChanged();
}

void UNBCarPartComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	if (bFailed && GetOwner()->HasAuthority())
	{
		TimeSinceFailure += DeltaTime;
		ApplyFailedEffect(TimeSinceFailure);
	}
}
