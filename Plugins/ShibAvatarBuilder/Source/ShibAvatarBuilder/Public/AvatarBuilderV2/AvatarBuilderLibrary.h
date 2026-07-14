// Copyright Shiba Inu Games LLC.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "AvatarBuilderLibrary.generated.h"

USTRUCT(BlueprintType)
struct FClothingTypeData : public FTableRowBase
{
    GENERATED_BODY()

    /** ✅ List of valid subtypes */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FName> ValidSubtypes;
};

USTRUCT(BlueprintType)
struct FCheckableOption
{
    GENERATED_BODY()

    /** The option name */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName OptionName;

    /** Whether this option is checked */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsChecked = false;
};


UCLASS()
class SHIBAVATARBUILDER_API UDataTableRowNameHelper : public UObject
{
    GENERATED_BODY()

public:
    /** ✅ Data Table Paths Mapped by Type */
    static const TMap<FString, FString>& GetDataTablePaths()
    {
        static const TMap<FString, FString> DataTablePaths = {
            {TEXT("Gender"), TEXT("/ShibAvatarBuilder/V2/Enums/DT_Gender")},
            {TEXT("BodyType"), TEXT("/ShibAvatarBuilder/V2/Enums/DT_BodyType")},
            {TEXT("ClothingType"), TEXT("/ShibAvatarBuilder/V2/Enums/DT_ClothingTypes")},
            {TEXT("AccessoryType"), TEXT("/ShibAvatarBuilder/V2/Enums/DT_AccessoryType")},
            {TEXT("TattooPlacement"), TEXT("/ShibAvatarBuilder/V2/Enums/DT_TattooPlacement")},
            {TEXT("ItemCategory"), TEXT("/ShibAvatarBuilder/V2/Enums/DT_ItemCategory")},
            {TEXT("BottomType"), TEXT("/ShibAvatarBuilder/V2/Enums/DT_BottomType")},
            {TEXT("ShoeType"), TEXT("/ShibAvatarBuilder/V2/Enums/DT_ShoeType")},
            {TEXT("FullBodyType"), TEXT("/ShibAvatarBuilder/V2/Enums/DT_FullBodyType")},
            {TEXT("DimensionType"), TEXT("/ShibAvatarBuilder/V2/Enums/DT_DimensionType")},
            {TEXT("FacialFeatureType"), TEXT("/ShibAvatarBuilder/V2/Enums/DT_FacialFeatureType")}
        };
        return DataTablePaths;
    }

    /** ✅ Fetches row names from the provided Data Table path */
    static TArray<FName> GetRowNamesFromTable(const FString& TablePath)
    {
        TArray<FName> RowNames;

        if (UDataTable* Table = Cast<UDataTable>(StaticLoadObject(UDataTable::StaticClass(), nullptr, *TablePath)))
        {
            RowNames = Table->GetRowNames();
        }

        // ✅ Debugging: Log row names (if needed)
        if (RowNames.Num() == 0)
        {
            UE_LOG(LogTemp, Warning, TEXT("UDataTableRowNameHelper::GetRowNamesFromTable - No rows found in DataTable: %s"), *TablePath);
        }

        return RowNames;
    }

    /** ✅ Utility Function to Get Row Names by Enum Type */
    static TArray<FName> GetRowNamesByType(const FString& Type)
    {
        const TMap<FString, FString>& Paths = GetDataTablePaths();

        if (const FString* TablePath = Paths.Find(Type))
        {
            return GetRowNamesFromTable(*TablePath);
        }

        UE_LOG(LogTemp, Error, TEXT("UDataTableRowNameHelper::GetRowNamesByType - No Data Table found for type: %s"), *Type);
        return {};
    }

