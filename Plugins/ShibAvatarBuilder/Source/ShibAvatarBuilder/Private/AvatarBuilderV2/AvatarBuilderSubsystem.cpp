// Copyright Shiba Inu Games LLC.

#include "AvatarBuilderV2/AvatarBuilderSubsystem.h"

#include "Kismet/GameplayStatics.h"

// Macro to reduce repeated static struct calls
#define GET_STRUCT_TYPE(StructType) StructType::StaticStruct()
#define COPY_DYN_MATERIAL_PARAMETER(OldMaterial, NewMaterial, Param, ParamType) \
NewMaterial->Set##ParamType##ParameterValue(FName(Param), OldMaterial->K2_Get##ParamType##ParameterValue(FName(Param)));


void UAvatarBuilderSubsystem::RemoveItem(FName ItemCategory,FName ItemSubCategory)
{
    if (ItemCategory.ToString().Contains("Clothing"))
    {
        FClothingItem ClothingItem;
        ClothingItem.ClothingType.ClothingType=ItemSubCategory;

        ApplyNewMesh(GetMeshHolders(ClothingItem),nullptr);
    }
    else if (ItemCategory.ToString().Contains("Accessories"))
    {
        FAccessoryItem AccessoryItem;
        AccessoryItem.AccessoryType.AccessoryType=ItemSubCategory;
        
        ApplyNewMesh(GetMeshHolders(AccessoryItem),nullptr);
    }
    else
    {
        FMeshItem MeshItem;
        MeshItem.ItemCategory.ItemCategory=ItemCategory;
        ApplyNewMesh(GetMeshHolders(MeshItem),nullptr);
    }
}

void UAvatarBuilderSubsystem::ApplyButtonItem(FUIButtonsData UIButton)
{
    ApplyDataTableRow(UIButton.DataTable,UIButton.RowName,true);
}


// Changes in ItemCategories need to be handled here as well
void UAvatarBuilderSubsystem::ApplyDataTableRow(UDataTable* DataTable,FName RowName,bool bFinal)
{
    const UScriptStruct* RowStruct = DataTable->GetRowStruct();
    if (!RowStruct)
    {
        UE_LOG(LogTemp, Warning, TEXT("ApplyButtonItem: Invalid DataTable or RowStruct"));
        return;
    }
    
    if (RowStruct->IsChildOf(GET_STRUCT_TYPE(FAccessoryItem)))
    {
        ApplyTypedItem<FAccessoryItem>(DataTable, RowName,bFinal);
    }
    else if (RowStruct->IsChildOf(GET_STRUCT_TYPE(FClothingItem)))
    {
        ApplyTypedItem<FClothingItem>(DataTable, RowName,bFinal);
    }
    else if (RowStruct->IsChildOf(GET_STRUCT_TYPE(FHairItem)))
    {
        ApplyTypedItem<FHairItem>(DataTable, RowName,bFinal);
    }
    else if (RowStruct->IsChildOf(GET_STRUCT_TYPE(FTattooItem)))
    {
        ApplyTypedItem<FTattooItem>(DataTable, RowName,bFinal);
    }
    else if (RowStruct->IsChildOf(GET_STRUCT_TYPE(FSkinItem)))
    {
        ApplyTypedItem<FSkinItem>(DataTable, RowName,bFinal);
    }
    else if (RowStruct->IsChildOf(GET_STRUCT_TYPE(FPresetItem)))
    {
        ApplyTypedItem<FPresetItem>(DataTable, RowName,bFinal);
    }
}


void UAvatarBuilderSubsystem::UpdateCurrentPresetSkin()
{
    
    if(AvatarOperationsManager->GetCurrentState().ShibTextureAssetRowNameData.Contains("Preset"))
    {
        FName RowName=AvatarOperationsManager->GetCurrentState().ShibTextureAssetRowNameData["Preset"];
        FString TablePath = TEXT("/ShibAvatarBuilder/V2/DataTables/DT_Preset_V2");

        UDataTable* DT = Cast<UDataTable>(StaticLoadObject(UDataTable::StaticClass(), nullptr, *TablePath));

        FPresetItem* PresetItem=DT->FindRow<FPresetItem>(RowName,"Finding Preset Item");

        ApplyItem(*PresetItem,RowName);
    }
}

void UAvatarBuilderSubsystem::ApplyNewMesh(USkeletalMeshComponent* Holder, USkeletalMesh* NewMesh)
{
    if (!Holder) return;
    Holder->SetSkeletalMeshAsset(nullptr);
    Holder->SetMaterial(0, nullptr);
    Holder->SetSkeletalMeshAsset(NewMesh);
}

void UAvatarBuilderSubsystem::ApplyNewTexture(UMaterialInstanceDynamic* Material, const TArray<FName> Parameters, const TArray<UTexture*>& Textures)
{
    for (int32 i = 0; i < Textures.Num(); ++i)
    {
        if (Material && Parameters.IsValidIndex(i))
        {
            Material->SetTextureParameterValue(Parameters[i], Textures[i]);
        }
    }
}

void UAvatarBuilderSubsystem::ApplyItem(FMeshItem& MeshItem,FName RowName,bool bFinal)
{
    auto& GenderedMeshes = MeshItem.MeshesByGender;
    FGenderEntryHandle UnisexGenderEntry;
    UnisexGenderEntry.Gender="Unisex";

    FBodyTypeEntryHandle AllBodyTypesEntry;
    AllBodyTypesEntry.BodyType="All";

    FCurrentAvatarState AvatarState;
    AvatarState.bIsMale= (GetGender().Gender == "Male");

    FName Category;
    
    if (MeshItem.ItemCategory.ItemCategory=="Clothing")
    {
        if(FClothingItem* ClothingItem=static_cast<FClothingItem*>(&MeshItem))
        {
            Category=ClothingItem->ClothingType.ClothingType;
            AvatarState.MeshAssetRowNameData.Add(ClothingItem->ClothingType.ClothingType,RowName);
        }   
    }
    else if (MeshItem.ItemCategory.ItemCategory=="Accessories")
    {
        if(FAccessoryItem* AccessoryItem=static_cast<FAccessoryItem*>(&MeshItem))
        {
            AvatarState.AccessoryRowNameData.Add(AccessoryItem->AccessoryType.AccessoryType,RowName);
            Category=AccessoryItem->AccessoryType.AccessoryType;
        }
    }
    else
    {
        Category=MeshItem.ItemCategory.ItemCategory;
        AvatarState.MeshAssetRowNameData.Add(MeshItem.ItemCategory.ItemCategory,RowName);
    }
    
    FCurrentAvatarState TempState=AvatarOperationsManager->AddToStateTemporary(AvatarState);
    bool ItemAddedInConflict=false;
    bool ResolveInConflictingItem=false;
    if (!MeshItem.CompatibilityRules.IsEmpty())
    {
        for (FItemCompatibility CompatibilityRule :MeshItem.CompatibilityRules)
        {
            if (AvatarOperationsManager->CheckCurrentStateForConflicts(CompatibilityRule,TempState,ResolveInConflictingItem,MeshItem.AllowedExceptions,bFinal))
            {
                ItemAddedInConflict=true;
            }
        }
    }

    if (!ItemAddedInConflict)
    {
        // First, check if Unisex is available
        if (GenderedMeshes.Contains(UnisexGenderEntry))
        {
            auto& UnisexMeshes = GenderedMeshes[UnisexGenderEntry].MeshesByBodyType;
            
            // If "All" is present, use it; otherwise, use the specific body type
            if (UnisexMeshes.Contains(AllBodyTypesEntry))
            {
                ApplyNewMesh(GetMeshHolders(MeshItem), UnisexMeshes[AllBodyTypesEntry]);

                // If this mesh uses morphs, apply them
                if(MeshItem.bHasWeightMorphs)
                {
                    ApplyMorph(GetMeshHolders(MeshItem),MeshItem.WeightMorphs,GetBodyWeight());
                }
            }
            else if (UnisexMeshes.Contains(GetBodyType())) // Ensure body type exists
            {
                ApplyNewMesh(GetMeshHolders(MeshItem), UnisexMeshes[GetBodyType()]);
            }
            return; // Exit early, as Unisex takes precedence
        }

        USkeletalMesh* Mesh=nullptr;
        // If Unisex was not found, check for the specific gender
        if (GenderedMeshes.Contains(GetGender()))
        {
            auto& GenderMeshes = GenderedMeshes[GetGender()].MeshesByBodyType;

            // Use "All" if available, otherwise use the specific body type
            if (GenderMeshes.Contains(AllBodyTypesEntry))
            {
                ApplyNewMesh(GetMeshHolders(MeshItem), GenderMeshes[AllBodyTypesEntry]);
                Mesh=GenderMeshes[AllBodyTypesEntry];
                
                // If this mesh uses morphs, apply them
                if(MeshItem.bHasWeightMorphs)
                {
                    ApplyMorph(GetMeshHolders(MeshItem),MeshItem.WeightMorphs,GetBodyWeight());
                }
            }
            else if (GenderMeshes.Contains(GetBodyType())) // Ensure body type exists
            {
                ApplyNewMesh(GetMeshHolders(MeshItem), GenderMeshes[GetBodyType()]);
                Mesh=GenderMeshes[GetBodyType()];
            }
        }
        else
        {
            ApplyNewMesh(GetMeshHolders(MeshItem),nullptr);
            
            if (MeshItem.ItemCategory.ItemCategory=="Clothing")
            {
                if(FClothingItem* ClothingItem=static_cast<FClothingItem*>(&MeshItem))
                {
                    TempState.MeshAssetRowNameData.Remove(ClothingItem->ClothingType.ClothingType);
                    TempState.ShibMeshColorData.Remove(ClothingItem->ClothingType.ClothingType);
                }   
            }
            else if (MeshItem.ItemCategory.ItemCategory=="Accessories")
            {
                if(FAccessoryItem* AccessoryItem=static_cast<FAccessoryItem*>(&MeshItem))
                {
                    TempState.AccessoryRowNameData.Remove(AccessoryItem->AccessoryType.AccessoryType);
                    TempState.AccessoryShibColorData.Remove(AccessoryItem->AccessoryType.AccessoryType);
                }
            }
            else
            {
                TempState.MeshAssetRowNameData.Remove(MeshItem.ItemCategory.ItemCategory);
                TempState.ShibMeshColorData.Remove(MeshItem.ItemCategory.ItemCategory);
            }
        }

    
        if (!ResolveInConflictingItem)
        {
            if (bFinal)
            {
                TemporaryStateForPreview=TempState;
                AvatarOperationsManager->AddToState(TempState);
            }
            else
            {
                TemporaryStateForPreview=TempState;
            }

            USkeletalMeshComponent* Holder=GetMeshHolders(MeshItem);

            FLinearColor Color=MeshItem.MaterialData.DefaultColor;
            if (MeshItem.ItemCategory.ItemCategory=="Accessories")
            {
                if (TempState.AccessoryShibColorData.Contains(Category))
                {
                    Color=TempState.AccessoryShibColorData[Category];
                }
            }
            else
            {
                if (TempState.ShibMeshColorData.Contains(Category))
                {
                    Color=TempState.ShibMeshColorData[Category];
                }
            }
            for (auto & Index:MeshItem.MaterialData.MaterialIndices)
            {
                if (IsValid(Mesh))
                {
                    auto&MaterialsArray=Mesh->GetMaterials();
                    UMaterialInstanceDynamic* Material;
                    
                    if (!Holder)
                    {
                        UE_LOG(LogTemp, Error, TEXT("CreateDynamicMaterialInstance failed: Holder is NULL!"));
                        return;
                    }

                    if (Index < 0 || Index >= MaterialsArray.Num())
                    {
                        UE_LOG(LogTemp, Error, TEXT("CreateDynamicMaterialInstance failed: Index %d is out of bounds! MaterialsArray size: %d"), Index, MaterialsArray.Num());
                        return;
                    }

                    Material=Holder->CreateDynamicMaterialInstance(Index,MaterialsArray[Index].MaterialInterface);
            
                    if(IsValid(Material))
                    {
                        Material->SetVectorParameterValue(MeshItem.MaterialData.ColorParameter,Color);
                    }
                }
            }
            
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("Not allowed to add to stack right now"));    
        }
        
        
        AvatarOperationsManager->UpdateBaseMesh();
    }
    
    AvatarOperationsManager->RemoveUneccesaryUndergarments();
}



