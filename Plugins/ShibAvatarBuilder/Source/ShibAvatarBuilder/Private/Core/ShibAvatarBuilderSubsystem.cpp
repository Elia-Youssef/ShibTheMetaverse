// Fill out your copyright notice in the Description page of Project Settings.

#include "ShibAvatarBuilderSubsystem.h"

#include "AvatarHistory.h"
#include "ShibAPIsSubsystem.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Components/CapsuleComponent.h"
#include "ShibSaveGame.h"
#include "Components/SkeletalMeshComponent.h"
#include "ShibAvatarCharacter.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Engine/SkinnedAssetCommon.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ShibAvatarBuilderSubsystem)

void UShibAvatarBuilderSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	AvatarHistory = NewObject<UAvatarHistory>();
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(FName("AssetRegistry"));
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();
	// Must wait until all assets are discovered before populating list of other GameInfoBase assets.
	if (AssetRegistry.IsLoadingAssets()) {
		AssetRegistry.OnFilesLoaded().AddUObject(this, &UShibAvatarBuilderSubsystem::Init);
		LOG_SHIB(Log, "Waiting On Asset Registry To Load.");
	} else {
		Init();
	}
}

void UShibAvatarBuilderSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

void UShibAvatarBuilderSubsystem::Init()
{
	LOG_SHIB(Log, "Asset Registry Loaded, Trying to Get AvatarData Asset.");
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	FARFilter Filter;
	Filter.ClassPaths.Add(UShibAvatarDataAsset::StaticClass()->GetClassPathName());
	Filter.PackagePaths.Add(FName("/ShibAvatarBuilder/Core")); 
	Filter.bRecursivePaths = true;

	TArray<FAssetData> Assets;
	AssetRegistryModule.Get().GetAssets(Filter, Assets);
	UE_LOG(LogShib, Log, TEXT("%hs - Assets found are - %d"), __FUNCTION__, Assets.Num());

	for (auto Asset : Assets)
	{
		UE_LOG(LogShib, Log, TEXT("%hs - Assets found : - %s "), __FUNCTION__, *Asset.GetFullName());

		int id = -1;
		FName AssetRegistrySearchablePropertyName = GET_MEMBER_NAME_CHECKED(UShibAvatarDataAsset, AssetID);
		Asset.GetTagValue(AssetRegistrySearchablePropertyName,id);
		
		// Get Asset with id 0 for now just in case more data assets are made.
		if(id == 0 )
		{
			LOG_SHIB(Log, "Got Avatar Data Asset!!");
			AvatarDataAsset = Cast<UShibAvatarDataAsset>(Asset.GetAsset());
			break;
		}
	}
}

AShibAvatarCharacter* UShibAvatarBuilderSubsystem::GetAvatarCharacter()
{
	if(!IsValid(AvatarCharacter))
	{
		AvatarCharacter = Cast<AShibAvatarCharacter>(UGameplayStatics::GetActorOfClass(this, AShibAvatarCharacter::StaticClass()));
		if(!IsValid(AvatarCharacter))
		{
			LOG_SHIB(Error, "Avatar not found in world!");
		}
	}
	return AvatarCharacter;
}

const FCustomButtonUIData  UShibAvatarBuilderSubsystem::GetThumbnails(const UDataTable* Table, EAvatarBuilderGender MyGender)
{
	// Check if table is valid
	if(!IsValid(Table))
	{
		//LOG_SHIB(Warning, "DataTable is invalid!");
		return FCustomButtonUIData();
	}

	TArray<TObjectPtr<UTexture>> Thumbs{};
	TArray<int> IDs{};

	// Loop through all rows in table
	const TArray<FName>& RowNames = Table->GetRowNames();
	for (const auto& Row : RowNames)
	{
		// If Table using FMesh 
		if(Table->GetRowStruct() == FMesh::StaticStruct())
		{
			FMesh* NewRow = Table->FindRow<FMesh>(Row, TEXT(""));
			if (NewRow && NewRow->Gender == MyGender)
			{
				Thumbs.Add(NewRow->Thumbnail);
				IDs.Add(NewRow->MeshID);
			}
		}
		// If Table using FShibTexture 
		else if (Table->GetRowStruct() == FShibTexture::StaticStruct() )
		{
			FShibTexture* NewRow = Table->FindRow<FShibTexture>(Row, TEXT(""));

			// Use Preset Thumbnail function to get correct presets based on skin id
			if (NewRow && NewRow->Gender == MyGender && NewRow->TextureType != EShibTexture::Preset)
			{
				Thumbs.Add(NewRow->Thumbnail);
				IDs.Add(NewRow->TextureID);
			}
		}
	}

	// Warn if no thumbnails were found 
	if(Thumbs.IsEmpty())
	{
		LOG_SHIB(Warning, "No Thumbnails found in DataTable [%s]!",*Table->GetName());
	}
	FCustomButtonUIData ButtonData{IDs,Thumbs};
	return ButtonData;
}



const FCustomButtonUIData UShibAvatarBuilderSubsystem::GetPresetThumbnails(EAvatarBuilderGender MyGender)
{
	TArray<TObjectPtr<UTexture>> Thumbs{};
	TArray<int> IDs{};
	if(!IsValid(AvatarDataAsset) || !IsValid(AvatarDataAsset->Preset)) return FCustomButtonUIData();

	int SkinID = 0;
	FShibTexture SkinData;

	// Get Currently Applied SkinID 
	if(ShibSaveGame && ShibSaveGame->GetSavedShibTextureAssetData(EShibTexture::Skin,SkinData))
	{
		SkinID = SkinData.TextureID;
	}

	for (auto PresetRows : AvatarDataAsset->Preset->GetRowNames())
	{
		FShibTexture PresetData = *AvatarDataAsset->Preset->FindRow<FShibTexture>(PresetRows, TEXT(""));
		if(PresetData.Gender == MyGender)
		{
			// Add Thumbnail based on skinID 
			Thumbs.Add(PresetData.MaterialAndThumbnailData[SkinID].MaterialThumbnail);
			IDs.Add(PresetData.TextureID);
		}
	}
	FCustomButtonUIData ButtonData{IDs,Thumbs};

	return ButtonData;
	

	
}

const FCustomButtonUIData UShibAvatarBuilderSubsystem::GetTattooThumbnails(const UDataTable* Table, EShibTattoo TattooType, EAvatarBuilderGender MyGender)
{
	//LOG_SHIB(Warning, "Gender [%s]", *UEnum::GetDisplayValueAsText(MyGender).ToString());
	if (!Table)return FCustomButtonUIData();
	TArray<TObjectPtr<UTexture>> Thumbs{};
	TArray<int> TexID;
	const TArray<FName>& RowNames = Table->GetRowNames();
	for (const auto& Row : RowNames)
	{
		FShibTexture* NewRow = Table->FindRow<FShibTexture>(Row, TEXT(""));
		if (NewRow && NewRow->Gender == MyGender && NewRow->TattooType == TattooType)
		{
			Thumbs.Add(NewRow->Thumbnail);
			TexID.Add(NewRow->TextureID);
		}
	}
	FCustomButtonUIData ButtonData{TexID,Thumbs};
	return ButtonData;
}

const FCustomButtonUIData UShibAvatarBuilderSubsystem::GetAccessoryThumbnails(const UDataTable* Table, EShibAccessory AccessoryType, EAvatarBuilderGender MyGender)
{
	LOG_SHIB(Warning, "Gender [%s]", *UEnum::GetDisplayValueAsText(MyGender).ToString());
	if (!Table)return FCustomButtonUIData();
	TArray<TObjectPtr<UTexture>> Thumbs{};
	TArray<int> MeshIDs{};
	const TArray<FName>& RowNames = Table->GetRowNames();
	for (const auto& Row : RowNames)
	{
		FMesh* NewRow = Table->FindRow<FMesh>(Row, TEXT(""));
		if (NewRow && NewRow->Gender == MyGender && NewRow->AccessoryType == AccessoryType)
		{
			LOG_SHIB(Warning, "Accessory according to type [%s]", *NewRow->Thumbnail.GetName());
			Thumbs.Add(NewRow->Thumbnail);
			MeshIDs.Add(NewRow->MeshID);
		}
	}

	FCustomButtonUIData ButtonData{MeshIDs,Thumbs};
	return ButtonData;
}




void UShibAvatarBuilderSubsystem::ApplyBodyMorphData(EShibBodyMorphs Type,const FBase& Base, const float& Value, const bool& bFinal)
{
	if (!GetAvatarCharacter() || Type == EShibBodyMorphs::NONE )return;

	// Different implementations for Height/Weight
	if(Type == EShibBodyMorphs::AvatarHeight)
	{
		ApplyNewHeight(Value, bFinal);
		return;
	}
	
	 if (Type == EShibBodyMorphs::AvatarWeight)
	{
		ApplyNewWeight(Base,Value,bFinal);
	 	return;
	}
	
	ApplyNewMorphs(GetAvatarCharacter()->GetBody(), 0, GetFacialMorphNames(Base,Type),Value);
	ApplyNewMorphs(GetAvatarCharacter()->GetHair(), 0, GetBeardMorphNames(Base,Type),Value);
	
	if(bFinal && ShibSaveGame)
	{
		ShibSaveGame->SetMorphData(Type,Value);
		AvatarHistory->Add(ShibSaveGame->GetCurrentAvatarData());
		
	}
}

void UShibAvatarBuilderSubsystem::UpdateMeshesBasedOnMorphWeight(const float Weight)
{
	FAvatarSave CurrentAvatar = ShibSaveGame->GetCurrentAvatarData();
	
	for (const auto& [MeshType, RowName] : CurrentAvatar.MeshAssetRowNameData)
	{
		TObjectPtr<UDataTable> TableToSearch;
		switch (MeshType)
		{
		case EShibMesh::Top:
			TableToSearch = AvatarDataAsset->Top;
			break;
		case EShibMesh::Bottom:
			TableToSearch = AvatarDataAsset->Bottom;
			break;
		case EShibMesh::Shoe:
			TableToSearch = AvatarDataAsset->Shoes;
			break;
		case EShibMesh::FullBody:
			TableToSearch = AvatarDataAsset->FullBody;
			break;
		default:
			continue;
		}

		FMesh* Row = TableToSearch->FindRow<FMesh>(RowName, "");
		if (!Row) continue;
		
		int8 Index = Weight * 2; // Morph weight is 0 <-> 1
		if (Row->Meshes.IsValidIndex(Index))
		{
			ApplyNewMesh(GetMeshHolders(MeshType), Row->Meshes[Index]);
			ManageLowerBodySense(*Row,Weight);
		}
	}
}

