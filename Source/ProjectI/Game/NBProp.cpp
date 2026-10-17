// Squirrel Wheels prototype.

#include "Game/NBProp.h"

#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"

ANBProp::ANBProp()
{
	PrimaryActorTick.bCanEverTick = false;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Mesh->SetCanEverAffectNavigation(false);
	RootComponent = Mesh;

	Blocker = CreateDefaultSubobject<UBoxComponent>(TEXT("Blocker"));
	Blocker->SetupAttachment(Mesh);
	Blocker->SetCollisionProfileName(UCollisionProfile::BlockAll_ProfileName);
	Blocker->SetHiddenInGame(true);
	Blocker->SetCanEverAffectNavigation(false);
}

void ANBProp::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	FitBlocker();
}

void ANBProp::SetProp(UStaticMesh* NewMesh, float NewBlockFraction)
{
	Mesh->SetStaticMesh(NewMesh);
	BlockFraction = NewBlockFraction;
	FitBlocker();
}

void ANBProp::FitBlocker()
{
	const UStaticMesh* StaticMesh = Mesh->GetStaticMesh();
	if (!StaticMesh || BlockFraction <= 0.f)
	{
		Blocker->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Blocker->SetBoxExtent(FVector(1.f), false);
		return;
	}
	// In the mesh's own space; the component's scale (the prop's size) applies on top.
	const FBoxSphereBounds Bounds = StaticMesh->GetBounds();
	Blocker->SetRelativeLocation(Bounds.Origin);
	Blocker->SetBoxExtent(FVector(Bounds.BoxExtent.X * BlockFraction, Bounds.BoxExtent.Y * BlockFraction, Bounds.BoxExtent.Z), false);
	Blocker->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
}