void UAvatarBuilderSubsystem::ApplyItem(FAccessoryItem& AccessoryItem,FName RowName,bool bFinal)
{
    if(FMeshItem* MeshItem=static_cast<FMeshItem*>(&AccessoryItem))
    {
        ApplyItem(*MeshItem,RowName,bFinal);
    }

    if(AccessoryItem.bHasFacialMorphs)
    {
        
        // Get the facial morphs that it contains
        for (auto [Feature,Dimensions] : AccessoryItem.FacialMorphNames[GetGender()].FacialMorphData)
        {
            // Get dimension morph of the feature
            for(auto [DimensionHandle, Morphs] : Dimensions.FeatureMorphs)
            {
                
                // For each feature-dimension (Nose-width) saved in body morphs, apply the same value to the accessory
                auto FeatureVal = AvatarOperationsManager->GetCurrentState().MorphData.Find(GetMappedJsonBodyMorphName(Feature,DimensionHandle));
                if(FeatureVal)
                {
                    ApplyMorph(GetMeshHolders(AccessoryItem),Morphs.Names,*FeatureVal);
                }
            }
        }
    }

    
    if (GetBodyTypeName().ToString().Contains("Overweight"))
    {
        GetMeshHolders(AccessoryItem)->SetMorphTarget("BS_Overweight",1);
        GetMeshHolders(AccessoryItem)->SetMorphTarget("BS_Thin",0);
    }
    else if (GetBodyTypeName().ToString().Contains("Thin"))
    {
        GetMeshHolders(AccessoryItem)->SetMorphTarget("BS_Overweight",0);
        GetMeshHolders(AccessoryItem)->SetMorphTarget("BS_Thin",1);
    }
    else if (GetBodyTypeName().ToString().Contains("Fit"))
    {
        GetMeshHolders(AccessoryItem)->SetMorphTarget("BS_Overweight",0);
        GetMeshHolders(AccessoryItem)->SetMorphTarget("BS_Thin",0);
    }
    
    
}

void UAvatarBuilderSubsystem::ApplyItem(FTextureItem& TextureItem,FName RowName,bool bFinal)
{
    auto&GenderedTextures=TextureItem.Textures;

    FGenderEntryHandle UnisexGenderEntry;
    UnisexGenderEntry.Gender="Unisex";

    FCurrentAvatarState TempState;
    TempState.bIsMale = (GetGender().Gender == "Male");

    FName Category=TextureItem.ItemCategory.ItemCategory;

    bool bIsTattoo=false;
    
    if (GenderedTextures.Contains(UnisexGenderEntry))
    {
        auto& UnisexTextures = GenderedTextures[UnisexGenderEntry].Textures;
        auto& UnisexParams=GenderedTextures[UnisexGenderEntry].TextureParameters;

        if (TextureItem.ItemCategory.ItemCategory=="Tattoos")
        {
            if(FTattooItem* TattooItem=static_cast<FTattooItem*>(&TextureItem))
            {
                TempState.TattooRowNameData.Add(TattooItem->Placement.TattooPlacement,RowName);
                bIsTattoo=true;
                Category=TattooItem->Placement.TattooPlacement;
            }   
        }
        if (TextureItem.ItemCategory.ItemCategory=="Eyebrows")
        {
            if(FTattooItem* TattooItem=static_cast<FTattooItem*>(&TextureItem))
            {
                TempState.ShibTextureAssetRowNameData.Add("eyebrowShape",RowName);
                Category="eyebrowShape";
            }   
        }
        ApplyNewTexture(GetAvatarCharacter()->GetBodyDMI(),UnisexParams,UnisexTextures);
    }
    else if (GenderedTextures.Contains(GetGender()))
    {
        auto& GenderTextures = GenderedTextures[GetGender()].Textures;
        auto& GenderParams=GenderedTextures[GetGender()].TextureParameters;

        if (TextureItem.ItemCategory.ItemCategory=="Tattoos")
        {
            if(FTattooItem* TattooItem=static_cast<FTattooItem*>(&TextureItem))
            {
                TempState.TattooRowNameData.Add(TattooItem->Placement.TattooPlacement,RowName);
            }   
        }
        if (TextureItem.ItemCategory.ItemCategory=="Eyebrows")
        {
            if(FTattooItem* TattooItem=static_cast<FTattooItem*>(&TextureItem))
            {
                TempState.ShibTextureAssetRowNameData.Add("eyebrowShape",RowName);
            }   
        }
        ApplyNewTexture(GetAvatarCharacter()->GetBodyDMI(),GenderParams,GenderTextures);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("No Texture Data Found"));    
    }
    
    
    TempState=AvatarOperationsManager->AddToStateTemporary(TempState);
    if (bFinal)
    {
        TemporaryStateForPreview=TempState;
        AvatarOperationsManager->AddToState(TempState);
    }
    else
    {
        TemporaryStateForPreview=TempState;
    }

    FLinearColor Color=TextureItem.MaterialData.DefaultColor;
    if (TextureItem.ItemCategory.ItemCategory=="Tattoos")
    {
        if (TempState.TattooShibColorData.Contains(Category))
        {
            Color=TempState.TattooShibColorData[Category];
        }
    }
    else
    {
        if (TempState.ShibTextureColorData.Contains(Category))
        {
            Color=TempState.ShibTextureColorData[Category];
        }
    }
    FMaterialCustomization MaterialCustomization=TextureItem.MaterialData;
    MaterialCustomization.DefaultColor=Color;
    ApplyTextureColors(AvatarCharacter->GetBody(),Category,MaterialCustomization,false,bIsTattoo);
}

void UAvatarBuilderSubsystem::SetBodyDynMatInstAndCopyParams(const AShibAvatarCharacter* InAvatarCharacter, UMaterialInterface* NewMaterial)
{
    UMaterialInstanceDynamic* OldMaterial = const_cast<AShibAvatarCharacter*>(InAvatarCharacter)->GetBodyDMI();
    if (!OldMaterial)
    {
        return;
    }
	
    UMaterialInstanceDynamic* NewDynMaterial = InAvatarCharacter->GetBody()->CreateDynamicMaterialInstance(0, NewMaterial);
	
    // vectors
    COPY_DYN_MATERIAL_PARAMETER(OldMaterial, NewDynMaterial, "VarEyeBrowColor", Vector)
    COPY_DYN_MATERIAL_PARAMETER(OldMaterial, NewDynMaterial, "VarTattoo_HeadColor", Vector)
    COPY_DYN_MATERIAL_PARAMETER(OldMaterial, NewDynMaterial, "VarTattoo_ChestColor", Vector)
    COPY_DYN_MATERIAL_PARAMETER(OldMaterial, NewDynMaterial, "VarTattoo_ArmLColor", Vector)
    COPY_DYN_MATERIAL_PARAMETER(OldMaterial, NewDynMaterial, "VarTattoo_ArmRColor", Vector)
    COPY_DYN_MATERIAL_PARAMETER(OldMaterial, NewDynMaterial, "VarTattoo_BackColor", Vector)

    // scalar
    COPY_DYN_MATERIAL_PARAMETER(OldMaterial, NewDynMaterial, "VarMixAdjustment", Scalar)
    COPY_DYN_MATERIAL_PARAMETER(OldMaterial, NewDynMaterial, "TatooBlendAmount", Scalar)
	
    // texture
    COPY_DYN_MATERIAL_PARAMETER(OldMaterial, NewDynMaterial, "EyebrowsTexture", Texture)
    COPY_DYN_MATERIAL_PARAMETER(OldMaterial, NewDynMaterial, "EyebrowsNormal", Texture)
    COPY_DYN_MATERIAL_PARAMETER(OldMaterial, NewDynMaterial, "T_Tattoo_Head", Texture)
    COPY_DYN_MATERIAL_PARAMETER(OldMaterial, NewDynMaterial, "T_Tattoo_Chest", Texture)
    COPY_DYN_MATERIAL_PARAMETER(OldMaterial, NewDynMaterial, "T_Tattoo_Back", Texture)
    COPY_DYN_MATERIAL_PARAMETER(OldMaterial,NewDynMaterial, "T_Tattoo_ArmL",Texture)
    COPY_DYN_MATERIAL_PARAMETER(OldMaterial,NewDynMaterial, "T_Tattoo_ArmR",Texture)
}

