// Squirrel Wheels prototype.

#include "Game/NBWreckingBall.h"

#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/CollisionProfile.h"
#include "EngineUtils.h"
#include "GameFramework/GameStateBase.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "Player/NBSquirrel.h"
#include "UObject/ConstructorHelpers.h"

ANBWreckingBall::ANBWreckingBall()
{
	PrimaryActorTick.bCanEverTick = true;
	// Moved before physics, so the car meets the ball where it is this frame.
	PrimaryActorTick.TickGroup = TG_PrePhysics;
	bReplicates = false;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderMesh(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> ShapeMaterial(TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));

	UMaterialInterface* Material = ShapeMaterial.Object;
	auto Make = [this, Material](const TCHAR* Name, UStaticMesh* Mesh, bool bCollide)
	{
		UStaticMeshComponent* Part = CreateDefaultSubobject<UStaticMeshComponent>(Name);
		Part->SetupAttachment(Root);
		Part->SetStaticMesh(Mesh);
		Part->SetMaterial(0, Material);
		Part->SetCollisionProfileName(bCollide ? UCollisionProfile::BlockAll_ProfileName : UCollisionProfile::NoCollision_ProfileName);
		Part->SetGenerateOverlapEvents(false);
		return Part;
	};
	PostL = Make(TEXT("PostL"), CubeMesh.Object, true);
	PostR = Make(TEXT("PostR"), CubeMesh.Object, true);
	Beam = Make(TEXT("Beam"), CubeMesh.Object, false);
	Chain = Make(TEXT("Chain"), CylinderMesh.Object, false);
	Ball = Make(TEXT("Ball"), SphereMesh.Object, true);
	// A kinematic body: moved every tick without simulating, so it shoves the car like an immovable wall.
	Ball->SetMobility(EComponentMobility::Movable);
	Ball->SetSimulatePhysics(false);
}

void ANBWreckingBall::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	const TPair<UStaticMeshComponent*, FLinearColor> Colors[] = {
		{PostL, FLinearColor(0.9f, 0.65f, 0.05f)}, {PostR, FLinearColor(0.9f, 0.65f, 0.05f)},
		{Beam, FLinearColor(0.9f, 0.65f, 0.05f)}, {Chain, FLinearColor(0.25f, 0.25f, 0.25f)},
		{Ball, FLinearColor(0.08f, 0.08f, 0.09f)}};
	for (const TPair<UStaticMeshComponent*, FLinearColor>& Part : Colors)
	{
		if (UMaterialInstanceDynamic* Material = Part.Key->CreateDynamicMaterialInstance(0))
		{
			Material->SetVectorParameterValue(TEXT("Color"), Part.Value);
		}
	}
	PlaceParts();
}

void ANBWreckingBall::PlaceParts()
{
	// The cubes and cylinder are 100 cm, centred; the sphere 100 cm across.
	const float PostHeight = PivotHeight + PostDepth + 50.f;
	for (UStaticMeshComponent* Post : {PostL.Get(), PostR.Get()})
	{
		const float Side = Post == PostL ? -1.f : 1.f;
		Post->SetRelativeLocation(FVector(0.f, Side * PostSpan, PivotHeight + 50.f - PostHeight * 0.5f));
		Post->SetRelativeScale3D(FVector(0.5f, 0.5f, PostHeight / 100.f));
	}
	Beam->SetRelativeLocation(FVector(0.f, 0.f, PivotHeight + 25.f));
	Beam->SetRelativeScale3D(FVector(0.5f, (PostSpan * 2.f + 50.f) / 100.f, 0.5f));
	Ball->SetRelativeScale3D(FVector(BallRadius / 50.f));
	Chain->SetRelativeScale3D(FVector(0.08f, 0.08f, ChainLength / 100.f));

	// At rest (in the editor) it hangs straight down; Tick swings it.
	Ball->SetRelativeLocation(FVector(0.f, 0.f, PivotHeight - ChainLength));
	Chain->SetRelativeLocation(FVector(0.f, 0.f, PivotHeight - ChainLength * 0.5f));
}

void ANBWreckingBall::GetSwing(double Time, float& OutAngle, float& OutRate) const
{
	const double Phase = 2.0 * PI * (Time / FMath::Max(Period, 0.5f) + PhaseOffset);
	OutAngle = Amplitude * FMath::Sin(Phase);
	OutRate = Amplitude * 2.f * PI / FMath::Max(Period, 0.5f) * FMath::Cos(Phase);
}

void ANBWreckingBall::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	const AGameStateBase* GameState = GetWorld()->GetGameState();
	const double Now = GameState ? GameState->GetServerWorldTimeSeconds() : GetWorld()->GetTimeSeconds();
	float Angle, Rate;
	GetSwing(Now, Angle, Rate);
	const float Radians = FMath::DegreesToRadians(Angle);

	// The pendulum swings in the actor's YZ plane.
	const FVector Pivot(0.f, 0.f, PivotHeight);
	const FVector Arm(0.f, FMath::Sin(Radians) * ChainLength, -FMath::Cos(Radians) * ChainLength);
	const FTransform& ActorTransform = GetActorTransform();
	Ball->SetWorldLocation(ActorTransform.TransformPosition(Pivot + Arm), false, nullptr, ETeleportType::None);
	Chain->SetRelativeLocationAndRotation(Pivot + Arm * 0.5f, FRotationMatrix::MakeFromZ(-Arm).Rotator());

	if (!HasAuthority())
	{
		return;
	}
	// Knock out any squirrel it touches, seated or on foot.
	const FVector BallCentre = Ball->GetComponentLocation();
	const FVector LocalVelocity = FVector(0.f, FMath::Cos(Radians), FMath::Sin(Radians)) * ChainLength * FMath::DegreesToRadians(Rate);
	const FVector Velocity = ActorTransform.TransformVectorNoScale(LocalVelocity);
	for (TActorIterator<ANBSquirrel> It(GetWorld()); It; ++It)
	{
		if (It->IsRagdolled() || FVector::Dist(It->GetActorLocation(), BallCentre) > BallRadius + KnockReach)
		{
			continue;
		}
		It->Eject(Velocity * KnockVelocity.X + FVector(0.f, 0.f, KnockVelocity.Y), ENBEjectReason::Ball);
		NBFeedback::PlayAt(this, KnockFeedback, BallCentre);
		UE_LOG(LogTemp, Log, TEXT("NBBall: knocks out %s at %.0f cm/s"), *It->GetName(), Velocity.Size());
	}
}
