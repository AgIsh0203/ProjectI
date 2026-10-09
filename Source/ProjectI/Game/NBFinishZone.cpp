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

	for (const TCHAR* Name : {TEXT("PostL"), TEXT("PostR")})
	{
		UStaticMeshComponent* Post = CreateDefaultSubobject<UStaticMeshComponent>(Name);
		Post->SetupAttachment(Box);
		Post->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Post->SetCastShadow(false);
		if (CubeMesh.Succeeded())
		{
			Post->SetStaticMesh(CubeMesh.Object);
		}
		if (ShapeMaterial.Succeeded())
		{
			Post->SetMaterial(0, ShapeMaterial.Object);
		}
		Posts.Add(Post);
	}
}

void ANBFinishZone::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	Box->SetBoxExtent(HalfExtent);
	// The cube is 100 cm wide; squash it into a floor stripe at the zone's own height,
	// so place the zone ~10 cm above the ground.
	Visual->SetRelativeLocation(FVector::ZeroVector);
	Visual->SetRelativeScale3D(FVector(HalfExtent.X * 2.f / 100.f, HalfExtent.Y * 2.f / 100.f, 0.1f));
	for (int32 i = 0; i < Posts.Num(); ++i)
	{
		Posts[i]->SetRelativeLocation(FVector(0.f, (i == 0 ? -1.f : 1.f) * HalfExtent.Y, 150.f));
		Posts[i]->SetRelativeScale3D(FVector(0.4f, 0.4f, 3.f));
	}
}

bool ANBFinishZone::Contains(const ANBCar& Car) const
{
	const FVector Local = Box->GetComponentTransform().InverseTransformPositionNoScale(Car.GetActorLocation());
	return FMath::Abs(Local.X) <= HalfExtent.X && FMath::Abs(Local.Y) <= HalfExtent.Y && FMath::Abs(Local.Z) <= HalfExtent.Z + 100.f;
}