void UAvatarBuilderSubsystem::ApplyItem(FPresetItem& PresetItem,FName RowName,bool bFinal)
{
    USkeletalMeshComponent* BodyMesh = GetAvatarCharacter()->GetBody();
    TArray<FString> MorphTargets;
    FCurrentAvatarState AvatarState;
    AvatarState.bIsMale = (GetGender().Gender == "Male");

    // Reset all presets
    for (const FName& PresetMorph : GetPresetMorphNames())
    {
        BodyMesh->SetMorphTarget(PresetMorph, 0);
    }

    // Apply New Preset, check if it exists
    if (PresetItem.PresetMorphName.Contains(GetGender()))
    {
   
        ApplyMorph(BodyMesh,PresetItem.PresetMorphName[GetGender()],1);

        AvatarState.ShibTextureAssetRowNameData.Add("Skin",NewSkintoOld[CurrentSkin]);
        AvatarState.ShibTextureAssetRowNameData.Add("Preset", RowName);

        SetBodyDynMatInstAndCopyParams(GetAvatarCharacter(),PresetItem.MaterialAndThumbnailData[CurrentSkin].MaterialByGender[GetGender()].MaterialInstance);
    
        AvatarState=AvatarOperationsManager->AddToStateTemporary(AvatarState);
        if (bFinal)
        {
            TemporaryStateForPreview=AvatarState;
            AvatarOperationsManager->AddToState(AvatarState);
            OnItemApplied.Broadcast(AvatarState);
        }
        else
        {
            TemporaryStateForPreview=AvatarState;
        }
    }
}

USkeletalMeshComponent* UAvatarBuilderSubsystem::GetMeshHolders(FPrimitiveItem& Item)
{
    if (!GetAvatarCharacter()) return nullptr;

    // Clothing Items
    if (Item.ItemCategory.ItemCategory == TEXT("Clothing"))
    {
        FClothingItem* ClothingItem = static_cast<FClothingItem*>(&Item);
        if (!ClothingItem) return nullptr;

        if (ClothingItem->ClothingType.ClothingType == TEXT("Top")) return GetAvatarCharacter()->GetTop();
        if (ClothingItem->ClothingType.ClothingType == TEXT("Bottom")) return GetAvatarCharacter()->GetBottoms();
        if (ClothingItem->ClothingType.ClothingType == TEXT("Shoes")) return GetAvatarCharacter()->GetShoes();
        if (ClothingItem->ClothingType.ClothingType == TEXT("FullBody")) return GetAvatarCharacter()->GetFullBody();
    }

    // Hair Items
    if (Item.ItemCategory.ItemCategory == TEXT("Hair"))
    {
        return GetAvatarCharacter()->GetHair();
    }

    // Accessory Items
    if (Item.ItemCategory.ItemCategory == TEXT("Accessories"))
    {
        FAccessoryItem* AccessoryItem = static_cast<FAccessoryItem*>(&Item);
        if (!AccessoryItem) return nullptr;

        if (AccessoryItem->AccessoryType.AccessoryType == TEXT("HeadTop")) return GetAvatarCharacter()->GetHeadTop();
        if (AccessoryItem->AccessoryType.AccessoryType == TEXT("Face")) return GetAvatarCharacter()->GetFace();
        if (AccessoryItem->AccessoryType.AccessoryType == TEXT("Ears")) return GetAvatarCharacter()->GetEars();
        if (AccessoryItem->AccessoryType.AccessoryType == TEXT("Back")) return GetAvatarCharacter()->GetBack();
        if (AccessoryItem->AccessoryType.AccessoryType == TEXT("Neck")) return GetAvatarCharacter()->GetNeck();
        if (AccessoryItem->AccessoryType.AccessoryType == TEXT("Arms")) return nullptr; 
        if (AccessoryItem->AccessoryType.AccessoryType == TEXT("LeftArm")) return GetAvatarCharacter()->GetLeftArm();
        if (AccessoryItem->AccessoryType.AccessoryType == TEXT("RightArm")) return GetAvatarCharacter()->GetRightArm();
    }

    UE_LOG(LogTemp, Warning, TEXT("GetMeshHolders: No valid SkeletalMeshComponent found for this item."));
    return nullptr;
}

USkeletalMeshComponent* UAvatarBuilderSubsystem::GetMeshHoldersByCategory(const FName& Category)
{
    if (!GetAvatarCharacter()) return nullptr;

    if (Category == TEXT("Top")) return GetAvatarCharacter()->GetTop();
    if (Category == TEXT("Bottom")) return GetAvatarCharacter()->GetBottoms();
    if (Category == TEXT("Shoes")) return GetAvatarCharacter()->GetShoes();
    if (Category == TEXT("FullBody")) return GetAvatarCharacter()->GetFullBody();
    if (Category == TEXT("Hair")) return GetAvatarCharacter()->GetHair();
    
    if (Category == TEXT("HeadTop")) return GetAvatarCharacter()->GetHeadTop();
    if (Category == TEXT("Face")) return GetAvatarCharacter()->GetFace();
    if (Category == TEXT("Ears")) return GetAvatarCharacter()->GetEars();
    if (Category == TEXT("Back")) return GetAvatarCharacter()->GetBack();
    if (Category == TEXT("Neck")) return GetAvatarCharacter()->GetNeck();
    if (Category == TEXT("LeftArm")) return GetAvatarCharacter()->GetLeftArm();
    if (Category == TEXT("RightArm")) return GetAvatarCharacter()->GetRightArm();

    return nullptr;
}

void UAvatarBuilderSubsystem::ClearAllMeshHolders()
{
    if (!GetAvatarCharacter()) return;

    GetAvatarCharacter()->GetTop()->SetSkeletalMeshAsset(nullptr);
    GetAvatarCharacter()->GetBottoms()->SetSkeletalMeshAsset(nullptr);
    GetAvatarCharacter()->GetShoes()->SetSkeletalMeshAsset(nullptr);
    GetAvatarCharacter()->GetFullBody()->SetSkeletalMeshAsset(nullptr);
    GetAvatarCharacter()->GetHair()->SetSkeletalMeshAsset(nullptr);
}

void UAvatarBuilderSubsystem::ClearAllAccessoryHolders()
{
    if (!GetAvatarCharacter()) return;

    GetAvatarCharacter()->GetHeadTop()->SetSkeletalMeshAsset(nullptr);
    GetAvatarCharacter()->GetFace()->SetSkeletalMeshAsset(nullptr);
    GetAvatarCharacter()->GetEars()->SetSkeletalMeshAsset(nullptr);
    GetAvatarCharacter()->GetBack()->SetSkeletalMeshAsset(nullptr);
    GetAvatarCharacter()->GetNeck()->SetSkeletalMeshAsset(nullptr);
    GetAvatarCharacter()->GetLeftArm()->SetSkeletalMeshAsset(nullptr);
    GetAvatarCharacter()->GetRightArm()->SetSkeletalMeshAsset(nullptr);
}

void UAvatarBuilderSubsystem::ClearAllTattoos()
{
    FString TablePath = TEXT("/ShibAvatarBuilder/V2/DataTables/DT_Tattoo_V2");

    UDataTable* DT = Cast<UDataTable>(StaticLoadObject(UDataTable::StaticClass(), nullptr, *TablePath));
    TArray<FName> RowNames = DT->GetRowNames();
    
    FGenderEntryHandle UnisexGenderEntry;
    UnisexGenderEntry.Gender="Unisex";
    
    for(auto&Name:RowNames)
    {
        if (Name.ToString().Contains("Remove"))
        {
            FTattooItem* TattooItem=DT->FindRow<FTattooItem>(Name,"Finding Tattoo Item row");
            auto&GenderedTextures=TattooItem->Textures;
        
            if (GenderedTextures.Contains(UnisexGenderEntry))
            {
                auto& UnisexTextures = GenderedTextures[UnisexGenderEntry].Textures;
                auto& UnisexParams=GenderedTextures[UnisexGenderEntry].TextureParameters;
            
                ApplyNewTexture(GetAvatarCharacter()->GetBodyDMI(),UnisexParams,UnisexTextures);
            }
            else if (GenderedTextures.Contains(GetGender()))
            {
                auto& GenderTextures = GenderedTextures[GetGender()].Textures;
                auto& GenderParams=GenderedTextures[GetGender()].TextureParameters;

                ApplyNewTexture(GetAvatarCharacter()->GetBodyDMI(),GenderParams,GenderTextures);
            }
        }
    }
}


AShibAvatarCharacter* UAvatarBuilderSubsystem::GetAvatarCharacter()
{
    if (!IsValid(AvatarCharacter))
    {
        AvatarCharacter = Cast<AShibAvatarCharacter>(UGameplayStatics::GetActorOfClass(this, AShibAvatarCharacter::StaticClass()));
        if (!IsValid(AvatarCharacter))
        {
            LOG_SHIB(Error, "Avatar not found in world!");
        }
    }
    return AvatarCharacter;
}



