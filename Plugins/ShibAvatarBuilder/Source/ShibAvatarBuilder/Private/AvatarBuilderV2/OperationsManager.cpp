// Copyright Shiba Inu Games LLC.


#include "AvatarBuilderV2/OperationsManager.h"
#include "AvatarBuilderV2/AvatarBuilderSubsystem.h"
#include "Kismet/GameplayStatics.h"

DEFINE_LOG_CATEGORY_STATIC(LogAvatarState, Log, All);
#define LOAD_MONTAGE_FROM_PATH(Path) Cast<UAnimMontage>(StaticLoadObject(UAnimMontage::StaticClass(), nullptr, Path))

#define MaleMontage LOAD_MONTAGE_FROM_PATH(TEXT("/ShibAvatarBuilder/Assets/AvatarAssets/MasBodyAssets/AM_FATSO_MALE_Montage"))
#define FemaleMontage LOAD_MONTAGE_FROM_PATH(TEXT("/ShibAvatarBuilder/Assets/AvatarAssets/FemBodyAssets/AM_FATSO_FEMALE_Montage"))


void UAvatarOperationsManager::Initialize(const FCurrentAvatarState& InitialState)
{
	CurrentState = InitialState;
	UndoStack.Push(CurrentState.ToJson());
	ApplyState();
}

void UAvatarOperationsManager::ApplyState()
{
	// Retrieve Avatar Builder Subsystem
	UAvatarBuilderSubsystem* AvatarBuilder = GetWorld()->GetGameInstance()->GetSubsystem<UAvatarBuilderSubsystem>();
	if (!AvatarBuilder) return;

	// Apply the current state to the Avatar Builder
	AvatarBuilder->ApplyAvatarCustomization(CurrentState);	
}

void UAvatarOperationsManager::SaveState()
{
	FString NewStateJson = CurrentState.ToJson();

	// Ensure the stack isn't empty and avoid adding duplicate states
	if (UndoStack.Num() == 0 || UndoStack.Top() != NewStateJson)
	{
		UndoStack.Push(MoveTemp(NewStateJson));
		RedoStack.Empty(); // Clear redo stack when a new state is saved
	}

	PrintCurrentState();
}

void UAvatarOperationsManager::AddToState(FCurrentAvatarState AvatarState)
{
	CurrentState=AvatarState;
	
	SaveState();
}

FCurrentAvatarState UAvatarOperationsManager::AddToStateTemporary(FCurrentAvatarState AvatarState)
{
	FCurrentAvatarState TemporaryState=CurrentState;
	
	// Update simple members if they are valid
	if (!AvatarState.AvatarBaseRow.IsNone())
	{
		TemporaryState.AvatarBaseRow = AvatarState.AvatarBaseRow;
	}

	// Only update `bIsMale` if the provided value is different (avoids unnecessary writes)
	if (AvatarState.bIsMale != CurrentState.bIsMale)
	{
		TemporaryState.bIsMale = AvatarState.bIsMale;
	}

	// Append or replace only non-empty map data
	if (!AvatarState.MeshAssetRowNameData.IsEmpty())
	{
		for (const auto& Pair : AvatarState.MeshAssetRowNameData)
		{
			TemporaryState.MeshAssetRowNameData.Add(Pair.Key, Pair.Value);
		}
	}

	if (!AvatarState.ShibTextureAssetRowNameData.IsEmpty())
	{
		for (const auto& Pair : AvatarState.ShibTextureAssetRowNameData)
		{
			TemporaryState.ShibTextureAssetRowNameData.Add(Pair.Key, Pair.Value);
		}
	}

	if (!AvatarState.AccessoryRowNameData.IsEmpty())
	{
		for (const auto& Pair : AvatarState.AccessoryRowNameData)
		{
			TemporaryState.AccessoryRowNameData.Add(Pair.Key, Pair.Value);
		}
	}

	if (!AvatarState.TattooRowNameData.IsEmpty())
	{
		for (const auto& Pair : AvatarState.TattooRowNameData)
		{
			TemporaryState.TattooRowNameData.Add(Pair.Key, Pair.Value);
		}
	}

	if (!AvatarState.MorphData.IsEmpty())
	{
		for (const auto& Pair : AvatarState.MorphData)
		{
			TemporaryState.MorphData.Add(Pair.Key, Pair.Value);
		}
	}

	if (!AvatarState.ShibMeshColorData.IsEmpty())
	{
		for (const auto& Pair : AvatarState.ShibMeshColorData)
		{
			TemporaryState.ShibMeshColorData.Add(Pair.Key, Pair.Value);
		}
	}

	if (!AvatarState.ShibTextureColorData.IsEmpty())
	{
		for (const auto& Pair : AvatarState.ShibTextureColorData)
		{
			TemporaryState.ShibTextureColorData.Add(Pair.Key, Pair.Value);
		}
	}

	if (!AvatarState.AccessoryShibColorData.IsEmpty())
	{
		for (const auto& Pair : AvatarState.AccessoryShibColorData)
		{
			TemporaryState.AccessoryShibColorData.Add(Pair.Key, Pair.Value);
		}
	}

	if (!AvatarState.TattooShibColorData.IsEmpty())
	{
		for (const auto& Pair : AvatarState.TattooShibColorData)
		{
			TemporaryState.TattooShibColorData.Add(Pair.Key, Pair.Value);
		}
	}

	return TemporaryState;
}

