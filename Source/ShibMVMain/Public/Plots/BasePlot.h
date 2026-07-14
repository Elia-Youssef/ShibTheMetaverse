// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Game/PlotBuilderSubsystem.h"
#include "GameFramework/Actor.h"
#include "Metaverse/MetaverseApisTypes.h"
#include "BasePlot.generated.h"

struct FObjectBuiltData;

UCLASS()
class SHIBMVMAIN_API ABasePlot : public AActor
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<FObjectBuiltData> BuiltObjects;
	
public:	
	ABasePlot();
	void CalculateCoordinatesUsingRef();
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category=PlotInfo)
	FVector2D Coordinates;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category=PlotInfo)
	FServerPlotInfo ServerPlotInfo;

	// For general plot assembly in hubs
	UFUNCTION(BlueprintCallable, Category=Plot)
	void AssemblePlot();

	// For Plot Builder ONLY
	UFUNCTION(BlueprintCallable, Category=Plot)
	void AssemblePlotForPlotBuilder();

	// Setup plot right before receiving an update
	// This function is called by the PlotBuilderSubsystem when it's about to update the plot
	UFUNCTION()
	void OnPlotUpdating();

	/**Will move all characters within actor bounds slightly outside of the bounds.*/
	UFUNCTION(BlueprintCallable)
	void MoveCharactersToLandingPlatform();
	
	UPROPERTY(EditDefaultsOnly, Category=PlotInfo)
	UDataTable* ObjectsDataTable;

protected:
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void BeginPlay() override;

private:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess = "true"))
	TObjectPtr<USceneComponent> PlotRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess = "true"))
	TObjectPtr<UChildActorComponent> LandingPlatform;

	// Delegate to trigger when this plot is receiving an update
	FOnPlotUpdating OnPlotUpdatingDelegate;
};