// Copyright Shiba Inu Games LLC.

#pragma once

#include "CoreMinimal.h"
#include "AvatarBuilderLibrary.h"
#include "JsonObjectConverter.h"
#include "UObject/NoExportTypes.h"
#include "OperationsManager.generated.h"

class UAvatarBuilderSubsystem;

static TMap<FName,int> OldSkintoNew={
	{FName("NewRow"), 0},
	   {FName("NewRow_0"), 1},
	   {FName("NewRow_1"), 2},
	   {FName("NewRow_2"), 3},
		{FName("NewRow_3"), 4},
		{FName("NewRow_4"), 5},
		{FName("NewRow_5"), 6},
		{FName("NewRow_6"), 7},
	   {FName("NewRow_7"), 0},
		{FName("NewRow_8"), 1},
		{FName("NewRow_9"), 2},
		{FName("NewRow_10"), 3},
		{FName("NewRow_11"), 4},
		{FName("NewRow_12"), 5},
		{FName("NewRow_13"), 6},
		{FName("NewRow_14"), 7},
};

static TMap<int, FName> NewSkintoOld = {
	{0, FName("NewRow")},
	{1, FName("NewRow_0")},
	{2, FName("NewRow_1")},
	{3, FName("NewRow_2")},
	{4, FName("NewRow_3")},
	{5, FName("NewRow_4")},
	{6, FName("NewRow_5")},
	{7, FName("NewRow_6")}
};

USTRUCT(BlueprintType)
struct FCurrentAvatarState
{
	GENERATED_BODY()

	FCurrentAvatarState()
	{
		bIsMale=true;
	}

	void SetAsDefaultState()
	{
		bIsMale=true;
		MeshAssetRowNameData.Add("Bottom","Bottom_NoBottom");
		MorphData.Add("nONE", 0.5);
		MorphData.Add("avatarHeight", 0.5);
		MorphData.Add("avatarWeight", 0.5);
		MorphData.Add("jawLength", 0.5);
		MorphData.Add("jawWidth", 0.5);
		MorphData.Add("jawHeight", 0.5);
		MorphData.Add("noseLength", 0.5);
		MorphData.Add("noseWidth", 0.5);
		MorphData.Add("noseHeight", 0.5);
		MorphData.Add("mouthThickness", 0.5);
		MorphData.Add("mouthWidth", 0.5);
		MorphData.Add("mouthHeight", 0.5);
		MorphData.Add("accessoriesNoseLength", 0.5);
		MorphData.Add("accessoriesNoseWidth", 0.5);
		MorphData.Add("accessoriesNoseHeight", 0.5);
		ShibTextureAssetRowNameData.Add("Skin",NewSkintoOld[0]);
		ShibTextureAssetRowNameData.Add("Preset", "_Caucasian");
	}

	UPROPERTY(BlueprintReadWrite)
	FName AvatarBaseRow;

	UPROPERTY(BlueprintReadWrite)
	bool bIsMale;

	UPROPERTY(BlueprintReadWrite)
	TMap<FName, FName> MeshAssetRowNameData;

	UPROPERTY(BlueprintReadWrite)
	TMap<FName, FName> ShibTextureAssetRowNameData;

	UPROPERTY(BlueprintReadWrite)
	TMap<FName, FName> AccessoryRowNameData;

	UPROPERTY(BlueprintReadWrite)
	TMap<FName, FName> TattooRowNameData;

	UPROPERTY(BlueprintReadWrite)
	TMap<FName, float> MorphData;

	UPROPERTY(BlueprintReadWrite)
	TMap<FName, FLinearColor> ShibMeshColorData;

	UPROPERTY(BlueprintReadWrite)
	TMap<FName, FLinearColor> ShibTextureColorData;

	UPROPERTY(BlueprintReadWrite)
	TMap<FName, FLinearColor> AccessoryShibColorData;

	UPROPERTY(BlueprintReadWrite)
	TMap<FName, FLinearColor> TattooShibColorData;

	// Convert struct to JSON string
	FString ToJson() const
	{
		FString JsonString;
		FJsonObjectConverter::UStructToJsonObjectString(*this, JsonString);
		return JsonString;
	}

	// Convert JSON string to struct
	static bool FromJson(const FString& JsonString, FCurrentAvatarState& OutState)
	{
		return FJsonObjectConverter::JsonObjectStringToUStruct(JsonString, &OutState, 0, 0);
	}
};

UCLASS(Blueprintable)
class SHIBAVATARBUILDER_API UAvatarOperationsManager : public UObject
{
	GENERATED_BODY()

private:
	UPROPERTY()
	TArray<FString> UndoStack;

	UPROPERTY()
	TArray<FString> RedoStack;

	UPROPERTY()
	FCurrentAvatarState CurrentState;

	UPROPERTY()
	FCurrentAvatarState DefaultState;

public:

	static inline const FString& AvatarCustomizationSlot{"AvatarCustomizationSlot"};
	
	// Initializes with current avatar data
	void Initialize(const FCurrentAvatarState& InitialState);

	// Applies the current state to the Avatar Builder Subsystem
	void ApplyState();

	void GetUndoStack(TArray<FString>& UndoStackRef){UndoStackRef=UndoStack;}

	void GetRedoStack(TArray<FString>& RedoStackRef){RedoStackRef=RedoStack;}
	
	// Saves current state to undo stack
	void SaveState();
	
	void AddToState(FCurrentAvatarState AvatarState);

	FCurrentAvatarState AddToStateTemporary(FCurrentAvatarState AvatarState);
	void PopUndoStack();

	// Undo last change
	void Undo();
	void RemoveStoppedEntryFromStack();

	// Redo last undone change
	void Redo();

	// Updates base mesh based on equipped items
	void UpdateBaseMesh();
	
	FName ApplyUndergarments(UAvatarBuilderSubsystem* AvatarBuilder, const FString& TablePath,
	                         const FString& CategoryName,
	                         USkeletalMeshComponent* TargetComponent);

	void EnsureUndergarments();
	void RemoveUneccesaryUndergarments();

	UDataTable* FindDataTableByCompatibilityItem(FItemCompatibility& ItemCompatibility);
	
	UDataTable* FindDataTableByName(FName ItemCategory);

	FCurrentAvatarState GetCurrentState() {return CurrentState;}

	bool CheckCurrentStateForConflicts(FItemCompatibility& ItemCompatibility, FCurrentAvatarState& TempCurrentState, bool& ResolveInConflictingItem,TArray<FDataTableRowHandle> AllowedExceptions,bool bFinal);
	bool HandleClothingConflicts(FItemCompatibility& ItemCompatibility, FCurrentAvatarState& TempCurrentState,
	                             bool& ResolveInConflictingItem, UAvatarBuilderSubsystem* ,bool bFinal);

	bool HandleAccessoryConflicts(FItemCompatibility& ItemCompatibility, FCurrentAvatarState& TempCurrentState,
	                              bool& ResolveInConflictingItem, UAvatarBuilderSubsystem* AvatarBuilder,bool bFinal);
	bool HandleGenericMeshConflicts(FItemCompatibility& ItemCompatibility, FCurrentAvatarState& TempCurrentState,
	                                bool& ResolveInConflictingItem, UAvatarBuilderSubsystem* AvatarBuilder,bool bFinal);
	void RemoveFromCurrentState(const FCurrentAvatarState& AvatarState);
	
	void PrintCurrentState() const;
	
	void SaveCurrentAvatarData();
	void PrintUndoStack() const;
	
};