void UShibAvatarBuilderSubsystem::ApplyNewWeight(const FBase& Base, const float& Weight, const bool& bFinal)
{
	if (!GetAvatarCharacter())return;
	
	ApplyNewMorphs(GetAvatarCharacter()->GetBody(), nullptr, Base.MorphData.BodyMorphs, Weight);
	ApplyNewMorphs(GetAvatarCharacter()->GetBody(), nullptr, Base.MorphData.HeadMorphs, Weight);
	ApplyNewMorphs(GetAvatarCharacter()->GetLeftArm(), nullptr, Base.MorphData.ArmMorphs, Weight);
	ApplyNewMorphs(GetAvatarCharacter()->GetRightArm(), nullptr, Base.MorphData.ArmMorphs, Weight);
	ApplyNewMorphs(GetAvatarCharacter()->GetBack(), nullptr, Base.MorphData.BackMorphs, Weight);
	ApplyNewMorphs(GetAvatarCharacter()->GetHeadTop(), nullptr, Base.MorphData.HeadTopMorphs, Weight);
	ApplyNewMorphs(GetAvatarCharacter()->GetNeck(), nullptr, Base.MorphData.NeckMorphs, Weight);
	ApplyNewMorphs(GetAvatarCharacter()->GetFace(), nullptr, Base.MorphData.FaceMorphs, Weight);
	ApplyNewMorphs(GetAvatarCharacter()->GetEars(), nullptr, Base.MorphData.EarsMorphs, Weight);
	ApplyNewMorphs(GetAvatarCharacter()->GetHair(),nullptr,Base.MorphData.BeardMorphs.BodyMorphs, Weight);
	
	UpdateMeshesBasedOnMorphWeight(Weight);

	if(bFinal && ShibSaveGame)
	{
		ShibSaveGame->SetMorphData(EShibBodyMorphs::AvatarWeight,Weight);
		AvatarHistory->Add(ShibSaveGame->GetCurrentAvatarData());
	}
	
	
}

void UShibAvatarBuilderSubsystem::UpdateSkmBasedOnMesh()
{
	if (!GetShibSaveGame() || !AvatarDataAsset || !GetAvatarCharacter() || !GetAvatarCharacter()->GetBody()) return;
	
	FAvatarSave CurrentAvatar = ShibSaveGame->GetCurrentAvatarData();

	bool bTopEquipped = false;
	EBottomSubType BottomSubType = EBottomSubType::NONE;
	EFullBodySubType FullBodySubType = EFullBodySubType::NONE;
	
	for (const auto& [MeshType, RowName] : CurrentAvatar.MeshAssetRowNameData)
	{
		TObjectPtr<UDataTable> TableToSearch;
		switch (MeshType)
		{
		case EShibMesh::Top:
			TableToSearch = AvatarDataAsset->Top;
			break;
		case EShibMesh::Bottom:
			TableToSearch = AvatarDataAsset->Bottom;
			break;
		case EShibMesh::FullBody:
			TableToSearch = AvatarDataAsset->FullBody;
			break;
		default:
			continue;
		}

		FMesh* Row = TableToSearch->FindRow<FMesh>(RowName, "");
		if (!Row) continue;

		if (Row->MeshType == EShibMesh::Top && Row->MeshID != 0) bTopEquipped = true;
		if (Row->BottomSubType != EBottomSubType::NONE) BottomSubType = Row->BottomSubType;
		if (Row->FullBodySubType != EFullBodySubType::NONE) FullBodySubType = Row->FullBodySubType;
	}
	
	TObjectPtr<USkeletalMesh> BodySKM = nullptr;
	for (const FName& RowName : AvatarDataAsset->SkeletalBodyVariations->GetRowNames())
	{
		const FShibAvatarSkeletalBodyVariations* Row = AvatarDataAsset->SkeletalBodyVariations->FindRow<FShibAvatarSkeletalBodyVariations>(RowName, "");
		if (!Row) continue;

		bool bTopMatches = Row->MeshCombinations.Contains(EShibMesh::Top) == bTopEquipped;
		bool bBottomMatches = Row->BottomSubType == BottomSubType;
		bool bFullBodyMatches = Row->FullBodySubType == FullBodySubType;
		bool bGenderMatches =  CurrentAvatar.bIsMale ? Row->Gender == EAvatarBuilderGender::Male : Row->Gender == EAvatarBuilderGender::Female;

		if (bTopMatches && bBottomMatches && bFullBodyMatches && bGenderMatches)
		{
			BodySKM = Row->BodyVariation;
			break;
		}
	}

	if (!BodySKM) return;

	
	GetAvatarCharacter()->SetCurrentMontagePosition();
	GetAvatarCharacter()->GetBody()->SetSkeletalMeshAsset(BodySKM);

	if (ShibSaveGame)
	{
		FShibTexture Preset;
		ShibSaveGame->GetSavedShibTextureAssetData(EShibTexture::Preset, Preset);
		ApplyNewMorphs(GetAvatarCharacter()->GetBody(), 0, Preset.Morph, 1);
		//ApplyPresetMI(Preset);
	}

	for (const FName& RowName : AvatarDataAsset->Base->GetRowNames())
	{
		const FBase* Row = AvatarDataAsset->Base->FindRow<FBase>(RowName, "");
		if (!Row) continue;
		
		bool bGenderMatches =  CurrentAvatar.bIsMale ? Row->Gender == EAvatarBuilderGender::Male : Row->Gender == EAvatarBuilderGender::Female;
		if (bGenderMatches)
		{
			UE_LOG(LogShib, Log, TEXT("%hs - Updating Skeletal Mesh Variant Morph with weight value %f "),__FUNCTION__,ShibSaveGame->GetSavedMorphData(EShibBodyMorphs::AvatarWeight));

			ApplyNewMorphs(GetAvatarCharacter()->GetBody(),nullptr,Row->MorphData.BodyMorphs,ShibSaveGame->GetSavedMorphData(EShibBodyMorphs::AvatarWeight));
			ApplyNewMorphs(GetAvatarCharacter()->GetHair(),nullptr,Row->MorphData.BeardMorphs.BodyMorphs,ShibSaveGame->GetSavedMorphData(EShibBodyMorphs::AvatarWeight));

			
			for (auto MorphType : TEnumRange<EShibBodyMorphs>())
			{
				ApplyNewMorphs(GetAvatarCharacter()->GetBody(),nullptr,GetFacialMorphNames(*Row,MorphType),ShibSaveGame->GetSavedMorphData(MorphType));
				ApplyNewMorphs(GetAvatarCharacter()->GetHair(),nullptr,GetBeardMorphNames(*Row,MorphType),ShibSaveGame->GetSavedMorphData(MorphType));
			}
			
			if (bShouldPlayMontage) GetAvatarCharacter()->PlayMontageFromCurrentPosition(Row->AnimMontage);
			break;
		}
	}
}

void UShibAvatarBuilderSubsystem::UpdateUndergarments()
{
	TArray<EShibMesh> CurrentMeshes;
	ShibSaveGame->GetCurrentAvatarData().MeshAssetRowNameData.GetKeys(CurrentMeshes);

	// No need to set any undergarments if fullbody is applied
	if(CurrentMeshes.Contains(EShibMesh::FullBody)) return;

	FMesh Temp;
	
	// If top not applied get correct default top gender. (Default Mesh = Undergarment = MeshID 0 )
	if(!CurrentMeshes.Contains(EShibMesh::Top))
	{
		for (auto TopsRow : AvatarDataAsset->Top->GetRowNames())
		{
			Temp = *AvatarDataAsset->Top->FindRow<FMesh>(TopsRow,"");
			if(Temp.MeshID == 0 && Temp.Gender == Gender && Temp.Mesh != nullptr)
			{
				ApplyNewBodyMesh(EShibMesh::Top,Temp,true);
				break;
			}
		}
	}

	//Same thing for bottom
	if(!CurrentMeshes.Contains(EShibMesh::Bottom))
	{
		for (auto BottomsRow : AvatarDataAsset->Bottom->GetRowNames())
		{
			Temp = *AvatarDataAsset->Bottom->FindRow<FMesh>(BottomsRow,"");
			if(Temp.MeshID == 0 && Temp.Gender == Gender && Temp.Mesh != nullptr)
			{
				ApplyNewBodyMesh(EShibMesh::Bottom,Temp,true);
				break;
			}
		}
	}
	
	
	
}

void UShibAvatarBuilderSubsystem::ApplyNewHeight(float Value, bool bFinal)
{
	 if (!GetAvatarCharacter())return;
	
	SetHeight(GetAvatarCharacter(),Value);
	
	if(bFinal && ShibSaveGame)
	{
		ShibSaveGame->SetMorphData(EShibBodyMorphs::AvatarHeight,Value);
		AvatarHistory->Add(ShibSaveGame->GetCurrentAvatarData());
	}
}

TArray<FName> UShibAvatarBuilderSubsystem::GetFacialMorphNames(const FBase& Base, EShibBodyMorphs Type)
{
	switch (Type)
	{
		case EShibBodyMorphs::NoseHeight:
			return Base.MorphData.NoseHeightMorphs;
		case EShibBodyMorphs::NoseLength:
			return Base.MorphData.NoseLengthMorphs;
		case EShibBodyMorphs::NoseWidth:
			return Base.MorphData.NoseWidthMorphs;
		
		case EShibBodyMorphs::JawLength:
			return Base.MorphData.JawLengthMorphs; 
		case EShibBodyMorphs::JawWidth:
			return Base.MorphData.JawWidthMorphs; 
		case EShibBodyMorphs::JawHeight:
			return Base.MorphData.JawHeightMorphs;
	
		case EShibBodyMorphs::MouthThickness:
			return Base.MorphData.MouthLengthMorphs; 
		case EShibBodyMorphs::MouthWidth:
			return Base.MorphData.MouthWidthMorphs; 
		case EShibBodyMorphs::MouthHeight:
			return Base.MorphData.MouthHeightMorphs;

		case EShibBodyMorphs::AccessoriesNoseLength:
			return Base.MorphData.AccessoriesNoseLengthMorphs;
		case EShibBodyMorphs::AccessoriesNoseHeight:
			return Base.MorphData.AccessoriesNoseHeightMorphs;
		case EShibBodyMorphs::AccessoriesNoseWidth:
			return Base.MorphData.AccessoriesNoseWidthMorphs;
		
	default:
			
			//LOG_SHIB(Warning, "No Facial Morphs found for EShibBodyMorphs Type: [%s]", *UEnum::GetDisplayValueAsText(Type).ToString());
			return TArray<FName>(); // Return an empty array
	}
}

