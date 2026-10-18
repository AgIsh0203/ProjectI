// Squirrel Wheels prototype.

#include "FX/NBPuffFX.h"

#include "Components/InstancedStaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

namespace
{
	/** A stream can't pile up more than this per puff, whatever its rate. */
	constexpr int32 MaxParticlesPerPuff = 200;
}

ANBPuffFX::ANBPuffFX()
{
	PrimaryActorTick.bCanEverTick = true;
	SetReplicates(false);
	SetRootComponent(CreateDefaultSubobject<USceneComponent>(TEXT("Root")));

	static ConstructorHelpers::FObjectFinder<UStaticMesh> Sphere(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> Cube(TEXT("/Engine/BasicShapes/Cube.Cube"));
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> Lit(TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
	// /Game/FX is always cooked; only this constructor references the glow material.
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> Glow(TEXT("/Game/FX/M_PuffGlow.M_PuffGlow"));
	SphereMesh = Sphere.Object;
	CubeMesh = Cube.Object;
	LitMaterial = Lit.Object;
	GlowMaterial = Glow.Object;
}

ANBPuffFX* ANBPuffFX::Spawn(UWorld* World, const TArray<FNBPuff>& Puffs, const FVector& Origin,
	USceneComponent* AttachTo, const FVector& RelativeLocation, float Intensity, bool bLoop)
{
	if (!World || World->GetNetMode() == NM_DedicatedServer || !Puffs.ContainsByPredicate([](const FNBPuff& Puff) { return Puff.IsSet(); }))
	{
		return nullptr;
	}
	const FVector Start = AttachTo ? AttachTo->GetComponentTransform().TransformPosition(RelativeLocation) : Origin;
	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	ANBPuffFX* FX = World->SpawnActor<ANBPuffFX>(Start, FRotator::ZeroRotator, Params);
	if (FX)
	{
		FX->Init(Puffs, Start, AttachTo, RelativeLocation, Intensity, bLoop);
	}
	return FX;
}

void ANBPuffFX::Init(const TArray<FNBPuff>& Puffs, const FVector& Origin, USceneComponent* AttachTo, const FVector& RelativeLocation, float InIntensity, bool bLoop)
{
	FixedOrigin = Origin;
	FollowComponent = AttachTo;
	FollowOffset = RelativeLocation;
	Intensity = FMath::Clamp(InIntensity, 0.f, 1.f);
	bLooping = bLoop;
	if (!bLoop)
	{
		// A burst is over in a second or two; this only guards against a stalled tick.
		SetLifeSpan(30.f);
	}

	for (const FNBPuff& Puff : Puffs)
	{
		if (!Puff.IsSet())
		{
			continue;
		}
		FEmitter& Emitter = Emitters.AddDefaulted_GetRef();
		Emitter.Puff = Puff;
		Emitter.Meshes[0] = MakeMesh(Puff, Puff.ColorA);
		Emitter.Meshes[1] = Puff.ColorB.Equals(Puff.ColorA) ? Emitter.Meshes[0].Get() : MakeMesh(Puff, Puff.ColorB);
		if (!bLoop)
		{
			// Harder hits throw more, faster.
			const int32 Count = FMath::Max(1, FMath::RoundToInt(Puff.Count * FMath::Lerp(0.4f, 1.f, Intensity)));
			Emit(Emitter, Count, FMath::Lerp(0.6f, 1.f, Intensity), 1.f);
		}
	}
}

UInstancedStaticMeshComponent* ANBPuffFX::MakeMesh(const FNBPuff& Puff, const FLinearColor& Color)
{
	UInstancedStaticMeshComponent* Mesh = NewObject<UInstancedStaticMeshComponent>(this);
	Mesh->SetupAttachment(GetRootComponent());
	Mesh->SetStaticMesh(Puff.Shape == ENBPuffShape::Cube ? CubeMesh : SphereMesh);
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Mesh->SetGenerateOverlapEvents(false);
	Mesh->SetCastShadow(false);
	Mesh->SetMobility(EComponentMobility::Movable);

	const bool bGlow = Puff.Glow > 0.f && GlowMaterial;
	if (UMaterialInstanceDynamic* Material = UMaterialInstanceDynamic::Create(bGlow ? GlowMaterial.Get() : LitMaterial.Get(), this))
	{
		Material->SetVectorParameterValue(TEXT("Color"), Color);
		if (bGlow)
		{
			Material->SetScalarParameterValue(TEXT("Glow"), Puff.Glow);
		}
		Mesh->SetMaterial(0, Material);
	}
	Mesh->RegisterComponent();
	return Mesh;
}

FVector ANBPuffFX::GetOrigin() const
{
	if (const USceneComponent* Follow = FollowComponent.Get())
	{
		return Follow->GetComponentTransform().TransformPosition(FollowOffset);
	}
	return FixedOrigin;
}

void ANBPuffFX::Emit(FEmitter& Emitter, int32 Count, float SpeedScale, float SizeScale)
{
	const FNBPuff& Puff = Emitter.Puff;
	const FVector Origin = GetOrigin();
	const bool bTwoColors = Emitter.Meshes[1] != Emitter.Meshes[0];
	auto Jittered = [&Puff](float Value) { return Value * (1.f + FMath::FRandRange(-Puff.Jitter, Puff.Jitter)); };

	for (int32 i = 0; i < Count; ++i)
	{
		// Reuse a dead particle's instance slot before adding a new one.
		FParticle* Particle = Emitter.Particles.FindByPredicate([](const FParticle& P) { return !P.bAlive; });
		if (!Particle)
		{
			if (Emitter.Particles.Num() >= MaxParticlesPerPuff)
			{
				return;
			}
			Particle = &Emitter.Particles.AddDefaulted_GetRef();
			Particle->Mesh = bTwoColors && FMath::RandBool() ? 1 : 0;
			Particle->Instance = Emitter.Meshes[Particle->Mesh]->AddInstance(FTransform(FQuat::Identity, Origin, FVector::ZeroVector), /*bWorldSpace*/ true);
		}
		Particle->bAlive = true;
		Particle->Age = 0.f;
		Particle->Lifetime = FMath::Max(0.05f, Jittered(Puff.Lifetime));
		Particle->Size = Jittered(Puff.Size) * SizeScale;
		Particle->Location = Origin + FMath::VRand() * FMath::FRand() * Puff.SpawnRadius;
		const FVector Direction = FMath::VRandCone(FVector::UpVector, FMath::DegreesToRadians(Puff.SpreadDegrees));
		Particle->Velocity = Direction * Jittered(Puff.Speed) * SpeedScale;
		Particle->Rotation = FRotator(FMath::FRandRange(0.f, 360.f), FMath::FRandRange(0.f, 360.f), FMath::FRandRange(0.f, 360.f));
	}
}

void ANBPuffFX::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (bLooping && !FollowComponent.IsValid())
	{
		// Whatever it was following (a car part) is gone.
		bLooping = false;
	}

	int32 Alive = 0;
	for (FEmitter& Emitter : Emitters)
	{
		const FNBPuff& Puff = Emitter.Puff;
		if (bLooping)
		{
			// A worse fire burns faster and bigger.
			Emitter.SpawnDebt += Puff.Count * FMath::Lerp(0.6f, 1.f, Strength) * Damp * DeltaSeconds;
			const int32 Count = FMath::FloorToInt(Emitter.SpawnDebt);
			Emitter.SpawnDebt -= Count;
			Emit(Emitter, Count, 1.f, FMath::Lerp(0.75f, 1.f, Strength) * Damp);
		}

		const float DragFactor = FMath::Exp(-Puff.Drag * DeltaSeconds);
		for (FParticle& Particle : Emitter.Particles)
		{
			if (!Particle.bAlive)
			{
				continue;
			}
			Particle.Age += DeltaSeconds;
			float Scale = 0.f;
			if (Particle.Age < Particle.Lifetime)
			{
				Particle.Velocity.Z -= Puff.Gravity * DeltaSeconds;
				Particle.Velocity *= DragFactor;
				Particle.Location += Particle.Velocity * DeltaSeconds;
				// Pops up to full size over the first quarter, then shrinks away to nothing.
				const float T = Particle.Age / Particle.Lifetime;
				const float Grow = T < 0.25f ? FMath::InterpEaseOut(0.f, 1.f, T / 0.25f, 2.f) : FMath::InterpEaseIn(1.f, 0.f, (T - 0.25f) / 0.75f, 2.f);
				Scale = Particle.Size * Grow / 100.f;
				++Alive;
			}
			else
			{
				Particle.bAlive = false;
			}
			Emitter.Meshes[Particle.Mesh]->UpdateInstanceTransform(Particle.Instance,
				FTransform(Particle.Rotation, Particle.Location, FVector(Scale)), /*bWorldSpace*/ true, /*bMarkRenderStateDirty*/ false, /*bTeleport*/ true);
		}
		Emitter.Meshes[0]->MarkRenderStateDirty();
		if (Emitter.Meshes[1] != Emitter.Meshes[0])
		{
			Emitter.Meshes[1]->MarkRenderStateDirty();
		}
	}

	if (!bLooping && Alive == 0)
	{
		Destroy();
	}
}
