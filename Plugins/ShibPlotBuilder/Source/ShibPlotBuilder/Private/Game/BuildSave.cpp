// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/BuildSave.h"
#include "JsonObjectConverter.h"

FString UBuildSave::SaveGame(TArray<FObjectBuiltData> Data)
{
	ObjectsBuilt = Data;
	FString JsonStringObject = SerializeObjectBuiltData(ObjectsBuilt);
	return JsonStringObject;
	
	// FString FilePath = GetSaveJsonFilePath();
	// IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	//
	// if (PlatformFile.FileExists(*FilePath))
	// {
	// 	if(PlatformFile.DeleteFile(*FilePath))
	// 	{
	// 		UE_LOG(LogTemp, Log, TEXT("Successfully deleted existing save file"));
	// 	}
	// 	else
	// 	{
	// 		UE_LOG(LogTemp, Error, TEXT("Failed to delete existing save file"));
	// 	
	// 	}
	// }
	//
	// if(FFileHelper::SaveStringToFile(JsonStringObject, *FilePath))
	// {
	// 	FString VerificationString;
	// 	if (FFileHelper::LoadFileToString(VerificationString, *FilePath))
	// 	{
	// 		if (VerificationString.Equals(JsonStringObject))
	// 		{
	// 			UE_LOG(LogTemp, Log, TEXT("Save successful and verified"));
	// 		}
	// 		else
	// 		{
	// 			UE_LOG(LogTemp, Error, TEXT("Save verification failed - content mismatch"));
	// 		}
	// 	}
	// 	else
	// 	{
	// 		UE_LOG(LogTemp, Error, TEXT("Save verification failed - couldn't read file"));
	// 	}
	// }
	// else
	// {
	// 	UE_LOG(LogTemp, Error, TEXT("Failed to save JSON data"));
	// }
	//
	// return JsonStringObject;
}

void UBuildSave::LoadGame()
{
	if (!CheckJsonSaveGameExists())
	{
		return;
	}

	FString FilePath = GetSaveJsonFilePath();
	FString FileString;
   
	if(!FFileHelper::LoadFileToString(FileString, *FilePath) || FileString.IsEmpty())
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to load file from path: or file is empty %s"), *FilePath);
		return;
	}

	TArray<FObjectBuiltData> LoadedData = DeserializeJsonToObjectBuiltData(FileString);
   
	if (LoadedData.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("No objects were loaded from save file"));
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("Loaded JSON Data:\n%s"), *FileString);
	}

	ObjectsBuilt = LoadedData;
}

void UBuildSave::LoadGameFromString(const FString& Json)
{
	if (Json.IsEmpty()) return;
	
	ObjectsBuilt = DeserializeJsonToObjectBuiltData(Json);
   
	if (ObjectsBuilt.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("No objects were loaded from save file"));
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("Loaded JSON Data:\n%s"), *Json);
	}
}

FString UBuildSave::SerializeObjectBuiltData(TArray<FObjectBuiltData>& Data)
{
	FObjectBuiltDataArray Wrapper;
	Wrapper.Data = Data;
   
	FString JsonString;
	FJsonObjectConverter::UStructToJsonObjectString(Wrapper, JsonString);
	return JsonString;
}

TArray<FObjectBuiltData> UBuildSave::DeserializeJsonToObjectBuiltData(const FString& JsonString)
{
	FObjectBuiltDataArray Wrapper;
	if (FJsonObjectConverter::JsonObjectStringToUStruct(JsonString, &Wrapper, 0, 0))
	{
		return Wrapper.Data;
	}
	return TArray<FObjectBuiltData>();
}

FString UBuildSave::GetSaveJsonFilePath()
{
	return  FPaths::Combine(FPaths::ProjectSavedDir(),"SaveGames",PlotBuilderCustomizationSlot + ".json");
}

bool UBuildSave::CheckJsonSaveGameExists()
{
	return FPaths::FileExists(GetSaveJsonFilePath());
}