void UAvatarOperationsManager::PopUndoStack()
{
	if (UndoStack.Num() == 1) return;

	UndoStack.Pop();
}

void UAvatarOperationsManager::Undo()
{
	if (UndoStack.Num() == 1) return;

	UAvatarBuilderSubsystem* AvatarBuilder = GetWorld()->GetGameInstance()->GetSubsystem<UAvatarBuilderSubsystem>();
	AvatarBuilder->SetBodyTypeCalls=0;
	
	RedoStack.Push(UndoStack.Pop()); // Save current state to redo stack
	FCurrentAvatarState PreviousState;
	if (FCurrentAvatarState::FromJson(UndoStack.Top(), PreviousState))
	{
		CurrentState = PreviousState;
		ApplyState();
	}
}


void UAvatarOperationsManager::RemoveStoppedEntryFromStack()
{
	if (UndoStack.Num() > 1)
	{
		UndoStack.RemoveAt(UndoStack.Num() - 2);
	}
}


void UAvatarOperationsManager::Redo()
{
	if (RedoStack.Num() == 0) return;

	UAvatarBuilderSubsystem* AvatarBuilder = GetWorld()->GetGameInstance()->GetSubsystem<UAvatarBuilderSubsystem>();
	AvatarBuilder->SetBodyTypeCalls=0;

	UndoStack.Push(RedoStack.Pop()); // Move the top redo state back to undo stack
	FCurrentAvatarState NextState;
	if (FCurrentAvatarState::FromJson(UndoStack.Top(), NextState))
	{
		CurrentState = NextState;
		ApplyState();
	}
}

