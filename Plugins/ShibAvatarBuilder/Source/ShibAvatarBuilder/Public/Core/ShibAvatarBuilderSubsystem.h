// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "ShibLibrary.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "I_PlayerInterractions.h"
#include "AvatarCommand.h"
#include <type_traits>
#include "ShibSaveGame.h"
#include "ShibAvatarBuilderSubsystem.generated.h"

#define COPY_DYN_MATERIAL_PARAMETER(OldMaterial, NewMaterial, Param, ParamType) \
	NewMaterial->Set##ParamType##ParameterValue(FName(Param), OldMaterial->K2_Get##ParamType##ParameterValue(FName(Param)));


struct FAvatarSave;
enum class ELoadAvatarType : uint8;
class UAvatarHistory;
class USkeletalMeshComponent;
class AShibController;
class AShibAvatarCharacter;
class IAvatarCommand;


DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAvatarLoaded);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGenderSwapped);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAvatarBuilderSave, const FString&, Json);

/**
* 
*/
UCLASS()
class SHIBAVATARBUILDER_API UShibAvatarBuilderSubsystem : public UGameInstanceSubsystem, public II_PlayerInterractions
{
	GENERATED_BODY()
public:
	
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	void Init();

	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UShibAvatarDataAsset> AvatarDataAsset;

	UPROPERTY(BlueprintAssignable)
	FOnAvatarBuilderSave OnAvatarBuilderSave;
	
	// Save Game //

	// Saves Avatar in json format
	UFUNCTION(BlueprintCallable)
	void SaveAvatar();

	// Tries to Load Avatar from Json File in SaveDir 
	UFUNCTION(BlueprintCallable)
	void LoadAvatarFromJsonFile();

	// Tries to Load avatar from json string passed
	UFUNCTION(BlueprintCallable)
	void LoadAvatarFromJsonString(const FString& JsonString);

	// Loads the data in the Save struct and applies it on to the avatar
	UFUNCTION(BlueprintCallable)
	void LoadAvatarData(FAvatarSave AvatarData,bool bSaveChanges = true);

	UFUNCTION(BlueprintCallable)
	void LoadReplicatedAvatar(AShibAvatarCharacter* Character, const FString& String);

	

	// Gender // 
	
	UFUNCTION(BlueprintCallable)
	void SwapGender(const FBase& Base = FBase(), EAvatarBuilderGender InGender = EAvatarBuilderGender::Male, bool bFinal = true);
	
	// Thumbnails //
	
	UFUNCTION(BlueprintCallable)
	const FCustomButtonUIData  GetThumbnails(const UDataTable* Table, EAvatarBuilderGender MyGender = EAvatarBuilderGender::Male);
	
	UFUNCTION(BlueprintCallable)
	const FCustomButtonUIData  GetPresetThumbnails(EAvatarBuilderGender MyGender = EAvatarBuilderGender::Male);
	
	UFUNCTION(BlueprintCallable)
	const FCustomButtonUIData GetAccessoryThumbnails(const UDataTable* Table, EShibAccessory AccessoryType = EShibAccessory::NONE, EAvatarBuilderGender MyGender = EAvatarBuilderGender::Male);

	UFUNCTION(BlueprintCallable)
	const FCustomButtonUIData GetTattooThumbnails(const UDataTable* Table, EShibTattoo TattooType = EShibTattoo::NONE, EAvatarBuilderGender MyGender = EAvatarBuilderGender::Male);


	
	// Morph Data
	UFUNCTION(BlueprintCallable)
	void ApplyBodyMorphData(EShibBodyMorphs Type, const FBase& Base = FBase(), const float& Value = 0.5f, const bool& bFinal = false);
	

	// Body Meshes
	UFUNCTION(BlueprintCallable)
	void ApplyNewBodyMesh(EShibMesh Type, const FMesh& Mesh = FMesh(), bool bFinal = true);
	
	UFUNCTION(BlueprintCallable)
	USkeletalMeshComponent* GetMeshHolders(EShibMesh Type);

	
	// Accessories
	UFUNCTION(BlueprintCallable)
	void ApplyNewAccessory(const FMesh& Mesh = FMesh(), bool bFinal = true);
	
