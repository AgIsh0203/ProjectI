// Squirrel Wheels prototype.

#include "Game/NBInstancedLayer.h"

#include "Components/InstancedStaticMeshComponent.h"

ANBInstancedLayer::ANBInstancedLayer()
{
	PrimaryActorTick.bCanEverTick = false;
	Instances = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("Instances"));
	Instances->SetCanEverAffectNavigation(false);
	Instances->SetMobility(EComponentMobility::Static);
	RootComponent = Instances;
}
