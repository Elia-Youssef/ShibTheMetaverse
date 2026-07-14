// Copyright Shiba Inu Games LLC.

#pragma once

#include "CoreMinimal.h"
#include "AvatarBuilderLibrary.h"
#include "OperationsManager.h"
#include "ShibAvatarCharacter.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "AvatarBuilderSubsystem.generated.h"


// Delegate to Update currently selected Sliders and Buttons .
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnItemApplied, FCurrentAvatarState, CurrentState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnColorExtractedFromButton, FLinearColor, Color);

USTRUCT(BlueprintType)
struct FUIButtonsData
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TObjectPtr<UDataTable> DataTable;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FName RowName;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TMap<FGenderEntryHandle, TObjectPtr<UTexture2D>> Thumbnails;
};

UCLASS(Blueprintable)
class SHIBAVATARBUILDER_API UAvatarBuilderSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

	UAvatarBuilderSubsystem()
	{
		Gender.Gender = TEXT("Male");
		BodyType.BodyType = TEXT("Fit");

		if (!AvatarOperationsManager)
		{
			AvatarOperationsManager = CreateDefaultSubobject<UAvatarOperationsManager>(TEXT("AvatarOperationsManager"));
		}
	}

	template <typename T>
	void ApplyTypedItem(UDataTable* DataTable, FName RowName, bool bFinal = true)
	{
		if (!DataTable)
		{
			return;
		}

		T* Item = DataTable->FindRow<T>(RowName, TEXT("Finding Row"));
		if (Item)
		{
			UE_LOG(LogTemp, Warning, TEXT("FName: %s"), *RowName.ToString());
			UAvatarBuilderSubsystem::ApplyItem(*Item, RowName, bFinal);
		}
	}

	template <typename T>
	void AssignThumbnailsFromItem(const T* Item, FUIButtonsData& ButtonData)
	{
		if (!Item)
		{
			UE_LOG(LogTemp, Warning, TEXT("AssignThumbnailsFromItem: Null Item"));
			return;
		}

		if constexpr (std::is_same_v<T, FPresetItem>)
		{
			if (Item->MaterialAndThumbnailData.IsEmpty())
			{
				UE_LOG(LogTemp, Warning, TEXT("AssignThumbnailsFromPresetItem: Empty MaterialAndThumbnailData"));
				return;
			}

			if (CurrentSkin < 0 || CurrentSkin >= Item->MaterialAndThumbnailData.Num())
			{
				UE_LOG(LogTemp, Warning, TEXT("AssignThumbnailsFromPresetItem: Invalid CurrentSkin index"));
				return;
			}

			auto& ArrayIndex = Item->MaterialAndThumbnailData[CurrentSkin];

			for (const auto& Entry : ArrayIndex.MaterialByGender)
			{
				FGenderEntryHandle UnisexGenderEntry;
				UnisexGenderEntry.Gender = "Unisex";

				FGenderEntryHandle MaleGenderEntry;
				MaleGenderEntry.Gender = "Male";

				FGenderEntryHandle FemaleGenderEntry;
				FemaleGenderEntry.Gender = "Female";

				if (Entry.Key == UnisexGenderEntry)
				{
					if (Entry.Value.MaterialThumbnail != nullptr)
					{
						ButtonData.Thumbnails.Add(MaleGenderEntry, Cast<UTexture2D>(Entry.Value.MaterialThumbnail));
						ButtonData.Thumbnails.Add(FemaleGenderEntry, Cast<UTexture2D>(Entry.Value.MaterialThumbnail));
					}
				}
				else
				{
					if (Entry.Value.MaterialThumbnail != nullptr)
					{
						ButtonData.Thumbnails.Add(Entry.Key, Cast<UTexture2D>(Entry.Value.MaterialThumbnail));
					}
				}
			}
		}
		else
		{
			for (const auto& Entry : Item->Thumbnails)
			{
				FGenderEntryHandle UnisexGenderEntry;
				UnisexGenderEntry.Gender = "Unisex";

				FGenderEntryHandle MaleGenderEntry;
				MaleGenderEntry.Gender = "Male";

				FGenderEntryHandle FemaleGenderEntry;
				FemaleGenderEntry.Gender = "Female";

				if (Entry.Key == UnisexGenderEntry)
				{
					ButtonData.Thumbnails.Add(MaleGenderEntry, Entry.Value);
					ButtonData.Thumbnails.Add(FemaleGenderEntry, Entry.Value);
				}
				else
				{
					ButtonData.Thumbnails.Add(Entry.Key, Entry.Value);
				}
			}
		}
	}

	UPROPERTY()
	int CurrentSkin = 0;

	UPROPERTY()
	FCurrentAvatarState TemporaryStateForPreview;

	UFUNCTION(BlueprintCallable)
	void ApplyButtonItem(FUIButtonsData UIButton);

	void ApplyItem(FMeshItem& MeshItem, FName RowName, bool bFinal = true);
	void ApplyItem(FAccessoryItem& AccessoryItem, FName RowName, bool bFinal = true);
	void ApplyItem(FTextureItem& TextureItem, FName RowName, bool bFinal = true);
	void ApplyItem(FPresetItem& PresetItem, FName RowName, bool bFinal = true);

	void SetBodyDynMatInstAndCopyParams(const AShibAvatarCharacter* InAvatarCharacter, UMaterialInterface* NewMaterial);

	UFUNCTION(BlueprintCallable)
	USkeletalMeshComponent* GetMeshHolders(FPrimitiveItem& Item);
	USkeletalMeshComponent* GetMeshHoldersByCategory(const FName& Category);
	void ClearAllMeshHolders();
	void ClearAllAccessoryHolders();
	void ClearAllTattoos();

	UFUNCTION(BlueprintCallable)
	AShibAvatarCharacter* GetAvatarCharacter();

	UPROPERTY()
	TObjectPtr<UAvatarOperationsManager> AvatarOperationsManager;

	FGenderEntryHandle Gender;
	FBodyTypeEntryHandle BodyType;

	TMap<FName, float> LastMorphValues = {
		{TEXT("nONE"), 0.5f},
		{TEXT("avatarHeight"), 0.5f},
		{TEXT("avatarWeight"), 0.5f},
		{TEXT("jawLength"), 0.5f},
		{TEXT("jawWidth"), 0.5f},
		{TEXT("jawHeight"), 0.5f},
		{TEXT("noseLength"), 0.5f},
		{TEXT("noseWidth"), 0.5f},
		{TEXT("noseHeight"), 0.5f},
		{TEXT("mouthThickness"), 0.5f},
		{TEXT("mouthWidth"), 0.5f},
		{TEXT("mouthHeight"), 0.5f},
		{TEXT("accessoriesNoseLength"), 0.5f},
		{TEXT("accessoriesNoseWidth"), 0.5f},
		{TEXT("accessoriesNoseHeight"), 0.5f}
	};

	TArray<FName> PresetNames = {
		"_Caucasian", "_African", "_Chinese", "_Indian", "_Japenese", "_PuertoRican", "_Russian"
	};


	TArray<FName> GetPresetMorphNames();

	// Facial morphs are set in the default avatar
	FAvatarFacialMorphs GetFacialMorphs();

	// Utility function that applies morphs if there is a single morphTarget (i.e. Presets) or a pair of morph targets (i.e. Overweight+Thin)
	void ApplyMorph(USkeletalMeshComponent* Holder, TArray<FName> MorphTargets, float Value);
	void ApplyMorph(USkeletalMeshComponent* Holder, FName MorphTarget, float Value);

	// Gets the weight value based on BodyType
	float GetBodyWeight();

	// Gets mapped value of body morph, appropriate to the old body morph keys in the json save file
	FName GetMappedJsonBodyMorphName(FAvatarFacialFeatureHandle Feature, FAvatarDimensionHandle Dimension);
	void GetFeatureAndDimensionFromMappedJsonBodyMorphName(const FName& MappedName,
	                                                       FAvatarFacialFeatureHandle& OutFeature,
	                                                       FAvatarDimensionHandle& OutDimension);