TArray<FName> UShibAvatarBuilderSubsystem::GetBeardMorphNames(const FBase& Base, EShibBodyMorphs Type)
{
	switch (Type)
	{
	case EShibBodyMorphs::NoseHeight:
		return Base.MorphData.BeardMorphs.NoseHeightMorphs;
	case EShibBodyMorphs::NoseLength:
		return Base.MorphData.BeardMorphs.NoseLengthMorphs;
	case EShibBodyMorphs::NoseWidth:
		return Base.MorphData.BeardMorphs.NoseWidthMorphs;
		
	case EShibBodyMorphs::JawLength:
		return Base.MorphData.BeardMorphs.JawLengthMorphs; 
	case EShibBodyMorphs::JawWidth:
		return Base.MorphData.BeardMorphs.JawWidthMorphs;
	case EShibBodyMorphs::JawHeight:
		return Base.MorphData.BeardMorphs.JawHeightMorphs;
	
	case EShibBodyMorphs::MouthThickness:
		return Base.MorphData.BeardMorphs.MouthLengthMorphs; 
	case EShibBodyMorphs::MouthWidth:
		return Base.MorphData.BeardMorphs.MouthWidthMorphs;
	case EShibBodyMorphs::MouthHeight:
		return Base.MorphData.BeardMorphs.MouthHeightMorphs;
	
	default:
		return TArray<FName>(); // Return an empty array
	}
}


bool UShibAvatarBuilderSubsystem::CheckAvatarChanged()
{
	
	if(ShibSaveGame->GetCurrentAvatarData() == InitialAvatarSaveData)
	{
		UE_LOG(LogShib, Log, TEXT("%hs - No Changes were made."), __FUNCTION__);
		return false;
	}
	UE_LOG(LogShib, Log, TEXT("%hs - Changes to avatar were found!"), __FUNCTION__);
	return true;
	
}

void UShibAvatarBuilderSubsystem::ApplyNewBodyMesh(EShibMesh Type, const FMesh& Mesh,bool bFinal)
{
	if (!GetAvatarCharacter() || Type == EShibMesh::Accessory || Type == EShibMesh::NONE || Mesh.MeshType != Type)return;
	
	if (!CheckConflicts(Mesh,bFinal)) return;
	RemoveConflicts(Mesh,bFinal);

	// If applying null mesh just remove the mesh and return.
	if(Mesh.Mesh == nullptr)
	{
		RemoveBodyMesh(Type,bFinal);
		UpdateSkmBasedOnMesh();
		return;
	}
	
	
	GetMeshHolders(Type)->SetMaterial(0, nullptr);
	if (Type == EShibMesh::FullBody)
	{
		GetMeshHolders(Type)->SetMaterial(1, nullptr);
	}

	
	int CurrentWeightIndex = ShibSaveGame->GetSavedMorphData(EShibBodyMorphs::AvatarWeight)*2;
	if(Mesh.Meshes.IsValidIndex(CurrentWeightIndex))
	{
		UE_LOG(LogShib, Log, TEXT("%hs - Applying Mesh with Weight type %d"), __FUNCTION__,CurrentWeightIndex);

		ApplyNewMesh(GetMeshHolders(Type), Mesh.Meshes[CurrentWeightIndex]);
	}
	else if (Mesh.Meshes.Num() == 0 && Mesh.Mesh)
	{
		ApplyNewMesh(GetMeshHolders(Type), Mesh.Mesh);
	}
	
	
	ManageLowerBodySense(Mesh,ShibSaveGame->GetSavedMorphData(EShibBodyMorphs::AvatarWeight));

	// Apply Preset again to morph beard when applied
	if(Mesh.MeshType == EShibMesh::Hair && Mesh.Gender == EAvatarBuilderGender::Male)
	{
		if(FShibTexture CurrentPreset; ShibSaveGame->GetSavedShibTextureAssetData(EShibTexture::Preset,CurrentPreset))
		{
			ApplyPresetMI(CurrentPreset);
		}
		
	}
	
	// Save Mesh Rowname
	if(ShibSaveGame && bFinal)
	{
		
		 if(IsValid(AvatarDataAsset) && AvatarDataAsset->MeshAssetDataTables.Contains(Type) && IsValid(AvatarDataAsset->MeshAssetDataTables[Type]))
		{
			for (auto RowName : AvatarDataAsset->MeshAssetDataTables[Type]->GetRowNames())
			{
				FMesh* CurrentMesh = AvatarDataAsset->MeshAssetDataTables[Type]->FindRow<FMesh>(RowName,"MeshLookup");
				if(CurrentMesh->MeshID == Mesh.MeshID && CurrentMesh->Gender == Mesh.Gender )
				{
					ShibSaveGame->SetSavedMeshAssetRowName(Type,RowName);
					AvatarHistory->Add(ShibSaveGame->GetCurrentAvatarData());
				}
			}
		}
	}
	else
	{
		LOG_SHIB(Warning ,"ShibSaveGame not set yet");
	}

	UpdateUndergarments();
	UpdateSkmBasedOnMesh();

	

}

void UShibAvatarBuilderSubsystem::RemoveBodyMesh(EShibMesh Type,bool bFinal)
{
	if (IsValid(GetMeshHolders(Type)) && GetMeshHolders(Type)->GetSkeletalMeshAsset() != nullptr)
	{
		ApplyNewMesh(GetMeshHolders(Type),nullptr);
		UpdateSkmBasedOnMesh();
	}
	
	if(ShibSaveGame && bFinal)
	{
		ShibSaveGame->RemoveSavedMeshData(Type);
	}
}

void UShibAvatarBuilderSubsystem::RemoveConflicts(const FMesh& Mesh, bool bFinal)
{
	
	for (const auto& Conflict : Mesh.NonCompatible)
	{

		UE_LOG(LogShib, Warning, TEXT("%hs - Called deprecated Remove Conflicts function"), __FUNCTION__);
		FMesh* MyMesh = Conflict.DataTable->FindRow<FMesh>(Conflict.RowName, TEXT(""));
		if (MyMesh)
		{
			
			if (MyMesh->MeshType == EShibMesh::Accessory)
			{
				if (GetAccessoryHolder(MyMesh->AccessoryType)->GetSkeletalMeshAsset() == MyMesh->Mesh)
				{
					LOG_SHIB(Warning, "Conflict Mesh [%s], Removing...", *MyMesh->Thumbnail.GetName());
					RemoveAccessoryMesh(MyMesh->AccessoryType,bFinal);
				}
			}
			else
			{
				// Check with mesh variations
				if(MyMesh->MeshVariations.Num() > 0)
				{
					for (auto Variation : MyMesh->MeshVariations)
					{
						if (GetMeshHolders(MyMesh->MeshType)->GetSkeletalMeshAsset() == Variation.MeshVariation)
						{
							LOG_SHIB(Log, "Conflicting Mesh [%s] with applied mesh [%s], Removing...", *MyMesh->Thumbnail.GetName(),*Mesh.Thumbnail->GetName());
							RemoveBodyMesh(MyMesh->MeshType,bFinal);
							break;
						}
					}
				}
				else if(GetMeshHolders(MyMesh->MeshType)->GetSkeletalMeshAsset() == MyMesh->Mesh)
				{
					LOG_SHIB(Log, "Conflicting Mesh [%s] with applied mesh [%s], Removing...", *MyMesh->Thumbnail.GetName(),*Mesh.Thumbnail->GetName());
					RemoveBodyMesh(MyMesh->MeshType,bFinal);
				}
			}
		}
	}
}

bool UShibAvatarBuilderSubsystem::CheckConflicts(const FMesh& Mesh, bool bFinal)
{
	FAvatarSave CurrentAvatar = ShibSaveGame->GetCurrentAvatarData();
	
	TArray<EShibAccessory> CurrentAccessories;
	CurrentAvatar.AccessoryRowNameData.GetKeys(CurrentAccessories);
	
	TArray<EShibMesh> CurrentMeshes;
	CurrentAvatar.MeshAssetRowNameData.GetKeys(CurrentMeshes);

	/**
	 * TODO: I need to clean this, but it works for now.
	 */

	TArray<EShibAccessory> AccessoriesToRemove;
	TArray<EShibMesh> MeshesToRemove;

	for (const FMeshTypeCompatibility& Conflict : Mesh.NonCompatibleTypes)
	{
		if (Conflict.MeshType == EShibMesh::Accessory)
		{
			// do we have this accessory?
			if (CurrentAccessories.Contains(Conflict.AccessoryType))
			{
				// is this accessory excluded from the compatibility list?
				bool bIsExcluded = false;
				for (FDataTableRowHandle Excluded : Conflict.Excluding)
				{
					if (Excluded.RowName == CurrentAvatar.AccessoryRowNameData[Conflict.AccessoryType])
					{
						bIsExcluded = true;
						break;
					}
				}

				// if it's not excluded we should remove it
				if (!bIsExcluded)
				{
					// if we shouldn't remove it either, we can't apply this new mesh
					if (!Conflict.bReplaceExisting) return false;
				
					AccessoriesToRemove.Add(Conflict.AccessoryType);
				}
			}
		}

		else
		{
			if (CurrentMeshes.Contains(Conflict.MeshType))
			{
				bool bIsExcluded = false;
				for (FDataTableRowHandle Excluded : Conflict.Excluding)
				{
					if (Excluded.RowName == CurrentAvatar.MeshAssetRowNameData[Conflict.MeshType])
					{
						bIsExcluded = true;
						break;
					}
				}

				if (!bIsExcluded)
				{
					if (!Conflict.bReplaceExisting) return false;
					
					MeshesToRemove.Add(Conflict.MeshType);
				}
			}
		}
	}

	// remove the meshes after going through the whole flow above
	// we don't want to remove a mesh then find out we shouldn't apply the new mesh
	
	for (EShibAccessory AccessoryToRemove : AccessoriesToRemove) RemoveAccessoryMesh(AccessoryToRemove,bFinal);
	for (EShibMesh MeshToRemove : MeshesToRemove) RemoveBodyMesh(MeshToRemove,bFinal);
	
	return true;
}

USkeletalMeshComponent* UShibAvatarBuilderSubsystem::GetMeshHolders(EShibMesh Type)
{
	if (!GetAvatarCharacter())return nullptr;
	
	switch (Type)
	{
		case EShibMesh::Top:
			return GetAvatarCharacter()->GetTop();
		case EShibMesh::Bottom:
			return GetAvatarCharacter()->GetBottoms();
		case EShibMesh::Shoe:
			return GetAvatarCharacter()->GetShoes();
		case EShibMesh::Hair:
			return GetAvatarCharacter()->GetHair();
		case EShibMesh::FullBody:
			return GetAvatarCharacter()->GetFullBody();
		case EShibMesh::Accessory:
		LOG_SHIB(Warning, "Use GetAccessoryHolder Function to get Skeletal Mesh based on EShibAccessory Type");
		return nullptr;
		default:
			LOG_SHIB(Warning, "No SkeletalMeshComponent Exists for Type: %d", static_cast<uint8>(Type));
			return nullptr;
	}
}