    UFUNCTION(CallInEditor, BlueprintCallable)
    static TArray<FCheckableOption> GetCheckableOptions(FName ClothingType)
    {
        TArray<FCheckableOption> Options;

        // ✅ Load the ClothingType DataTable
        UDataTable* ClothingTypeTable = Cast<UDataTable>(StaticLoadObject(UDataTable::StaticClass(), nullptr, TEXT("/ShibAvatarBuilder/V2/Enums/DT_ClothingTypes")));
        if (!ClothingTypeTable)
        {
            UE_LOG(LogTemp, Warning, TEXT("GetCheckableOptions: Failed to load DT_ClothingTypes!"));
            return Options;
        }

        // ✅ Find the row corresponding to the selected ClothingType
        FClothingTypeData* RowData = ClothingTypeTable->FindRow<FClothingTypeData>(ClothingType, TEXT("Fetching Clothing Type"));
        if (!RowData)
        {
            UE_LOG(LogTemp, Warning, TEXT("GetCheckableOptions: Clothing type %s not found!"), *ClothingType.ToString());
            return Options;
        }

        // ✅ Convert the ValidSubtypes array into a checkable list
        for (const FName& Subtype : RowData->ValidSubtypes)
        {
            FCheckableOption NewOption;
            NewOption.OptionName = Subtype;
            NewOption.bIsChecked = false; // Default unchecked
            Options.Add(NewOption);
        }

        return Options;
    }

    /** ✅ Specific Functions for Blueprint `GetOptions` */
    UFUNCTION(CallInEditor, BlueprintCallable) static TArray<FName> GetRowNamesGender() { return GetRowNamesByType("Gender"); }
    UFUNCTION(CallInEditor, BlueprintCallable) static TArray<FName> GetRowNamesBodyType() { return GetRowNamesByType("BodyType"); }
    UFUNCTION(CallInEditor, BlueprintCallable) static TArray<FName> GetRowNamesClothingType() { return GetRowNamesByType("ClothingType"); }
    UFUNCTION(CallInEditor, BlueprintCallable) static TArray<FName> GetRowNamesAccessoryType() { return GetRowNamesByType("AccessoryType"); }
    UFUNCTION(CallInEditor, BlueprintCallable) static TArray<FName> GetRowNamesTattooPlacement() { return GetRowNamesByType("TattooPlacement"); }
    UFUNCTION(CallInEditor, BlueprintCallable) static TArray<FName> GetRowNamesItemCategory() { return GetRowNamesByType("ItemCategory"); }
    UFUNCTION(CallInEditor, BlueprintCallable) static TArray<FName> GetRowNamesBottomType() { return GetRowNamesByType("BottomType"); }
    UFUNCTION(CallInEditor, BlueprintCallable) static TArray<FName> GetRowNamesShoeType() { return GetRowNamesByType("ShoeType"); }
    UFUNCTION(CallInEditor, BlueprintCallable) static TArray<FName> GetRowNamesFullBodyType() { return GetRowNamesByType("FullBodyType"); }
    UFUNCTION(CallInEditor, BlueprintCallable) static TArray<FName> GetRowNamesDimensionType() { return GetRowNamesByType("DimensionType"); }
    UFUNCTION(CallInEditor, BlueprintCallable) static TArray<FName> GetRowNamesFacialFeatureType() { return GetRowNamesByType("FacialFeatureType"); }

};




USTRUCT(BlueprintType)
struct SHIBAVATARBUILDER_API FDataTableEntryHandle
{
    GENERATED_BODY()
};

/** Gender Entry Handle */
USTRUCT(BlueprintType)
struct SHIBAVATARBUILDER_API FGenderEntryHandle : public FDataTableEntryHandle
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = DataTableRowHandle, meta = (GetOptions = "ShibAvatarBuilder.DataTableRowNameHelper.GetRowNamesGender"))
    FName Gender;

    friend FORCEINLINE uint32 GetTypeHash(const FGenderEntryHandle& Handle)
    {
        return GetTypeHash(Handle.Gender);
    }

    bool operator==(const FGenderEntryHandle& Other) const
    {
        return Gender == Other.Gender;
    }

    bool operator!=(const FGenderEntryHandle& Other) const
    {
        return !(*this == Other);
    }
};

/** Body Type Entry Handle */
USTRUCT(BlueprintType)
struct SHIBAVATARBUILDER_API FBodyTypeEntryHandle : public FDataTableEntryHandle
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = DataTableRowHandle, meta = (GetOptions = "ShibAvatarBuilder.DataTableRowNameHelper.GetRowNamesBodyType"))
    FName BodyType;

    friend FORCEINLINE uint32 GetTypeHash(const FBodyTypeEntryHandle& Handle)
    {
        return GetTypeHash(Handle.BodyType);
    }

    bool operator==(const FBodyTypeEntryHandle& Other) const
    {
        return BodyType == Other.BodyType;
    }

    bool operator!=(const FBodyTypeEntryHandle& Other) const
    {
        return !(*this == Other);
    }
};

