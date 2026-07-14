// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/BuildGameMode.h"
#include "Engine/TextureRenderTarget2D.h"
#include "ImageUtils.h"
#include "Game/BuildSave.h"
#include "HttpModule.h"
#include "HttpManager.h"
#include "Interfaces/IHttpResponse.h"
#include "Kismet/GameplayStatics.h"

void ABuildGameMode::BeginPlay()
{
	Super::BeginPlay();

	// BuildSave = Cast<UBuildSave>(UGameplayStatics::CreateSaveGameObject(UBuildSave::StaticClass()));
	// BuildSave->LoadGame();
	
	BuildSave = Cast<UBuildSave>(UGameplayStatics::CreateSaveGameObject(UBuildSave::StaticClass()));

	if (BuildSave)
	{
		// UE_LOG(LogTemp, Display, TEXT("Loading GameMode: UBuildSave object created successfully."));
		// BuildSave->LoadGame();
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Loading GameMode: Failed to create UBuildSave object."));
	}
}

void ABuildGameMode::SaveRenderTargetToPNG(UTextureRenderTarget2D* RenderTarget)
{
	if ( ! RenderTarget){return;}

	// Convert RenderTarget to PNG data
	TArray64<uint8> PNGData;
	if ( ! ConvertRenderTargetToPNG(RenderTarget, PNGData))
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to convert RenderTarget to PNG"));
		return;
	}

	// Save the PNG to a file
	const FString CompleteFilePath = FPaths::ProjectDir() + FilePath;
	if (FFileHelper::SaveArrayToFile(PNGData, *CompleteFilePath))
	{
		UE_LOG(LogTemp, Log, TEXT("PNG successfully saved to: %s"), *CompleteFilePath);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to save PNG file."));
	}
}

void ABuildGameMode::UploadFileToCloud(UTextureRenderTarget2D* RenderTarget)
{
    if (!RenderTarget)
    {
        UE_LOG(LogTemp, Warning, TEXT("Invalid RenderTarget"));
        return;
    }

    // Convert RenderTarget to PNG data
    TArray64<uint8> PNGData;
    if (!ConvertRenderTargetToPNG(RenderTarget, PNGData))
    {
        UE_LOG(LogTemp, Warning, TEXT("Failed to convert RenderTarget to PNG"));
        return;
    }

    // Prepare the multipart/form-data request body
    const FString Boundary = TEXT("--BoundaryString\r\n");
    const FString ContentDisposition = TEXT("Content-Disposition: form-data; name=\"file\"; filename=\"image.png\"\r\nContent-Type: image/png\r\n\r\n");
    const FString EndingBoundary = TEXT("\r\n--BoundaryString--\r\n");

    // Construct the request body
    TArray<uint8> RequestContent;
    RequestContent.Append((uint8*)TCHAR_TO_ANSI(*Boundary), Boundary.Len());
    RequestContent.Append((uint8*)TCHAR_TO_ANSI(*ContentDisposition), ContentDisposition.Len());
    RequestContent.Append(PNGData);
    RequestContent.Append((uint8*)TCHAR_TO_ANSI(*EndingBoundary), EndingBoundary.Len());

    // Send the HTTP request
    FHttpModule* Http = &FHttpModule::Get();
    TSharedRef<IHttpRequest> Request = Http->CreateRequest();

    Request->SetURL(CloudEndPoint);
    Request->SetVerb("POST");
    Request->SetHeader("Content-Type", "multipart/form-data; boundary=BoundaryString");

    // Set the content to the constructed multipart body
    Request->SetContent(RequestContent);

    // Handle the response //TODO possibly bind this to a separate function
    Request->OnProcessRequestComplete().BindLambda([](FHttpRequestPtr Req, FHttpResponsePtr Res, bool Success)
    {
        if (Success && Res.IsValid())
        {
            UE_LOG(LogTemp, Log, TEXT("Uploading png to cloud was successful: %s"), *Res->GetContentAsString());
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("Upload of png to cloud failed"));
        }
    });

    Request->ProcessRequest();
}

bool ABuildGameMode::ConvertRenderTargetToPNG(UTextureRenderTarget2D* RenderTarget, TArray64<uint8>& OutPNGData)
{
	// Read the pixels from the render target
	FTextureRenderTargetResource* RenderTargetResource = RenderTarget->GameThread_GetRenderTargetResource();
	if ( ! RenderTargetResource) return false;
	
	TArray<FColor> Pixels;
	RenderTargetResource->ReadPixels(Pixels);

	// Force the alpha of each pixel to be fully visible
	for (FColor& Pixel : Pixels)
	{
		Pixel.A = 255; // 255 = fully opaque (1.0f alpha)
	}

	// Declaring the return value
	TArray64<uint8> PNGData64;

	// Compress the pixels to PNG
	FImageUtils::PNGCompressImageArray(
		RenderTarget->SizeX, 
		RenderTarget->SizeY, 
		TArrayView64<const FColor>(Pixels.GetData(), Pixels.Num()), 
		PNGData64
	);
	
	// Set the return value to the new data
	OutPNGData = PNGData64;

#pragma region Debug
// #if WITH_EDITOR
// 	for (int32 i = 0; i < FMath::Min(Pixels.Num(), 10); ++i)
// 	{
// 		const FColor& Pixel = Pixels[i];
// 		UE_LOG(LogTemp, Log, TEXT("Pixel %d: R=%d G=%d B=%d A=%d"), i, Pixel.R, Pixel.G, Pixel.B, Pixel.A);
// 	}
// #endif
#pragma endregion Debug

	// Validate that compression succeeded by checking if OutPNGData is not empty
	return OutPNGData.Num() > 0;
}