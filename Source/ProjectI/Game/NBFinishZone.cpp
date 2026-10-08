// Squirrel Wheels prototype.

#include "Game/NBFinishZone.h"

#include "Car/NBCar.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"

ANBFinishZone::ANBFinishZone()
{
	bReplicates = true;
	bAlwaysRelevant = true;

	Box = CreateDefaultSubobject<UBoxComponent>(TEXT("Box"));
	SetRootComponent(Box);
	Box->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Box->SetBoxExtent(HalfExtent);

	Visual = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Visual"));
	Visual->SetupAttachment(Box);
	Visual->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Visual->SetCastShadow(false);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> ShapeMaterial(TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
	if (CubeMesh.Succeeded())
	{
		Visual->SetStaticMesh(CubeMesh.Object);
	}
	if (ShapeMaterial.Succeeded())
	{
		Visual->SetMaterial(0, ShapeMaterial.Object);
	}
}

void ANBFinishZone::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	Box->SetBoxExtent(HalfExtent);
	// The cube is 100 cm wide; squash it into a floor stripe.
	Visual->SetRelativeLocation(FVector(0.f, 0.f, -HalfExtent.Z + 5.f));
	Visual->SetRelativeScale3D(FVector(HalfExtent.X * 2.f / 100.f, HalfExtent.Y * 2.f / 100.f, 0.1f));
}

bool ANBFinishZone::Contains(const ANBCar& Car) const
{
	const FVector Local = Box->GetComponentTransform().InverseTransformPositionNoScale(Car.GetActorLocation());
	return FMath::Abs(Local.X) <= HalfExtent.X && FMath::Abs(Local.Y) <= HalfExtent.Y && FMath::Abs(Local.Z) <= HalfExtent.Z;
}