/** Clothing Type Entry Handle */
USTRUCT(BlueprintType)
struct SHIBAVATARBUILDER_API FClothingTypeEntryHandle : public FDataTableEntryHandle
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = DataTableRowHandle, meta = (GetOptions = "ShibAvatarBuilder.DataTableRowNameHelper.GetRowNamesClothingType"))
    FName ClothingType;
    
    friend FORCEINLINE uint32 GetTypeHash(const FClothingTypeEntryHandle& Handle)
    {
        return GetTypeHash(Handle.ClothingType);
    }

    bool operator==(const FClothingTypeEntryHandle& Other) const
    {
        return ClothingType == Other.ClothingType;
    }

    bool operator!=(const FClothingTypeEntryHandle& Other) const
    {
        return !(*this == Other);
    }
};

/** Accessory Type Entry Handle */
USTRUCT(BlueprintType)
struct SHIBAVATARBUILDER_API FAccessoryTypeEntryHandle : public FDataTableEntryHandle
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = DataTableRowHandle, meta = (GetOptions = "ShibAvatarBuilder.DataTableRowNameHelper.GetRowNamesAccessoryType"))
    FName AccessoryType;

    friend FORCEINLINE uint32 GetTypeHash(const FAccessoryTypeEntryHandle& Handle)
    {
        return GetTypeHash(Handle.AccessoryType);
    }

    bool operator==(const FAccessoryTypeEntryHandle& Other) const
    {
        return AccessoryType == Other.AccessoryType;
    }

    bool operator!=(const FAccessoryTypeEntryHandle& Other) const
    {
        return !(*this == Other);
    }
};

/** Tattoo Placement Entry Handle */
USTRUCT(BlueprintType)
struct SHIBAVATARBUILDER_API FTattooPlacementEntryHandle : public FDataTableEntryHandle
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = DataTableRowHandle, meta = (GetOptions = "ShibAvatarBuilder.DataTableRowNameHelper.GetRowNamesTattooPlacement"))
    FName TattooPlacement;

    friend FORCEINLINE uint32 GetTypeHash(const FTattooPlacementEntryHandle& Handle)
    {
        return GetTypeHash(Handle.TattooPlacement);
    }

    bool operator==(const FTattooPlacementEntryHandle& Other) const
    {
        return TattooPlacement == Other.TattooPlacement;
    }

    bool operator!=(const FTattooPlacementEntryHandle& Other) const
    {
        return !(*this == Other);
    }
};

/** Item Category Entry Handle */
USTRUCT(BlueprintType)
struct SHIBAVATARBUILDER_API FItemCategoryEntryHandle : public FDataTableEntryHandle
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = DataTableRowHandle, meta = (GetOptions = "ShibAvatarBuilder.DataTableRowNameHelper.GetRowNamesItemCategory"))
    FName ItemCategory;

    friend FORCEINLINE uint32 GetTypeHash(const FItemCategoryEntryHandle& Handle)
    {
        return GetTypeHash(Handle.ItemCategory);
    }

    bool operator==(const FItemCategoryEntryHandle& Other) const
    {
        return ItemCategory == Other.ItemCategory;
    }

    bool operator!=(const FItemCategoryEntryHandle& Other) const
    {
        return !(*this == Other);
    }
};

/** Bottom Type Entry Handle */
USTRUCT(BlueprintType)
struct SHIBAVATARBUILDER_API FBottomTypeEntryHandle : public FDataTableEntryHandle
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = DataTableRowHandle, meta = (GetOptions = "ShibAvatarBuilder.DataTableRowNameHelper.GetRowNamesBottomType"))
    FName BottomType;

    friend FORCEINLINE uint32 GetTypeHash(const FBottomTypeEntryHandle& Handle)
    {
        return GetTypeHash(Handle.BottomType);
    }

    bool operator==(const FBottomTypeEntryHandle& Other) const
    {
        return BottomType == Other.BottomType;
    }

    bool operator!=(const FBottomTypeEntryHandle& Other) const
    {
        return !(*this == Other);
    }
};