void UAvatarOperationsManager::UpdateBaseMesh()
{
    FString TablePath = TEXT("/ShibAvatarBuilder/V2/DataTables/DT_SkeletalBodyVariations_V2");
    UAvatarBuilderSubsystem* AvatarBuilder = GetWorld()->GetGameInstance()->GetSubsystem<UAvatarBuilderSubsystem>();

    if (!AvatarBuilder)
    {
        UE_LOG(LogAvatarState, Error, TEXT("AvatarBuilderSubsystem is NULL!"));
        return;
    }

    UDataTable* DT = Cast<UDataTable>(StaticLoadObject(UDataTable::StaticClass(), nullptr, *TablePath));
    if (!DT)
    {
        UE_LOG(LogAvatarState, Error, TEXT("DataTable not found at path: %s"), *TablePath);
        return;
    }

    TArray<FName> RowNames = DT->GetRowNames();
    if (RowNames.IsEmpty())
    {
        UE_LOG(LogAvatarState, Error, TEXT("DataTable contains no rows!"));
        return;
    }

    // Default skeletal mesh
    FAvatarSkeletalBodyVariations* DefaultVariation = DT->FindRow<FAvatarSkeletalBodyVariations>(RowNames[0], TEXT("Finding Default Variation"));
    if (!DefaultVariation)
    {
        UE_LOG(LogAvatarState, Error, TEXT("Failed to find default Avatar Skeletal Body Variation!"));
        return;
    }

    USkeletalMesh* BestMesh = DefaultVariation->BodyVariations[AvatarBuilder->GetGender()];
    int32 MaxMatchedCombinations = 0;

    // Iterate through Skeletal Body Variations
    for (const FName& RowName : RowNames)
    {
        FAvatarSkeletalBodyVariations* SkeletalBodyVariations = DT->FindRow<FAvatarSkeletalBodyVariations>(RowName, TEXT("Finding AvatarSkeletalBodyVariations Row"));
        if (!SkeletalBodyVariations) continue;

        int32 TotalCombinations = SkeletalBodyVariations->ClothingCombinations.Num();
        int32 MatchedCombinations = 0;

        //UE_LOG(LogAvatarState, Warning, TEXT("Checking Variation: %s (Total Required Combinations: %d)"), *RowName.ToString(), TotalCombinations);

        if (!SkeletalBodyVariations->ClothingCombinations.IsEmpty())
        {
            for (auto Combination : SkeletalBodyVariations->ClothingCombinations)
            {
                if (!CurrentState.MeshAssetRowNameData.Contains(Combination.ClothingType.ClothingType))
                {
                   // UE_LOG(LogAvatarState, Warning, TEXT("Character does not have required clothing type: %s"), *Combination.ClothingType.ClothingType.ToString());
                    continue;
                }

                FItemCompatibility ItemCompatibility;
                ItemCompatibility.bClothing = true;
                ItemCompatibility.ClothingType = Combination.ClothingType;
                UDataTable* DataTable = FindDataTableByCompatibilityItem(ItemCompatibility);

                if (!DataTable)
                {
                    //UE_LOG(LogAvatarState, Error, TEXT("Could not find DataTable for clothing type: %s"), *Combination.ClothingType.ClothingType.ToString());
                    continue;
                }

                // Get selected subtypes for the current combination
                TArray<FName> CombinationSelectedSubtypes;
                for (const auto& Subtype : Combination.SelectSubtype)
                {
                    if (Subtype.bIsChecked)
                    {
                        CombinationSelectedSubtypes.Add(Subtype.OptionName);
                    }
                }

                // Find clothing item in the table
                FClothingItem* ClothingItem = DataTable->FindRow<FClothingItem>(
                    CurrentState.MeshAssetRowNameData[Combination.ClothingType.ClothingType],
                    TEXT("Finding ClothingItem Row")
                );

                if (!ClothingItem)
                {
                    //UE_LOG(LogAvatarState, Error, TEXT("Could not find clothing item row for: %s"), *Combination.ClothingType.ClothingType.ToString());
                    continue;
                }

                // Check if selected subtypes match the clothing item subtypes
                bool bMatched = false;
                for (const auto& Subtype : ClothingItem->SelectSubtype)
                {
                    if (Subtype.bIsChecked && CombinationSelectedSubtypes.Contains(Subtype.OptionName))
                    {
                        MatchedCombinations++;
                        bMatched = true;
                        //UE_LOG(LogAvatarState, Warning, TEXT("Matched Combination: %s -> Subtype: %s"), 
                      //         *Combination.ClothingType.ClothingType.ToString(), *Subtype.OptionName.ToString());
                        break;
                    }
                }

                // If both ClothingItem and Combination have no subtypes, check by ClothingType match
                if (!bMatched && Combination.SelectSubtype.IsEmpty() && ClothingItem->SelectSubtype.IsEmpty() &&
                    Combination.ClothingType.ClothingType == ClothingItem->ClothingType.ClothingType)
                {
                    MatchedCombinations++;
                   // UE_LOG(LogAvatarState, Warning, TEXT("Matched Clothing Type without subtypes: %s"), *Combination.ClothingType.ClothingType.ToString());
                }
            }

            // Update best match if it's better than the previous
            if (MatchedCombinations == TotalCombinations && MatchedCombinations > MaxMatchedCombinations)
            {
                MaxMatchedCombinations = MatchedCombinations;
            	FGenderEntryHandle GenderKey = AvatarBuilder->GetGender();

            	if (SkeletalBodyVariations->BodyVariations.Contains(GenderKey))
            	{
            		BestMesh = SkeletalBodyVariations->BodyVariations[GenderKey];
            	}
            	else
            	{
            		UE_LOG(LogTemp, Error, TEXT("GetBodyVariation: No body variation found for gender: %s"), *GenderKey.Gender.ToString());
            		BestMesh = nullptr; // Prevents crash by setting to null
            	}

                //UE_LOG(LogAvatarState, Warning, TEXT("New Best Match Found: %s (Matched: %d)"), *RowName.ToString(), MatchedCombinations);
            }
        }
    }

    // Apply the best matched skeletal mesh
	if(BestMesh!=nullptr && (IsValid(BestMesh)))
	{
		UE_LOG(LogAvatarState, Warning, TEXT("Final Selected Skeletal Mesh: %s"), *BestMesh->GetName());
	}
	AvatarBuilder->AvatarCharacter->SetCurrentMontagePosition();
	if (AvatarBuilder->GetGender().Gender=="Male")
	{
		AvatarBuilder->UpdateSKM(BestMesh,MaleMontage);
	}
	else if (AvatarBuilder->GetGender().Gender=="Female")
	{
		AvatarBuilder->UpdateSKM(BestMesh,FemaleMontage);
	}

	EnsureUndergarments();
}