void UShibAvatarBuilderSubsystem::ApplyNewAccessory(const FMesh& Mesh,bool bFinal)
{
	if (!GetAvatarCharacter())return;


	if (!CheckConflicts(Mesh,bFinal)) return;
	RemoveConflicts(Mesh,bFinal);

	USkeletalMeshComponent* Holder = GetAccessoryHolder(Mesh.AccessoryType);
	Holder->SetMaterial(0, nullptr);

	ApplyNewMesh(Holder,Mesh.Mesh);
	
	if (Mesh.Morphs.Num() != 0)
	{
		ApplyNewMorphs(Holder, 0, Mesh.Morphs,ShibSaveGame->GetSavedMorphData(EShibBodyMorphs::AvatarWeight));
	}
	
	// Save Accessory Row name
	if(ShibSaveGame && bFinal)
	{
		 if(IsValid(AvatarDataAsset) && IsValid(AvatarDataAsset->Accessory))
		{
			for (auto RowName : AvatarDataAsset->Accessory->GetRowNames())
			{
				FMesh* CurrentMesh  = AvatarDataAsset->Accessory->FindRow<FMesh>(RowName,"MeshLookup");
				if(CurrentMesh->MeshID == Mesh.MeshID && CurrentMesh->Gender == Mesh.Gender && CurrentMesh->AccessoryType== Mesh.AccessoryType )
				{
					ShibSaveGame->SetSavedAccessoryRowName(Mesh.AccessoryType,RowName);
					AvatarHistory->Add(ShibSaveGame->GetCurrentAvatarData());
				}
			}
		}
	}
	else
	{
		LOG_SHIB(Warning ,"ShibSaveGame not set yet");
	}
	
	
}

void UShibAvatarBuilderSubsystem::RemoveAccessoryMesh(EShibAccessory Type,bool bFinal)
{
	if (IsValid(GetAccessoryHolder(Type)) && GetAccessoryHolder(Type)->GetSkeletalMeshAsset() != nullptr)
	{
		ApplyNewMesh(GetAccessoryHolder(Type),nullptr);
	}

	if(ShibSaveGame && bFinal)
	{
		ShibSaveGame->RemoveSavedAccessoryData(Type);
	}
}


void UShibAvatarBuilderSubsystem::ManageLowerBodySense(const FMesh& Mesh,float Weight)
{
	if(Mesh.Gender != Gender) return;
	// Retrieve previous applied Bottom and Shoe data
	FMesh ShoeHistory; 
	FMesh BottomHistory;
	FMesh FullbodyHistroy;
	int WeightIndex = Weight*2;
	
	

	// Handle Shoe MeshType
	if(Mesh.MeshType == EShibMesh::Shoe)
	{
		// Check if applied bottom is valid and has variations 
		if(ShibSaveGame->GetSavedMeshAssetData(EShibMesh::Bottom,BottomHistory) && BottomHistory.MeshVariations.Num() > 1)
		{
			// Long Shoe = WithTucked (1)
			 if(Mesh.ShoeSubType == EShoeSubType::Long && BottomHistory.MeshVariations[1].WeightMeshVariation.IsValidIndex(WeightIndex))
			{
				ApplyNewMesh(GetMeshHolders(EShibMesh::Bottom),BottomHistory.MeshVariations[1].WeightMeshVariation[WeightIndex]);
			 	UE_LOG(LogShib, Warning, TEXT("%hs - Applied mesh variation 1 for bottom with weight index %d"), __FUNCTION__,WeightIndex);
			}
			else if(BottomHistory.MeshVariations[0].WeightMeshVariation.IsValidIndex(WeightIndex))
			{
				ApplyNewMesh(GetMeshHolders(EShibMesh::Bottom),BottomHistory.MeshVariations[0].WeightMeshVariation[WeightIndex]);
				UE_LOG(LogShib, Warning, TEXT("%hs - Applied mesh variation 0  for bottom with weight index %d"), __FUNCTION__,WeightIndex);
			}
			else
			{
				UE_LOG(LogShib, Warning, TEXT("%hs - No valid mesh variations for bottom with weight index %d"), __FUNCTION__,WeightIndex);
			}
		}
		// Check if applied fullbody is valid and has variations
		else if (ShibSaveGame->GetSavedMeshAssetData(EShibMesh::FullBody,FullbodyHistroy) && FullbodyHistroy.MeshVariations.Num() > 1)
		{
			if(Mesh.ShoeSubType == EShoeSubType::Long && FullbodyHistroy.MeshVariations[1].WeightMeshVariation.IsValidIndex(WeightIndex))
			{
				ApplyNewMesh(GetMeshHolders(EShibMesh::FullBody),FullbodyHistroy.MeshVariations[1].WeightMeshVariation[WeightIndex]);
				UE_LOG(LogShib, Warning, TEXT("%hs - Applied mesh variation 1 for Fullbody with weight index %d"), __FUNCTION__,WeightIndex);
			}
			else if(FullbodyHistroy.MeshVariations[0].WeightMeshVariation.IsValidIndex(WeightIndex))
			{
				ApplyNewMesh(GetMeshHolders(EShibMesh::FullBody),FullbodyHistroy.MeshVariations[0].WeightMeshVariation[WeightIndex]);
				UE_LOG(LogShib, Warning, TEXT("%hs - Applied mesh variation 0 for Fullbody with weight index %d"), __FUNCTION__,WeightIndex);
			}
			else
			{
				UE_LOG(LogShib, Warning, TEXT("%hs - No valid mesh variations for Fullbody with weight index %d"), __FUNCTION__,WeightIndex);
			}
		}
	}
	else if (Mesh.MeshType == EShibMesh::Bottom || Mesh.MeshType == EShibMesh::FullBody)
	{
		// Check if Shoe is applied and bottom has variations
		if(ShibSaveGame->GetSavedMeshAssetData(EShibMesh::Shoe,ShoeHistory) && Mesh.MeshVariations.Num() > 1)
		{
			// Long Shoe = WithTucked (1)
			if(ShoeHistory.ShoeSubType == EShoeSubType::Long && Mesh.MeshVariations[1].WeightMeshVariation.IsValidIndex(WeightIndex))
			{
				ApplyNewMesh(GetMeshHolders(Mesh.MeshType),Mesh.MeshVariations[1].WeightMeshVariation[WeightIndex]);
				UE_LOG(LogShib, Warning, TEXT("%hs - Applied Mesh type (Var 1) [%s] with weight index %d"), __FUNCTION__,*UEnum::GetDisplayValueAsText(Mesh.MeshType).ToString(),WeightIndex);
			}
			else if(Mesh.MeshVariations[0].WeightMeshVariation.IsValidIndex(WeightIndex))
			{
				ApplyNewMesh(GetMeshHolders(Mesh.MeshType),Mesh.MeshVariations[0].WeightMeshVariation[WeightIndex]);
				UE_LOG(LogShib, Warning, TEXT("%hs - Applied Mesh type (Var 0) [%s] with weight index %d"), __FUNCTION__,*UEnum::GetDisplayValueAsText(Mesh.MeshType).ToString(),WeightIndex);
			}
			else
			{
				UE_LOG(LogShib, Warning, TEXT("%hs - No valid mesh variations for Mesh [%s] with weight index %d"), __FUNCTION__,*UEnum::GetDisplayValueAsText(Mesh.MeshType).ToString(),WeightIndex);
			}
		}
	}



}



USkeletalMeshComponent* UShibAvatarBuilderSubsystem::GetAccessoryHolder(EShibAccessory Type)
{
	switch(Type)
	{
	case EShibAccessory::HeadTop:
		return GetAvatarCharacter()->GetHeadTop();
	case EShibAccessory::Arm:
	case EShibAccessory::LeftArm:
		return GetAvatarCharacter()->GetLeftArm();
	case EShibAccessory::RightArm:
		return GetAvatarCharacter()->GetRightArm();
	case EShibAccessory::Back:
		return GetAvatarCharacter()->GetBack();
	case EShibAccessory::Ears:
		return GetAvatarCharacter()->GetEars();
	case EShibAccessory::Face:
		return GetAvatarCharacter()->GetFace();
	case EShibAccessory::Neck:
		return GetAvatarCharacter()->GetNeck();
		
	default:
		return nullptr;
	}
}

void UShibAvatarBuilderSubsystem::ApplyPresetMI(const FShibTexture& Texture)
{
	if(!GetAvatarCharacter())return;
	if(!IsValid(GetAvatarCharacter()->GetBodyDMI())) return;
	
	// Get the current skin id if any applied
	int SkinID = 0;
	FShibTexture SkinData;
	
	if(ShibSaveGame && ShibSaveGame->GetSavedShibTextureAssetData(EShibTexture::Skin, SkinData))
	{
		SkinID = SkinData.TextureID;
	}
	
	

	// Apply Material instance based on skin id
	FShibMaterialWithThumbnail PresetData =  Texture.MaterialAndThumbnailData[SkinID];
	UMaterialInstance* ParentMaterialInstance = Cast<UMaterialInstance>(GetAvatarCharacter()->GetBodyDMI()->Parent);
	if (ParentMaterialInstance)
	{
		UE_LOG(LogShib, Log, TEXT("%hs - Saving Old Material instance [%s] and applying new material instance [%s]"), __FUNCTION__,*ParentMaterialInstance->GetName(),*PresetData.MaterialInstance.GetName());
		SetBodyDynMatInstAndCopyParams(GetAvatarCharacter(), PresetData.MaterialInstance);
	}
	else
	{
		UE_LOG(LogShib, Warning, TEXT("%hs - Failed to cast DMI to Parent MI"), __FUNCTION__);
	}
		
	// Remove Morphs
	if (Texture.RemoveMorph.Num() != 0)
	{
		//LOG_SHIB(Warning, "No empty entry in morph");
		TArray<FName> UseMorph{};
		for (const auto& Morph : Texture.RemoveMorph)
		{
			UseMorph.Add(Morph);
			//LOG_SHIB(Warning, "Remove Morph [%s]", *UseMorph[0].ToString());
			ApplyNewMorphs(GetAvatarCharacter()->GetBody(), 0, UseMorph,0);
			UseMorph.Empty();
		}
	}
	// Add morph
	if (Texture.Morph.Num() != 0)
	{
		ApplyNewMorphs(GetAvatarCharacter()->GetBody(), 0, Texture.Morph,1);
	}

	// Remove Hair Morphs
	if (Texture.HairRemoveMorph.Num() != 0)
	{
		//LOG_SHIB(Warning, "No empty entry in morph");
		TArray<FName> UseMorph{};
		for (const auto& Morph : Texture.HairRemoveMorph)
		{
			UseMorph.Add(Morph);
			ApplyNewMorphs(GetAvatarCharacter()->GetHair(), 0, UseMorph,0);
			UseMorph.Empty();
		}
	}
	// Add Hair morph
	if (Texture.HairMorphs.Num() != 0)
	{
		ApplyNewMorphs(GetAvatarCharacter()->GetHair(), 0, Texture.HairMorphs,1);
	}
}