void UAvatarBuilderSubsystem::ApplyAvatarCustomization(FCurrentAvatarState AvatarState)
{
    if (!AvatarOperationsManager)
    {
        return;
    }
    bCustomizationInProgress = true;

    
    //  Set Gender
    FGenderEntryHandle GenderEntryHandle;
    GenderEntryHandle.Gender = AvatarState.bIsMale ? TEXT("Male") : TEXT("Female");
    SetGender(GenderEntryHandle,false);
    
    SetBodyTypeCalls=0;
    // Apply Body Morphs
    if (!AvatarState.MorphData.IsEmpty()) 
    {
        if (AvatarState.MorphData.Contains("avatarHeight"))
        {
            ApplyHeightMorph(AvatarState.MorphData["avatarHeight"], false);
        }
        if (AvatarState.MorphData.Contains("avatarWeight"))
        {
            SetBodyType(AvatarState.MorphData["avatarWeight"],false);
        }
    }
    
    //  Clear existing meshes and textures
    ClearAllMeshHolders();
    ClearAllAccessoryHolders();
    ClearAllTattoos();
    
    // Apply Clothing & Mesh Assets
    if (!AvatarState.MeshAssetRowNameData.IsEmpty())
    {
        for (const auto& RowNameData : AvatarState.MeshAssetRowNameData)
        {
            UDataTable* DataTable = AvatarOperationsManager->FindDataTableByName(RowNameData.Key);
            if (DataTable)
            {
                ApplyDataTableRow(DataTable, RowNameData.Value,false);
            }
        }
    }

    FlushRenderingCommands();

    // Apply Accessories
    if (!AvatarState.AccessoryRowNameData.IsEmpty())
    {
        for (const auto& RowNameData : AvatarState.AccessoryRowNameData)
        {
            UDataTable* DataTable = AvatarOperationsManager->FindDataTableByName("Accessories");
            if (DataTable)
            {
                ApplyDataTableRow(DataTable, RowNameData.Value,false);
            }
        }
    }
    
    FlushRenderingCommands();

    // Apply Tattoos
    if (!AvatarState.TattooRowNameData.IsEmpty())
    {
        UDataTable* DataTable = AvatarOperationsManager->FindDataTableByName("Tattoos");
        for (const auto& Entry : AvatarState.TattooRowNameData)
        {
            ApplyDataTableRow(DataTable, Entry.Value,false);
        }
    }

    //  Apply Skin & Preset
    if (!AvatarState.ShibTextureAssetRowNameData.IsEmpty())
    {
        if (AvatarState.ShibTextureAssetRowNameData.Contains("Skin"))
        {
            FName SkinRow = AvatarState.ShibTextureAssetRowNameData["skin"];
            CurrentSkin = OldSkintoNew[SkinRow];
        }
        if (AvatarState.ShibTextureAssetRowNameData.Contains("Preset"))
        {
            FName PresetRow = AvatarState.ShibTextureAssetRowNameData["preset"];
            UE_LOG(LogTemp, Warning, TEXT("Applying preset row: %s"), *PresetRow.ToString());
            ApplyDataTableRow(AvatarOperationsManager->FindDataTableByName("Preset"), PresetRow,false);
        }
        if (AvatarState.ShibTextureAssetRowNameData.Contains("eyebrowShape"))
        {
            FName EyebrowRow = AvatarState.ShibTextureAssetRowNameData["eyebrowShape"];
            UE_LOG(LogTemp, Warning, TEXT("Applying eyebrow row: %s"), *EyebrowRow.ToString());
            ApplyDataTableRow(AvatarOperationsManager->FindDataTableByName("Eyebrows"), EyebrowRow,false);
        }
    }

    FlushRenderingCommands();

    if (!AvatarState.ShibTextureColorData.IsEmpty()) 
    {
        if (AvatarState.ShibTextureColorData.Contains("eyebrowShape"))
        {
            FMaterialCustomization EyebrowColor;
            EyebrowColor.DefaultColor=AvatarState.ShibTextureColorData["eyebrowShape"];
            EyebrowColor.ColorParameter="VarEyeBrowColor";
            EyebrowColor.MaterialIndices.Add(0);
            ApplyTextureColors(AvatarCharacter->GetBody(),"eyebrowShape",EyebrowColor,false);
        }
        if (AvatarState.ShibTextureColorData.Contains("eyeShape"))
        {
            FMaterialCustomization EyebrowColor;
            EyebrowColor.DefaultColor=AvatarState.ShibTextureColorData["eyeShape"];
            EyebrowColor.ColorParameter="EyeColorAdjustment";
            EyebrowColor.MaterialIndices.Add(3);
            ApplyTextureColors(AvatarCharacter->GetBody(),"eyeShape",EyebrowColor,false);
        }
    }

    if (!AvatarState.MorphData.IsEmpty())
    {
        for (const auto& Entry : AvatarState.MorphData)
        {
            FAvatarFacialFeatureHandle Feature;
            FAvatarDimensionHandle Dimension;
            GetFeatureAndDimensionFromMappedJsonBodyMorphName(Entry.Key, Feature, Dimension);
            ApplyBodyMorph(Entry.Value, Feature,Dimension,false);
        }
    }

    if (!AvatarState.TattooShibColorData.IsEmpty())
    {
        for (auto& TattooColorData:AvatarState.TattooShibColorData)
        {
            UDataTable* TattooTable = AvatarOperationsManager->FindDataTableByName("Tattoos");
            if (TattooTable && AvatarState.TattooRowNameData.Contains(TattooColorData.Key))
            {
                FTattooItem* TattooItem = TattooTable->FindRow<FTattooItem>(AvatarState.TattooRowNameData[TattooColorData.Key], TEXT("Fetching Tattoo Item"));
                if(!TattooItem) {continue;}
                FTattooItem Copy=*TattooItem;
                Copy.MaterialData.DefaultColor=TattooColorData.Value;
                
                if (IsValid(AvatarCharacter->GetBody()) && IsValid(AvatarCharacter->GetBody()->GetSkeletalMeshAsset()))
                {
                    ApplyTextureColors(AvatarCharacter->GetBody(),TattooColorData.Key,Copy.MaterialData,false,true);
                }
            }
        }
    }
    
    FlushRenderingCommands();
    
    OnItemApplied.Broadcast(AvatarState);
    bCustomizationInProgress = false;
}

void UAvatarBuilderSubsystem::SetGender(FGenderEntryHandle NewGender,bool bFinal)
{
    if (GetGender()==NewGender){return;}
    
    Gender = NewGender;
    
    FCurrentAvatarState AvatarState;
    AvatarState.bIsMale = (Gender.Gender == "Male");
    
    AvatarState=AvatarOperationsManager->AddToStateTemporary(AvatarState);
    ApplyAvatarCustomization(AvatarState);

    if (bFinal)
    {
        AvatarOperationsManager->AddToState(AvatarState);
        //OnItemApplied.Broadcast(AvatarState);
    }
}

void UAvatarBuilderSubsystem::SetBodyType(float Weight,bool bFinal)
{
    UE_LOG(LogTemp, Log, TEXT("SetBodyType called with bFinal: %s"), bFinal ? TEXT("true") : TEXT("false"));
    if (Weight < 0.5)
    {
        BodyType.BodyType = TEXT("Thin");
    }
    else if (Weight > 0.5)
    {
        BodyType.BodyType = TEXT("Overweight");
    }
    else
    {
        BodyType.BodyType = TEXT("Fit");
    }
    AvatarOperationsManager->EnsureUndergarments();

    TemporaryStateForPreview=FCurrentAvatarState();
    TemporaryStateForPreview=AvatarOperationsManager->AddToStateTemporary(TemporaryStateForPreview);
    TemporaryStateForPreview.MorphData.Add("avatarWeight",Weight);
    TemporaryStateForPreview.bIsMale=(GetGender().Gender=="Male");
    
    ApplyWeightMorph();
    if (SetBodyTypeCalls!=0)
    {
        TemporaryApplyMeshes(bFinal);
        if (bFinal)
        {
            TemporaryStateForPreview.MorphData.Add("avatarWeight",Weight);
            AvatarOperationsManager->AddToState(TemporaryStateForPreview);
        }
    }
    SetBodyTypeCalls++;
}

void UAvatarBuilderSubsystem::TemporaryApplyMeshes(bool bFinal)
{
    // Apply Mesh Assets Safely
    if (!TemporaryStateForPreview.MeshAssetRowNameData.IsEmpty())
    {
        TMap<FName, FName> MeshDataCopy = TemporaryStateForPreview.MeshAssetRowNameData; // Copy to avoid modification during iteration

        for (const auto& MeshRowNameData : MeshDataCopy)
        {
            if (MeshRowNameData.Key.IsNone() || MeshRowNameData.Value.IsNone()) continue; // Ensure Key & Value are valid

            UDataTable* DataTable = AvatarOperationsManager->FindDataTableByName(MeshRowNameData.Key);
            if (IsValid(DataTable))
            {
                ApplyDataTableRow(DataTable, MeshRowNameData.Value, bFinal);
            }
        }
    }
    // Apply Accessories Safely
    if (!TemporaryStateForPreview.AccessoryRowNameData.IsEmpty())
    {
        UDataTable* AccessoryTable = AvatarOperationsManager->FindDataTableByName("Accessories");

        if (IsValid(AccessoryTable))
        {
            TMap<FName, FName> AccessoryDataCopy = TemporaryStateForPreview.AccessoryRowNameData; // Copy to avoid modification during iteration

            for (const auto& AccessoryRowNameData : AccessoryDataCopy)
            {
                if (AccessoryRowNameData.Value.IsNone()) continue; // Ensure Value is valid

                ApplyDataTableRow(AccessoryTable, AccessoryRowNameData.Value, bFinal);
            }
        }
    }
}

TArray<FUIButtonsData> UAvatarBuilderSubsystem::GetButtonsDataFromDataTable(UDataTable* ItemDataTable)
{
    TArray<FUIButtonsData> ButtonsDataArray;

    if (!ItemDataTable)
    {
        UE_LOG(LogTemp, Warning, TEXT("GetButtonsDataFromDataTable: Invalid DataTable"));
        return ButtonsDataArray;
    }

    for (const FName& RowName : ItemDataTable->GetRowNames())
    {
        void* RowData = ItemDataTable->FindRowUnchecked(RowName);
        if (!RowData) continue;

        FUIButtonsData ButtonData;
        ButtonData.DataTable = ItemDataTable;
        ButtonData.RowName = RowName;

        if (ItemDataTable->GetRowStruct()->IsChildOf(FBaseItem::StaticStruct()))
        {
            AssignThumbnailsFromItem(static_cast<FBaseItem*>(RowData), ButtonData);
        }
        else if (ItemDataTable->GetRowStruct()->IsChildOf(FPresetItem::StaticStruct()))
        {
            AssignThumbnailsFromItem(static_cast<FPresetItem*>(RowData), ButtonData);
        }

        ButtonsDataArray.Add(ButtonData);
    }

    return ButtonsDataArray;
}