FName UAvatarOperationsManager::ApplyUndergarments(UAvatarBuilderSubsystem* AvatarBuilder, const FString& TablePath, const FString& CategoryName, USkeletalMeshComponent* TargetComponent)
{
	if (!AvatarBuilder || !TargetComponent)
	{
		return "";
	}
    
	if (UDataTable* CategoryDT = Cast<UDataTable>(StaticLoadObject(UDataTable::StaticClass(), nullptr, *TablePath)))
	{
		FName RowName = "Clothing";
		if (FItemCategoryStruct* ItemCategoryStruct = CategoryDT->FindRow<FItemCategoryStruct>(RowName, TEXT("Finding Row")))
		{
			UDataTable* ClothingDataTable = (ItemCategoryStruct->DataTablesByNames.Find(FName(CategoryName))->Get());
			for (const auto& ClothingRowName : ClothingDataTable->GetRowNames())
			{
				FClothingItem* ClothingItem = ClothingDataTable->FindRow<FClothingItem>(ClothingRowName, TEXT("Finding ClothingItem Row"));
                
				for (const auto& Subtype : ClothingItem->SelectSubtype)
				{
					if (Subtype.bIsChecked && Subtype.OptionName == "Undergarment")
					{
						if (ClothingItem->MeshesByGender.Contains(AvatarBuilder->GetGender()))
						{
							auto Mesh=ClothingItem->MeshesByGender.Find(AvatarBuilder->GetGender())->MeshesByBodyType[AvatarBuilder->GetBodyType()];
							AvatarBuilder->ApplyNewMesh(TargetComponent,
								Mesh);
							return ClothingRowName;
						}
						return "";
					}
				}
			}
		}
	}
	return "";
}

void UAvatarOperationsManager::EnsureUndergarments()
{
	FString TablePath = TEXT("/ShibAvatarBuilder/V2/Enums/DT_ItemCategory");
	UAvatarBuilderSubsystem* AvatarBuilder = GetWorld()->GetGameInstance()->GetSubsystem<UAvatarBuilderSubsystem>();
	
	if ( (AvatarBuilder->AvatarCharacter->GetBottoms()->GetSkeletalMeshAsset()==nullptr) && AvatarBuilder->AvatarCharacter->GetFullBody()->GetSkeletalMeshAsset()==nullptr)
	{
		ApplyUndergarments(AvatarBuilder, TablePath, "Bottom", AvatarBuilder->AvatarCharacter->GetBottoms());
	}
	if (AvatarBuilder->AvatarCharacter->GetTop()->GetSkeletalMeshAsset()==nullptr && AvatarBuilder->AvatarCharacter->GetFullBody()->GetSkeletalMeshAsset()==nullptr)
	{
		ApplyUndergarments(AvatarBuilder, TablePath, "Top", AvatarBuilder->AvatarCharacter->GetTop());
	}
}

void UAvatarOperationsManager::RemoveUneccesaryUndergarments()
{
	UAvatarBuilderSubsystem* AvatarBuilder = GetWorld()->GetGameInstance()->GetSubsystem<UAvatarBuilderSubsystem>();
	
	if (AvatarBuilder->AvatarCharacter->GetBottoms()->GetSkeletalMeshAsset()!=nullptr && AvatarBuilder->AvatarCharacter->GetFullBody()->GetSkeletalMeshAsset()!=nullptr)
	{
		AvatarBuilder->ApplyNewMesh(AvatarBuilder->AvatarCharacter->GetBottoms(),nullptr);
		//CurrentState.MeshAssetRowNameData.Remove("Bottom");
	}
	if (AvatarBuilder->AvatarCharacter->GetTop()->GetSkeletalMeshAsset()!=nullptr && AvatarBuilder->AvatarCharacter->GetFullBody()->GetSkeletalMeshAsset()!=nullptr)
	{
		AvatarBuilder->ApplyNewMesh(AvatarBuilder->AvatarCharacter->GetTop(),nullptr);
		//CurrentState.MeshAssetRowNameData.Remove("Top");
	}
}