void UShibAvatarBuilderSubsystem::ApplyNewMaterialValue(const FShibTexture& Texture, bool bFinal)
{
	if (!GetAvatarCharacter()) return;
	TArray<TSharedPtr<IAvatarCommand>> Commands;
	TArray<UTexture*> Previous;

	
	if(Texture.TextureType != EShibTexture::Preset)
	{
		
		for (const auto& Parameter : Texture.TextureParameter)
		{
			FHashedMaterialParameterInfo Info(Parameter, EMaterialParameterAssociation::GlobalParameter);
			if (UTexture* OutTexture; GetAvatarCharacter()->GetBodyDMI()->GetTextureParameterValue(Info, OutTexture))
			{
				Previous.Add(OutTexture);
			}
		}

		if (Texture.Morph.Num() != 0)
		{
			//LOG_SHIB(Warning, "No empty entry in morph");
			//LOG_SHIB(Warning, "Morphs [%s]", *Texture.Morph[0].ToString());
			ApplyNewMorphs(GetAvatarCharacter()->GetBody(), 0, Texture.Morph,1);
		}
		
		if (Texture.Texture != Previous)
		{
			ApplyNewTexture(GetAvatarCharacter()->GetBodyDMI(),Texture.TextureParameter,Texture.Texture);
		}
	}
	else // In the case of Presets we will be applying Material instances based on skin instead of an array of textures
	{
		ApplyPresetMI(Texture);
	}
	

	if(Texture.TattooType != EShibTattoo::NONE && bFinal)
	{
		// Save Tattoo Row name
		if(ShibSaveGame)
		{
			if(IsValid(AvatarDataAsset) && IsValid(AvatarDataAsset->Tattoo))
			{
				for (auto RowName : AvatarDataAsset->Tattoo->GetRowNames())
				{
					FShibTexture* CurrentTexture = AvatarDataAsset->Tattoo->FindRow<FShibTexture>(RowName,"TattooLookup");
					if(CurrentTexture->TextureID == Texture.TextureID && CurrentTexture->Gender == Texture.Gender && CurrentTexture->TattooType == Texture.TattooType)
					{
						ShibSaveGame->SetSavedTattooRowName(Texture.TattooType,RowName);
						AvatarHistory->Add(ShibSaveGame->GetCurrentAvatarData());
					}
				}
			}
		}
		else
		{
			LOG_SHIB(Warning ,"ShibSaveGame not set yet");
		}
	}
	if (Texture.TextureType!=EShibTexture::NONE && bFinal)
	{
		
		// Save Texture Row name
		if(ShibSaveGame)
		{
			if(IsValid(AvatarDataAsset) && AvatarDataAsset->ShibTextureAssetDataTables.Contains(Texture.TextureType) && IsValid(AvatarDataAsset->ShibTextureAssetDataTables[Texture.TextureType]))
			{
				for (auto RowName : AvatarDataAsset->ShibTextureAssetDataTables[Texture.TextureType]->GetRowNames())
				{
					FShibTexture* CurrentTexture = AvatarDataAsset->ShibTextureAssetDataTables[Texture.TextureType]->FindRow<FShibTexture>(RowName,"TextureLookup");
					if(CurrentTexture->TextureID == Texture.TextureID && CurrentTexture->Gender == Texture.Gender && CurrentTexture->TextureType == Texture.TextureType)
					{
						ShibSaveGame->SetSavedShibTextureAssetRowName(Texture.TextureType,RowName);
						AvatarHistory->Add(ShibSaveGame->GetCurrentAvatarData());
					}
				}
			}
		}
		else
		{
			LOG_SHIB(Warning ,"ShibSaveGame not set yet");
		}
	}
	

	// If Skin, update preset instance 
	if(Texture.TextureType == EShibTexture::Skin)
	{
	
	FShibTexture PresetData;
	if(!ShibSaveGame->GetSavedShibTextureAssetData(EShibTexture::Preset, PresetData))
	{
		FName DefaultPresetName = AvatarDataAsset->Preset->GetRowNames()[0];
		PresetData = *AvatarDataAsset->Preset->FindRow<FShibTexture>(DefaultPresetName,"PresetLookup");
	}
	ApplyPresetMI(PresetData);
			
		
	}
	
	
}

void UShibAvatarBuilderSubsystem::SetBodyDynMatInstAndCopyParams(const AShibAvatarCharacter* InAvatarCharacter, UMaterialInterface* NewMaterial)
{
	UMaterialInstanceDynamic* OldMaterial = InAvatarCharacter->BodyDMI;
	if (!OldMaterial) return;
	
	UMaterialInstanceDynamic* NewDynMaterial = InAvatarCharacter->GetBody()->CreateDynamicMaterialInstance(0, NewMaterial);
	
	// vectors
	COPY_DYN_MATERIAL_PARAMETER(OldMaterial, NewDynMaterial, "VarEyeBrowColor", Vector)
	COPY_DYN_MATERIAL_PARAMETER(OldMaterial, NewDynMaterial, "VarTattooHeadColor01", Vector)
	COPY_DYN_MATERIAL_PARAMETER(OldMaterial, NewDynMaterial, "VarTattooChestColor01", Vector)
	COPY_DYN_MATERIAL_PARAMETER(OldMaterial, NewDynMaterial, "VarTattooBackColor01", Vector)

	// scalar
	COPY_DYN_MATERIAL_PARAMETER(OldMaterial, NewDynMaterial, "VarMixAdjustment", Scalar)
	COPY_DYN_MATERIAL_PARAMETER(OldMaterial, NewDynMaterial, "TatooBlendAmount", Scalar)
	
	// texture
	COPY_DYN_MATERIAL_PARAMETER(OldMaterial, NewDynMaterial, "EyebrowsTexture", Texture)
	COPY_DYN_MATERIAL_PARAMETER(OldMaterial, NewDynMaterial, "EyebrowsNormal", Texture)
	COPY_DYN_MATERIAL_PARAMETER(OldMaterial, NewDynMaterial, "TattooHead01", Texture)
	COPY_DYN_MATERIAL_PARAMETER(OldMaterial, NewDynMaterial, "TattooChest01", Texture)
	COPY_DYN_MATERIAL_PARAMETER(OldMaterial, NewDynMaterial, "TattooBack01", Texture)
	COPY_DYN_MATERIAL_PARAMETER(OldMaterial,NewDynMaterial, "TattooArmR01",Texture)
	COPY_DYN_MATERIAL_PARAMETER(OldMaterial,NewDynMaterial, "TattooArmL01",Texture)
}

void UShibAvatarBuilderSubsystem::ApplyNewColor(USkeletalMeshComponent* Holder, const FShibColor& Texture, const FLinearColor& Color, bool bFinal)
{
	if (!Holder)return;

	
	UMaterialInstanceDynamic* Material = Holder->CreateDynamicMaterialInstance(Texture.MaterialIndex, Holder->GetMaterial(Texture.MaterialIndex));

		FHashedMaterialParameterInfo Info(Texture.ColorParameter, EMaterialParameterAssociation::GlobalParameter);
		if(Material)
		{
			Material->SetVectorParameterValue(Texture.ColorParameter, Color);
		}

	FLinearColor CurrentColor;
	// If Accessory
	if(bFinal && Texture.AccessoryType != EShibAccessory::NONE && ShibSaveGame)
	{
		// If color is already saved, make sure its not the same color
		if(!ShibSaveGame->GetSavedAccessoryColorData(Texture.AccessoryType, CurrentColor) || CurrentColor != Color)
		{
			ShibSaveGame->SetAccessoryShibColorData(Texture.AccessoryType, Color);
			AvatarHistory->Add(ShibSaveGame->GetCurrentAvatarData());
		}
	}
	
	// If Tattoo
	if(bFinal && Texture.TattooType != EShibTattoo::NONE && ShibSaveGame)
	{
		// If color is already saved, make sure its not the same color
		if(!ShibSaveGame->GetSavedTattooColorData(Texture.TattooType, CurrentColor) || CurrentColor != Color)
		{
			ShibSaveGame->SetTattooShibColorData(Texture.TattooType, Color);
			AvatarHistory->Add(ShibSaveGame->GetCurrentAvatarData());
		}
	}
	
	// If mesh and not accessory
	if(bFinal &&  Texture.MeshType != EShibMesh::NONE && Texture.MeshType != EShibMesh::Accessory && ShibSaveGame)
	{
		// If color is already saved, make sure its not the same color
		if(!ShibSaveGame->GetSavedShibMeshColorData(Texture.MeshType, CurrentColor) || CurrentColor != Color)
		{
			ShibSaveGame->SetShibMeshColorData(Texture.MeshType, Color);
			AvatarHistory->Add(ShibSaveGame->GetCurrentAvatarData());
		}
	}

	if(bFinal && Texture.TextureType != EShibTexture::NONE && ShibSaveGame)
	{
		// If color is already saved, make sure its not the same color
		if(!ShibSaveGame->GetSavedShibTextureColorData(Texture.TextureType, CurrentColor) || CurrentColor != Color)
		{
			ShibSaveGame->SetShibTextureColorData(Texture.TextureType, Color);
			AvatarHistory->Add(ShibSaveGame->GetCurrentAvatarData());
		}
	}

	

	
	

}


void UShibAvatarBuilderSubsystem::SaveAvatar()
{
	
	if (IsValid(ShibSaveGame))
	{
		if (FString OutJson; ShibSaveGame->SaveCurrentAvatarDataToJson(OutJson))
		{
			OnAvatarBuilderSave.Broadcast(OutJson);
		}
	}
}


void UShibAvatarBuilderSubsystem::LoadAvatarFromJsonFile()
{
	UShibSaveGame* SaveGame = Cast<UShibSaveGame>(UGameplayStatics::CreateSaveGameObject(UShibSaveGame::StaticClass()));
	SaveGame->SetShibBuilder(this);
	SetShibSaveGame(SaveGame);
	AvatarHistory->ResetHistory();
	
	if (IsValid(GetAvatarCharacter()))
	{
		// Check if we were able to load data
		if (SaveGame->LoadCurrentAvatarDataFromJson())
		{
			UE_LOG(LogShib, Log, TEXT("Game loaded successfully from file : %s"), *UShibSaveGame::GetSaveJsonFilePath());
			LoadAvatarData(SaveGame->GetCurrentAvatarData());
		}
		else
		{
			FAvatarSave AvatarSave;
			if (!GetAvatarCharacter()->DefaultHair.IsNull())
				AvatarSave.MeshAssetRowNameData.Add(EShibMesh::Hair, GetAvatarCharacter()->DefaultHair.RowName);
			if (!GetAvatarCharacter()->DefaultTop.IsNull())
				AvatarSave.MeshAssetRowNameData.Add(EShibMesh::Top, GetAvatarCharacter()->DefaultTop.RowName);
			if (!GetAvatarCharacter()->DefaultBottom.IsNull())
				AvatarSave.MeshAssetRowNameData.Add(EShibMesh::Bottom, GetAvatarCharacter()->DefaultBottom.RowName);
			if (!GetAvatarCharacter()->DefaultShoes.IsNull())
				AvatarSave.MeshAssetRowNameData.Add(EShibMesh::Shoe, GetAvatarCharacter()->DefaultShoes.RowName);

			SaveGame->SetCurrentAvatarData(AvatarSave);
			LoadAvatarData(AvatarSave);
			UE_LOG(LogShib, Log, TEXT("%hs - Could not load avatar, setting default values."), __FUNCTION__);
		}
	}

	InitialAvatarSaveData = SaveGame->GetCurrentAvatarData();
	AvatarHistory->Add(InitialAvatarSaveData);
	ShibSaveGame->bTriedToLoad = true;
	OnAvatarLoaded.Broadcast();
	
	
}

