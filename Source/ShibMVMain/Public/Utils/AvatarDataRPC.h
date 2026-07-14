#pragma once

#include "ShibLibrary.h"
// #include "CoreMinimal.h"
#include "AvatarDataRPC.generated.h"

#pragma region SupportedStructs

struct FAvatarSave;

USTRUCT(BlueprintType)
struct FShibMeshData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EShibMesh MeshType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName RowName;
};

USTRUCT(BlueprintType)
struct FShibTextureData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EShibTexture TextureType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName RowName;
};


USTRUCT(BlueprintType)
struct FShibAccessoryData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EShibAccessory AccessoryType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName RowName;
};

USTRUCT(BlueprintType)
struct FShibTattooData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EShibTattoo TattooType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName RowName;
};

USTRUCT(BlueprintType)
struct FShibMorphData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EShibBodyMorphs MorphType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	double Value;
};

USTRUCT(BlueprintType)
struct FShibMeshColorData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EShibMesh MeshType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor Color;
};

USTRUCT(BlueprintType)
struct FShibTextureColorData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EShibTexture TextureType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor Color;
};

USTRUCT(BlueprintType)
struct FShibAccessoryColorData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EShibAccessory AccessoryType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor Color;
};

USTRUCT(BlueprintType)
struct FShibTattooColorData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EShibTattoo TattooType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor Color;
};

#pragma endregion SupportedStructs

USTRUCT(BlueprintType)
struct FAvatarDataRPC
{
	GENERATED_BODY()

	// For gender
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName AvatarBaseRow{"NewRow"};    

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsMale{true};

	// Mesh Data
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FShibMeshData> MeshAssetData;

	// Texture Data
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FShibTextureData> TextureAssetData;

	// Accessory Data
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FShibAccessoryData> AccessoryData;

	// Tattoo Data
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FShibTattooData> TattooData;

	// Morph Data
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FShibMorphData> MorphData;

	// Mesh Colors
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FShibMeshColorData> MeshColorData;

	// Texture Colors
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FShibTextureColorData> TextureColorData;

	// Accessory Colors
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FShibAccessoryColorData> AccessoryColorData;

	// Tattoo Colors
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FShibTattooColorData> TattooColorData;

	/**Turns SaveData into AvatarData*/
	void CopySaveFileToAvatarData(const ::FAvatarSave& SaveFile, FAvatarDataRPC& AvatarData);

	/**Turns AvatarData into SaveData*/
	void CopyAvatarDataToSaveFile(const FAvatarDataRPC& AvatarData, FAvatarSave& SaveFile);
};