void UAvatarBuilderSubsystem::UpdateSKM(USkeletalMesh* NewMesh,TObjectPtr<UAnimMontage> Montage)
{
    GetAvatarCharacter()->GetBody()->SetSkeletalMeshAsset(NewMesh);
    GetAvatarCharacter()->PlayMontageFromCurrentPosition(Montage);
    
    FCurrentAvatarState AvatarState=TemporaryStateForPreview;
    if (!AvatarState.ShibTextureAssetRowNameData.IsEmpty())
    {
        if (AvatarState.ShibTextureAssetRowNameData.Contains("Skin"))
        {
            FName SkinRow = AvatarState.ShibTextureAssetRowNameData["skin"];
            CurrentSkin = OldSkintoNew[SkinRow];
        }
        if (AvatarState.ShibTextureAssetRowNameData.Contains("Preset"))
        {
            FName PresetRow = AvatarState.ShibTextureAssetRowNameData["preset"];
            UE_LOG(LogTemp, Warning, TEXT("Applying preset row: %s"), *PresetRow.ToString());
            ApplyDataTableRow(AvatarOperationsManager->FindDataTableByName("Preset"), PresetRow,false);
        }
        if (AvatarState.ShibTextureAssetRowNameData.Contains("eyebrowShape"))
        {
            FName EyebrowRow = AvatarState.ShibTextureAssetRowNameData["eyebrowShape"];
            UE_LOG(LogTemp, Warning, TEXT("Applying eyebrow row: %s"), *EyebrowRow.ToString());
            ApplyDataTableRow(AvatarOperationsManager->FindDataTableByName("Eyebrows"), EyebrowRow,false);
        }
    }
    
    if (!AvatarState.MorphData.IsEmpty())
    {
        for (const auto& Entry : AvatarState.MorphData)
        {
            FAvatarFacialFeatureHandle Feature;
            FAvatarDimensionHandle Dimension;
            GetFeatureAndDimensionFromMappedJsonBodyMorphName(Entry.Key, Feature, Dimension);
            ApplyBodyMorph(Entry.Value, Feature,Dimension,false);
        }
    }

    ApplyWeightMorph();
    
}

USkeletalMesh* UAvatarBuilderSubsystem::GetDefaultBase()
{
    FString TablePath = TEXT("/ShibAvatarBuilder/V2/DataTables/DT_SkeletalBodyVariations_V2");

    UDataTable* DT = Cast<UDataTable>(StaticLoadObject(UDataTable::StaticClass(), nullptr, *TablePath));
    TArray<FName> RowNames = DT->GetRowNames();
    
    // Default skeletal mesh
    FAvatarSkeletalBodyVariations* DefaultVariation= DT->FindRow<FAvatarSkeletalBodyVariations>(RowNames[0], TEXT("Finding Default Variation"));
    return DefaultVariation->BodyVariations[GetGender()];
}

void UAvatarBuilderSubsystem::TemporaryAddUndergarments()
{
    FString TablePath = TEXT("/ShibAvatarBuilder/V2/Enums/DT_ItemCategory");
    UAvatarBuilderSubsystem* AvatarBuilder = GetWorld()->GetGameInstance()->GetSubsystem<UAvatarBuilderSubsystem>();

    TattooContainerTempSave=AvatarOperationsManager->GetCurrentState();
    ClearAllMeshHolders();
    ClearAllAccessoryHolders();
    AvatarCharacter->SetCurrentMontagePosition();
    // Cache current morph targets
    TMap<FName, float> CachedMorphs;
    USkeletalMeshComponent* BodyMesh = AvatarCharacter->GetBody();

    for (const FString& MorphName : BodyMesh->GetSkeletalMeshAsset()->K2_GetAllMorphTargetNames())
    {
        CachedMorphs.Add(FName(MorphName), BodyMesh->GetMorphTarget(FName(MorphName)));
    }

    // Cache the currently playing montage and its position
    UAnimInstance* AnimInstance = BodyMesh->GetAnimInstance();
    UAnimMontage* CachedMontage = nullptr;

    if (AnimInstance && AnimInstance->Montage_IsPlaying(AnimInstance->GetCurrentActiveMontage()))
    {
        CachedMontage = AnimInstance->GetCurrentActiveMontage();
    }

    // Set the new skeletal mesh
    BodyMesh->SetSkeletalMesh(GetDefaultBase());

    // Restore morph targets
    for (const auto& MorphPair : CachedMorphs)
    {
        BodyMesh->SetMorphTarget(MorphPair.Key, MorphPair.Value);
    }

    // Restore the animation montage if there was one playing
    if (CachedMontage)
    {
        AvatarCharacter->PlayMontageFromCurrentPosition(CachedMontage);
    }


    
    
    
    AvatarOperationsManager->ApplyUndergarments(AvatarBuilder, TablePath, "Bottom", AvatarBuilder->AvatarCharacter->GetBottoms());
    AvatarOperationsManager->ApplyUndergarments(AvatarBuilder, TablePath, "Top", AvatarBuilder->AvatarCharacter->GetTop());
}

void UAvatarBuilderSubsystem::RemoveTemporaryUndergarments()
{
    ApplyAvatarCustomization(TattooContainerTempSave);
    
    TArray<FString> UndoStack;
    AvatarOperationsManager->GetUndoStack(UndoStack);
    
    if (UndoStack.Num()>=2)
    {
        if (UndoStack.Top()==UndoStack[UndoStack.Num()-2])
        {
            AvatarOperationsManager->PopUndoStack();
        }
    }
    
}


void UAvatarBuilderSubsystem::ApplyWeightMorph()
{
    FString BodyTypeString = GetBodyTypeName().ToString();
    USkeletalMeshComponent* BodyMesh = GetAvatarCharacter()->GetBody();

    if (!BodyMesh->GetSkeletalMeshAsset()) return;

    // Determine morph target values
    float OverweightValue = BodyTypeString.Contains("Overweight") ? 0.6f : 0.0f;
    float ThinValue = BodyTypeString.Contains("Thin") ? 1.0f : 0.0f;

    // Get all morph target names
    TArray<FString> MorphTargets=BodyMesh->GetSkeletalMeshAsset()->K2_GetAllMorphTargetNames();


    // Iterate over morph targets and set values dynamically
    for (const FString& MorphName : MorphTargets)
    {
        if (MorphName.Contains("Overweight"))
        {
            BodyMesh->SetMorphTarget(FName(MorphName), OverweightValue);
        }
        else if (MorphName.Contains("Thin"))
        {
            BodyMesh->SetMorphTarget(FName(MorphName), ThinValue);
        }
    }
}

void UAvatarBuilderSubsystem::AvatarLoad()
{
    // Define the save file path
    FString SaveFilePath = FPaths::ProjectSavedDir() / "SaveGames/AvatarCustomization.json";

    // Read the file content into a string
    FString LoadDataJson;
    if (!FFileHelper::LoadFileToString(LoadDataJson, *SaveFilePath))
    {
        UE_LOG(LogShib, Warning, TEXT("Save file not found: %s. Loading default state."), *SaveFilePath);

        
        FCurrentAvatarState DefaultState;
        DefaultState.SetAsDefaultState();

        if (IsValid(GetAvatarCharacter()) && IsValid(GetAvatarCharacter()->GetBody()))
        {
            AvatarOperationsManager->Initialize(DefaultState);
        }
        return;
    }

    // Deserialize JSON into the CurrentState
    FCurrentAvatarState LoadedState;
    if (!FCurrentAvatarState::FromJson(LoadDataJson, LoadedState))
    {
        UE_LOG(LogShib, Error, TEXT("Failed to parse avatar data from JSON."));
        return;
    }

    // Apply the loaded state
    if (IsValid(GetAvatarCharacter()) && IsValid(GetAvatarCharacter()->GetBody()))
    {
        AvatarOperationsManager->Initialize(LoadedState);
    }
    UE_LOG(LogShib, Log, TEXT("Game loaded successfully from file: %s"), *SaveFilePath);
}

void UAvatarBuilderSubsystem::AvatarSave()
{
    AvatarOperationsManager->SaveCurrentAvatarData();
}


TArray<FName> UAvatarBuilderSubsystem::GetPresetMorphNames()
{
    FString TablePath = TEXT("/ShibAvatarBuilder/V2/DataTables/DT_Preset_V2");
    UDataTable* DT = Cast<UDataTable>(StaticLoadObject(UDataTable::StaticClass(), nullptr, *TablePath));

    TArray<FName> PresetMorphs;
    // Get Preset DT
    if(DT)
    {
        // Loop through rows
        for (auto Row : DT->GetRowNames())
        {
            FPresetItem PresetItem = *DT->FindRow<FPresetItem>(Row,"");

            // Add morph names
            if(!PresetItem.PresetMorphName.IsEmpty())
            PresetMorphs.Add(PresetItem.PresetMorphName[Gender]);
        }
    }
    return PresetMorphs;
}

FAvatarFacialMorphs UAvatarBuilderSubsystem::GetFacialMorphs()
{
    FString TablePath = TEXT("/ShibAvatarBuilder/V2/DataTables/DT_SkeletalBodyVariations_V2");

    UDataTable* DT = Cast<UDataTable>(StaticLoadObject(UDataTable::StaticClass(), nullptr, *TablePath));
    TArray<FName> RowNames = DT->GetRowNames();
    
    // Default skeletal mesh
    FAvatarSkeletalBodyVariations* DefaultVariation= DT->FindRow<FAvatarSkeletalBodyVariations>(RowNames[0], TEXT("Finding Default Variation"));
    return DefaultVariation->FacialMorphNames[Gender];
}

void UAvatarBuilderSubsystem::ApplyMorph(USkeletalMeshComponent* Holder, TArray<FName> MorphTargets, float Value)
{
    if(MorphTargets.Num() == 2) // If there are a pair of morphs that need to be changed, i.e. Overweight and Thin
    {
        if(Value < 0.5) // i.e. Thin
        {
            float EarlyMorphValue = FMath::GetMappedRangeValueClamped(FVector2D(0.0f, 0.5f), FVector2D(0.5f, 0.0f), Value);
            Holder->SetMorphTarget(MorphTargets[0],EarlyMorphValue);
            Holder->SetMorphTarget(MorphTargets[1],0);
        }
        else if(Value > 0.5) // i.e. Overweight
        {
            float LateMorphValue = FMath::GetMappedRangeValueClamped(FVector2D(0.5f, 1.0f), FVector2D(0.0f, 0.5f), Value);
            Holder->SetMorphTarget(MorphTargets[0],0);
            Holder->SetMorphTarget(MorphTargets[1],LateMorphValue);
        }
        else // if exactly in between set both to zero
        {
            Holder->SetMorphTarget(MorphTargets[1],0);
            Holder->SetMorphTarget(MorphTargets[0],0);
        }
    }

        

}