UDataTable * UAvatarOperationsManager::FindDataTableByCompatibilityItem(FItemCompatibility& ItemCompatibility)
{
	FString TablePath=TEXT("/ShibAvatarBuilder/V2/Enums/DT_ItemCategory");
	FName IncompatibleCategory=ItemCompatibility.GetIncompatibleItemCategory();
	
	if (UDataTable * DT=Cast<UDataTable>(StaticLoadObject(UDataTable::StaticClass(), nullptr, *TablePath)) )
	{
		TArray<FName> RowNames;
		RowNames=DT->GetRowNames();

		for (FName RowName : RowNames)
		{
			if(FItemCategoryStruct* ItemCategoryStruct=DT->FindRow<FItemCategoryStruct>(RowName,TEXT("Finding Row")))
			{
				if (ItemCategoryStruct->bMultipleDataTables)
				{
					if (ItemCompatibility.bClothing)
					{
						return (ItemCategoryStruct->DataTablesByNames.Find(ItemCompatibility.GetClothingType()))->Get();
					}
				}
				else
				{
					if (RowName.ToString().Contains(IncompatibleCategory.ToString()))
					{
						return ItemCategoryStruct->DataTable;
					}
				}
			}
		}
	}
	
	return nullptr;
}

UDataTable* UAvatarOperationsManager::FindDataTableByName(FName ItemCategory)
{
	FString TablePath=TEXT("/ShibAvatarBuilder/V2/Enums/DT_ItemCategory");
	if (UDataTable * DT=Cast<UDataTable>(StaticLoadObject(UDataTable::StaticClass(), nullptr, *TablePath)) )
	{
		TArray<FName> RowNames;
		RowNames=DT->GetRowNames();

		for (FName RowName : RowNames)
		{
			if(FItemCategoryStruct* ItemCategoryStruct=DT->FindRow<FItemCategoryStruct>(RowName,TEXT("Finding Row")))
			{
				if (ItemCategoryStruct->bMultipleDataTables)
				{
					if (ItemCategoryStruct->DataTablesByNames.Contains(ItemCategory))
					{
						UDataTable * DataTable=ItemCategoryStruct->DataTablesByNames.Find(ItemCategory)->Get();
						if (DataTable)
						{
							UE_LOG(LogAvatarState, Log, TEXT("DataTable Found: %s"),*DataTable->GetName());
							return DataTable;
						}
						UE_LOG(LogAvatarState, Log, TEXT("Cant Find Data Table In Multiple Data Tables"));
					}
				}
				else if (RowName.ToString().Contains(ItemCategory.ToString()))
				{
					if (ItemCategoryStruct->DataTable)
					{
						UDataTable * DataTable=ItemCategoryStruct->DataTable;
						if (DataTable)
						{
							UE_LOG(LogAvatarState, Log, TEXT("DataTable Found: %s"),*DataTable->GetName());
							return DataTable;
						}
					}
				}
			}
		}
	}
	UE_LOG(LogAvatarState, Log, TEXT("Cant Find Data Table of %s"),*ItemCategory.ToString());
	return nullptr;
}

