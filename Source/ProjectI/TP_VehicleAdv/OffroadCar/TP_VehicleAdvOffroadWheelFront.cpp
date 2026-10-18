// Copyright Epic Games, Inc. All Rights Reserved.


#include "TP_VehicleAdvOffroadWheelFront.h"

UTP_VehicleAdvOffroadWheelFront::UTP_VehicleAdvOffroadWheelFront()
{
	WheelRadius = 50.0f;
	CorneringStiffness = 750.0f;
	FrictionForceMultiplier = 4.0f;
	bAffectedByEngine = true;

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