void UAvatarBuilderSubsystem::ApplyMorph(USkeletalMeshComponent* Holder, FName MorphTarget, float Value)
{
    if (!MorphTarget.IsNone())  // If we are applying one type of morph, i.e. presets
    {
        Holder->SetMorphTarget(MorphTarget,Value);
    }
}

float UAvatarBuilderSubsystem::GetBodyWeight()
{
    FString BodyTypeString = GetBodyTypeName().ToString();

    if(BodyTypeString.Contains("OverWeight")) return 1;
    if (BodyTypeString.Contains("Thin")) return 0;
    return 0.5;
    
}

FName UAvatarBuilderSubsystem::GetMappedJsonBodyMorphName(FAvatarFacialFeatureHandle Feature, FAvatarDimensionHandle Dimension)
{
    
    FName MappedName;

    // Check if morph type is MouthLength, change it to mouthThickness since that was being used for the morph in the prev json structure
    if(Feature.FacialFeatureType.IsEqual("Mouth") && Dimension.DimensionType.IsEqual("Length"))
    {
        MappedName = "mouthThickness";
    }
    else // Old json structure followed this format for morphs; Feature(lowercase)+Dimension(first letter capitalized). i.e. mouthThickness
    {
        MappedName = FName(Feature.FacialFeatureType.ToString().ToLower() + Dimension.DimensionType.ToString());
    }

    return MappedName;
}

void UAvatarBuilderSubsystem::GetFeatureAndDimensionFromMappedJsonBodyMorphName(
    const FName& MappedName, 
    FAvatarFacialFeatureHandle& OutFeature, 
    FAvatarDimensionHandle& OutDimension)
{
    FString MappedNameStr = MappedName.ToString();

    // Special case: Reverse the hardcoded "mouthThickness" mapping
    if (MappedNameStr == "mouthThickness")
    {
        OutFeature.FacialFeatureType = FName("Mouth");
        OutDimension.DimensionType = FName("Length");
        return;
    }

    // Extract feature and dimension from the formatted morph name
    int32 FirstCapitalIndex = -1;
    for (int32 i = 0; i < MappedNameStr.Len(); i++)
    {
        if (FChar::IsUpper(MappedNameStr[i]))
        {
            FirstCapitalIndex = i;
            break;
        }
    }

    // Ensure valid split point found
    if (FirstCapitalIndex > 0)
    {
        FString FeatureStr = MappedNameStr.Left(FirstCapitalIndex); // Lowercase feature
        FString DimensionStr = MappedNameStr.Mid(FirstCapitalIndex); // Capitalized dimension

        OutFeature.FacialFeatureType = FName(*FeatureStr);
        OutDimension.DimensionType = FName(*DimensionStr);
    }
    else
    {
        // Default case: Invalid format, return empty handles
        OutFeature.FacialFeatureType = FName();
        OutDimension.DimensionType = FName();
    }
}


void UAvatarBuilderSubsystem::ApplyBodyMorph(float SliderValue, FAvatarFacialFeatureHandle Feature, FAvatarDimensionHandle Dimension, bool bFinal)
{
    // Validate Feature and Dimension before proceeding
    if (Feature.FacialFeatureType.IsNone() || Dimension.DimensionType.IsNone()) 
    {
        UE_LOG(LogTemp, Warning, TEXT("%hs - Invalid Feature or Dimension! Skipping ApplyBodyMorph."), __FUNCTION__);
        return;
    }

    if (!AvatarCharacter)
    {
        UE_LOG(LogTemp, Error, TEXT("%hs - AvatarCharacter is nullptr! Cannot apply body morph."), __FUNCTION__);
        return;
    }

    const auto& BodyMesh = AvatarCharacter->GetBody();
    if (!BodyMesh)
    {
        UE_LOG(LogTemp, Error, TEXT("%hs - Body Mesh is nullptr! Cannot apply body morph."), __FUNCTION__);
        return;
    }

    // Validate Gender
    FCurrentAvatarState AvatarState;
    AvatarState.bIsMale = (GetGender().Gender == "Male");
    AvatarState=AvatarOperationsManager->AddToStateTemporary(AvatarState);
    
    // Validate Facial Morph Data
    const auto& FacialMorphData = GetFacialMorphs().FacialMorphData;
    if (!FacialMorphData.Contains(Feature))
    {
        UE_LOG(LogTemp, Warning, TEXT("%hs - No morph data found for feature: %s"), __FUNCTION__, *Feature.FacialFeatureType.ToString());
        return;
    }

    if (!FacialMorphData[Feature].FeatureMorphs.Contains(Dimension))
    {
        UE_LOG(LogTemp, Warning, TEXT("%hs - No morph data found for feature: %s and dimension: %s"), 
               __FUNCTION__, *Feature.FacialFeatureType.ToString(), *Dimension.DimensionType.ToString());
        return;
    }

    // Retrieve Morph Targets
    const TArray<FName>& MorphTargets = FacialMorphData[Feature].FeatureMorphs[Dimension].Names;
    if (MorphTargets.IsEmpty())
    {
        UE_LOG(LogTemp, Warning, TEXT("%hs - No morph targets found for feature: %s, dimension: %s"), 
               __FUNCTION__, *Feature.FacialFeatureType.ToString(), *Dimension.DimensionType.ToString());
        return;
    }

    // Apply Morphs to Body
    ApplyMorph(BodyMesh, MorphTargets, SliderValue);
    UE_LOG(LogTemp, Log, TEXT("%hs - Applied morph for feature: %s, dimension: %s with slider value: %f"), 
           __FUNCTION__, *Feature.FacialFeatureType.ToString(), *Dimension.DimensionType.ToString(), SliderValue);

    // **Apply Morphs to Accessories that have Facial Morphs**
    for (auto& AccessoryPair : AvatarState.AccessoryRowNameData)
    {
        FName AccessoryCategory = AccessoryPair.Key;
        FName AccessoryRow = AccessoryPair.Value;

        // Find accessory data table
        UDataTable* AccessoryTable = AvatarOperationsManager->FindDataTableByName("Accessories");
        if (!AccessoryTable) continue;

        // Find the Accessory Item
        FAccessoryItem* AccessoryItem = AccessoryTable->FindRow<FAccessoryItem>(AccessoryRow, "Finding AccessoryItem");
        if (!AccessoryItem || !AccessoryItem->bHasFacialMorphs) continue;

        // Apply morphs if the accessory supports it
        for (auto [AccessoryFeature, Dimensions] : AccessoryItem->FacialMorphNames[GetGender()].FacialMorphData)
        {
            if (AccessoryFeature == Feature) // Check if this accessory supports the same morph feature
            {
                for (auto [DimensionHandle, Morphs] : Dimensions.FeatureMorphs)
                {
                    if (DimensionHandle == Dimension) // Check if the dimension matches
                    {
                        ApplyMorph(GetMeshHolders(*AccessoryItem), Morphs.Names, SliderValue);
                    }
                }
            }
        }
    }

    if (bFinal)
    {
        // Validate AvatarOperationsManager before proceeding
        if (!AvatarOperationsManager)
        {
            UE_LOG(LogTemp, Error, TEXT("%hs - AvatarOperationsManager is nullptr! Cannot save morph state."), __FUNCTION__);
            return;
        }

        // Retrieve save name for JSON mapping
        FName SaveName = GetMappedJsonBodyMorphName(Feature, Dimension);
        UE_LOG(LogTemp, Log, TEXT("%hs - SaveName is: %s"), __FUNCTION__, *SaveName.ToString());

        // Update Morph Data and Save State
        AvatarState.MorphData.Add(SaveName, SliderValue);
        AvatarOperationsManager->AddToState(AvatarState);

        UE_LOG(LogTemp, Log, TEXT("%hs - Successfully saved morph state for %s with value %f"), 
               __FUNCTION__, *SaveName.ToString(), SliderValue);
    }
}



void UAvatarBuilderSubsystem::ApplyHeightMorph(float SliderValue, bool bFinal)
{
    FCurrentAvatarState AvatarState;
    AvatarState.bIsMale = (GetGender().Gender == "Male");
    
    AvatarCharacter->SetNewHeightFromScale(SliderValue);
    
    if (bFinal)
    {
        AvatarState.MorphData.Add("avatarHeight",SliderValue);
        AvatarState=AvatarOperationsManager->AddToStateTemporary(AvatarState);
        AvatarOperationsManager->AddToState(AvatarState);
        
        // Log for Current State (Temporary)
        
    }
    
}

void UAvatarBuilderSubsystem::ApplyTextureColors(USkeletalMeshComponent* Holder,const FName& ItemName,
    const FMaterialCustomization& Color, bool bFinal,bool IsTattoo)
{
    if (!Holder)return;
    
    FCurrentAvatarState AvatarState;

    if(!Color.MaterialIndices.IsValidIndex(0))
    {return;}
    
    UMaterialInstanceDynamic* Material = Holder->CreateDynamicMaterialInstance(Color.MaterialIndices[0], Holder->GetMaterial(Color.MaterialIndices[0]));
    
    if(Material)
    {
        Material->SetVectorParameterValue(Color.ColorParameter, Color.DefaultColor);

        if (bFinal)
        {
            AvatarState.bIsMale= (GetGender().Gender == "Male");
            if (IsTattoo)
            {
                AvatarState.TattooShibColorData.Add(ItemName,Color.DefaultColor);
            }
            else
            {
                AvatarState.ShibTextureColorData.Add(ItemName, Color.DefaultColor);  
            }
            
            AvatarState=AvatarOperationsManager->AddToStateTemporary(AvatarState);
            AvatarOperationsManager->AddToState(AvatarState);
        }
    }
}

