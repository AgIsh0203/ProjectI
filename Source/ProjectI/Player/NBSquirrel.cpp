// Squirrel Wheels prototype.

#include "Player/NBSquirrel.h"

#include "Camera/CameraComponent.h"
#include "Car/NBCar.h"
#include "Car/NBSeatComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "Engine/StaticMesh.h"
#include "Engine/CollisionProfile.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Player/NBPlayerState.h"
#include "TimerManager.h"
#include "EngineUtils.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/GameStateBase.h"
#include "Interaction/NBInteractableComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "InputModifiers.h"
#include "Net/UnrealNetwork.h"
#include "UObject/ConstructorHelpers.h"

ANBSquirrel::ANBSquirrel()
{
	GetCapsuleComponent()->InitCapsuleSize(16.f, 22.f);

	bUseControllerRotationYaw = false;
	UCharacterMovementComponent* Movement = GetCharacterMovement();
	Movement->bOrientRotationToMovement = true;
	Movement->RotationRate = FRotator(0.f, 720.f, 0.f);
	Movement->MaxWalkSpeed = 650.f;
	Movement->JumpZVelocity = 520.f;
	Movement->AirControl = 0.5f;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderMesh(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));

	BodyVisual = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BodyVisual"));
	BodyVisual->SetupAttachment(GetCapsuleComponent());
	BodyVisual->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	BodyVisual->SetRelativeScale3D(FVector(0.34f, 0.3f, 0.44f));
	if (SphereMesh.Succeeded())
	{
		BodyVisual->SetStaticMesh(SphereMesh.Object);
	}

	TailVisual = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TailVisual"));
	TailVisual->SetupAttachment(GetCapsuleComponent());
	TailVisual->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	TailVisual->SetRelativeLocation(FVector(-20.f, 0.f, 12.f));
	TailVisual->SetRelativeRotation(FRotator(-35.f, 0.f, 0.f));
	TailVisual->SetRelativeScale3D(FVector(0.16f, 0.16f, 0.42f));
	if (CylinderMesh.Succeeded())
	{
		TailVisual->SetStaticMesh(CylinderMesh.Object);
	}

	// The basic-shape meshes default to the grey checker DefaultMaterial, which has no
	// "Color" parameter; ApplyFurColor tints this one instead.
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> FurMaterial(TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
	if (FurMaterial.Succeeded())
	{
		BodyVisual->SetMaterial(0, FurMaterial.Object);
		TailVisual->SetMaterial(0, FurMaterial.Object);
	}

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = OnFootArmLength;
	CameraBoom->bUsePawnControlRotation = true;
	CameraBoom->bEnableCameraLag = true;
	CameraBoom->CameraLagSpeed = 12.f;
	CameraBoom->SocketOffset = FVector(0.f, 0.f, 60.f);

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	FurColors = {
		FLinearColor(1.f, 0.35f, 0.02f),  // orange
		FLinearColor(0.05f, 0.55f, 1.f),  // blue
		FLinearColor(0.95f, 0.05f, 0.6f), // pink
		FLinearColor(0.3f, 0.9f, 0.05f),  // lime
	};
}

void ANBSquirrel::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ANBSquirrel, CurrentSeat);
	DOREPLIFETIME(ANBSquirrel, ColorIndex);
	DOREPLIFETIME(ANBSquirrel, ClingTarget);
	DOREPLIFETIME(ANBSquirrel, Ragdoll);
	DOREPLIFETIME(ANBSquirrel, RespawnServerTime);
}

void ANBSquirrel::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	if (ColorIndex != INDEX_NONE || FurColors.IsEmpty())
	{
		return;
	}
	// Lowest color no other squirrel is wearing.
	TSet<int32> Taken;
	for (TActorIterator<ANBSquirrel> It(GetWorld()); It; ++It)
	{
		if (*It != this)
		{
			Taken.Add(It->ColorIndex);
		}
	}
	ColorIndex = 0;
	while (Taken.Contains(ColorIndex) && ColorIndex < FurColors.Num() - 1)
	{
		++ColorIndex;
	}
	ApplyFurColor();
}

void ANBSquirrel::OnRep_ColorIndex()
{
	ApplyFurColor();
}

