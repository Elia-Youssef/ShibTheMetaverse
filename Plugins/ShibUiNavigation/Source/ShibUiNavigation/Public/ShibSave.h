// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "ShibSave.generated.h"

USTRUCT(BlueprintType)
struct FShibSettings
{
	GENERATED_BODY()

	// General
	UPROPERTY(BlueprintReadWrite, Category = "General")
	float MouseSensitivity = 0.5f;
	
	UPROPERTY(BlueprintReadWrite, Category = "General")
	bool bInvertYAxis = true;
	
	UPROPERTY(BlueprintReadWrite, Category = "General")
	bool bShowAvatarNames = true;

	UPROPERTY(BlueprintReadWrite, Category = "General")
	bool bShowShortcutHints = false;

	// Audio
	UPROPERTY(BlueprintReadWrite, Category = "Audio")
	float MasterVolume = 1.f;
	
	UPROPERTY(BlueprintReadWrite, Category = "Audio")
	float MusicVolume = 1.f;
	
	UPROPERTY(BlueprintReadWrite, Category = "Audio")
	float EffectsVolume = 1.f;

	UPROPERTY(BlueprintReadWrite, Category = "Audio")
	float StreamVolume = 1.f;

	UPROPERTY(BlueprintReadWrite, Category = "Audio")
	float VoiceChatVolume = 1.f;

	// Graphics - CURRENTLY USING UE's BUILT IN SAVE
	UPROPERTY(BlueprintReadWrite, Category = "Graphics")
	TEnumAsByte<EWindowMode::Type> WindowMode = EWindowMode::Fullscreen;

	UPROPERTY(BlueprintReadWrite, Category = "Graphics")
	FIntPoint Resolution = FIntPoint(1920, 1080); // Dummy data, will be overridden
	
	UPROPERTY(BlueprintReadWrite, Category = "Graphics")
	int32 GraphicsQuality = 3; // Ultra

	UPROPERTY(BlueprintReadWrite, Category = "Graphics")
	bool BenchmarkOptimizedGraphics = false;
	
	UPROPERTY(BlueprintReadWrite, Category = "Graphics")
	float Brightness = 1.f;
	
	UPROPERTY(BlueprintReadWrite, Category = "Graphics")
	bool bMotionBlur = true;
	
	// Controls - CURRENTLY USING UE's BUILT IN SAVE
	UPROPERTY(BlueprintReadWrite, Category = "Controls")
	TMap<FName, FKey> KeyMappings = TMap<FName, FKey>{
		{FName("Forward"), EKeys::W},
		{FName("Left"), EKeys::A},
		{FName("Backward"), EKeys::S},
		{FName("Right"), EKeys::D},
		{FName("Jump"), EKeys::SpaceBar},
		{FName("Interact"), EKeys::F},
		{FName("Reload"), EKeys::R},
		{FName("LeftCorner"), EKeys::Q},
		{FName("RightCorner"), EKeys::E},
		{FName("Run"), EKeys::LeftShift}
	};
};

/**
 * 
 */
UCLASS()
class SHIBUINAVIGATION_API  UShibSave : public USaveGame
{
	GENERATED_BODY()

public:
	/**
	 * @return Default shib settings struct
	 */
	UFUNCTION(BlueprintPure, Category = "Settings")
	FShibSettings GetDefaultShibSettings() { return FShibSettings(); }
	
	UPROPERTY(BlueprintReadWrite, Category="Settings")
	FShibSettings ShibSettings;

	/**This will be saved to false after completing or skipping the initial tutorial level.*/
	UPROPERTY(BlueprintReadWrite, Category = "Tutorial")
	bool bSkipTutorial = true;
};