bool UAvatarOperationsManager::CheckCurrentStateForConflicts(FItemCompatibility& ItemCompatibility, FCurrentAvatarState& TempCurrentState, bool& ResolveInConflictingItem,TArray<FDataTableRowHandle> AllowedExceptions,bool bFinal)
{
    UAvatarBuilderSubsystem* AvatarBuilder = GetWorld()->GetGameInstance()->GetSubsystem<UAvatarBuilderSubsystem>();
    if (!AvatarBuilder) return false;

	bool bIsException=false;

	for (auto Exception:AllowedExceptions)
	{
		FMeshItem* MeshItem=Exception.DataTable->FindRow<FMeshItem>(Exception.RowName,"");
		if (ItemCompatibility.bClothing )
		{
			if(TempCurrentState.MeshAssetRowNameData.Contains(ItemCompatibility.GetClothingType()) && Exception.RowName==TempCurrentState.MeshAssetRowNameData[ItemCompatibility.GetClothingType()])
			{
				bIsException=true;
			}
		}
		else if (ItemCompatibility.bAccessory)
		{
			if(TempCurrentState.AccessoryRowNameData.Contains(ItemCompatibility.GetAccessoryType()) && Exception.RowName==TempCurrentState.AccessoryRowNameData[ItemCompatibility.GetAccessoryType()])
			{
				bIsException=true;
			}
		}
		else
		{
			if(TempCurrentState.MeshAssetRowNameData.Contains(ItemCompatibility.GetIncompatibleItemCategory()) && Exception.RowName==TempCurrentState.MeshAssetRowNameData[ItemCompatibility.GetIncompatibleItemCategory()])
			{
				bIsException=true;
			}
		}
	}
	
	if(!bIsException)
	{
		// Handle conflicts for Clothing (with subtype checking)
		if (ItemCompatibility.bClothing)
		{
			return HandleClothingConflicts(ItemCompatibility, TempCurrentState, ResolveInConflictingItem, AvatarBuilder,bFinal);
		}
		if (ItemCompatibility.bAccessory)
		{
			return HandleAccessoryConflicts(ItemCompatibility, TempCurrentState, ResolveInConflictingItem, AvatarBuilder,bFinal);
		}
    
		return HandleGenericMeshConflicts(ItemCompatibility, TempCurrentState, ResolveInConflictingItem, AvatarBuilder,bFinal);
	}
	return false;
}

//  Function for Handling Clothing Conflicts (Only Clothing has SelectSubtype)
bool UAvatarOperationsManager::HandleClothingConflicts(FItemCompatibility& ItemCompatibility, FCurrentAvatarState& TempCurrentState, bool& ResolveInConflictingItem, UAvatarBuilderSubsystem* AvatarBuilder,bool bFinal)
{
    FName ClothingType = FName(ItemCompatibility.GetClothingType().ToString().ToLower());

    if (TempCurrentState.MeshAssetRowNameData.Contains(ClothingType))
    {
        UE_LOG(LogTemp, Warning, TEXT("Potential conflict with clothing type: %s - %s"),
               *ClothingType.ToString(), *TempCurrentState.MeshAssetRowNameData[ClothingType].ToString());

        bool bSubtypeMatch = false;

        //  `SelectSubtype` is only valid for Clothing Items
        UDataTable* DT = FindDataTableByCompatibilityItem(ItemCompatibility);
        if (DT)
        {
            FClothingItem* ClothingItem = DT->FindRow<FClothingItem>(TempCurrentState.MeshAssetRowNameData[ClothingType], "Finding ClothingItem");

            if (ClothingItem && !ItemCompatibility.SelectSubtype.IsEmpty())
            {
                FName ExistingSubtype;
                for (const auto& ClothingSubtype : ClothingItem->SelectSubtype)
                {
                    if (ClothingSubtype.bIsChecked)
                    {
                        ExistingSubtype = ClothingSubtype.OptionName;
                        break;
                    }
                }

                for (const auto& CompatibilitySubtype : ItemCompatibility.SelectSubtype)
                {
                    if (CompatibilitySubtype.bIsChecked && CompatibilitySubtype.OptionName == ExistingSubtype)
                    {
                        bSubtypeMatch = true;
                        break;
                    }
                }
            }
        }

        //  Resolve Conflict Based on Subtype Matching
        if (ItemCompatibility.SelectSubtype.IsEmpty() || bSubtypeMatch)
        {
            if (ItemCompatibility.bExecuteRuleInConflictingItemInstead)
            {
                CurrentState = TempCurrentState;
                AvatarBuilder->ApplyDataTableRow(DT, TempCurrentState.MeshAssetRowNameData[ClothingType],bFinal);
                ResolveInConflictingItem = true;
                return false;
            }

            if (!ItemCompatibility.ResolveByApplyingMesh.IsNull())
            {
                AvatarBuilder->ApplyDataTableRow(
                    const_cast<UDataTable*>((ItemCompatibility.ResolveByApplyingMesh.DataTable).Get()),
                    ItemCompatibility.ResolveByApplyingMesh.RowName,bFinal);
                return true;
            }

            AvatarBuilder->RemoveItem(ItemCompatibility.GetIncompatibleItemCategory(), ItemCompatibility.GetClothingType());
            TempCurrentState.MeshAssetRowNameData.Remove(ClothingType);
        }
        return false;
    }
    return false;
}

