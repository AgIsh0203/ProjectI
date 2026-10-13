// Squirrel Wheels prototype.

#pragma once

#include "CoreMinimal.h"
#include "FX/NBFeedback.h"
#include "GameFramework/Actor.h"
#include "NBPuffFX.generated.h"

class UInstancedStaticMeshComponent;
class UMaterialInterface;
class UStaticMesh;

/**
 * Simulates the toon particles (FNBPuff) of one feedback play: a burst that dies out, or a stream
 * that follows a component until it's stopped. Cosmetic and local: every machine spawns its own
 * through NBFeedback, nothing replicates. Particles live in world space as ISM instances, one
 * instanced mesh per puff and colour, so a whole effect costs a handful of draw calls.
 */
UCLASS(NotPlaceable, Transient)
class PROJECTI_API ANBPuffFX : public AActor
{
	GENERATED_BODY()

public:
	ANBPuffFX();

	/** Spawns the puffs at Origin (world), or following AttachTo (offset by RelativeLocation) if given.
	 *  A loop keeps streaming at Count per second until StopLoop. Returns null if nothing to show. */
	static ANBPuffFX* Spawn(UWorld* World, const TArray<FNBPuff>& Puffs, const FVector& Origin,
		USceneComponent* AttachTo, const FVector& RelativeLocation, float Intensity, bool bLoop);

	/** Loop strength (e.g. engine fire severity, 0..1): scales the stream's rate and particle size. */
	void SetStrength(float Value) { Strength = FMath::Clamp(Value, 0.f, 1.f); }

	/** Stops streaming; live particles finish, then the actor goes. */
	void StopLoop() { bLooping = false; }

	virtual void Tick(float DeltaSeconds) override;

private:
	struct FParticle
	{
		FVector Location = FVector::ZeroVector;
		FVector Velocity = FVector::ZeroVector;
		FRotator Rotation = FRotator::ZeroRotator;
		float Age = 0.f;
		float Lifetime = 1.f;
		float Size = 0.f;
		/** Which mesh (and so which colour) of the puff draws it. */
		int32 Mesh = 0;
		/** Instance slot in that mesh; dead particles keep their slot at zero scale for reuse. */
		int32 Instance = INDEX_NONE;
		bool bAlive = false;
	};

	struct FEmitter
	{
		FNBPuff Puff;
		/** ColorA and ColorB. */
		TObjectPtr<UInstancedStaticMeshComponent> Meshes[2] = {nullptr, nullptr};
		TArray<FParticle> Particles;
		/** Fractional particles owed by the stream. */
		float SpawnDebt = 0.f;
	};

	void Init(const TArray<FNBPuff>& Puffs, const FVector& Origin, USceneComponent* AttachTo, const FVector& RelativeLocation, float Intensity, bool bLoop);
	UInstancedStaticMeshComponent* MakeMesh(const FNBPuff& Puff, const FLinearColor& Color);
	void Emit(FEmitter& Emitter, int32 Count, float SpeedScale, float SizeScale);
	FVector GetOrigin() const;

	TArray<FEmitter> Emitters;
	TWeakObjectPtr<USceneComponent> FollowComponent;
	FVector FollowOffset = FVector::ZeroVector;
	FVector FixedOrigin = FVector::ZeroVector;
	float Intensity = 1.f;
	float Strength = 1.f;
	bool bLooping = false;

	UPROPERTY(Transient)
	TObjectPtr<UStaticMesh> SphereMesh;

	UPROPERTY(Transient)
	TObjectPtr<UStaticMesh> CubeMesh;

	/** BasicShapeMaterial (lit, "Color" parameter). */
	UPROPERTY(Transient)
	TObjectPtr<UMaterialInterface> LitMaterial;

	/** M_PuffGlow (unlit, "Color" and "Glow" parameters). Falls back to lit if missing. */
	UPROPERTY(Transient)
	TObjectPtr<UMaterialInterface> GlowMaterial;
};
