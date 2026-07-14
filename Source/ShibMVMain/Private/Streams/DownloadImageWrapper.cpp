// Copyright Shiba Inu Games LLC.


#include "Streams/DownloadImageWrapper.h"

#include "Blueprint/AsyncTaskDownloadImage.h"


void UDownloadImageWrapper::Start(const FString& Url, FOnImageDownloaded Event)
{
	OnImageDownloaded = Event;
	if (!OnImageDownloaded.IsBound()) return;
	
	auto* DownloadTask = NewObject<UAsyncTaskDownloadImage>();
	if (!DownloadTask) return;
	
	DownloadTask->OnSuccess.AddUniqueDynamic(this, &ThisClass::OnDownloadComplete);
	DownloadTask->OnFail.AddUniqueDynamic(this, &ThisClass::OnDownloadComplete);

	DownloadTask->Start(Url);
}

void UDownloadImageWrapper::OnDownloadComplete(UTexture2DDynamic* Texture)
{
	OnImageDownloaded.Broadcast(Texture);
}