void UShibAvatarBuilderSubsystem::LoadAvatarFromJsonString(const FString& JsonString)
{
	UShibSaveGame* SaveGame = Cast<UShibSaveGame>(UGameplayStatics::CreateSaveGameObject(UShibSaveGame::StaticClass()));
	SaveGame->SetShibBuilder(this);
	SetShibSaveGame(SaveGame);
	AvatarHistory->ResetHistory();
	
	// Check if we were able to load data from string
	if (SaveGame->LoadCurrentAvatarDataFromJson(JsonString) && IsValid(GetAvatarCharacter()) )
	{
		UE_LOG(LogShib, Log, TEXT("Game loaded successfully from Json String : %s"), *UShibSaveGame::GetSaveJsonFilePath());
		LoadAvatarData(SaveGame->GetCurrentAvatarData());
	}
	else
	{
		UE_LOG(LogShib, Log, TEXT("%hs - Could not load avatar."), __FUNCTION__);
	}

	InitialAvatarSaveData = SaveGame->GetCurrentAvatarData();
	AvatarHistory->Add(InitialAvatarSaveData);
	ShibSaveGame->bTriedToLoad = true;
	OnAvatarLoaded.Broadcast();

}

void UShibAvatarBuilderSubsystem::LoadAvatarData(FAvatarSave AvatarData,bool bSaveChanges)
{
	if(!ShibSaveGame || !AvatarCharacter) return;
	GetAvatarCharacter()->ResetAvatar();
	
	if(!IsValid(AvatarDataAsset))
	{
		UE_LOG(LogShib, Log, TEXT("%hs - Could not find Data Asset, initializing from Avatar Character"), __FUNCTION__);
		AvatarDataAsset = GetAvatarCharacter()->AvatarDataAsset;

	}

	
	FShibTexture TexData;
	FMesh MeshData;
	FLinearColor ColorData;
	FBase AvatarBase;
	
	
	// Set Gender
	
	if(ShibSaveGame->GetSavedBaseAssetData(AvatarData,AvatarBase))
	{
		LOG_SHIB(Log,"Swapping gender");
		SwapGender(AvatarBase,AvatarBase.Gender,bSaveChanges);
	}
	else
	{
		ShibSaveGame->bTriedToLoad = true;
		return;
	}
	
	LOG_SHIB(Log,"Applying Morphs");
	for (EShibBodyMorphs MorphType : TEnumRange<EShibBodyMorphs>())
	{
		if (MorphType != EShibBodyMorphs::NONE)
		{
			ApplyBodyMorphData(MorphType,AvatarBase,ShibSaveGame->GetSavedMorphData(AvatarData,MorphType),bSaveChanges);
		}
	}

	
	LOG_SHIB(Log,"Applying Accessories");
	for (EShibAccessory AccessoryType : TEnumRange<EShibAccessory>())
	{
		if (AccessoryType == EShibAccessory::NONE) continue;
		
		if (ShibSaveGame->GetSavedAccessoryMeshData(AvatarData, AccessoryType, MeshData))
		{
			ApplyNewAccessory(MeshData,bSaveChanges);
			if (ShibSaveGame->GetSavedAccessoryColorData(AvatarData, AccessoryType, ColorData))
			{
				// Apply Accessory Color
				for (auto Index : MeshData.MaterialIndices)
				{
					FShibColor SColor;
					SColor.MeshType=EShibMesh::Accessory;
					SColor.AccessoryType = MeshData.AccessoryType;
					SColor.ColorParameter = MeshData.ColorParameter;
					SColor.MaterialIndex = Index;
					ApplyNewColor(GetAccessoryHolder(AccessoryType), SColor, ColorData, bSaveChanges);
				}
			}
		}
	}
	
	
	LOG_SHIB(Log,"Applying Meshes");
	for (EShibMesh MeshType : TEnumRange<EShibMesh>())
	{
		if (MeshType != EShibMesh::NONE || MeshType != EShibMesh::Accessory)
		{
			if (ShibSaveGame->GetSavedMeshAssetData(AvatarData,MeshType,MeshData))
			{
				ApplyNewBodyMesh(MeshType,MeshData,bSaveChanges);
			}
	
			if(ShibSaveGame->GetSavedShibMeshColorData(AvatarData,MeshType,ColorData))
			{
				for (auto Index : MeshData.MaterialIndices)
				{
					FShibColor SColor;
					SColor.MeshType = MeshType;
					SColor.ColorParameter = MeshData.ColorParameter;
					SColor.MaterialIndex = Index;
					ApplyNewColor(GetMeshHolders(MeshType),SColor,ColorData,bSaveChanges);
				}
			}
			
		}
	}
	
	LOG_SHIB(Log,"Applying Textures");
	for (EShibTexture ShibTexture : TEnumRange<EShibTexture>())
	{
		if (ShibTexture == EShibTexture::NONE) continue;
		
		if (ShibSaveGame->GetSavedShibTextureAssetData(AvatarData,ShibTexture, TexData))
		{
			UE_LOG(LogShib, Log, TEXT("%hs - Applying Texture type [%s]"), __FUNCTION__,*UEnum::GetDisplayValueAsText(ShibTexture).ToString());

			ApplyNewMaterialValue(TexData, bSaveChanges);
		}

		
		// Only eyebrow and eye textures can have custom colors
		if(ShibTexture == EShibTexture::EyebrowShape)
		{
			FShibColor SColor;
			SColor.ColorParameter = TEXT("VarEyeBrowColor");
			SColor.TextureType = ShibTexture;
			if(ShibSaveGame->GetSavedShibTextureColorData(AvatarData,ShibTexture,ColorData))
			{
				ApplyNewColor(AvatarCharacter->GetBody(), SColor, ColorData, bSaveChanges);
			}
		}
		else if(ShibTexture == EShibTexture::EyeShape)
		{
			FShibColor SColor;
			SColor.ColorParameter = "EyeColorAdjustment";
			SColor.MaterialIndex = 3;
			SColor.TextureType = ShibTexture;
			if(ShibSaveGame->GetSavedShibTextureColorData(AvatarData,ShibTexture,ColorData))
			{
				ApplyNewColor(AvatarCharacter->GetBody(), SColor, ColorData, bSaveChanges);
			}
		}
	}
	
	
	
	LOG_SHIB(Log,"Applying Tattoos");
	for (EShibTattoo TattooType : TEnumRange<EShibTattoo>())
	{
		if (TattooType == EShibTattoo::NONE) continue;
		
		if (ShibSaveGame->GetSavedTattooTextureData(AvatarData, TattooType, TexData))
		{
			ApplyNewMaterialValue(TexData, bSaveChanges);
	
			if (ShibSaveGame->GetSavedTattooColorData(AvatarData,TattooType, ColorData))
			{
				// Apply Tattoo Color
				FShibColor SColor;
				SColor.TattooType=TattooType;
				SColor.ColorParameter = TexData.ColorParameter;
				SColor.MaterialIndex = TexData.MaterialIndex;
				ApplyNewColor(AvatarCharacter->GetBody(), SColor, ColorData, bSaveChanges);
			}
		}
	}

	UpdateSkmBasedOnMesh();

	LOG_SHIB(Log,"Load Avatar Completed");
	ShibSaveGame->bTriedToLoad = true;
	OnAvatarLoaded.Broadcast();

}

void UShibAvatarBuilderSubsystem::LoadReplicatedAvatar(AShibAvatarCharacter* Character, const FString& String)
{
	if (!IsValid(Character)) return;

	Character->ResetAvatar();
	FAvatarSave Avatar;
	
	SetAvatarCharacter(Character, false);
	ShibSaveGame = Cast<UShibSaveGame>(UGameplayStatics::CreateSaveGameObject(UShibSaveGame::StaticClass()));
	ShibSaveGame->SetShibBuilder(this);
	
	if (String.IsEmpty() || !FJsonObjectConverter::JsonObjectStringToUStruct<FAvatarSave>(String, &Avatar))
	{
		if (Character->IsLocallyControlled() && ShibSaveGame->LoadCurrentAvatarDataFromJson())
		{
			Avatar = ShibSaveGame->GetCurrentAvatarData();
		} else
		{
			if (!GetAvatarCharacter()->DefaultHair.IsNull())
				Avatar.MeshAssetRowNameData.Add(EShibMesh::Hair, Character->DefaultHair.RowName);
			if (!GetAvatarCharacter()->DefaultTop.IsNull())
				Avatar.MeshAssetRowNameData.Add(EShibMesh::Top, Character->DefaultTop.RowName);
			if (!GetAvatarCharacter()->DefaultBottom.IsNull())
				Avatar.MeshAssetRowNameData.Add(EShibMesh::Bottom, Character->DefaultBottom.RowName);
			if (!GetAvatarCharacter()->DefaultShoes.IsNull())
				Avatar.MeshAssetRowNameData.Add(EShibMesh::Shoe, Character->DefaultShoes.RowName);
		}
	}
	
	ShibSaveGame->SetCurrentAvatarData(Avatar);
	
	LoadAvatarData(Avatar, false);
	
	SetAvatarCharacter(nullptr, true);
	ShibSaveGame = nullptr;
}

void UShibAvatarBuilderSubsystem::Undo()
{

	FAvatarSave AvatarData;
	if (!AvatarHistory->Undo(AvatarData)) return;
	GetAvatarCharacter()->SetCurrentMontagePosition();
	// Disable adding in new states when avatar is loading in a previous state
	AvatarHistory->LockHistory();
	
	// apply above struct the same way we apply when loading a save
	ShibSaveGame->SetCurrentAvatarData(AvatarData);
	LoadAvatarData(AvatarData);
	// Enable saving once previous state has been fully applied
	AvatarHistory->UnlockHistory();
	
}