//  Function for Handling Accessory Conflicts (No SelectSubtype)
bool UAvatarOperationsManager::HandleAccessoryConflicts(FItemCompatibility& ItemCompatibility, FCurrentAvatarState& TempCurrentState, bool& ResolveInConflictingItem, UAvatarBuilderSubsystem* AvatarBuilder,bool bFinal)
{
    FName AccessoryType = FName(ItemCompatibility.GetAccessoryType().ToString().ToLower());

    if (TempCurrentState.AccessoryRowNameData.Contains(AccessoryType))
    {
        UE_LOG(LogTemp, Warning, TEXT("Potential conflict with accessory type: %s - %s"),
               *AccessoryType.ToString(), *TempCurrentState.AccessoryRowNameData[AccessoryType].ToString());

        if (ItemCompatibility.bExecuteRuleInConflictingItemInstead)
        {
            CurrentState = TempCurrentState;
            UDataTable* DT = FindDataTableByCompatibilityItem(ItemCompatibility);
            AvatarBuilder->ApplyDataTableRow(DT, TempCurrentState.AccessoryRowNameData[AccessoryType],bFinal);

            ResolveInConflictingItem = true;
            return false;
        }

        if (!ItemCompatibility.ResolveByApplyingMesh.IsNull())
        {
            AvatarBuilder->ApplyDataTableRow(
                const_cast<UDataTable*>((ItemCompatibility.ResolveByApplyingMesh.DataTable).Get()),
                ItemCompatibility.ResolveByApplyingMesh.RowName,bFinal);
            return true;
        }

        AvatarBuilder->RemoveItem(ItemCompatibility.GetIncompatibleItemCategory(), ItemCompatibility.GetAccessoryType());
        TempCurrentState.AccessoryRowNameData.Remove(AccessoryType);
    }
    return false;
}

//  Function for Handling Generic Mesh Conflicts (Hair, etc. - No SelectSubtype)
bool UAvatarOperationsManager::HandleGenericMeshConflicts(FItemCompatibility& ItemCompatibility, FCurrentAvatarState& TempCurrentState, bool& ResolveInConflictingItem, UAvatarBuilderSubsystem* AvatarBuilder,bool bFinal)
{
    FName MeshType = FName(ItemCompatibility.GetIncompatibleItemCategory().ToString().ToLower()); 

    if (TempCurrentState.MeshAssetRowNameData.Contains(MeshType))
    {
        UE_LOG(LogTemp, Warning, TEXT("Potential conflict detected with mesh type: %s - %s"),
               *MeshType.ToString(), *TempCurrentState.MeshAssetRowNameData[MeshType].ToString());

        if (ItemCompatibility.bExecuteRuleInConflictingItemInstead)
        {
            CurrentState = TempCurrentState;
            UDataTable* DT = FindDataTableByCompatibilityItem(ItemCompatibility);
            AvatarBuilder->ApplyDataTableRow(DT, TempCurrentState.MeshAssetRowNameData[MeshType],bFinal);

            ResolveInConflictingItem = true;
            return false;
        }

        if (!ItemCompatibility.ResolveByApplyingMesh.IsNull())
        {
            AvatarBuilder->ApplyDataTableRow(
                const_cast<UDataTable*>((ItemCompatibility.ResolveByApplyingMesh.DataTable).Get()),
                ItemCompatibility.ResolveByApplyingMesh.RowName,bFinal);
            return true;
        }

        AvatarBuilder->RemoveItem(ItemCompatibility.GetIncompatibleItemCategory(), "");
        TempCurrentState.MeshAssetRowNameData.Remove(MeshType);
    }
    return false;
}


void UAvatarOperationsManager::RemoveFromCurrentState(const FCurrentAvatarState& AvatarState)
{
	if (!AvatarState.MeshAssetRowNameData.IsEmpty())
	{
		for (const auto& Pair : AvatarState.MeshAssetRowNameData)
		{
			if(CurrentState.MeshAssetRowNameData.Contains(Pair.Key))
			{
				if (CurrentState.MeshAssetRowNameData[Pair.Key]==Pair.Value)
				{
					CurrentState.MeshAssetRowNameData.Remove(Pair.Key);
				}
			}
		}
	}
}

