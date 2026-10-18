// Squirrel Wheels prototype.

#include "Player/NBControlSettings.h"

#include "Engine/GameInstance.h"
#include "Kismet/GameplayStatics.h"

void UNBControlSettings::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	LookSensitivity = FMath::Clamp(LookSensitivity, MinLookSensitivity, MaxLookSensitivity);
	CameraShake = FMath::Clamp(CameraShake, 0.f, 1.f);
	KeyBindings.SetNum(static_cast<int32>(ENBBinding::Count));
}

UNBControlSettings* UNBControlSettings::Get(const UObject* WorldContext)
{
	const UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(WorldContext);
	return GameInstance ? GameInstance->GetSubsystem<UNBControlSettings>() : nullptr;
}

void UNBControlSettings::SetLookSensitivity(float Value)
{
	LookSensitivity = FMath::Clamp(Value, MinLookSensitivity, MaxLookSensitivity);
	SaveConfig();
}

void UNBControlSettings::SetLookInverted(bool bInverted)
{
	bInvertLook = bInverted;
	SaveConfig();
}

void UNBControlSettings::SetCameraShake(float Value)
{
	CameraShake = FMath::Clamp(Value, 0.f, 1.f);
	SaveConfig();
}

FKey UNBControlSettings::GetDefaultKey(ENBBinding Binding)
{
	switch (Binding)
	{
	case ENBBinding::Forward: return EKeys::W;
	case ENBBinding::Back: return EKeys::S;
	case ENBBinding::Left: return EKeys::A;
	case ENBBinding::Right: return EKeys::D;
	case ENBBinding::Jump: return EKeys::SpaceBar;
	case ENBBinding::Interact: return EKeys::E;
	case ENBBinding::Work: return EKeys::LeftMouseButton;
	case ENBBinding::Horn: return EKeys::H;
	case ENBBinding::ResetCar: return EKeys::R;
	case ENBBinding::Handbrake: return EKeys::LeftShift;
	default: return EKeys::Invalid;
	}
}

const TCHAR* UNBControlSettings::GetBindingLabel(ENBBinding Binding)
{
	switch (Binding)
	{
	case ENBBinding::Forward: return TEXT("Forward / gas");
	case ENBBinding::Back: return TEXT("Back / brake");
	case ENBBinding::Left: return TEXT("Left / steer left");
	case ENBBinding::Right: return TEXT("Right / steer right");
	case ENBBinding::Jump: return TEXT("Jump / jump out");
	case ENBBinding::Interact: return TEXT("Get in / hop seat");
	case ENBBinding::Work: return TEXT("Work on a part");
	case ENBBinding::Horn: return TEXT("Horn");
	case ENBBinding::ResetCar: return TEXT("Reset the car (hold)");
	case ENBBinding::Handbrake: return TEXT("Handbrake (Gears seat, hold)");
	default: return TEXT("?");
	}
}

bool UNBControlSettings::IsBindable(const FKey& Key)
{
	static const FKey Fixed[] = {EKeys::Escape, EKeys::Tab, EKeys::One, EKeys::Two, EKeys::Three, EKeys::Four,
		EKeys::Five, EKeys::Six, EKeys::Seven, EKeys::Eight};
	if (!Key.IsValid() || Key.IsGamepadKey() || Key.IsAxis1D() || Key.IsAxis2D() || Key.IsAxis3D() || Key.IsTouch())
	{
		return false;
	}
	for (const FKey& Each : Fixed)
	{
		if (Key == Each)
		{
			return false;
		}
	}
	return true;
}

FString UNBControlSettings::GetKeyLabel(const FKey& Key)
{
	if (Key == EKeys::LeftMouseButton) { return TEXT("LMB"); }
	if (Key == EKeys::RightMouseButton) { return TEXT("RMB"); }
	if (Key == EKeys::MiddleMouseButton) { return TEXT("MMB"); }
	if (Key == EKeys::ThumbMouseButton) { return TEXT("Mouse 4"); }
	if (Key == EKeys::ThumbMouseButton2) { return TEXT("Mouse 5"); }
	if (Key == EKeys::SpaceBar) { return TEXT("Space"); }
	return Key.GetDisplayName(/*bLongDisplayName=*/false).ToString();
}

FKey UNBControlSettings::GetKey(ENBBinding Binding) const
{
	const int32 Index = static_cast<int32>(Binding);
	if (KeyBindings.IsValidIndex(Index) && !KeyBindings[Index].IsEmpty())
	{
		const FKey Key(*KeyBindings[Index]);
		if (IsBindable(Key))
		{
			return Key;
		}
	}
	return GetDefaultKey(Binding);
}

ENBBinding UNBControlSettings::SetKey(ENBBinding Binding, const FKey& Key)
{
	if (!IsBindable(Key) || Binding == ENBBinding::Count)
	{
		return ENBBinding::Count;
	}
	KeyBindings.SetNum(static_cast<int32>(ENBBinding::Count));
	const FKey Previous = GetKey(Binding);
	ENBBinding Swapped = ENBBinding::Count;
	for (int32 i = 0; i < static_cast<int32>(ENBBinding::Count); ++i)
	{
		const ENBBinding Other = static_cast<ENBBinding>(i);
		if (Other != Binding && GetKey(Other) == Key)
		{
			// One key, one job: the other action takes this one's old key.
			KeyBindings[i] = Previous.ToString();
			Swapped = Other;
		}
	}
	KeyBindings[static_cast<int32>(Binding)] = Key.ToString();
	++BindingsRevision;
	SaveConfig();
	return Swapped;
}

void UNBControlSettings::ResetKeys()
{
	KeyBindings.Reset();
	KeyBindings.SetNum(static_cast<int32>(ENBBinding::Count));
	++BindingsRevision;
	SaveConfig();
}
