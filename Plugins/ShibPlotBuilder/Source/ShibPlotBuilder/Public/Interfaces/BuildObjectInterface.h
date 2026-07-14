// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "BuildObjectInterface.generated.h"

UINTERFACE(MinimalAPI)
class UBuildObjectInterface : public UInterface
{
	GENERATED_BODY()
};

class SHIBPLOTBUILDER_API IBuildObjectInterface
{
	GENERATED_BODY()

public:
	// Get all sockets without the `off` tag
	UFUNCTION(BlueprintNativeEvent)
	TArray<USceneComponent*> GetAvailableSockets(const TArray<FName>& TagsToCheck);
	
	// Check if `AvailableSocketTags` the given `Tag`
	UFUNCTION(BlueprintNativeEvent)
	bool IsSocketAvailable(const FName& Tag);
	
	// Check if `AvailableChildrenTags` the given `Tag`
	UFUNCTION(BlueprintNativeEvent)
	bool IsChildAvailable(const FName& Tag);
	
	// Check if the object accepts a given tag even if the object has no sockets for it
	UFUNCTION(BlueprintNativeEvent)
	bool IsTagAvailableWithoutSockets(const FName& Tag);
	
	
	// Ask all build actors to block the target actor
	UFUNCTION(BlueprintNativeEvent)
	void InitializeSocketStatus();
	
	// Unblock sockets overlapping the given actor -> TODO: response
	UFUNCTION(BlueprintNativeEvent)
	void UnblockSocketWithActor(const AActor* Actor);
	
	// Unblock sockets overlapping the given actor -> send response back to target to block back
	UFUNCTION(BlueprintNativeEvent)
	void BlockSocketWithActor(AActor* Actor, bool bResponse = false);
	
	// Unblock all sockets
	UFUNCTION(BlueprintNativeEvent)
	void UnblockAllSockets();
	
	
	// Set the material based on the selected material type (ex: wood, stone, metal...)
	UFUNCTION(BlueprintNativeEvent)
	void SetSpawnMaterialType(const FName& MaterialType);
	
	// Set the can/can't build material
	UFUNCTION(BlueprintNativeEvent)
	void UpdatePreviewMaterial(bool bCanBuild);
	
	
	// Get the actors tag (should be equal to it's `RowName` in the data table)
	UFUNCTION(BlueprintNativeEvent)
	FName GetActorTag();
	
	// Unblock all sockets from this actor -> destroy actor if not `bSocketsOnly`
	UFUNCTION(BlueprintNativeEvent)
	void DestroyObject(bool bSocketsOnly);

	UFUNCTION(BlueprintNativeEvent)
	void InteractWithObject();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void InteractWithObjectWithInVillas();

	UFUNCTION(BlueprintNativeEvent)
	void GetSelectedStaticMesh(FName Tag,const TArray<UMaterialInstance*>& PossibleMaterials);

	UFUNCTION(BlueprintNativeEvent)
	void GetSelectedMesh();

	UFUNCTION(BlueprintNativeEvent)
	void ReturnSelectedMeshMaterials(const TArray<UMaterialInstance*>& PossibleMaterials);

	UFUNCTION(BlueprintNativeEvent)
	void UpdateMeshMaterial(UDataTable* ObjectsDataTable, FName RowName, UMaterialInstance* UpdatingMaterial);

	UFUNCTION(BlueprintNativeEvent)
	void OnPostPreviewModeSpawnEvent(AActor* Actor);

	UFUNCTION(BlueprintNativeEvent)
	void OnPreviewModeSpawnEvent(AActor* Actor);

	UFUNCTION(BlueprintNativeEvent)
	void OnSelectedObjectToMove(AActor* Actor);
};