/** Shoe Type Entry Handle */
USTRUCT(BlueprintType)
struct SHIBAVATARBUILDER_API FShoeTypeEntryHandle : public FDataTableEntryHandle
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = DataTableRowHandle, meta = (GetOptions = "ShibAvatarBuilder.DataTableRowNameHelper.GetRowNamesShoeType"))
    FName ShoeType;

    friend FORCEINLINE uint32 GetTypeHash(const FShoeTypeEntryHandle& Handle)
    {
        return GetTypeHash(Handle.ShoeType);
    }

    bool operator==(const FShoeTypeEntryHandle& Other) const
    {
        return ShoeType == Other.ShoeType;
    }

    bool operator!=(const FShoeTypeEntryHandle& Other) const
    {
        return !(*this == Other);
    }
};

/** Full Body Type Entry Handle */
USTRUCT(BlueprintType)
struct SHIBAVATARBUILDER_API FFullBodyTypeEntryHandle : public FDataTableEntryHandle
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = DataTableRowHandle, meta = (GetOptions = "ShibAvatarBuilder.DataTableRowNameHelper.GetRowNamesFullBodyType"))
    FName FullBodyType;

    friend FORCEINLINE uint32 GetTypeHash(const FFullBodyTypeEntryHandle& Handle)
    {
        return GetTypeHash(Handle.FullBodyType);
    }

    bool operator==(const FFullBodyTypeEntryHandle& Other) const
    {
        return FullBodyType == Other.FullBodyType;
    }

    bool operator!=(const FFullBodyTypeEntryHandle& Other) const
    {
        return !(*this == Other);
    }
};



/** Facial Dimension Type Entry Handle */
USTRUCT(BlueprintType)
struct SHIBAVATARBUILDER_API FAvatarDimensionHandle : public FDataTableEntryHandle
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = DataTableRowHandle, meta = (GetOptions = "ShibAvatarBuilder.DataTableRowNameHelper.GetRowNamesDimensionType"))
    FName DimensionType;

    friend FORCEINLINE uint32 GetTypeHash(const FAvatarDimensionHandle& Handle)
    {
        return GetTypeHash(Handle.DimensionType);
    }

    bool operator==(const FAvatarDimensionHandle& Other) const
    {
        return DimensionType == Other.DimensionType;
    }

    bool operator!=(const FAvatarDimensionHandle& Other) const
    {
        return !(*this == Other);
    }
};

/** Full Body Type Entry Handle */
USTRUCT(BlueprintType)
struct SHIBAVATARBUILDER_API FAvatarFacialFeatureHandle : public FDataTableEntryHandle
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = DataTableRowHandle, meta = (GetOptions = "ShibAvatarBuilder.DataTableRowNameHelper.GetRowNamesFacialFeatureType"))
    FName FacialFeatureType;

    friend FORCEINLINE uint32 GetTypeHash(const FAvatarFacialFeatureHandle& Handle)
    {
        return GetTypeHash(Handle.FacialFeatureType);
    }

    bool operator==(const FAvatarFacialFeatureHandle& Other) const
    {
        return FacialFeatureType == Other.FacialFeatureType;
    }

    bool operator!=(const FAvatarFacialFeatureHandle& Other) const
    {
        return !(*this == Other);
    }
};

// ----------------------- --------------------------------- ----------------------------- --------------------------- -----------------------



USTRUCT(BlueprintType)
struct FAvatarMorphNames
{
    GENERATED_BODY();

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FName> Names;
};

USTRUCT(BlueprintType)
struct FAvatarFacialDimensionMorphs
{
    GENERATED_BODY();

    UPROPERTY(EditAnywhere,BlueprintReadWrite)
    TMap<FAvatarDimensionHandle,FAvatarMorphNames> FeatureMorphs;
};

USTRUCT(BlueprintType)
struct FAvatarFacialMorphs
{
    GENERATED_BODY();

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<FAvatarFacialFeatureHandle,FAvatarFacialDimensionMorphs> FacialMorphData;
    
};



