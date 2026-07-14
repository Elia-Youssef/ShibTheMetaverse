// Copyright Shiba Inu Games LLC.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Tools/BuildTypes.h"
#include "PlotBuilderSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlotBuilderSave, const FString&, Json);
DECLARE_DYNAMIC_DELEGATE(FOnPlotUpdating);

/**
 * 
 */
UCLASS()
class SHIBPLOTBUILDER_API UPlotBuilderSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable)
	FOnPlotBuilderSave OnPlotBuilderSave;

	/**
	 * Load objects replacing existing ones.
	 * @param UpdateCallback Callback function to call when the plot is getting a new update
	 * @param ObjectsDataTable Data table to get the data from
	 * @param Origin Plot origin to spawn objects relative to it
	 * @param Json Saved string json data
	 * @param ExistingObjects Existing plot objects to compare with. It will be modified with the newly loaded objects.
	 */
	void LoadPlot(const FOnPlotUpdating& UpdateCallback, const UDataTable* ObjectsDataTable, const FTransform& Origin, const FString& Json, TArray<FObjectBuiltData>& ExistingObjects);

};
