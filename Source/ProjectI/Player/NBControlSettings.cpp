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
	PadBindings.SetNum(static_cast<int32>(ENBBinding::Count));
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
	case ENBBinding::PushToTalk: return EKeys::V;
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
	case ENBBinding::PushToTalk: return TEXT("Talk (push to talk, hold)");
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
	PadBindings.Reset();
	PadBindings.SetNum(static_cast<int32>(ENBBinding::Count));
	++BindingsRevision;
	SaveConfig();
}

FKey UNBControlSettings::GetDefaultPadKey(ENBBinding Binding)
{
	switch (Binding)
	{
	case ENBBinding::Jump: return EKeys::Gamepad_FaceButton_Bottom;
	case ENBBinding::Interact: return EKeys::Gamepad_FaceButton_Left;
	case ENBBinding::Work: return EKeys::Gamepad_FaceButton_Right;
	case ENBBinding::Horn: return EKeys::Gamepad_FaceButton_Top;
	case ENBBinding::ResetCar: return EKeys::Gamepad_Special_Left;
	case ENBBinding::Handbrake: return EKeys::Gamepad_RightShoulder;
	case ENBBinding::PushToTalk: return EKeys::Gamepad_LeftThumbstick;
	default: return EKeys::Invalid;
	}
}

namespace
{
	struct FNBPadName
	{
		FKey Key;
		const TCHAR* Name;
	};

	/** Every bindable gamepad button and its on-screen name. */
	const TArray<FNBPadName>& PadNames()
	{
		static const TArray<FNBPadName> Names = {
			{EKeys::Gamepad_FaceButton_Bottom, TEXT("A")},
			{EKeys::Gamepad_FaceButton_Right, TEXT("B")},
			{EKeys::Gamepad_FaceButton_Left, TEXT("X")},
			{EKeys::Gamepad_FaceButton_Top, TEXT("Y")},
			{EKeys::Gamepad_RightShoulder, TEXT("RB")},
			{EKeys::Gamepad_LeftTrigger, TEXT("LT")},
			{EKeys::Gamepad_RightTrigger, TEXT("RT")},
			{EKeys::Gamepad_LeftThumbstick, TEXT("L3")},
			{EKeys::Gamepad_RightThumbstick, TEXT("R3")},
			{EKeys::Gamepad_Special_Left, TEXT("View")},
		};
		return Names;
	}
}

bool UNBControlSettings::IsPadBindable(const FKey& Key)
{
	return PadNames().ContainsByPredicate([&Key](const FNBPadName& Each) { return Each.Key == Key; });
}

FString UNBControlSettings::GetPadKeyLabel(const FKey& Key)
{
	if (const FNBPadName* Found = PadNames().FindByPredicate([&Key](const FNBPadName& Each) { return Each.Key == Key; }))
	{
		return Found->Name;
	}
	if (Key == EKeys::Gamepad_LeftShoulder) { return TEXT("LB"); }
	if (Key == EKeys::Gamepad_Special_Right) { return TEXT("Menu"); }
	return Key.IsValid() ? Key.GetDisplayName(/*bLongDisplayName=*/false).ToString() : FString(TEXT("-"));
}

FKey UNBControlSettings::GetPadKey(ENBBinding Binding) const
{
	if (!HasPadButton(Binding))
	{
		return EKeys::Invalid;
	}
	const int32 Index = static_cast<int32>(Binding);
	if (PadBindings.IsValidIndex(Index) && !PadBindings[Index].IsEmpty())
	{
		const FKey Key(*PadBindings[Index]);
		if (IsPadBindable(Key))
		{
			return Key;
		}
	}
	return GetDefaultPadKey(Binding);
}

ENBBinding UNBControlSettings::SetPadKey(ENBBinding Binding, const FKey& Key)
{
	if (!IsPadBindable(Key) || !HasPadButton(Binding))
	{
		return ENBBinding::Count;
	}
	PadBindings.SetNum(static_cast<int32>(ENBBinding::Count));
	const FKey Previous = GetPadKey(Binding);
	ENBBinding Swapped = ENBBinding::Count;
	for (int32 i = 0; i < static_cast<int32>(ENBBinding::Count); ++i)
	{
		const ENBBinding Other = static_cast<ENBBinding>(i);
		if (Other != Binding && GetPadKey(Other) == Key)
		{
			PadBindings[i] = Previous.ToString();
			Swapped = Other;
		}
	}
	PadBindings[static_cast<int32>(Binding)] = Key.ToString();
	++BindingsRevision;
	SaveConfig();
	return Swapped;
}