void ANBSquirrel::ApplyFurColor()
{
	if (!FurColors.IsValidIndex(ColorIndex))
	{
		return;
	}
	for (UStaticMeshComponent* Part : {BodyVisual.Get(), TailVisual.Get()})
	{
		if (UMaterialInstanceDynamic* Material = Part->CreateAndSetMaterialInstanceDynamic(0))
		{
			Material->SetVectorParameterValue(TEXT("Color"), FurColors[ColorIndex]);
		}
	}
}

void ANBSquirrel::BuildInputAssets()
{
	if (InputContext)
	{
		return;
	}

	MoveAction = NewObject<UInputAction>(this, TEXT("IA_Move"));
	MoveAction->ValueType = EInputActionValueType::Axis2D;
	LookAction = NewObject<UInputAction>(this, TEXT("IA_Look"));
	LookAction->ValueType = EInputActionValueType::Axis2D;
	JumpAction = NewObject<UInputAction>(this, TEXT("IA_Jump"));
	InteractAction = NewObject<UInputAction>(this, TEXT("IA_Interact"));
	ActionAction = NewObject<UInputAction>(this, TEXT("IA_Action"));

	InputContext = NewObject<UInputMappingContext>(this, TEXT("IMC_Squirrel"));

	// WASD -> 2D: W/S go to Y via swizzle, A/S are negated.
	auto MapMoveKey = [this](const FKey& Key, bool bSwizzle, bool bNegate)
	{
		FEnhancedActionKeyMapping& Mapping = InputContext->MapKey(MoveAction, Key);
		if (bSwizzle)
		{
			Mapping.Modifiers.Add(NewObject<UInputModifierSwizzleAxis>(InputContext));
		}
		if (bNegate)
		{
			Mapping.Modifiers.Add(NewObject<UInputModifierNegate>(InputContext));
		}
	};
	MapMoveKey(EKeys::W, true, false);
	MapMoveKey(EKeys::S, true, true);
	MapMoveKey(EKeys::D, false, false);
	MapMoveKey(EKeys::A, false, true);
	InputContext->MapKey(MoveAction, EKeys::Gamepad_Left2D);

	FEnhancedActionKeyMapping& MouseLook = InputContext->MapKey(LookAction, EKeys::Mouse2D);
	UInputModifierNegate* InvertY = NewObject<UInputModifierNegate>(InputContext);
	InvertY->bX = false;
	InvertY->bZ = false;
	MouseLook.Modifiers.Add(InvertY);
	InputContext->MapKey(LookAction, EKeys::Gamepad_Right2D);

	InputContext->MapKey(JumpAction, EKeys::SpaceBar);
	InputContext->MapKey(JumpAction, EKeys::Gamepad_FaceButton_Bottom);
	InputContext->MapKey(InteractAction, EKeys::E);
	InputContext->MapKey(InteractAction, EKeys::Gamepad_FaceButton_Left);
	// The repair/work button: hold, mash or time it depending on the interactable.
	InputContext->MapKey(ActionAction, EKeys::LeftMouseButton);
	InputContext->MapKey(ActionAction, EKeys::Gamepad_FaceButton_Right);
}

void ANBSquirrel::PawnClientRestart()
{
	BuildInputAssets();
	if (const APlayerController* PC = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(InputContext, 0);
		}
	}
	Super::PawnClientRestart();
}

void ANBSquirrel::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	BuildInputAssets();

	UEnhancedInputComponent* Input = CastChecked<UEnhancedInputComponent>(PlayerInputComponent);
	Input->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ANBSquirrel::Move);
	Input->BindAction(MoveAction, ETriggerEvent::Completed, this, &ANBSquirrel::MoveCompleted);
	Input->BindAction(LookAction, ETriggerEvent::Triggered, this, &ANBSquirrel::Look);
	Input->BindAction(JumpAction, ETriggerEvent::Started, this, &ANBSquirrel::JumpPressed);
	Input->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
	Input->BindAction(InteractAction, ETriggerEvent::Started, this, &ANBSquirrel::Interact);
	Input->BindAction(ActionAction, ETriggerEvent::Started, this, &ANBSquirrel::ActionPressed);
	Input->BindAction(ActionAction, ETriggerEvent::Completed, this, &ANBSquirrel::ActionReleased);
}

