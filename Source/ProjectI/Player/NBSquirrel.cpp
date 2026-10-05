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
#include "Materials/MaterialInstanceDynamic.h"
#include "EngineUtils.h"
#include "GameFramework/CharacterMovementComponent.h"
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
}

void ANBSquirrel::Move(const FInputActionValue& Value)
{
	const FVector2D Axis = Value.Get<FVector2D>();
	if (CurrentSeat)
	{
		SendSeatInput(CurrentSeat->Role == ENBSeatRole::Wheel ? Axis.X : Axis.Y);
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
	Jump();
}

void ANBSquirrel::Interact()
{
	Server_Interact();
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

	Seat->SetOccupant(this);
	CurrentSeat = Seat;
	ApplySeatedState(true);
	AttachToComponent(Seat, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
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
	ApplySeatedState(false);
}

void ANBSquirrel::OnRep_CurrentSeat()
{
	ApplySeatedState(CurrentSeat != nullptr);
	LastSentSeatInput = 0.f;
}

void ANBSquirrel::ApplySeatedState(bool bSeated)
{
	SetActorEnableCollision(!bSeated);
	if (bSeated)
	{
		GetCharacterMovement()->DisableMovement();
	}
	else
	{
		GetCharacterMovement()->SetMovementMode(MOVE_Walking);
	}
	// Seated, the boom starts inside the car; probing would snap the camera onto the squirrel.
	CameraBoom->bDoCollisionTest = !bSeated;
	CameraBoom->TargetArmLength = bSeated ? SeatedArmLength : OnFootArmLength;
	CameraBoom->SocketOffset = FVector(0.f, 0.f, bSeated ? 420.f : 60.f);
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
