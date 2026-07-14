// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "Tools/BuildTypes.h"
#include "BuildSave.generated.h"

class ABaseBuildActor;

UCLASS()
class SHIBPLOTBUILDER_API UBuildSave : public USaveGame
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly)
	TArray<FObjectBuiltData> ObjectsBuilt;

	UFUNCTION(BlueprintCallable)
	FString SaveGame(TArray<FObjectBuiltData> Data);
	
	UFUNCTION(BlueprintCallable)
	void LoadGame();
	
	UFUNCTION(BlueprintCallable)
	void LoadGameFromString(const FString& Json);

	static FString SerializeObjectBuiltData(TArray<FObjectBuiltData>& Data);
	static TArray<FObjectBuiltData> DeserializeJsonToObjectBuiltData(const FString& JsonString);

	static inline const FString& PlotBuilderCustomizationSlot{"PlotBuilderCustomizationSlot"};
	FString GetSaveJsonFilePath();
	bool CheckJsonSaveGameExists();
};

USTRUCT()
struct FObjectBuiltDataArray
{
	GENERATED_BODY()
    
	UPROPERTY()
	TArray<FObjectBuiltData> Data;
};