USTRUCT(BlueprintType)
struct FBodyTypeMeshes {
    GENERATED_BODY()

    /** Stores meshes for Thin, Fit, and Overweight */
    UPROPERTY(EditAnywhere, BlueprintReadWrite,meta=(ShowOnlyInnerProperties))
    TMap<FBodyTypeEntryHandle, TObjectPtr<USkeletalMesh>> MeshesByBodyType;
};

USTRUCT(BlueprintType)
struct FMaterialCustomization {
    GENERATED_BODY()

    /** Default color */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FLinearColor DefaultColor = FLinearColor::White;

    /** Material parameter name */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName ColorParameter;

    /** Material slot indices */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<int32> MaterialIndices;
};

USTRUCT(BlueprintType)
struct FItemCompatibility
{
    GENERATED_BODY()
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Compatibility")
    FItemCategoryEntryHandle IncompatibleCategory;

    UPROPERTY(Transient)
    bool bClothing=false;

    UPROPERTY(Transient)
    bool bAccessory=false;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite,meta=(EditCondition=bClothing,EditConditionHides))
    FClothingTypeEntryHandle ClothingType;

    UPROPERTY()
    FClothingTypeEntryHandle PreviousClothingType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ShowOnlyInnerProperties,EditCondition=bClothing,EditConditionHides))
    TArray<FCheckableOption> SelectSubtype;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite,meta=(EditCondition=bAccessory,EditConditionHides))
    FAccessoryTypeEntryHandle AccessoryType;

    UPROPERTY(EditAnywhere,BlueprintReadWrite)
    bool bExecuteRuleInConflictingItemInstead;

    UPROPERTY(EditAnywhere, BlueprintReadWrite,meta=(EditCondition="!bExecuteRuleInConflictingItemInstead",EditConditionHides))
    FDataTableRowHandle ResolveByApplyingMesh;
    
    FName GetIncompatibleItemCategory()
    {
        return IncompatibleCategory.ItemCategory;
    }

    FName GetClothingType()
    {
        return ClothingType.ClothingType;
    }

    FName GetAccessoryType()
    {
        return AccessoryType.AccessoryType;
    }
    
};

/** 🔹 Primitive Struct for All Items */
USTRUCT(BlueprintType)
struct FPrimitiveItem : public FTableRowBase
{
    GENERATED_BODY()
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FItemCategoryEntryHandle ItemCategory;
};

/** Base Struct for Items Other than PresetItem */
USTRUCT(BlueprintType)
struct FBaseItem : public FPrimitiveItem
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<FGenderEntryHandle, TObjectPtr<UTexture2D>> Thumbnails;
};


USTRUCT(BlueprintType)
struct FMeshItem : public FBaseItem
{
    GENERATED_BODY()

    // Morph Types - Weight and Dimension Morphs
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bHasWeightMorphs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Editcondition="bHasWeightMorphs",EditConditionHides))
    TArray<FName> WeightMorphs;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite,meta=(ShowOnlyInnerProperties))
    TMap<FGenderEntryHandle, FBodyTypeMeshes> MeshesByGender;

    /** Compatibility rules */
    UPROPERTY(EditAnywhere, BlueprintReadWrite,meta=(ShowOnlyInnerProperties))
    TArray<FItemCompatibility> CompatibilityRules;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FDataTableRowHandle> AllowedExceptions;

    /** Material customization */
    UPROPERTY(EditAnywhere, BlueprintReadWrite,meta=(ShowOnlyInnerProperties))
    FMaterialCustomization MaterialData;

    

    virtual void OnDataTableChanged(const UDataTable* InDataTable, const FName InRowName) override
    {
        for (FItemCompatibility& CompatibilityRule : CompatibilityRules)
        {
            if(CompatibilityRule.IncompatibleCategory.ItemCategory=="Clothing")
            {
                
                CompatibilityRule.bClothing=true;
                CompatibilityRule.bAccessory=false;

                if (CompatibilityRule.PreviousClothingType==CompatibilityRule.ClothingType)
                {
                    
                }
                else
                {
                    CompatibilityRule.PreviousClothingType=CompatibilityRule.ClothingType;
                    CompatibilityRule.SelectSubtype = UDataTableRowNameHelper::GetCheckableOptions(CompatibilityRule.ClothingType.ClothingType);
                }
                
                
            }
            else if(CompatibilityRule.IncompatibleCategory.ItemCategory=="Accessories")
            {
                CompatibilityRule.bClothing=false;
                CompatibilityRule.bAccessory=true;
                CompatibilityRule.SelectSubtype.Empty();
                CompatibilityRule.PreviousClothingType=FClothingTypeEntryHandle();
            }
            else
            {
                CompatibilityRule.bClothing=false;
                CompatibilityRule.bAccessory=false;
                CompatibilityRule.SelectSubtype.Empty();
                CompatibilityRule.PreviousClothingType=FClothingTypeEntryHandle();
            }
        }
    }
        
};

