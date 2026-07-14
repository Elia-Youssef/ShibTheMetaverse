// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "BuildGameMode.generated.h"

class UBuildSave;

UCLASS()
class SHIBPLOTBUILDER_API ABuildGameMode : public AGameMode
{
	GENERATED_BODY()

public:
	UPROPERTY()
	UBuildSave* BuildSave;

	/**The directory the screenshot is saved in. This path is added to the base directory of the project.*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FString FilePath = TEXT("Screenshots/PlotBuilderScreenshot.png");

	//TODO we need to fill in the correct cloud end point for when sending the png
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FString CloudEndPoint = TEXT("");

	/**Saves a local screenshot png file.*/
	UFUNCTION(BlueprintCallable)
	void SaveRenderTargetToPNG(UTextureRenderTarget2D* RenderTarget);

	/**Converts a RenderTarget to PNG data.*/
	bool ConvertRenderTargetToPNG(UTextureRenderTarget2D* RenderTarget, TArray64<uint8>& OutPNGData);

	/**Uploads a PNG file to a specified cloud API using multipart/form-data. The endpoint is defined by the CloudEndPoint exposed variable.*/
	UFUNCTION(BlueprintCallable)
	void UploadFileToCloud(UTextureRenderTarget2D* RenderTarget);

protected:
	virtual void BeginPlay() override;
	
};
