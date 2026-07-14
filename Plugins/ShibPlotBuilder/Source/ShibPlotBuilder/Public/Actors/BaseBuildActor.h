// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interfaces/BuildObjectInterface.h"
#include "BaseBuildActor.generated.h"

UCLASS()
class SHIBPLOTBUILDER_API ABaseBuildActor : public AActor, public IBuildObjectInterface
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;
	
public:	
	ABaseBuildActor();

	FName UniqueId;
	
	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = "Components")
	TObjectPtr<USceneComponent> RootScene;
	
	UPROPERTY(BlueprintReadWrite)
	TArray<UMaterialInstanceDynamic*> DynamicMaterials;

	TMap<UStaticMeshComponent*, TArray<UMaterialInstanceDynamic*>> BuildableMeshes;
	TMap<UStaticMeshComponent*, TArray<UMaterialInstanceDynamic*>>& GetBuildableMeshes()
	{
		return BuildableMeshes;
	}

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UStaticMeshComponent> ActorMesh;

	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	FText Title;

	// This actor's row name in the data table
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FName ActorTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TArray<FName> AvailableSocketTags;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TArray<FName> AvailableTagsWithoutSockets;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TArray<FName> AvailableChildrenTags;
	
	virtual TArray<UMaterialInstanceDynamic*> CreateDynamicMaterialInstances(UStaticMeshComponent* MeshComponent);
	virtual void AddOrUpdateDynamicMaterialInstances(UStaticMeshComponent* MeshComponent);
	virtual void AddOrUpdateDynamicMaterialInstances(TObjectPtr<UStaticMeshComponent> MeshComponent);

	// IBuildObjectInterface
	virtual TArray<USceneComponent*> GetAvailableSockets_Implementation(const TArray<FName>& TagsToCheck) override;
	virtual bool IsSocketAvailable_Implementation(const FName& Tag) override;
	virtual bool IsChildAvailable_Implementation(const FName& Tag) override;
	virtual bool IsTagAvailableWithoutSockets_Implementation(const FName& Tag) override;
	
	virtual void InitializeSocketStatus_Implementation() override;
	virtual void UnblockSocketWithActor_Implementation(const AActor* Actor) override;
	virtual void BlockSocketWithActor_Implementation(AActor* Actor, bool bResponse = false) override;
	virtual void UnblockAllSockets_Implementation() override;
	
	virtual void SetSpawnMaterialType_Implementation(const FName& MaterialType) override;
	virtual void UpdatePreviewMaterial_Implementation(bool bCanBuild) override;

	virtual void OnSelectedObjectToMove_Implementation(AActor* Actor) override;
	
	virtual FName GetActorTag_Implementation() override;
	virtual void DestroyObject_Implementation(bool bSocketsOnly) override;
	
	virtual void GetSelectedStaticMesh_Implementation(FName Tag,const TArray<UMaterialInstance*>& PossibleMaterials) override;
	
};
