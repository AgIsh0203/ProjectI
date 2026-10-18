// Copyright Epic Games, Inc. All Rights Reserved.


#include "TP_VehicleAdvOffroadWheelRear.h"

UTP_VehicleAdvOffroadWheelRear::UTP_VehicleAdvOffroadWheelRear()
{
	WheelRadius = 50.f;
	CorneringStiffness = 750.0f;
	FrictionForceMultiplier = 4.0f;
	
	SuspensionMaxRaise = 20.0f;
	SuspensionMaxDrop = 20.0f;
	WheelLoadRatio = 1.0f;
	SpringRate = 100.0f;
	SpringPreload = 100.0f;
	// Spherecast, not Shapecast: in UE 5.7 Chaos, Shapecast falls through to the raycast path, and a ray slips
	// between trail rocks and ford pebbles, so the suspension never felt them (18 Oct).
	SweepShape = ESweepShape::Spherecast;

	MaxBrakeTorque = 3000.0f;
	MaxHandBrakeTorque = 6000.0f;
}