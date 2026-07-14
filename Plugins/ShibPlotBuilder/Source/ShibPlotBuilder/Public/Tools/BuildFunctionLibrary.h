// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "BuildFunctionLibrary.generated.h"

/**
 * 
 */
UCLASS()
class SHIBPLOTBUILDER_API UBuildFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	static FTransform SnapTransformToGrid(const FTransform& Transform);
	
};