void UAvatarOperationsManager::PrintCurrentState() const
{
    UE_LOG(LogAvatarState, Log, TEXT("==== Current Avatar State ===="));
    UE_LOG(LogAvatarState, Log, TEXT("Avatar Base: %s"), *CurrentState.AvatarBaseRow.ToString());
    UE_LOG(LogAvatarState, Log, TEXT("Is Male: %s"), CurrentState.bIsMale ? TEXT("True") : TEXT("False"));

    // Print Mesh Assets
    for (const auto& Pair : CurrentState.MeshAssetRowNameData)
    {
        UE_LOG(LogAvatarState, Log, TEXT("Mesh: %s -> %s"), *Pair.Key.ToString(), *Pair.Value.ToString());
    }

    // Print Textures
    for (const auto& Pair : CurrentState.ShibTextureAssetRowNameData)
    {
        UE_LOG(LogAvatarState, Log, TEXT("Texture: %s -> %s"), *Pair.Key.ToString(), *Pair.Value.ToString());
    }

    // Print Accessories
    for (const auto& Pair : CurrentState.AccessoryRowNameData)
    {
        UE_LOG(LogAvatarState, Log, TEXT("Accessory: %s -> %s"), *Pair.Key.ToString(), *Pair.Value.ToString());
    }

    // Print Tattoos
    for (const auto& Pair : CurrentState.TattooRowNameData)
    {
        UE_LOG(LogAvatarState, Log, TEXT("Tattoo: %s -> %s"), *Pair.Key.ToString(), *Pair.Value.ToString());
    }

    // Print Morph Data
    for (const auto& Pair : CurrentState.MorphData)
    {
        UE_LOG(LogAvatarState, Log, TEXT("Morph: %s -> %f"), *Pair.Key.ToString(), Pair.Value);
    }

    // Print Mesh Colors
    for (const auto& Pair : CurrentState.ShibMeshColorData)
    {
        UE_LOG(LogAvatarState, Log, TEXT("Mesh Color: %s -> %s"), *Pair.Key.ToString(), *Pair.Value.ToString());
    }

    // Print Texture Colors
    for (const auto& Pair : CurrentState.ShibTextureColorData)
    {
        UE_LOG(LogAvatarState, Log, TEXT("Texture Color: %s -> %s"), *Pair.Key.ToString(), *Pair.Value.ToString());
    }

    // Print Accessory Colors
    for (const auto& Pair : CurrentState.AccessoryShibColorData)
    {
        UE_LOG(LogAvatarState, Log, TEXT("Accessory Color: %s -> %s"), *Pair.Key.ToString(), *Pair.Value.ToString());
    }

    // Print Tattoo Colors
    for (const auto& Pair : CurrentState.TattooShibColorData)
    {
        UE_LOG(LogAvatarState, Log, TEXT("Tattoo Color: %s -> %s"), *Pair.Key.ToString(), *Pair.Value.ToString());
    }

    UE_LOG(LogAvatarState, Log, TEXT("================================"));
}

void UAvatarOperationsManager::SaveCurrentAvatarData()
{
	// Define the save file path
	FString SaveFilePath = FPaths::ProjectSavedDir() / "SaveGames/AvatarCustomization.json";

	// Try to write to the file
	if (FFileHelper::SaveStringToFile(CurrentState.ToJson(), *SaveFilePath))
	{
		UE_LOG(LogShib, Log, TEXT("Game saved successfully to file: %s"), *SaveFilePath);
	}
	else
	{
		UE_LOG(LogShib, Error, TEXT("Failed to save game to file: %s"), *SaveFilePath);
	}
}

void UAvatarOperationsManager::PrintUndoStack() const
{
	UE_LOG(LogAvatarState, Log, TEXT("==== Undo Stack (Newest to Oldest) ===="));

	for (int32 i = UndoStack.Num() - 1; i >= 0; --i)
	{
		FCurrentAvatarState State;
		if (FCurrentAvatarState::FromJson(UndoStack[i], State))
		{
			FString CompactState = FString::Printf(TEXT("[%d] Base: %s | Gender: %s | Meshes: %d | Textures: %d | Accessories: %d"),
				i,
				*State.AvatarBaseRow.ToString(),
				State.bIsMale ? TEXT("Male") : TEXT("Female"),
				State.MeshAssetRowNameData.Num(),
				State.ShibTextureAssetRowNameData.Num(),
				State.AccessoryRowNameData.Num()
			);

			UE_LOG(LogAvatarState, Log, TEXT("%s"), *CompactState);
		}
		else
		{
			UE_LOG(LogAvatarState, Warning, TEXT("[%d] Invalid or corrupted state in stack"), i);
		}
	}

	UE_LOG(LogAvatarState, Log, TEXT("====================================="));
}