void UAvatarBuilderSubsystem::ApplyMeshColor(UDataTable* ItemTable, const FName& ItemRow,
    const FLinearColor& Color, bool bFinal)
{
    FCurrentAvatarState AvatarState;
    FMeshItem* MeshItem = ItemTable->FindRow<FMeshItem>(ItemRow, "ItemRow not Found");
    
    FName ItemColorParam = MeshItem->MaterialData.ColorParameter;
    int ItemIndices =MeshItem->MaterialData.MaterialIndices.Num();
	auto Holder = GetMeshHolders(*MeshItem);

    if (ItemColorParam != "None" && ItemIndices > 0)
    {
        for (auto & Index:MeshItem->MaterialData.MaterialIndices)
        {
            if (!IsValid(Holder) || !IsValid(Holder->GetSkeletalMeshAsset()) )
            {continue;}
            
            auto& MaterialsArray = Holder->GetSkeletalMeshAsset()->GetMaterials();
            
            if(!MaterialsArray.IsValidIndex(Index))
            {continue;}
            
            if(UMaterialInstanceDynamic* Material = Holder->CreateDynamicMaterialInstance(Index,MaterialsArray[Index].MaterialInterface))
            {
                if (IsValid(Material))
                {
                    Material->SetVectorParameterValue(MeshItem->MaterialData.ColorParameter, Color);
                }
            }
        }
    }
  
    if (bFinal)
    {
        // Save Clothing Subtype if Mesh is Clothing other wise Save the Mesh Info
        if (MeshItem->ItemCategory.ItemCategory == "Clothing")
        {
            FClothingItem* SubItem = ItemTable->FindRow<FClothingItem>(ItemRow, "ItemRow not Found");
            AvatarState.ShibMeshColorData.Add(SubItem->ClothingType.ClothingType, Color);
        }
        else if (MeshItem->ItemCategory.ItemCategory == "Accessories")
        {
            FAccessoryItem* AccessoryItem = ItemTable->FindRow<FAccessoryItem>(ItemRow, "ItemRow not Found");
            AvatarState.AccessoryShibColorData.Add(AccessoryItem->AccessoryType.AccessoryType, Color);
        }
        else
        {
            AvatarState.ShibMeshColorData.Add(MeshItem->ItemCategory.ItemCategory, Color);
        }

        
        AvatarState=AvatarOperationsManager->AddToStateTemporary(AvatarState);
        AvatarOperationsManager->AddToState(AvatarState);
    }
    
    AvatarOperationsManager->PrintCurrentState();
}

// Shows Color Picker
bool UAvatarBuilderSubsystem::ColorPickerEnable(UDataTable* ItemTable, const FName& ItemRow)
{
    FMeshItem* MeshItem = ItemTable->FindRow<FMeshItem>(ItemRow, "Finding ItemRow in ColorPicker Enable");

    // Checks if the struct is for a Tattoo
    if (!MeshItem)
    {
        if (FTextureItem* TextureItem = ItemTable->FindRow<FTextureItem>(ItemRow, "Finding ItemRow Texture in ColorPicker Enable"))
        {
            if (TextureItem)
            {
                FName ItemColorParam = TextureItem->MaterialData.ColorParameter;
                int ItemIndices =TextureItem->MaterialData.MaterialIndices.Num();
    
                if (ItemColorParam != "None" && ItemIndices > 0)
                {
                    return true;
                }
                return false;
            }
        }
        
        AvatarOperationsManager->PrintCurrentState();
        return false;
    }
    
    FName ItemColorParam = MeshItem->MaterialData.ColorParameter;
    int ItemIndices =MeshItem->MaterialData.MaterialIndices.Num();
    
    if (ItemColorParam != "None" && ItemIndices > 0)
    {
        return true;
    }
    
    return false;
}

void UAvatarBuilderSubsystem::SetColorAccordingToType(FUIButtonsData ButtonData,FLinearColor Color,bool bFinal)
{
    FPrimitiveItem *PrimitiveItem=ButtonData.DataTable->FindRow<FPrimitiveItem>(ButtonData.RowName,"Finding Row To Apply Color");
    if (PrimitiveItem)
    {
        FName ItemCategory=PrimitiveItem->ItemCategory.ItemCategory;
        if (ItemCategory=="Clothing" || ItemCategory=="Hair" || ItemCategory=="Accessories")
        {
          ApplyMeshColor(ButtonData.DataTable,ButtonData.RowName,Color,bFinal);  
        }
        else
        {
            FTattooItem *TattooItem=ButtonData.DataTable->FindRow<FTattooItem>(ButtonData.RowName,"Finding Row To Apply Color");
            if (TattooItem)
            {
                FTattooItem TattooItemCopy=*TattooItem;
                TattooItemCopy.MaterialData.DefaultColor=Color;
                ApplyTextureColors(AvatarCharacter->GetBody(),TattooItemCopy.Placement.TattooPlacement,TattooItemCopy.MaterialData,bFinal,true);
            }
        }
    }
}

TArray<UDataTable*> UAvatarBuilderSubsystem::GetAllItemDataTables()
{
    TArray<UDataTable*> DataTables;

    FString TablePath = TEXT("/ShibAvatarBuilder/V2/Enums/DT_ItemCategory");
    UDataTable* CategoryDT = Cast<UDataTable>(StaticLoadObject(UDataTable::StaticClass(), nullptr, *TablePath));

    if (!CategoryDT)
    {
        UE_LOG(LogTemp, Error, TEXT("GetAllItemDataTables: Failed to load item category data table."));
        return DataTables;
    }

    // Iterate through the item categories to get all associated data tables
    for (const FName& RowName : CategoryDT->GetRowNames())
    {
        FItemCategoryStruct* ItemCategoryStruct = CategoryDT->FindRow<FItemCategoryStruct>(RowName, TEXT("Finding Item Category Row"));
        if (!ItemCategoryStruct) continue;

        // If multiple tables exist under this category
        if (ItemCategoryStruct->bMultipleDataTables)
        {
            for (const auto& TableEntry : ItemCategoryStruct->DataTablesByNames)
            {
                if (IsValid(TableEntry.Value))
                {
                    DataTables.Add(TableEntry.Value.Get());
                }
            }
        }
        else if (ItemCategoryStruct->DataTable)
        {
            DataTables.Add(ItemCategoryStruct->DataTable);
        }
    }

    return DataTables;
}

bool UAvatarBuilderSubsystem::HasConflict(FItemCompatibility ItemCompatibility, FCurrentAvatarState CurrentState, FName ApplyingCategory)
{
    FName Category = ItemCompatibility.GetIncompatibleItemCategory();

    if (Category == "Clothing")
    {
        Category = ItemCompatibility.ClothingType.ClothingType;
    }

    // **Ignore conflicts between Shoes and Bottoms ONLY**
    if ((ApplyingCategory == "Shoes" && Category == "Bottom") || 
        (ApplyingCategory == "Bottom" && Category == "Shoes"))
    {
        UE_LOG(LogTemp, Log, TEXT("Ignoring conflict between %s and %s"), *ApplyingCategory.ToString(), *Category.ToString());
        return false;
    }

    // **Otherwise, apply normal conflict check**
    if (CurrentState.MeshAssetRowNameData.Contains(Category) || CurrentState.AccessoryRowNameData.Contains(Category))
    {
        UE_LOG(LogTemp, Warning, TEXT("Conflict detected: %s already in state"), *Category.ToString());
        return true;
    }

    return false;
}



