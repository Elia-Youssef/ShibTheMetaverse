// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Actors/BaseBuildActor.h"
#include "Engine/DataTable.h"
#include "UObject/ScriptDelegates.h"
#include "BuildTypes.generated.h"


UENUM(BlueprintType)
enum class EInteractionMode : uint8
{
	None,
	View,
	Build
};

UENUM(BlueprintType)
enum class EUICategories : uint8
{
	GeneralStorefront,
	RocketPond,
	Wagmi,
	SheikhZayedVillas,
	Interior,
	Exterior,
	Hardwood,
	FloorsTiles,
	Carpets,
	Stone,
	RoofTiles,
	ThatchedRoofing,
	Shingles,
	LivingRoom,
	BedRoom,
	DiningRoom,
	WallArt,
	Plants,
	Rugs,
	Lighting,
	WallDecorations,
	CurtainsBlinds,
	CeilingFans,
	DoorsWindows,
	OutdoorFurnitures,
	ExteriorDecorations,
	PlantsGarden,
	Kitchen,
	Bathroom,
	InteractiveItems,
	DecorativeItems
};

UENUM(BlueprintType)
enum class EUIParentCategories : uint8
{
	None,
	ShibHouses,
	SheikhZayedVillas,
	Wall,
	Floors,
	Roof,
	Furniture,
	Decorations,
	InteriorFixtures,
	Exterior,
	Appliances,
	SpecialObjects
};

UENUM(BlueprintType)
enum class EObjectInteractionState : uint8
{
	Spawn,
	Delete,
	Move,
	Redo,
	Undo
};

UENUM(BlueprintType)
enum class EFurnitureSnap : uint8
{
	None,
	Walls,
	Interior,
	OutsideFloors,
	InsideFloors,
	Roof,
	OutsideGround
};

USTRUCT(BlueprintType)
struct FObjectType
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FName StructureTag = FName();

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FName ParentTag = FName();

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FName Tag = FName();
};

USTRUCT(BlueprintType)
struct FObjectPossibelMaterials
{
	GENERATED_BODY()
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TArray<UMaterialInstance*> PossibleMaterial;
};

USTRUCT(BlueprintType)
struct FObjectUICategories
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	EUIParentCategories ParentUICategories = EUIParentCategories::None;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TArray<EUICategories> ChildUICategories;
};


USTRUCT(BlueprintType)
struct FObjectDetails : public FTableRowBase
{
	GENERATED_BODY()
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FObjectUICategories ObjectUICategories;
	
	// Inventory title
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FText Title = FText();

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FObjectType ObjectType = FObjectType();

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TSubclassOf<ABaseBuildActor> ClassToSpawn = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FSlateBrush PreviewImage = FSlateBrush();

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	bool bNeedsBaseObject = false;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	bool bNeedsSocket = false;

	// check for children in inventory if down arrow is pressed
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	bool bHasChildren = false;

	// required if bHasChildren
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FSlateBrush ParentPreviewImage = FSlateBrush();
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
    bool bInteractable = false;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TMap<FName, FObjectPossibelMaterials> ObjectPossibleMaterials;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	bool bIsHouseOrVilla = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TArray<EFurnitureSnap> FurnitureSnapLocation;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TArray<int32> MaterialIdsForColorPicker;
};


USTRUCT(BlueprintType)
struct FObjectBuiltData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	FName ID = FName();

	UPROPERTY(BlueprintReadOnly)
	FName Parent = FName();

	FName Title = FName();

	// UPROPERTY(BlueprintReadOnly)
	// TSubclassOf<ABaseBuildActor> ClassToSpawn = nullptr;

	UPROPERTY(BlueprintReadOnly)
	FName ClassToSpawnRowName = FName();
	
	UPROPERTY(BlueprintReadOnly)
	FTransform Transform;

	UPROPERTY(BlueprintReadOnly)
	FName MaterialType;
	
	UPROPERTY(BlueprintReadOnly)
	UStaticMesh* Mesh;
	
	UPROPERTY(BlueprintReadOnly)
	int32 TextureIndex = 0;
	
	// UPROPERTY(BlueprintReadOnly)
	// FName ItemDataTableRowName = FName();
	//
	// UPROPERTY(BlueprintReadOnly)
	// TMap<UStaticMesh*, UMaterial*> UpdatedMeshes;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	AActor* BuiltActor;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EObjectInteractionState InteractionState = EObjectInteractionState::Spawn;

	friend bool operator==(const FObjectBuiltData& Ob1, const FObjectBuiltData& Ob2) { return Ob1.ID == Ob2.ID; }

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bIsHouseOrVilla = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	AActor* ActorParentReference = nullptr;
};

USTRUCT(BlueprintType)
struct FMaterialType
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	FName Name = FName();

	UPROPERTY(BlueprintReadOnly)
	FName Type = FName();

	UPROPERTY(BlueprintReadOnly);
	FSlateBrush PreviewImage = FSlateBrush();
};

UENUM(BlueprintType)
enum class EWidgetMode : uint8
{
	None,
	Build,
	Kits,
	Assets,
	Inventory,
	Favorite,
	Deals
};

USTRUCT(BlueprintType)
struct FWidgetItem
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	FName Tag = FName();

	UPROPERTY(BlueprintReadOnly)
	FSlateBrush PreviewImage = FSlateBrush();

	UPROPERTY(BlueprintReadOnly)
	FString Title = FString();
};

USTRUCT(BlueprintType)
struct FItemArrowsAvailability
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	bool bUp = false;
	
	UPROPERTY(BlueprintReadOnly)
	bool bDown = false;
	
	UPROPERTY(BlueprintReadOnly)
	bool bLeft = false;
	
	UPROPERTY(BlueprintReadOnly)
	bool bRight = false;
};

USTRUCT(BlueprintType)
struct FSelectedHitActors
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	AActor* SelectedHitActorParentRef = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	AActor* SelectedHitActorRef = nullptr;
};