public:
	UFUNCTION(BlueprintCallable)
	void SetCurrentSkin(int index)
	{
		CurrentSkin = index;
		UpdateCurrentPresetSkin();
	}

	UFUNCTION(BlueprintCallable)
	int GetCurrentSkin()
	{
		if (AvatarOperationsManager->GetCurrentState().ShibTextureAssetRowNameData.Contains("skin"))
		{
			return OldSkintoNew[AvatarOperationsManager->GetCurrentState().ShibTextureAssetRowNameData["skin"]];
		}
		return 0;
	}

	UPROPERTY()
	FName AppliedPresetRow;

	bool bStopAddToState = false;

	void UpdateCurrentPresetSkin();

	UFUNCTION(BlueprintCallable)
	void AvatarUndo() { AvatarOperationsManager->Undo(); }

	UFUNCTION(BlueprintCallable)
	void AvatarRedo() { AvatarOperationsManager->Redo(); }

	UFUNCTION(BlueprintCallable)
	void ApplyNewMesh(USkeletalMeshComponent* Holder, USkeletalMesh* NewMesh);

	UFUNCTION(BlueprintCallable)
	void ApplyNewTexture(UMaterialInstanceDynamic* Material, const TArray<FName> Parameters,
	                     const TArray<UTexture*>& Textures);

	UFUNCTION(BlueprintCallable)
	void RemoveItem(FName ItemCategory, FName ItemSubCategory);

	bool bCustomizationInProgress = false;

	UFUNCTION(BlueprintCallable)
	void ApplyAvatarCustomization(FCurrentAvatarState AvatarState);

	UFUNCTION(BlueprintCallable)
	void ApplyDataTableRow(UDataTable* DataTable, FName RowName, bool bFinal = true);

	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<AShibAvatarCharacter> AvatarCharacter;

	/** Sets the gender and triggers delegate */
	UFUNCTION(BlueprintCallable)
	void SetGender(FGenderEntryHandle NewGender, bool bFinal);

	/** Gets the current gender */
	UFUNCTION(BlueprintCallable)
	FGenderEntryHandle GetGender() const { return Gender; }

	UPROPERTY()
	int SetBodyTypeCalls = 0;
	/** Sets the body type */
	UFUNCTION(BlueprintCallable)
	void SetBodyType(float Weight, bool bFinal);

	void TemporaryApplyMeshes(bool bFinal);

	/** Gets the current body type */
	UFUNCTION(BlueprintCallable)
	FBodyTypeEntryHandle GetBodyType() const { return BodyType; }

	UFUNCTION(BlueprintCallable)
	FName GetBodyTypeName() const { return BodyType.BodyType; }

	UFUNCTION(BlueprintCallable)
	TArray<FUIButtonsData> GetButtonsDataFromDataTable(UDataTable* WearableDataTable);

	void UpdateSKM(USkeletalMesh* NewMesh, TObjectPtr<UAnimMontage> Montage);

	FCurrentAvatarState TattooContainerTempSave;

	USkeletalMesh* GetDefaultBase();

	UFUNCTION(BlueprintCallable)
	void TemporaryAddUndergarments();

	UFUNCTION(BlueprintCallable)
	void RemoveTemporaryUndergarments();

	void ApplyWeightMorph();

	UFUNCTION(BlueprintCallable)
	void AvatarLoad();

	UFUNCTION(BlueprintCallable)
	void AvatarSave();

	// Morph Data
	UFUNCTION(BlueprintCallable)
	void ApplyBodyMorph(float SliderValue, FAvatarFacialFeatureHandle Feature, FAvatarDimensionHandle Dimension,
	                    bool bFinal);

	// Height Data
	UFUNCTION(BlueprintCallable)
	void ApplyHeightMorph(float SliderValue, bool bFinal);

	// Color Data
	UFUNCTION(BlueprintCallable)
	void ApplyTextureColors(USkeletalMeshComponent* Holder, const FName& ItemName, const FMaterialCustomization& Color,
	                        bool bFinal, bool IsTattoo = false);

	// Color Picker Data
	UFUNCTION(BlueprintCallable)
	void ApplyMeshColor(UDataTable* ItemTable, const FName& ItemRow, const FLinearColor& Color, bool bFinal);

	// Color Picker Data
	UFUNCTION(BlueprintCallable)
	bool ColorPickerEnable(UDataTable* ItemTable, const FName& ItemRow);

	// Delegate for Item Applying Update
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnItemApplied OnItemApplied;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnColorExtractedFromButton OnColorExtractedFromButton;

	UFUNCTION(BlueprintCallable)
	FCurrentAvatarState GetCurrentState() { return AvatarOperationsManager->GetCurrentState(); }

	UFUNCTION(BlueprintCallable)
	void SetColorAccordingToType(FUIButtonsData ButtonData, FLinearColor Color, bool bFinal);

	UFUNCTION(BlueprintCallable)
	TArray<UDataTable*> GetAllItemDataTables();
	bool HasConflict(FItemCompatibility ItemCompatibility, FCurrentAvatarState CurrentState, FName ApplyingCategory);

	UFUNCTION(BlueprintCallable)
	void GenerateRandomAvatar();
	bool IsShoeOrBottomConflict(FMeshItem* MeshItem);
	bool CheckForConflicts(TArray<FItemCompatibility>& CompatibilityRules, FCurrentAvatarState& CurrentState,
	                       FName ApplyingCategory);

	UFUNCTION(BlueprintCallable)
	FLinearColor GetColorFromButton(FUIButtonsData ButtonsData);
};