	UFUNCTION(BlueprintCallable)
	USkeletalMeshComponent* GetAccessoryHolder(EShibAccessory Type);

	
	// Applies ShibTexture
	UFUNCTION(BlueprintCallable)
	void ApplyNewMaterialValue(const FShibTexture& Texture = FShibTexture(), bool bFinal = false);

	
	// Applies Shib Color
	UFUNCTION(BlueprintCallable)
	void ApplyNewColor(USkeletalMeshComponent* Holder, const FShibColor& Texture, const FLinearColor& Color, bool bFinal);

	
	// Randomization
	UFUNCTION(BlueprintCallable)
	void BuildRandomAvatar();


	// Undo/Redo //
	
	UFUNCTION(BlueprintCallable)
	void Undo();
	
	UFUNCTION(BlueprintCallable)
	void Redo();
	
	
	// Changes the bodies skeletal mesh based on the applied meshes
	UFUNCTION(BlueprintCallable)
	void UpdateSkmBasedOnMesh();

	// Sets undergarments if clothes aren't applied on either upper or lower body.
	UFUNCTION(BlueprintCallable)
	void UpdateUndergarments();

	
	// Checks if there are any changes/differences between the initially loaded avatar and the current avatar
	UFUNCTION(BlueprintPure)
	bool CheckAvatarChanged();

	// Delegates //
	UPROPERTY(BlueprintAssignable)
	FOnGameReady OnGameReady;
	UPROPERTY(BlueprintAssignable)
	FOnAvatarLoaded OnAvatarLoaded;
	UPROPERTY(BlueprintAssignable)
	FOnAvatarLoaded OnGenderSwapped;


	//Setters//
	
	UFUNCTION(BlueprintCallable)
	void SetShibController(AShibController* InShibController) { ShibController = InShibController; }

	UFUNCTION(BlueprintCallable)
	void SetGender(EAvatarBuilderGender NewGender) { Gender = NewGender; }

	UFUNCTION(BlueprintCallable)
	void SetShibSaveGame(UShibSaveGame* InShibSaveGame) { ShibSaveGame = InShibSaveGame; }
	
	UFUNCTION(BlueprintCallable)
	void SetAvatarCharacter(AShibAvatarCharacter* InAvatarCharacter, bool bInShouldPlayMontage = true)
	{
		AvatarCharacter = InAvatarCharacter;
		bShouldPlayMontage = bInShouldPlayMontage;
	}
	
	//Getters
	
	UFUNCTION(BlueprintCallable)
	const AShibController* GetShibController() const { return ShibController; }

	UFUNCTION(BlueprintCallable)
	const EAvatarBuilderGender GetGender() const { return Gender; }
	
	UFUNCTION(BlueprintCallable)
	const UShibSaveGame* GetShibSaveGame() const { return ShibSaveGame; }
	
	UFUNCTION(BlueprintPure)
	AShibAvatarCharacter* GetAvatarCharacter();

	
private:

	
	UPROPERTY()
	TObjectPtr<AShibController> ShibController;

	UPROPERTY()
	TObjectPtr<AShibAvatarCharacter> AvatarCharacter;

	// Saves changes made to avatar, contains the current state of the avatar
	UPROPERTY()
	TObjectPtr<UShibSaveGame> ShibSaveGame;

	// Saves avatar state (from Shib Save Game Cache) into a stack for undoing and redoing changes
	UPROPERTY()
	TObjectPtr<UAvatarHistory> AvatarHistory;
	
	// Value should only be set once when avatar is first loaded in 
	UPROPERTY()
	FAvatarSave InitialAvatarSaveData;
	
	UPROPERTY()
	EAvatarBuilderGender Gender{EAvatarBuilderGender::Male};

	UPROPERTY()
	bool bShouldPlayMontage = true;
	

	// Body Morphs
	void UpdateMeshesBasedOnMorphWeight(const float Weight);

	void ApplyNewWeight(const FBase& Base = FBase(), const float& Weight = 0.5f, const bool& bFinal = false);
	void ApplyNewHeight(float Value, bool bFinal);
	
	TArray<FName> GetFacialMorphNames(const FBase& Base = FBase(), EShibBodyMorphs Type = EShibBodyMorphs::NONE);
	TArray<FName> GetBeardMorphNames(const FBase& Base = FBase(), EShibBodyMorphs Type = EShibBodyMorphs::NONE);
	