void UShibAvatarBuilderSubsystem::Redo()
{

	FAvatarSave AvatarData;
	if (!AvatarHistory->Redo(AvatarData)) return;
	GetAvatarCharacter()->SetCurrentMontagePosition();
	
	// Disable adding in new states when avatar is loading in a previous state
	AvatarHistory->LockHistory();
	// Apply above struct the same way we apply when loading a save
	ShibSaveGame->SetCurrentAvatarData(AvatarData);
	LoadAvatarData(AvatarData);
	// Enable saving once previous state has been fully applied
	AvatarHistory->UnlockHistory();
	
}



void UShibAvatarBuilderSubsystem::BuildRandomAvatar()
{
	if(!IsValid(AvatarDataAsset)) return;
	AvatarHistory->LockHistory();
	const TArray<FName>& Bases = AvatarDataAsset->Base->GetRowNames();

	UE_LOG(LogTemp, Warning, TEXT("Start randomizing!"));

	ApplyRandomBaseMorphs(Bases);

	for (auto AccType : TEnumRange<EShibAccessory>())
	{
		if(AccType == EShibAccessory::NONE) continue;
		ApplyRandomAccessory(AccType);
	}
	
	if (FMath::RandBool())
	{
		ApplyRandomMeshWithColor(EShibMesh::Top);
		ApplyRandomMeshWithColor(EShibMesh::Bottom);
		ApplyRandomMeshWithColor(EShibMesh::Hair);
	}
	else
	{
		ApplyRandomMeshWithColor(EShibMesh::FullBody);
	}
	 ApplyRandomMeshWithColor(EShibMesh::Shoe);
			
	
	ApplyRandomTextures();

	for (auto TatType : TEnumRange<EShibTattoo>())
	{
		if(TatType== EShibTattoo::NONE) continue;
		ApplyRandomTattoo(TatType);
	}
	


	{
		FShibColor Color;
		Color.ColorParameter = "EyeColorAdjustment";
		Color.MaterialIndex = 3;
		FLinearColor RandEyeColor = GenerateRandomColor();
		ApplyNewColor(GetAvatarCharacter()->GetBody(), Color, RandEyeColor, true);
		if(ShibSaveGame)
		{
			ShibSaveGame->SetShibTextureColorData(EShibTexture::EyeShape,RandEyeColor);
		}
	}

	
	UE_LOG(LogTemp, Warning, TEXT("Randomized!"));
	OnAvatarLoaded.Broadcast();
	AvatarHistory->UnlockHistory();
	ShibSaveGame->PrintCurrentSaveGame();
	AvatarHistory->Add(ShibSaveGame->GetCurrentAvatarData());
}

void UShibAvatarBuilderSubsystem::ApplyRandomMeshWithColor(EShibMesh MeshType)
{
	FName RandomRow = FName();
	FMesh RandomMesh = FMesh();
	bool bIsUndergarment = true;
	// Repeat until a mesh with the correct gender is found and it is not an undergarment
	while (RandomMesh.Gender != GetGender() || bIsUndergarment)
	{
		bIsUndergarment = false;
		
		// Get row names for the specified mesh type
		const TArray<FName>& RandomRows = AvatarDataAsset->MeshAssetDataTables[MeshType]->GetRowNames();
		RandomRow = RandomRows[FMath::RandRange(0, RandomRows.Num() - 1)];

		// Find and assign the selected mesh
		if (FMesh* SelectedMesh = AvatarDataAsset->MeshAssetDataTables[MeshType]->FindRow<FMesh>(RandomRow, TEXT("")))
		{
			RandomMesh = *SelectedMesh;
		}

		// Undergarments are Top/Bottom meshes with ID 0 
		if(RandomMesh.MeshType == EShibMesh::Top || RandomMesh.MeshType == EShibMesh::Bottom)
		{
			if(RandomMesh.MeshID == 0)
			{
				bIsUndergarment = true;
			}
		}
	}

	// Apply mesh
	ApplyNewBodyMesh(MeshType, RandomMesh);
	

	// Set color properties and apply
	if (MeshType == EShibMesh::FullBody && RandomMesh.ColorParameter == NAME_None)return;
	FLinearColor RandMeshColor = GenerateRandomColor();

	for (auto MatIndex: RandomMesh.MaterialIndices)
	{
		FShibColor Color;
		Color.MeshType = MeshType;
		Color.ColorParameter = RandomMesh.ColorParameter;
		Color.MaterialIndex = MatIndex;
		ApplyNewColor(GetMeshHolders(MeshType), Color, RandMeshColor, true);
	}
}

void UShibAvatarBuilderSubsystem::ApplyRandomAccessory(EShibAccessory AccessoryType)
{
	if(!AvatarDataAsset || !AvatarDataAsset->Accessory) return;
	const TArray<FName> Accessories = AvatarDataAsset->Accessory->GetRowNames();

	// Get the relevant row names for the data table for the accessory type
	TArray<FName> TargetAccessoryRows;
	for (auto RowName : Accessories)
	{
		FMesh *AccessoryMesh = AvatarDataAsset->Accessory->FindRow<FMesh>(RowName,"");
		if (AccessoryMesh && Gender == AccessoryMesh->Gender && AccessoryMesh->AccessoryType == AccessoryType)
		{
			TargetAccessoryRows.Add(RowName);
		}
	}

	if(TargetAccessoryRows.IsEmpty())
	{
		UE_LOG(LogShib, Log, TEXT("%hs - Could not find random accessories for type [%s]"), __FUNCTION__,*UEnum::GetDisplayValueAsText(AccessoryType).ToString());

		return;
	}
	
	FName RandomRow = TargetAccessoryRows[FMath::RandRange(0, TargetAccessoryRows.Num() - 1)];
	FMesh RandomAccessory = *AvatarDataAsset->Accessory->FindRow<FMesh>(RandomRow, TEXT(""));
	
	// Apply the accessory
	ApplyNewAccessory(RandomAccessory);

	FLinearColor RandAccessoryColor = GenerateRandomColor();
	for (auto MatIndex: RandomAccessory.MaterialIndices)
	{
		// Set color properties and apply
		FShibColor Color;
		Color.MeshType = EShibMesh::Accessory;
		Color.AccessoryType = RandomAccessory.AccessoryType;
		Color.ColorParameter = RandomAccessory.ColorParameter;
		Color.MaterialIndex =MatIndex;
		ApplyNewColor(GetAccessoryHolder(RandomAccessory.AccessoryType), Color,RandAccessoryColor , true);
	}
	
}

void UShibAvatarBuilderSubsystem::ApplyRandomBaseMorphs(const TArray<FName>& Bases)
{
	FBase RandomBase;
	FName RandomRow;

	// Repeat until a valid base with the correct gender is found
	while (RandomBase.Gender != GetGender())
	{
		RandomRow = Bases[FMath::RandRange(0, Bases.Num() - 1)];
		RandomBase = *AvatarDataAsset->Base->FindRow<FBase>(RandomRow, TEXT(""));
	}
	ShibSaveGame->SetBaseDataRowName(RandomRow, RandomBase.Gender);
	// Apply body morphs
	for (EShibBodyMorphs BodyMorph : TEnumRange<EShibBodyMorphs>())
	{
		if(BodyMorph == EShibBodyMorphs::AvatarWeight)
		{
			// 0, 0.5 or 1 for weight
			ApplyBodyMorphData(BodyMorph, RandomBase, FMath::RandRange(0, 2)/2.0f, true);
			continue;
		}
		ApplyBodyMorphData(BodyMorph, RandomBase, FMath::FRandRange(0.1f, 0.9f), true);
	}

	
}

void UShibAvatarBuilderSubsystem::ApplyRandomTattoo(EShibTattoo TattooType)
{

	if(!AvatarDataAsset || !AvatarDataAsset->Tattoo) return;
	const TArray<FName> Tattoos = AvatarDataAsset->Tattoo->GetRowNames();

	// Get the relevant row names for the data table for the tattoo type
	TArray<FName> TargetTattoosRows;
	for (auto RowName : Tattoos)
	{
		FShibTexture *TattooTexture = AvatarDataAsset->Tattoo->FindRow<FShibTexture>(RowName,"");
		if (TattooTexture && Gender == TattooTexture->Gender && TattooTexture->TattooType == TattooType)
		{
			TargetTattoosRows.Add(RowName);
		}
	}

	if(TargetTattoosRows.IsEmpty())
	{
		UE_LOG(LogShib, Log, TEXT("%hs - Could not find random tattoo for type [%s]"), __FUNCTION__,*UEnum::GetDisplayValueAsText(TattooType).ToString());

		return;
	}
	
	FName RandomRow = TargetTattoosRows[FMath::RandRange(0, TargetTattoosRows.Num() - 1)];
	FShibTexture RandomTattoo = *AvatarDataAsset->Tattoo->FindRow<FShibTexture>(RandomRow, TEXT(""));
	
	FShibColor Color;
	Color.TattooType = RandomTattoo.TattooType;
	Color.ColorParameter = RandomTattoo.ColorParameter;
	Color.MaterialIndex = RandomTattoo.MaterialIndex;

	// Apply tattoo logic based on thumbnail name
	if (RandomTattoo.Thumbnail.GetName() == "T_Remove")
	{
		ApplyNewColor(GetAvatarCharacter()->GetBody(), Color, FLinearColor::White, true);
	}
	else
	{
		ApplyNewMaterialValue(RandomTattoo,true);
		ApplyNewColor(GetAvatarCharacter()->GetBody(), Color, GenerateRandomColor(), true);
	}
	if (ShibSaveGame)
	{
		ShibSaveGame->SetSavedTattooRowName(RandomTattoo.TattooType, RandomRow);
		ShibSaveGame->SetTattooShibColorData(RandomTattoo.TattooType, GenerateRandomColor());
	}
}

void UShibAvatarBuilderSubsystem::ApplyRandomTextures()
{
	
	for (EShibTexture TexType : TEnumRange<EShibTexture>())
	{
		// Skip irrelevant texture types
		if (TexType == EShibTexture::NONE || TexType == EShibTexture::EyeShape) continue;
		
		FShibTexture RandomTex;
		FName RandomRow;

		// Repeat until a valid texture with the correct gender is found
		while (RandomTex.Gender != GetGender())
		{
			const TArray<FName>& RandomRows = AvatarDataAsset->ShibTextureAssetDataTables[TexType]->GetRowNames();
			RandomRow = RandomRows[FMath::RandRange(0, RandomRows.Num() - 1)];

			if (FShibTexture* SelectedMesh = AvatarDataAsset->ShibTextureAssetDataTables[TexType]->FindRow<FShibTexture>(RandomRow, TEXT("")))
			{
				RandomTex = *SelectedMesh;
			}
		}

		// Apply the texture material value
		ApplyNewMaterialValue(RandomTex);
		ShibSaveGame->SetSavedShibTextureAssetRowName(TexType, RandomRow);

		// Handle eyebrow shape separately to apply color
		if (TexType == EShibTexture::EyebrowShape)
		{
			FShibColor Color;
			Color.ColorParameter = RandomTex.ColorParameter;
			Color.MaterialIndex = RandomTex.MaterialIndex;

			FLinearColor RandEyeBrowColor = GenerateRandomColor();
			ApplyNewColor(GetAvatarCharacter()->GetBody(), Color, RandEyeBrowColor, true);
			ShibSaveGame->SetShibTextureColorData(EShibTexture::EyebrowShape, RandEyeBrowColor);
		}
	}
}


