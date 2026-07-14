// Copyright Shiba Inu Games LLC.

#pragma once

#include "CoreMinimal.h"
#include "DownloadImageWrapper.generated.h"

class UAsyncTaskDownloadImage;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnImageDownloaded, UTexture2DDynamic* Texture);

/**
 * 
 */
UCLASS()
class SHIBMVMAIN_API UDownloadImageWrapper : public UObject
{
	GENERATED_BODY()

	FOnImageDownloaded OnImageDownloaded;

public:
	void Start(const FString& Url, FOnImageDownloaded Event);

protected:
	UFUNCTION()
	void OnDownloadComplete(UTexture2DDynamic* Texture);
	
};