	// Body Meshes
	void RemoveBodyMesh(EShibMesh Type, bool bFinal);
	void RemoveConflicts(const FMesh& Mesh, bool bFinal);
	bool CheckConflicts(const FMesh& Mesh, bool bFinal);
	void ManageLowerBodySense(const FMesh& Mesh,float Weight);

	
	// Accessory
	void RemoveAccessoryMesh(EShibAccessory Type, bool bFinal);

	// Preset
	void ApplyPresetMI(const FShibTexture& Texture);
	
	
	// Randomizer
	void ApplyRandomMeshWithColor(EShibMesh MeshType);
	void ApplyRandomAccessory(EShibAccessory AccessoryType);
	void ApplyRandomBaseMorphs(const TArray<FName>& Bases);
	void ApplyRandomTattoo(EShibTattoo TattooType);
	void ApplyRandomTextures();
	FLinearColor GenerateRandomColor(float MinColorRange = 0.1f, float MaxColorRange = 1.0f)
	{
		return FLinearColor(
			FMath::RandRange(MinColorRange, MaxColorRange), // Red
			FMath::RandRange(MinColorRange, MaxColorRange), // Green
			FMath::RandRange(MinColorRange, MaxColorRange), // Blue
			1.0f // Alpha (fully opaque)
		);
	}

public:
	// statics
	UFUNCTION()
	static void SetHeight(AShibAvatarCharacter* Character, float Value);

	UFUNCTION()
	static void ApplyNewTexture(UMaterialInstanceDynamic* Material, const TArray<FName> Parameters, const TArray<UTexture*>& Textures);

	UFUNCTION()
	static void ApplyNewMorphs(USkeletalMeshComponent* Component, USkeletalMesh* Mesh, const TArray<FName> Morph, const float& Value);

	UFUNCTION()
	static void ApplyNewMesh(USkeletalMeshComponent* Holder, USkeletalMesh* NewMesh);

	// make sure the body DMI is already set before calling this function so we can copy from it
	UFUNCTION()
	static void SetBodyDynMatInstAndCopyParams(const AShibAvatarCharacter* InAvatarCharacter, UMaterialInterface* NewMaterial);
};



template<typename TStruct>
inline bool IsMatchingID(UDataTable* DataTable, const int32& CurrentMeshID, EAvatarBuilderGender InGender, TStruct& OutData)
{
	if (!DataTable) return false;
	LOG_SHIB(Warning, "[%s] is valid!", *DataTable->GetName());
	const TArray<FName> Rows = DataTable->GetRowNames();
	for (const auto& Row : Rows)
	{
		const TStruct* Data = DataTable->FindRow<TStruct>(Row, TEXT(""));
		if (Data && Data->Gender == InGender && Data->MeshID == CurrentMeshID)
		{
			OutData = *Data;
			LOG_SHIB(Warning, "Mesh [%s]", *Data->Mesh->GetName());
			return true;
		}
	}

	return false;
}

template<typename TStruct, typename TEnum>
inline bool IsMatchingIDForAllType(UDataTable* DataTable, const int32& CurrentMeshID, EAvatarBuilderGender InGender, TStruct& OutData = TStruct(), TEnum Type = TEnum())
{
	if (!DataTable) return false;
	LOG_SHIB(Log, "[%s] is valid!", *DataTable->GetName());
	const TArray<FName> Rows = DataTable->GetRowNames();
	for (const auto& Row : Rows)
	{
		const TStruct* Data = DataTable->FindRow<TStruct>(Row, TEXT(""));
		if(Data && Data->Mesh)
		{
			if (Data->bAccessory)
			{
				if ( Data->Gender == InGender && Data->MeshID == CurrentMeshID && Data->AccessoryType == Type)
				{
					OutData = *Data;
					LOG_SHIB(Log, "Mesh [%s]", *Data->Mesh->GetName());
					return true;
				}
			}
			else
			{
			
				if ( Data->Gender == InGender && Data->MeshID == CurrentMeshID)
				{
					OutData = *Data;
					LOG_SHIB(Log, "Mesh [%s]", *Data->Mesh->GetName());
					return true;
				}
			}
		}
		
	}

	return false;
}

