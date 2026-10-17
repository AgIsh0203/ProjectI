// Squirrel Wheels prototype.

#include "Game/NBProp.h"

#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"

ANBProp::ANBProp()
{
	PrimaryActorTick.bCanEverTick = false;

	// Static: props never move in a game, and the editor expects level dressing to be static.
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetMobility(EComponentMobility::Static);
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Mesh->SetCanEverAffectNavigation(false);
	RootComponent = Mesh;

	Blocker = CreateDefaultSubobject<UBoxComponent>(TEXT("Blocker"));
	Blocker->SetupAttachment(Mesh);
	Blocker->SetMobility(EComponentMobility::Static);
	Blocker->SetCollisionProfileName(UCollisionProfile::BlockAll_ProfileName);
	Blocker->SetHiddenInGame(true);
	Blocker->SetCanEverAffectNavigation(false);
}

void ANBProp::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	FitBlocker();
}

void ANBProp::SetProp(UStaticMesh* NewMesh, float NewBlockFraction, bool bNewBlockAtPivot)
{
	Mesh->SetStaticMesh(NewMesh);
	BlockFraction = NewBlockFraction;
	bBlockAtPivot = bNewBlockAtPivot;
	FitBlocker();
}

void ANBProp::BlockerBox(const UStaticMesh* Mesh, float Fraction, bool bAtPivot, FVector& OutCentre, FVector& OutExtent)
{
	const FBoxSphereBounds Bounds = Mesh->GetBounds();
	if (bAtPivot)
	{
		const float Half = FMath::Min(Bounds.BoxExtent.X, Bounds.BoxExtent.Y) * Fraction;
		OutCentre = FVector(0.f, 0.f, Bounds.Origin.Z);
		OutExtent = FVector(Half, Half, Bounds.BoxExtent.Z);
	}
	else
	{
		OutCentre = Bounds.Origin;
		OutExtent = FVector(Bounds.BoxExtent.X * Fraction, Bounds.BoxExtent.Y * Fraction, Bounds.BoxExtent.Z);
	}
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
	FVector Centre, Extent;
	BlockerBox(StaticMesh, BlockFraction, bBlockAtPivot, Centre, Extent);
	Blocker->SetRelativeLocation(Centre);
	Blocker->SetBoxExtent(Extent, false);
	Blocker->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
}
