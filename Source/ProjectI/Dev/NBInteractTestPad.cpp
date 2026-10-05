// Squirrel Wheels prototype.

#include "Dev/NBInteractTestPad.h"

#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Engine/StaticMesh.h"
#include "Interaction/NBInteractableComponent.h"
#include "TimerManager.h"
#include "UObject/ConstructorHelpers.h"

ANBInteractTestPad::ANBInteractTestPad()
{
	bReplicates = true;

	Block = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Block"));
	RootComponent = Block;
	Block->SetRelativeScale3D(FVector(0.8f, 0.8f, 0.4f));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (CubeMesh.Succeeded())
	{
		Block->SetStaticMesh(CubeMesh.Object);
	}

	Interactable = CreateDefaultSubobject<UNBInteractableComponent>(TEXT("Interactable"));
	Interactable->SetupAttachment(Block);
	Interactable->SetRelativeLocation(FVector(0.f, 0.f, 60.f));

	Label = CreateDefaultSubobject<UTextRenderComponent>(TEXT("Label"));
	Label->SetupAttachment(Block);
	Label->SetAbsolute(false, false, true);
	Label->SetRelativeLocation(FVector(0.f, 0.f, 260.f));
	Label->SetHorizontalAlignment(EHTA_Center);
	Label->SetWorldSize(40.f);
	Label->SetTextRenderColor(FColor::Yellow);
}

void ANBInteractTestPad::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	const FString ModeName = StaticEnum<ENBInteractMode>()->GetNameStringByValue(static_cast<int64>(Interactable->Mode));
	Label->SetText(FText::FromString(ModeName));
	Interactable->Prompt = FText::FromString(FString::Printf(TEXT("Test %s"), *ModeName));
}

void ANBInteractTestPad::BeginPlay()
{
	Super::BeginPlay();
	if (HasAuthority())
	{
		Interactable->OnCompleted.AddDynamic(this, &ANBInteractTestPad::HandleCompleted);
	}
}

void ANBInteractTestPad::HandleCompleted(UNBInteractableComponent* Completed)
{
	++Completions;
	UE_LOG(LogTemp, Log, TEXT("NBTestPad %s completed (%d)"), *GetName(), Completions);
	GetWorldTimerManager().SetTimer(RearmTimer, this, &ANBInteractTestPad::Rearm, RearmSeconds);
}

void ANBInteractTestPad::Rearm()
{
	Interactable->SetInteractEnabled(true);
}