USTRUCT(BlueprintType)
struct FTextureAndParameters
{
    GENERATED_BODY()
public:
    /** Textures (e.g., Base Texture, Normal Map) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Eyebrows", meta = (ShowOnlyInnerProperties))
    TArray<TObjectPtr<UTexture>> Textures;

    /** Material texture parameters */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Eyebrows", meta = (ShowOnlyInnerProperties))
    TArray<FName> TextureParameters;
};

USTRUCT(BlueprintType)
struct FTextureItem : public FBaseItem
{
    GENERATED_BODY()

    /** Textures By Gender */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<FGenderEntryHandle,FTextureAndParameters> Textures;

    UPROPERTY(EditAnywhere, BlueprintReadWrite,meta=(ShowOnlyInnerProperties))
    FMaterialCustomization MaterialData;
};



USTRUCT(BlueprintType)
struct FClothingItem : public FMeshItem
{
    GENERATED_BODY()

    FClothingItem()
    {
        ItemCategory.ItemCategory = TEXT("Clothing");
    }
    
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FClothingTypeEntryHandle ClothingType;

    UPROPERTY()
    FClothingTypeEntryHandle PreviousClothingType;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ShowOnlyInnerProperties))
    TArray<FCheckableOption> SelectSubtype;

    virtual void OnDataTableChanged(const UDataTable* InDataTable, const FName InRowName) override
    {
        Super::OnDataTableChanged(InDataTable, InRowName);
        
        int selectcount=0;
        for (const FCheckableOption& Subtype : SelectSubtype)
        {
            if(Subtype.bIsChecked)
            {
                selectcount++;
            }
        }
        if(selectcount>1)
        {
            for (FCheckableOption& Subtype : SelectSubtype)
            {
                if(Subtype.bIsChecked)
                {
                    Subtype.bIsChecked=false;
                }
            }
        }
        
        if (PreviousClothingType==ClothingType)
        {
            return;
        }

        PreviousClothingType=ClothingType;

        SelectSubtype =UDataTableRowNameHelper::GetCheckableOptions(ClothingType.ClothingType);
    }
};

USTRUCT(BlueprintType)
struct FHairItem : public FMeshItem
{
    GENERATED_BODY()

    FHairItem()
    {
        ItemCategory.ItemCategory = TEXT("Hair");
    }
    
};

/** 🔹 Struct for Accessories */
USTRUCT(BlueprintType)
struct FMorphTargetsByGender
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite,meta = (ShowOnlyInnerProperties))
    TArray<FName> MorphTargets;
    
};

/** Struct for Accessories */
USTRUCT(BlueprintType)
struct FAccessoryItem : public FMeshItem {
    GENERATED_BODY()

    FAccessoryItem()
    {
        ItemCategory.ItemCategory = TEXT("Accessories");
    }

    /** Type of accessory */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FAccessoryTypeEntryHandle AccessoryType;
    

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bHasFacialMorphs;

    UPROPERTY(BlueprintReadWrite, EditAnywhere,meta=(Editcondition="bHasDimensionMorphs",EditConditionHides,ShowOnlyInnerProperties))
    TMap<FGenderEntryHandle,FAvatarFacialMorphs> FacialMorphNames;
};


/** Struct for Tattoos */
USTRUCT(BlueprintType)
struct FTattooItem : public FTextureItem {
    GENERATED_BODY()

