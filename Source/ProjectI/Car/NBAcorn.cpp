// Squirrel Wheels prototype.

#include "Car/NBAcorn.h"

#include "Components/StaticMeshComponent.h"
#include "Engine/CollisionProfile.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"

ANBAcorn::ANBAcorn()
{
	bReplicates = true;
	SetReplicatingMovement(true);
	SetNetUpdateFrequency(15.f);
	InitialLifeSpan = 20.f;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	SetRootComponent(Mesh);
	Mesh->SetRelativeScale3D(FVector(0.14f));
	Mesh->SetCollisionProfileName(UCollisionProfile::PhysicsActor_ProfileName);
	// Acorns shouldn't shove squirrels around or jam the car's wheels.
	Mesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	Mesh->SetCollisionResponseToChannel(ECC_Vehicle, ECR_Ignore);
	Mesh->SetCollisionResponseToChannel(ECC_PhysicsBody, ECR_Ignore);
	Mesh->SetSimulatePhysics(true);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> ShapeMaterial(TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
	if (SphereMesh.Succeeded())
	{
		Mesh->SetStaticMesh(SphereMesh.Object);
	}
	if (ShapeMaterial.Succeeded())
	{
		Mesh->SetMaterial(0, ShapeMaterial.Object);
	}

	// The model is 1 m across like the sphere, so at the root's 0.14 scale it matches the ball.
	static ConstructorHelpers::FObjectFinder<UStaticMesh> AcornMesh(TEXT("/Game/ThirdParty/PolyPizza/acorn_n/StaticMeshes/SM_Acorn.SM_Acorn"));
	if (AcornMesh.Succeeded())
	{
		Visual = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Visual"));
		Visual->SetupAttachment(Mesh);
		Visual->SetStaticMesh(AcornMesh.Object);
		Visual->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		// Hide only the sphere, not its child.
		Mesh->SetVisibility(false, /*bPropagateToChildren*/ false);
	}
}

ANBAcorn* ANBAcorn::SpawnSpilled(UWorld* World, const FVector& Location, const FVector& Velocity)
{
	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	ANBAcorn* Acorn = World->SpawnActor<ANBAcorn>(StaticClass(), Location, FRotator::ZeroRotator, Params);
	if (Acorn)
	{
		Acorn->Mesh->SetPhysicsLinearVelocity(Velocity);
	}
	return Acorn;
}