void UAvatarBuilderSubsystem::GenerateRandomAvatar()
{
    if (!AvatarOperationsManager)
    {
        UE_LOG(LogTemp, Error, TEXT("GenerateRandomAvatar: AvatarOperationsManager is null"));
        return;
    }
    
    FCurrentAvatarState RandomState;
    RandomState.bIsMale = FMath::RandBool(); // Randomly decide gender
    
    FGenderEntryHandle MaleKey;
    MaleKey.Gender = "Male";
    FGenderEntryHandle FemaleKey;
    FemaleKey.Gender = "Female";
    FGenderEntryHandle SelectedGender = RandomState.bIsMale ? MaleKey : FemaleKey;

    //RandomState.MorphData.Add("avatarWeight", FMath::FRand());
    RandomState.MorphData.Add("avatarHeight", FMath::FRand());

    // Decide whether to use FullBody or separate Top & Bottom
    bool bFullBodyChosen = FMath::RandBool();
    TMap<FName, UDataTable*> CategoryToDataTableMap;

    // Load Item Category Table
    UDataTable* ItemCategoryTable = Cast<UDataTable>(StaticLoadObject(UDataTable::StaticClass(), nullptr, TEXT("/ShibAvatarBuilder/V2/Enums/DT_ItemCategory")));
    if (!ItemCategoryTable)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to load DT_ItemCategory!"));
        return;
    }

    // Populate Category to DataTable Map
    for (FName RowName : ItemCategoryTable->GetRowNames())
    {
        FItemCategoryStruct* CategoryRow = ItemCategoryTable->FindRow<FItemCategoryStruct>(RowName, TEXT("Looking up item category"));
        if (!CategoryRow) continue;

        if (CategoryRow->bMultipleDataTables)
        {
            for (auto Pair : CategoryRow->DataTablesByNames)
            {
                if (IsValid(Pair.Value))
                {
                    CategoryToDataTableMap.Add(Pair.Key, Pair.Value);
                }
            }
        }
        else
        {
            if (IsValid(CategoryRow->DataTable))
            {
                CategoryToDataTableMap.Add(RowName, CategoryRow->DataTable);
            }
        }
    }

    UE_LOG(LogTemp, Log, TEXT("Category to DataTable Map Populated with %d entries"), CategoryToDataTableMap.Num());

    // Initialize random seed only once for better randomness
    FMath::RandInit(FPlatformTime::Cycles());

    auto GetRandomColor = []() -> FLinearColor
    {
        return FLinearColor(FMath::RandRange(0.0f, 1.0f), FMath::RandRange(0.0f, 1.0f), FMath::RandRange(0.0f, 1.0f), 1.0f);
    };

    for (auto Entry : CategoryToDataTableMap)
    {
        // Enforce FullBody vs. Top+Bottom logic
        if (!bFullBodyChosen && Entry.Key.ToString().Contains("Fullbody")) continue;
        if (bFullBodyChosen && (Entry.Key.ToString().Contains("Top") || Entry.Key.ToString().Contains("Bottom"))) continue;

        UDataTable* DataTable = Entry.Value;
        TArray<FName> AllRowNames = DataTable->GetRowNames();
        if (AllRowNames.Num() == 0) continue;
        
        FName RowName;
        int32 RandomSelection;
        const int MaxRetries = 50;
        int RetryCount = 0;
        bool ItemHasConflict;
        bool ItemHasValidGender;
        FName ApplyingCategory = Entry.Key;

        RandomSelection = FMath::RandRange(0, AllRowNames.Num() - 1);
        RowName = AllRowNames[RandomSelection];
        
        const UScriptStruct* RowStruct = DataTable->GetRowStruct();
        if (RowStruct->IsChildOf(GET_STRUCT_TYPE(FMeshItem)))
        {
            do
            {
                RandomSelection = FMath::RandRange(0, AllRowNames.Num() - 1);
                RowName = AllRowNames[RandomSelection];

                // **Skip unwanted items ("NoTop", "NoBottom", "Remove")**
                if (RowName.ToString().Contains("NoTop") || RowName.ToString().Contains("NoBottom") || RowName.ToString().Contains("Remove"))
                {
                    UE_LOG(LogTemp, Warning, TEXT("Skipping unwanted item: %s"), *RowName.ToString());
                    continue;
                }

                
                if (!RowStruct)
                {
                    UE_LOG(LogTemp, Warning, TEXT("Invalid DataTable or RowStruct"));
                    return;
                }

                ItemHasConflict = false;
                ItemHasValidGender = false;

                // **Handle Mesh Conflicts & Gender Validation**
            
                FMeshItem* MeshItem = DataTable->FindRow<FMeshItem>(RowName, "");
                if (MeshItem)
                {
                    // Check if item has a valid gender mesh
                    FGenderEntryHandle UnisexKey;
                    UnisexKey.Gender = "Unisex";
                    if (MeshItem->MeshesByGender.Contains(SelectedGender) || MeshItem->MeshesByGender.Contains(UnisexKey))
                    {
                        ItemHasValidGender = true;
                    }
                    else
                    {
                        UE_LOG(LogTemp, Warning, TEXT("Item %s does not support selected gender: %s"), *RowName.ToString(), *SelectedGender.Gender.ToString());
                    }

                    // **Check for conflicts, but ignore only Shoe-Bottom conflicts**
                    ItemHasConflict = CheckForConflicts(MeshItem->CompatibilityRules, RandomState, ApplyingCategory);
                }
            

                RetryCount++;

            } while ((ItemHasConflict || !ItemHasValidGender) && RetryCount < MaxRetries);

            if (ItemHasConflict || !ItemHasValidGender)
            {
                UE_LOG(LogTemp, Warning, TEXT("Max retries reached! Could not resolve conflicts or find valid gender item for category: %s"), *Entry.Key.ToString());
                continue;
            }
        }

        UE_LOG(LogTemp, Log, TEXT("Final selected item for %s: %s"), *Entry.Key.ToString(), *RowName.ToString());

        // Add selected item and assign random color
        if (RowStruct->IsChildOf(GET_STRUCT_TYPE(FClothingItem)))
        {
            RandomState.MeshAssetRowNameData.Add(Entry.Key, RowName);
            RandomState.ShibMeshColorData.Add(Entry.Key, GetRandomColor());
        }
        else if (RowStruct->IsChildOf(GET_STRUCT_TYPE(FAccessoryItem)))
        {
            RandomState.AccessoryRowNameData.Add(Entry.Key, RowName);
            RandomState.AccessoryShibColorData.Add(Entry.Key, GetRandomColor());
        }
        else if (RowStruct->IsChildOf(GET_STRUCT_TYPE(FHairItem)))
        {
            RandomState.MeshAssetRowNameData.Add("Hair", RowName);
            RandomState.ShibMeshColorData.Add("Hair", GetRandomColor());
        }
        else if (RowStruct->IsChildOf(GET_STRUCT_TYPE(FSkinItem)))
        {
            int32 RowAsInt = FCString::Atoi(*RowName.ToString());
            RandomState.ShibTextureAssetRowNameData.Add("skin", NewSkintoOld[RowAsInt]);
        }
        else if (RowStruct->IsChildOf(GET_STRUCT_TYPE(FTattooItem)))
        {
            FTattooItem* TattooItem=Entry.Value->FindRow<FTattooItem>(RowName,"");
            if (TattooItem)
            {
                RandomState.TattooRowNameData.Add(TattooItem->Placement.TattooPlacement, RowName);
                RandomState.TattooShibColorData.Add(TattooItem->Placement.TattooPlacement, GetRandomColor());
            }
        }
        else if (RowStruct->IsChildOf(GET_STRUCT_TYPE(FPresetItem)))
        {
            RandomState.ShibTextureAssetRowNameData.Add("preset", RowName);
        }
    }
    
    ApplyAvatarCustomization(RandomState);
    
    AvatarOperationsManager->AddToState(RandomState);
    
    float tmpbodyweight = GetBodyWeight();
    
    SetBodyType(1, false);
    SetBodyType(0.5, false);
    SetBodyType(0, false);
    SetBodyType(tmpbodyweight, false);
}




/**
 * Checks if an item is a **Shoe** or **Bottom** and should be ignored for conflicts.
 */
bool UAvatarBuilderSubsystem::IsShoeOrBottomConflict(FMeshItem* MeshItem)
{
    if (!MeshItem) return false;

    if (MeshItem->ItemCategory.ItemCategory.ToString().Contains("Shoe") ||
        MeshItem->ItemCategory.ItemCategory.ToString().Contains("Bottom"))
    {
        return true;
    }
    return false;
}

/**
 * Checks if an item has conflicts with the current state.
 */
bool UAvatarBuilderSubsystem::CheckForConflicts(TArray<FItemCompatibility>& CompatibilityRules, FCurrentAvatarState& CurrentState, FName ApplyingCategory)
{
    for (FItemCompatibility Rule : CompatibilityRules)
    {
        if (HasConflict(Rule, CurrentState, ApplyingCategory))
        {
            UE_LOG(LogTemp, Warning, TEXT("Conflict detected for %s with rule: %s"), *ApplyingCategory.ToString(), *Rule.GetClothingType().ToString());
            return true;
        }
    }
    return false;
}

FLinearColor UAvatarBuilderSubsystem::GetColorFromButton(FUIButtonsData ButtonData)
{
    FMeshItem *MeshItem=ButtonData.DataTable->FindRow<FMeshItem>(ButtonData.RowName,"Finding Row To Find Item's Color");
    FTextureItem *TextureItem=ButtonData.DataTable->FindRow<FTextureItem>(ButtonData.RowName,"Finding Row To Find Item's Color");
    FCurrentAvatarState CurrentAvatarState=AvatarOperationsManager->GetCurrentState();
    if (MeshItem)
    {
        FName ItemCategory=MeshItem->ItemCategory.ItemCategory;
        if (ItemCategory=="Clothing")
        {
            FClothingItem * ClothingItem=static_cast<FClothingItem*>(MeshItem);
            if(ClothingItem)
            {
                if (CurrentAvatarState.ShibMeshColorData.Contains(ClothingItem->ClothingType.ClothingType))
                {
                    OnColorExtractedFromButton.Broadcast(CurrentAvatarState.ShibMeshColorData[ClothingItem->ClothingType.ClothingType]);
                    return CurrentAvatarState.ShibMeshColorData[ClothingItem->ClothingType.ClothingType];
                }
                OnColorExtractedFromButton.Broadcast(ClothingItem->MaterialData.DefaultColor);
                return ClothingItem->MaterialData.DefaultColor;
            }
        }
        else if (ItemCategory=="Accessories")
        {
            FAccessoryItem * AccessoryItem=static_cast<FAccessoryItem*>(MeshItem);
            if(AccessoryItem)
            {
                if (CurrentAvatarState.AccessoryShibColorData.Contains(AccessoryItem->AccessoryType.AccessoryType))
                {
                    OnColorExtractedFromButton.Broadcast(CurrentAvatarState.AccessoryShibColorData[AccessoryItem->AccessoryType.AccessoryType]);
                    return CurrentAvatarState.AccessoryShibColorData[AccessoryItem->AccessoryType.AccessoryType];
                }
                OnColorExtractedFromButton.Broadcast(AccessoryItem->MaterialData.DefaultColor);
                return AccessoryItem->MaterialData.DefaultColor;
            }
        }
        else
        {
            if (CurrentAvatarState.ShibMeshColorData.Contains(ItemCategory))
            {
                OnColorExtractedFromButton.Broadcast(CurrentAvatarState.ShibMeshColorData[ItemCategory]);
                return CurrentAvatarState.ShibMeshColorData[ItemCategory];
            }
            OnColorExtractedFromButton.Broadcast(MeshItem->MaterialData.DefaultColor);
            return MeshItem->MaterialData.DefaultColor;
        }
    }
    if (TextureItem)
    {
        FName ItemCategory=TextureItem->ItemCategory.ItemCategory;
        if (ItemCategory=="Tattoos")
        {
            FTattooItem * TattooItem=static_cast<FTattooItem*>(TextureItem);
            if(TattooItem)
            {
                if (CurrentAvatarState.TattooShibColorData.Contains(TattooItem->Placement.TattooPlacement))
                {
                    OnColorExtractedFromButton.Broadcast(CurrentAvatarState.TattooShibColorData[TattooItem->Placement.TattooPlacement]);
                    return CurrentAvatarState.TattooShibColorData[TattooItem->Placement.TattooPlacement];
                }
                OnColorExtractedFromButton.Broadcast(TattooItem->MaterialData.DefaultColor);
                return TattooItem->MaterialData.DefaultColor;
            }
        }
        else
        {
            if (CurrentAvatarState.ShibTextureColorData.Contains(ItemCategory))
            {
                OnColorExtractedFromButton.Broadcast(CurrentAvatarState.ShibTextureColorData[ItemCategory]);
                return CurrentAvatarState.ShibTextureColorData[ItemCategory];
            }
            OnColorExtractedFromButton.Broadcast(TextureItem->MaterialData.DefaultColor);
            return TextureItem->MaterialData.DefaultColor;
        }
    }
    OnColorExtractedFromButton.Broadcast(FLinearColor());
    return FLinearColor();
}














