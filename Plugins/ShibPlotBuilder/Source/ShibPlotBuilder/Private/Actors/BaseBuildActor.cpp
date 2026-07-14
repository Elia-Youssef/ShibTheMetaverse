// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/BaseBuildActor.h"

#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"

ABaseBuildActor::ABaseBuildActor()
{
	PrimaryActorTick.bCanEverTick = false;

	RootScene = CreateDefaultSubobject<USceneComponent>("RootScene");
	SetRootComponent(RootScene);

	ActorMesh = CreateDefaultSubobject<UStaticMeshComponent>("ActorMesh");
	ActorMesh->SetupAttachment(RootScene);
}

void ABaseBuildActor::BeginPlay()
{
	Super::BeginPlay();
	
	IBuildObjectInterface::Execute_InitializeSocketStatus(this);

	// AddOrUpdateDynamicMaterialInstances(ActorMesh);
}

void ABaseBuildActor::AddOrUpdateDynamicMaterialInstances(UStaticMeshComponent* MeshComponent)
{
	if (MeshComponent)
	{
		if (BuildableMeshes.Contains(MeshComponent))
		{
			BuildableMeshes[MeshComponent].Append(CreateDynamicMaterialInstances(MeshComponent));
			UE_LOG(LogTemp, Display, TEXT("MeshComponent %p already exists in BuildableMeshesMap. Appending new dynamic material instances."), MeshComponent);
		}
		else
		{
			BuildableMeshes.Add(MeshComponent, CreateDynamicMaterialInstances(MeshComponent));
			UE_LOG(LogTemp, Display, TEXT("MeshComponent %p added to BuildableMeshesMap with its dynamic material instances."), MeshComponent);
		}
	}
}
void ABaseBuildActor::AddOrUpdateDynamicMaterialInstances(TObjectPtr<UStaticMeshComponent> MeshPtr)
{
	if (MeshPtr)
	{
		UStaticMeshComponent* MeshComponent = Cast<UStaticMeshComponent>(MeshPtr.Get());
		UE_LOG(LogTemp, Display, TEXT("MeshPtr type: %s"), *MeshPtr->GetClass()->GetName());
		if (MeshComponent)
		{
			if (BuildableMeshes.Contains(MeshComponent))
			{
				BuildableMeshes[MeshComponent].Append(CreateDynamicMaterialInstances(MeshComponent));
				UE_LOG(LogTemp, Display, TEXT("MeshComponent %p already exists in BuildableMeshesMap. Appending new dynamic material instances."), MeshComponent);
			}
			else
			{
				BuildableMeshes.Add(MeshComponent, CreateDynamicMaterialInstances(MeshComponent));
				UE_LOG(LogTemp, Display, TEXT("MeshComponent %p added to BuildableMeshesMap with its dynamic material instances."), MeshComponent);
			}
		}
	}
}

TArray<UMaterialInstanceDynamic*> ABaseBuildActor::CreateDynamicMaterialInstances(UStaticMeshComponent* MeshComponent)
{
	TArray<UMaterialInterface*> ObjectMaterials = MeshComponent->GetMaterials();
	for (int32 i = 0; i < ObjectMaterials.Num(); i++) {
		DynamicMaterials.Add(MeshComponent->CreateDynamicMaterialInstance(i, ObjectMaterials[i]));
		UE_LOG(LogTemp, Display, TEXT("Dynamic MateriaL  %p"), DynamicMaterials[i]);
	}
	return DynamicMaterials;
}


TArray<USceneComponent*> ABaseBuildActor::GetAvailableSockets_Implementation(const TArray<FName>& TagsToCheck)
{
	TArray<USceneComponent*> Components;

	for (UActorComponent* Component : GetComponents())
	{
		if (Component->ComponentHasTag(FName("off"))) continue;
		
		for (FName Tag : TagsToCheck)
		{
			if (Component->ComponentHasTag(Tag))
			{
				Components.Add(Cast<USceneComponent>(Component));
				break;
			}
		}
	}
	
	return Components;
}

bool ABaseBuildActor::IsSocketAvailable_Implementation(const FName& Tag)
{
	return AvailableSocketTags.Contains(Tag);
}

bool ABaseBuildActor::IsChildAvailable_Implementation(const FName& Tag)
{
	return AvailableChildrenTags.Contains(Tag);
}

bool ABaseBuildActor::IsTagAvailableWithoutSockets_Implementation(const FName& Tag)
{
	return AvailableTagsWithoutSockets.Contains(Tag);
}

void ABaseBuildActor::InitializeSocketStatus_Implementation()
{
	TArray<AActor*> Actors;
	UGameplayStatics::GetAllActorsWithInterface(GetWorld(), UBuildObjectInterface::StaticClass(), Actors);
	for (AActor* Actor : Actors)
	{
		IBuildObjectInterface::Execute_BlockSocketWithActor(Actor, this, false);
	}
}

void ABaseBuildActor::UnblockSocketWithActor_Implementation(const AActor* Actor)
{
	TArray<USceneComponent*> Components;
	GetComponents<USceneComponent>(Components);
	for (USceneComponent* Component : Components)
	{
		if (UKismetMathLibrary::EqualEqual_VectorVector(Component->GetComponentLocation(), Actor->GetActorLocation(), 3.7f))
		{
			Component->ComponentTags.Remove(FName("off"));
			
			return;
		}
	}
}

void ABaseBuildActor::BlockSocketWithActor_Implementation(AActor* Actor, bool bResponse)
{
	TArray<USceneComponent*> Components;
	GetComponents<USceneComponent>(Components);
	for (USceneComponent* Component : Components)
	{
		if (UKismetMathLibrary::EqualEqual_VectorVector(Component->GetComponentLocation(), Actor->GetActorLocation(), 3.7f))
		{
			Component->ComponentTags.AddUnique(FName("off"));

			if (bResponse) return;
			IBuildObjectInterface::Execute_BlockSocketWithActor(Actor, this, true);
			
			return;
		}
	}
}

void ABaseBuildActor::UnblockAllSockets_Implementation()
{
	TArray<USceneComponent*> Components;
	GetComponents<USceneComponent>(Components);
	for (USceneComponent* Component : Components)
	{
		Component->ComponentTags.Remove(FName("off"));
	}
}

void ABaseBuildActor::SetSpawnMaterialType_Implementation(const FName& MaterialType)
{
	// blueprint
}

void ABaseBuildActor::UpdatePreviewMaterial_Implementation(bool bCanBuild)
{
	// blueprint
}

void ABaseBuildActor::OnSelectedObjectToMove_Implementation(AActor* Actor)
{
	//IBuildObjectInterface::OnSelectedObjectToMove_Implementation(Actor);
}

FName ABaseBuildActor::GetActorTag_Implementation()
{
	return ActorTag;
}

void ABaseBuildActor::DestroyObject_Implementation(bool bSocketsOnly)
{
	GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, FString::Printf(TEXT("Trying to destroy actor")));
	TArray<AActor*> Actors;
	UGameplayStatics::GetAllActorsWithInterface(GetWorld(), UBuildObjectInterface::StaticClass(), Actors);
	for (AActor* Actor : Actors)
	{
		IBuildObjectInterface::Execute_UnblockSocketWithActor(Actor, this);
	}

	if (!bSocketsOnly) Destroy();
}

void ABaseBuildActor::GetSelectedStaticMesh_Implementation(FName Tag,const TArray<UMaterialInstance*>& PossibleMaterials)
{
	
}