void ANBSquirrel::Move(const FInputActionValue& Value)
{
	const FVector2D Axis = Value.Get<FVector2D>();
	if (CurrentSeat)
	{
		SendSeatInput(CurrentSeat->Role == ENBSeatRole::Wheel ? Axis.X : Axis.Y);
		return;
	}
	if (ClingTarget)
	{
		return;
	}

	const FRotator YawRotation(0.f, GetControlRotation().Yaw, 0.f);
	AddMovementInput(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X), Axis.Y);
	AddMovementInput(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y), Axis.X);
}

void ANBSquirrel::MoveCompleted(const FInputActionValue& Value)
{
	if (CurrentSeat)
	{
		SendSeatInput(0.f);
	}
}

void ANBSquirrel::Look(const FInputActionValue& Value)
{
	const FVector2D Axis = Value.Get<FVector2D>();
	AddControllerYawInput(Axis.X);
	AddControllerPitchInput(Axis.Y);
}

void ANBSquirrel::JumpPressed()
{
	if (CurrentSeat)
	{
		Server_LeaveSeat();
		return;
	}
	if (ClingTarget)
	{
		Server_ActionReleased();
		return;
	}
	Jump();
}

void ANBSquirrel::Interact()
{
	Server_Interact();
}

void ANBSquirrel::ActionPressed()
{
	const AGameStateBase* GameState = GetWorld()->GetGameState();
	const double ServerTime = GameState ? GameState->GetServerWorldTimeSeconds() : GetWorld()->GetTimeSeconds();

	// Judge timing-ring presses locally too, so the HUD reacts instantly.
	const UNBInteractableComponent* Target = GetFocusedInteractable();
	if (Target && Target->Mode == ENBInteractMode::TimingRing)
	{
		bHasRingPress = true;
		bLastRingPressHit = Target->IsInSweetSpot(Target->GetRingPhaseAt(ServerTime));
		LastRingPressTime = GetWorld()->GetTimeSeconds();
	}

	Server_ActionPressed(ServerTime);
}

void ANBSquirrel::ActionReleased()
{
	Server_ActionReleased();
}

UNBInteractableComponent* ANBSquirrel::GetFocusedInteractable() const
{
	return UNBInteractableComponent::FindBestFor(this);
}

bool ANBSquirrel::GetLastRingPress(bool& bOutHit, double& OutWorldTime) const
{
	bOutHit = bLastRingPressHit;
	OutWorldTime = LastRingPressTime;
	return bHasRingPress;
}

void ANBSquirrel::Server_ActionPressed_Implementation(double PressServerTime)
{
	ReleaseActiveInteractable();
	if (Ragdoll.bActive)
	{
		return;
	}
	if (UNBInteractableComponent* Target = UNBInteractableComponent::FindBestFor(this))
	{
		ActiveInteractable = Target;
		Target->PressBy(this, PressServerTime);
		if (Target->bAttachUser && Target->IsUsedBy(this))
		{
			StartClinging(Target);
		}
	}
}

void ANBSquirrel::Server_ActionReleased_Implementation()
{
	ReleaseActiveInteractable();
}

void ANBSquirrel::ReleaseActiveInteractable()
{
	if (UNBInteractableComponent* Old = ActiveInteractable)
	{
		ActiveInteractable = nullptr;
		Old->ReleaseBy(this);
	}
	// ReleaseBy only reports back if we were still a user; let go regardless.
	StopClinging();
}

void ANBSquirrel::HandleInteractionEnded(UNBInteractableComponent* Interactable)
{
	check(HasAuthority());
	if (ActiveInteractable == Interactable)
	{
		ActiveInteractable = nullptr;
	}
	if (ClingTarget == Interactable)
	{
		StopClinging();
	}
}

