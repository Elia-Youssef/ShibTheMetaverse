// Fill out your copyright notice in the Description page of Project Settings.

// Testing for build automation changes this will be removed please ignore 


#include "Tools/BuildFunctionLibrary.h"
#include "Kismet/KismetMathLibrary.h"

FTransform UBuildFunctionLibrary::SnapTransformToGrid(const FTransform& Transform)
{
	FVector NewLocation = Transform.GetLocation();

	NewLocation.X = UKismetMathLibrary::GridSnap_Float(NewLocation.X, 50.f);
	NewLocation.Y = UKismetMathLibrary::GridSnap_Float(NewLocation.Y, 50.f);
	
	return FTransform{ Transform.GetRotation(), NewLocation, Transform.GetScale3D() };
}
