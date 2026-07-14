// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "ShibFunctionLibrary.generated.h"

USTRUCT(BlueprintType)
struct FRichTextToken
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Text = FString();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Link = FString();
};

/**
 * 
 */
UCLASS()
class SHIBMVMAIN_API UShibFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category="ShibUtils")
	static FDateTime GetLocalTimeFromUTC(FDateTime UTC);

	UFUNCTION(BlueprintPure, Category="ShibUtils")
	static bool IsPointInArea(FVector2D Point, FVector2D From, FVector2D To);

	UFUNCTION(BlueprintCallable, Category="ShibUtils")
	static void SetCommandLineToken(const FString Token);

	UFUNCTION(BlueprintCallable, Category="ShibUtils")
	static TArray<FRichTextToken> ExtractHyperLinks(const FString& Text);
	
	// Allow the blueprint to determine whether we are running with the editor or not
	UFUNCTION(BlueprintPure, BlueprintCallable, Category = "ShibUtils")
	static bool RunningInPIE();

	// EDITOR ONLY FUNCTIONS
	
	UFUNCTION(BlueprintCallable, Category="ShibUtils", meta = (DevelopmentOnly))
	static void UpdateLevelEditorPlaySettings(const FString& DevToken, const FString& DevWallet); 
	
	

	UFUNCTION(BlueprintPure, Category="ShibUtils")
	static bool IsClientOrStandalone();

	UFUNCTION(BlueprintPure, Category="ShibUtils")
	static bool IsPixelStreaming();
};