void ANBSquirrel::StartClinging(UNBInteractableComponent* Target)
{
	check(HasAuthority());
	ClingTarget = Target;
	RefreshAttachedState();
	AttachToComponent(Target, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
}

void ANBSquirrel::StopClinging()
{
	check(HasAuthority());
	if (!ClingTarget)
	{
		return;
	}
	ClingTarget = nullptr;
	DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	// Let go upright and a little higher so we don't spawn inside the tire.
	SetActorLocationAndRotation(GetActorLocation() + FVector(0.f, 0.f, 30.f), FRotator(0.f, GetActorRotation().Yaw, 0.f), false, nullptr, ETeleportType::TeleportPhysics);
	RefreshAttachedState();
}

void ANBSquirrel::OnRep_ClingTarget()
{
	RefreshAttachedState();
}


void ANBSquirrel::NBFail(const FString& PartName)
{
	Server_DevFail(PartName);
}

void ANBSquirrel::NBChaos()
{
	Server_DevChaos();
}

void ANBSquirrel::Server_DevFail_Implementation(const FString& PartName)
{
	for (TActorIterator<ANBCar> It(GetWorld()); It; ++It)
	{
		It->DevFail(PartName);
	}
}

void ANBSquirrel::Server_DevChaos_Implementation()
{
	for (TActorIterator<ANBCar> It(GetWorld()); It; ++It)
	{
		It->SetDevChaos(!It->IsDevChaos());
	}
}

void ANBSquirrel::SendSeatInput(float Value)
{
	// Only send changes; the server keeps the last value until told otherwise.
	if (FMath::IsNearlyEqual(Value, LastSentSeatInput, 0.01f))
	{
		return;
	}
	LastSentSeatInput = Value;
	Server_SetSeatInput(Value);
}

void ANBSquirrel::Server_Interact_Implementation()
{
	if (CurrentSeat)
	{
		HopToNextSeat();
		return;
	}
	if (ClingTarget || Ragdoll.bActive)
	{
		return;
	}

	UNBSeatComponent* BestSeat = nullptr;
	float BestDistSq = TNumericLimits<float>::Max();
	for (TActorIterator<ANBCar> It(GetWorld()); It; ++It)
	{
		if (UNBSeatComponent* Seat = It->FindNearestFreeSeat(GetActorLocation(), InteractRange))
		{
			const float DistSq = FVector::DistSquared(Seat->GetComponentLocation(), GetActorLocation());
			if (DistSq < BestDistSq)
			{
				BestDistSq = DistSq;
				BestSeat = Seat;
			}
		}
	}

	if (BestSeat)
	{
		EnterSeat(BestSeat);
	}
}

void ANBSquirrel::Server_LeaveSeat_Implementation()
{
	// Bailing out of a fast car throws you, it doesn't let you step out.
	const ANBCar* Car = GetSeatCar();
	if (Car && FMath::Abs(Car->GetForwardSpeed()) >= BailSpeed)
	{
		EjectFromCar();
		return;
	}
	LeaveSeat();
}

void ANBSquirrel::Server_SetSeatInput_Implementation(float Value)
{
	if (ANBCar* Car = GetSeatCar())
	{
		Car->SetSeatInput(CurrentSeat->Role, Value);
	}
}

void ANBSquirrel::EnterSeat(UNBSeatComponent* Seat)
{
	check(HasAuthority());
	if (!Seat || !Seat->IsFree())
	{
		return;
	}

	// Made it back in: no respawn penalty.
	ClearRespawn();
	Seat->SetOccupant(this);
	CurrentSeat = Seat;
	RefreshAttachedState();
	AttachToComponent(Seat, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
}

void ANBSquirrel::EjectFromCar()
{
	check(HasAuthority());
	const ANBCar* Car = GetSeatCar();
	if (!Car)
	{
		Eject(GetVelocity() + FVector(0.f, 0.f, EjectKick.Y));
		return;
	}
	// Keep the car's momentum, plus a kick out of the seat's side of the car and up.
	const FVector Right = Car->GetActorRightVector();
	const float Side = FVector::DotProduct(CurrentSeat->GetComponentLocation() - Car->GetActorLocation(), Right) >= 0.f ? 1.f : -1.f;
	Eject(Car->GetVelocity() + Right * Side * EjectKick.X + FVector(0.f, 0.f, EjectKick.Y));
}

void ANBSquirrel::HopToNextSeat()
{
	check(HasAuthority());
	ANBCar* Car = GetSeatCar();
	UNBSeatComponent* NewSeat = Car ? Car->FindNextFreeSeat(CurrentSeat) : nullptr;
	if (!NewSeat)
	{
		return;
	}

	Car->SetSeatInput(CurrentSeat->Role, 0.f);
	CurrentSeat->SetOccupant(nullptr);
	NewSeat->SetOccupant(this);
	CurrentSeat = NewSeat;
	AttachToComponent(NewSeat, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
}

void ANBSquirrel::LeaveSeat()
{
	check(HasAuthority());
	if (!CurrentSeat)
	{
		return;
	}

	UNBSeatComponent* OldSeat = CurrentSeat;
	if (ANBCar* Car = GetSeatCar())
	{
		Car->SetSeatInput(OldSeat->Role, 0.f);
		// Hop out on the side of the car the seat is on.
		const FVector Right = Car->GetActorRightVector();
		const float Side = FVector::DotProduct(OldSeat->GetComponentLocation() - Car->GetActorLocation(), Right) >= 0.f ? 1.f : -1.f;
		const FVector ExitLocation = OldSeat->GetComponentLocation() + Right * Side * ExitSideOffset + FVector(0.f, 0.f, 60.f);
		DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
		SetActorLocationAndRotation(ExitLocation, FRotator(0.f, Car->GetActorRotation().Yaw, 0.f), false, nullptr, ETeleportType::TeleportPhysics);
	}
	else
	{
		DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	}

	OldSeat->SetOccupant(nullptr);
	CurrentSeat = nullptr;
	RefreshAttachedState();
}

void ANBSquirrel::OnRep_CurrentSeat()
{
	RefreshAttachedState();
	LastSentSeatInput = 0.f;
}

void ANBSquirrel::RefreshAttachedState()
{
	// Three states: riding the car (seat or tire), tumbling, or free on foot.
	const bool bRiding = CurrentSeat != nullptr || ClingTarget != nullptr;
	const bool bFree = !bRiding && !bLocalRagdoll;

	SetActorEnableCollision(bFree);
	if (bFree)
	{
		GetCharacterMovement()->SetMovementMode(MOVE_Walking);
	}
	else
	{
		GetCharacterMovement()->DisableMovement();
	}
	// Riding, the boom starts inside the car; probing would snap the camera onto the squirrel.
	CameraBoom->bDoCollisionTest = !bRiding;
	CameraBoom->TargetArmLength = bRiding ? SeatedArmLength : OnFootArmLength;
	CameraBoom->SocketOffset = FVector(0.f, 0.f, bRiding ? 420.f : 60.f);
}

void ANBSquirrel::BeginPlay()
{
	Super::BeginPlay();
	DefaultBodyTransform = BodyVisual->GetRelativeTransform();
	DefaultTailTransform = TailVisual->GetRelativeTransform();
}

void ANBSquirrel::Eject(FVector LaunchVelocity)
{
	check(HasAuthority());
	if (Ragdoll.bActive)
	{
		return;
	}

	ReleaseActiveInteractable();
	// LeaveSeat drops us just outside the car on the seat's side, clear of its hull.
	LeaveSeat();

	Ragdoll.bActive = true;
	Ragdoll.Start = GetActorLocation();
	Ragdoll.Velocity = LaunchVelocity;
	StartLocalRagdoll();

	GetWorldTimerManager().SetTimer(RecoverTimer, this, &ANBSquirrel::Recover, RagdollSeconds);
	GetWorldTimerManager().SetTimer(RespawnTimer, this, &ANBSquirrel::RespawnAtCar, RespawnDelay);
	const AGameStateBase* GameState = GetWorld()->GetGameState();
	RespawnServerTime = (GameState ? GameState->GetServerWorldTimeSeconds() : GetWorld()->GetTimeSeconds()) + RespawnDelay;

	if (ANBPlayerState* Stats = GetPlayerState<ANBPlayerState>())
	{
		Stats->AddFall();
	}
}

void ANBSquirrel::OnRep_Ragdoll()
{
	if (Ragdoll.bActive)
	{
		StartLocalRagdoll();
	}
	else
	{
		StopLocalRagdoll();
	}
}

void ANBSquirrel::StartLocalRagdoll()
{
	if (bLocalRagdoll)
	{
		return;
	}
	bLocalRagdoll = true;

	// The tail rides along on the body; the body becomes a free physics ball.
	TailVisual->AttachToComponent(BodyVisual, FAttachmentTransformRules::KeepWorldTransform);
	BodyVisual->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
	BodyVisual->SetWorldLocation(Ragdoll.Start, false, nullptr, ETeleportType::TeleportPhysics);
	BodyVisual->SetCollisionProfileName(UCollisionProfile::PhysicsActor_ProfileName);
	BodyVisual->SetSimulatePhysics(true);
	BodyVisual->SetPhysicsLinearVelocity(Ragdoll.Velocity);
	BodyVisual->SetPhysicsAngularVelocityInDegrees(FVector(720.f, 540.f, 360.f));

	RefreshAttachedState();
}

void ANBSquirrel::StopLocalRagdoll()
{
	if (!bLocalRagdoll)
	{
		return;
	}
	bLocalRagdoll = false;

	BodyVisual->SetSimulatePhysics(false);
	BodyVisual->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	BodyVisual->AttachToComponent(GetCapsuleComponent(), FAttachmentTransformRules::KeepWorldTransform);
	BodyVisual->SetRelativeTransform(DefaultBodyTransform);
	TailVisual->AttachToComponent(GetCapsuleComponent(), FAttachmentTransformRules::KeepWorldTransform);
	TailVisual->SetRelativeTransform(DefaultTailTransform);

	RefreshAttachedState();
}

void ANBSquirrel::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (bLocalRagdoll)
	{
		// The capsule (and camera) follow the tumbling body.
		SetActorLocation(BodyVisual->GetComponentLocation());
	}
}

void ANBSquirrel::Recover()
{
	check(HasAuthority());
	if (!Ragdoll.bActive)
	{
		return;
	}
	const FVector GetUpLocation = BodyVisual->GetComponentLocation() + FVector(0.f, 0.f, 40.f);
	Ragdoll.bActive = false;
	StopLocalRagdoll();
	SnapTo(GetUpLocation, FRotator(0.f, GetActorRotation().Yaw, 0.f));
}

void ANBSquirrel::RespawnAtCar()
{
	check(HasAuthority());
	ClearRespawn();
	if (Ragdoll.bActive)
	{
		GetWorldTimerManager().ClearTimer(RecoverTimer);
		Ragdoll.bActive = false;
		StopLocalRagdoll();
	}
	ReleaseActiveInteractable();

	ANBCar* Car = nullptr;
	for (TActorIterator<ANBCar> It(GetWorld()); It; ++It)
	{
		Car = *It;
		break;
	}
	if (!Car)
	{
		return;
	}

	if (CurrentSeat)
	{
		return;
	}
	if (ANBPlayerState* Stats = GetPlayerState<ANBPlayerState>())
	{
		Stats->AddRespawn();
	}
	if (UNBSeatComponent* Seat = Car->FindNextFreeSeat(nullptr))
	{
		EnterSeat(Seat);
		return;
	}
	const FVector Beside = Car->GetActorLocation() + Car->GetActorRightVector() * 250.f + FVector(0.f, 0.f, 120.f);
	SnapTo(Beside, FRotator(0.f, Car->GetActorRotation().Yaw, 0.f));
}

void ANBSquirrel::ClearRespawn()
{
	GetWorldTimerManager().ClearTimer(RespawnTimer);
	RespawnServerTime = 0.0;
}

float ANBSquirrel::GetRespawnSecondsLeft() const
{
	if (RespawnServerTime <= 0.0)
	{
		return -1.f;
	}
	const AGameStateBase* GameState = GetWorld()->GetGameState();
	const double Now = GameState ? GameState->GetServerWorldTimeSeconds() : GetWorld()->GetTimeSeconds();
	return static_cast<float>(FMath::Max(RespawnServerTime - Now, 0.0));
}

void ANBSquirrel::SnapTo(const FVector& Location, const FRotator& Rotation)
{
	check(HasAuthority());
	SetActorLocationAndRotation(Location, Rotation, false, nullptr, ETeleportType::TeleportPhysics);
	// The owning client runs its own movement, so tell it directly.
	Client_SnapTo(Location, Rotation);
}

void ANBSquirrel::Client_SnapTo_Implementation(FVector_NetQuantize Location, FRotator Rotation)
{
	StopLocalRagdoll();
	SetActorLocationAndRotation(Location, Rotation, false, nullptr, ETeleportType::TeleportPhysics);
}

void ANBSquirrel::FellOutOfWorld(const UDamageType& DamageType)
{
	// Don't destroy the pawn: off the map just means "back to the car".
	if (HasAuthority())
	{
		RespawnAtCar();
	}
}

ANBCar* ANBSquirrel::GetSeatCar() const
{
	return CurrentSeat ? Cast<ANBCar>(CurrentSeat->GetOwner()) : nullptr;
}

void ANBSquirrel::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (HasAuthority() && CurrentSeat)
	{
		if (ANBCar* Car = GetSeatCar())
		{
			Car->SetSeatInput(CurrentSeat->Role, 0.f);
		}
		CurrentSeat->SetOccupant(nullptr);
		CurrentSeat = nullptr;
	}
	Super::EndPlay(EndPlayReason);
}