void UShibAvatarBuilderSubsystem::SwapGender( const FBase& Base, EAvatarBuilderGender InGender,bool bFinal)
{
	
	if (!GetAvatarCharacter() || !IsValid(AvatarDataAsset) || Gender==InGender) return;
	LOG_SHIB(Log,"Attempting to Swap Gender from [%s] to [%s]",*UEnum::GetDisplayValueAsText(Gender).ToString(),*UEnum::GetDisplayValueAsText(InGender).ToString())
	AvatarHistory->LockHistory();

	if (Base.Gender == InGender)
	{
		Gender=InGender;
		
		if(ShibSaveGame && bFinal)
		{
			if(IsValid(AvatarDataAsset) && IsValid(AvatarDataAsset->Base))
			{
				for (auto RowName : AvatarDataAsset->Base->GetRowNames())
				{
					if(AvatarDataAsset->Base->FindRow<FBase>(RowName,"GenderLookup")->Gender == InGender)
					{
						ShibSaveGame->SetBaseDataRowName(RowName,InGender);
					}
				}
			}
		}
		GetAvatarCharacter()->ResetAvatar(false);
		LOG_SHIB(Log,"Add gender change command for [%s]",*UEnum::GetDisplayValueAsText(InGender).ToString());
		
		GetAvatarCharacter()->SetCurrentMontagePosition();
		GetAvatarCharacter()->GetBody()->SetSkeletalMesh(Base.Mesh);
		if (bShouldPlayMontage) GetAvatarCharacter()->PlayMontageFromCurrentPosition(Base.AnimMontage);
		
		// Apply Body morph data
		FBase MyBase;
		FBase CurrentBase;
		
		ShibSaveGame->GetSavedBaseAssetData(CurrentBase);
		if (IsMatchingID(AvatarDataAsset->Base, CurrentBase.MeshID, InGender, MyBase))
		{
			
			{
			
				float CurrentWeight = ShibSaveGame->GetSavedMorphData(EShibBodyMorphs::AvatarWeight);
				
				ApplyNewMorphs(GetAvatarCharacter()->GetBody(),0,MyBase.MorphData.BodyMorphs, CurrentWeight);
				ApplyNewMorphs(GetAvatarCharacter()->GetBody(),0,MyBase.MorphData.BeardMorphs.BodyMorphs, CurrentWeight);
				ApplyNewMorphs(GetAvatarCharacter()->GetBody(), 0, MyBase.MorphData.LowerUndergarmentMorphs, CurrentWeight);
				ApplyNewMorphs(GetAvatarCharacter()->GetBody(), 0, MyBase.MorphData.UpperUndergarmentMorphs, CurrentWeight);
				
				ApplyNewHeight(ShibSaveGame->GetSavedMorphData(EShibBodyMorphs::AvatarHeight), bFinal);


				for (auto MorphType : TEnumRange<EShibBodyMorphs>())
				{
					ApplyNewMorphs(GetAvatarCharacter()->GetBody(),nullptr,GetFacialMorphNames(MyBase,MorphType),ShibSaveGame->GetSavedMorphData(MorphType));
					ApplyNewMorphs(GetAvatarCharacter()->GetHair(),nullptr,GetBeardMorphNames(MyBase,MorphType),ShibSaveGame->GetSavedMorphData(MorphType));
				}
			}
			
			// Swap Selected Preset
			if(ShibSaveGame)
			{
				FShibTexture AppliedPresetData;
				// Get applied preset (If none is saved then get default)
				if(!ShibSaveGame->GetSavedShibTextureAssetData(EShibTexture::Preset,AppliedPresetData))
				{
					FName DefaultPresetName = AvatarDataAsset->Preset->GetRowNames()[0];
					AppliedPresetData = *AvatarDataAsset->Preset->FindRow<FShibTexture>(DefaultPresetName,"PresetLookup");
				}
				
				for (auto Row : AvatarDataAsset->Preset->GetRowNames())
				{
					// Get Matching preset for other gender and apply
					FShibTexture* PresetTexture = AvatarDataAsset->Preset->FindRow<FShibTexture>(Row,"");
					if(PresetTexture->TextureID == AppliedPresetData.TextureID && PresetTexture->Gender == InGender)
					{
						ApplyPresetMI(*PresetTexture);

						// Update SaveGame
						if (bFinal) ShibSaveGame->SetSavedShibTextureAssetRowName(EShibTexture::Preset,Row);
					}
				}
			}
		}

		// Swap eyebrows
		FShibTexture CurrentEyebrows;
		if(ShibSaveGame->GetSavedShibTextureAssetData(EShibTexture::EyebrowShape,CurrentEyebrows))
		{
			for (auto Row : AvatarDataAsset->Eyebrow->GetRowNames())
			{
				FShibTexture* EyebrowTex = AvatarDataAsset->Eyebrow->FindRow<FShibTexture>(Row,"");
				if(EyebrowTex && EyebrowTex->TextureID == CurrentEyebrows.TextureID && EyebrowTex->Gender == InGender)
				{
					ApplyNewMaterialValue(*EyebrowTex,bFinal);
					break;
				}
			}
		}
		


		
		// Apply hair, top, bottom, fullbody, shoes
		for (EShibMesh MeshType : TEnumRange<EShibMesh>())
		{
		

			if (MeshType==EShibMesh::Accessory || MeshType==EShibMesh::NONE) continue;
			FMesh Mesh;
			FMesh CurrentMesh;
			
			ShibSaveGame->GetSavedMeshAssetData(MeshType,CurrentMesh);
			if (IsMatchingIDForAllType(AvatarDataAsset->MeshAssetDataTables[MeshType],CurrentMesh.MeshID, InGender, Mesh, EShibAccessory::NONE))
			{
				ApplyNewBodyMesh(MeshType,Mesh,bFinal);
				FLinearColor CurrentColor;
				if(!ShibSaveGame->GetSavedShibMeshColorData(MeshType,CurrentColor))
				{
					CurrentColor = Mesh.DefaultColor;
				}

				for (auto MatIndex: Mesh.MaterialIndices)
				{
					FShibColor Color;
					Color.MeshType=MeshType;
					Color.ColorParameter = Mesh.ColorParameter;
					Color.MaterialIndex = MatIndex;

					ApplyNewColor(GetMeshHolders(MeshType), Color, CurrentColor, bFinal);
				}
			}
			else
			{
				RemoveBodyMesh(MeshType,bFinal);
			}
		}


		// Apply accessories
		for (EShibAccessory AccessoryType : TEnumRange<EShibAccessory>())
		{
			if (AccessoryType == EShibAccessory::NONE) continue;

			FMesh AccessoryMesh;
			FMesh CurrentAccessoryMesh;
			ShibSaveGame->GetSavedAccessoryMeshData(AccessoryType,CurrentAccessoryMesh);
			if (IsMatchingIDForAllType(AvatarDataAsset->Accessory, CurrentAccessoryMesh.MeshID, InGender, AccessoryMesh,AccessoryType))
			{
				FShibColor Color;
				FLinearColor CurrentColor ;
				if(!ShibSaveGame->GetSavedAccessoryColorData(AccessoryType,CurrentColor))
				{
					CurrentColor = AccessoryMesh.DefaultColor;
				}
				ApplyNewAccessory(AccessoryMesh,bFinal);

				for (auto MatIndex: AccessoryMesh.MaterialIndices)
				{
					Color.MeshType=EShibMesh::Accessory;
					Color.AccessoryType=AccessoryType;
					Color.ColorParameter = AccessoryMesh.ColorParameter;
					Color.MaterialIndex = MatIndex;
					ApplyNewColor(GetAccessoryHolder(AccessoryType), Color, CurrentColor, bFinal);
				}
				
			}
			else
			{
				RemoveAccessoryMesh(AccessoryType,bFinal);
			}
		}
	}

	AvatarHistory->UnlockHistory();
	AvatarHistory->Add(ShibSaveGame->GetCurrentAvatarData());
	
	UpdateSkmBasedOnMesh();
	
	OnGenderSwapped.Broadcast();
}



// Used for Commands
void UShibAvatarBuilderSubsystem::SetHeight(AShibAvatarCharacter* AvatarCharacter, float Value)
{

	// TODO - Disabling Setting the height for now due to bugs with the capsule collider for remote clients that cause clipping

	
	if (!AvatarCharacter)
	{
		//LOG_SHIB(Warning, "Character is invalid!");
		return;
	}
	AvatarCharacter->SetNewHeightFromScale(Value);
	

	
}

void UShibAvatarBuilderSubsystem::ApplyNewTexture(UMaterialInstanceDynamic* Material, const TArray<FName> Parameters, const TArray<UTexture*>& Textures)
{
	for (int32 i = 0; i < Textures.Num(); ++i)
	{
		if (Material && Parameters.IsValidIndex(i))
		{
			Material->SetTextureParameterValue(Parameters[i], Textures[i]);
		}
	}
}

void UShibAvatarBuilderSubsystem::ApplyNewMorphs(USkeletalMeshComponent* Component, USkeletalMesh* Mesh, const TArray<FName> Morph, const float& Value)
{
	
	if (!Mesh && Morph.Num() > 0)
	{
		if (Morph.Num() == 1)
		{
		

			Component->SetMorphTarget(Morph[0], Value);
		}
		else if (Morph.Num() == 2)
		{
			
			if (Value < 0.5f)
			{
				float EarlyMorhValue = FMath::GetMappedRangeValueClamped(FVector2D(0.0f, 0.5f), FVector2D(0.5f, 0.0f), Value);
				Component->SetMorphTarget(Morph[0], EarlyMorhValue);
				Component->SetMorphTarget(Morph[1], 0.0f);
			}
			else
			{
				float LateMorphValue = FMath::GetMappedRangeValueClamped(FVector2D(0.5f, 1.0f), FVector2D(0.0f, 0.5f), Value);
				Component->SetMorphTarget(Morph[1], LateMorphValue);
				Component->SetMorphTarget(Morph[0], 0.0f);
			}
		}
	}
}

void UShibAvatarBuilderSubsystem::ApplyNewMesh(USkeletalMeshComponent* Holder, USkeletalMesh* NewMesh)
{
	if (NewMesh)
	{
		Holder->SetSkeletalMeshAsset(nullptr);
		Holder->SetSkeletalMeshAsset(NewMesh);
	}
	else
	{
		Holder->SetSkeletalMeshAsset(nullptr);
	}
}