    FTattooItem()
    {
        ItemCategory.ItemCategory = TEXT("Tattoos");
    }
    
    /** Tattoo placement */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FTattooPlacementEntryHandle Placement;
    
};

/** Struct for Skin Items */
USTRUCT(BlueprintType)
struct FSkinItem : public FTextureItem
{
    GENERATED_BODY()

    FSkinItem()
    {
        ItemCategory.ItemCategory = TEXT("Skin");
    }
};

USTRUCT(BlueprintType)
struct FMaterialWithThumbnail
{
    GENERATED_BODY()
public:
    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    TObjectPtr<UMaterialInstance> MaterialInstance;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    TObjectPtr<UTexture> MaterialThumbnail;
};

USTRUCT(BlueprintType)
struct FMaterialWithThumbnailArray
{
    GENERATED_BODY()
public:
    UPROPERTY(BlueprintReadWrite, EditAnywhere ,meta = (ShowOnlyInnerProperties))
    TMap<FGenderEntryHandle,FMaterialWithThumbnail> MaterialByGender;
};


/** Struct for Presets */
USTRUCT(BlueprintType)
struct FPresetItem : public FPrimitiveItem {
    GENERATED_BODY()

    FPresetItem()
    {
        ItemCategory.ItemCategory = TEXT("Preset");
    }
    
    /** Material instances */
    UPROPERTY(BlueprintReadWrite, EditAnywhere ,meta = (ShowOnlyInnerProperties))
    TArray<FMaterialWithThumbnailArray> MaterialAndThumbnailData;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    TMap<FGenderEntryHandle,FName> PresetMorphName;

};


/** 🔹 Struct for Eyebrows */
USTRUCT(BlueprintType)
struct FEyebrowItem : public FTextureItem
{
    GENERATED_BODY()
public:
    FEyebrowItem()
    {
        ItemCategory.ItemCategory = TEXT("Eyebrows");
    }
};


USTRUCT(BlueprintType)
struct FItemCategoryStruct : public FTableRowBase
{
    GENERATED_BODY()
public:
    
    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(DisplayName="MultipleDataTables?"))
    bool bMultipleDataTables;
    
    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(DisplayName="DataTable", MakeStructureDefaultValue="None", EditCondition="!bMultipleDataTables",EditConditionHides))
    TObjectPtr<UDataTable> DataTable;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(DisplayName="DataTable", EditCondition="bMultipleDataTables",EditConditionHides))
    TMap<FName,TObjectPtr<UDataTable>> DataTablesByNames;
};

USTRUCT(BlueprintType)
struct FSelectClothingAndSubtype
{
    GENERATED_BODY();

    UPROPERTY(EditAnywhere, BlueprintReadWrite,meta=(ShowOnlyInnerProperties))
    FClothingTypeEntryHandle ClothingType;

    UPROPERTY()
    FClothingTypeEntryHandle PreviousClothingType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ShowOnlyInnerProperties))
    TArray<FCheckableOption> SelectSubtype;
};


USTRUCT(BlueprintType)
struct FAvatarSkeletalBodyVariations : public FTableRowBase
{
    GENERATED_BODY();

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    TArray<FSelectClothingAndSubtype> ClothingCombinations;
    
    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    TMap<FGenderEntryHandle,TObjectPtr<USkeletalMesh>> BodyVariations;

    UPROPERTY(BlueprintReadWrite, EditAnywhere,meta=(ShowOnlyInnerProperties))
    TMap<FGenderEntryHandle,FAvatarFacialMorphs> FacialMorphNames;

   

    virtual void OnDataTableChanged(const UDataTable* InDataTable, const FName InRowName) override
    {
        Super::OnDataTableChanged(InDataTable, InRowName);
    
        for (auto& Combination : ClothingCombinations)
        {
            if (Combination.PreviousClothingType == Combination.ClothingType)
            {
                continue; // Skip this iteration, but continue processing the rest of the array
            }

            Combination.PreviousClothingType = Combination.ClothingType;
            Combination.SelectSubtype = UDataTableRowNameHelper::GetCheckableOptions(Combination.ClothingType.ClothingType);
        }
    }

};



