// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/BuildingComponent.h"
#include "Actors/BaseBuildActor.h"
#include "Engine/DataTable.h"
#include "Interfaces/BuildObjectInterface.h"
#include "Kismet/KismetMathLibrary.h"
#include "Player/BuildCharacter.h"
#include "Player/BuildController.h"
#include "Tools/BuildFunctionLibrary.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"
#include "Engine/OverlapResult.h"


UBuildingComponent::UBuildingComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

}

void UBuildingComponent::Initialize(ABuildCharacter* Character, ABuildController* Controller)
{
	BuildCharacter = Character;
	BuildController = Controller;
	UGameplayStatics::PlaySound2D(GetWorld(), Controller->Music);
}

void UBuildingComponent::BeginPlay()
{
	Super::BeginPlay();

	AllowedToBuildSize = FVector2D{LandSize.X - NotAllowedBorderSize, LandSize.Y - NotAllowedBorderSize};

	CustomTickDelegate.BindUFunction(this, FName("CustomTick"));
	GetWorld()->GetTimerManager().SetTimer(CustomTickHandler, CustomTickDelegate, 1/FMath::Max(0.01, TicksPerSecond), true);
}

void UBuildingComponent::CustomTick()
{
	if (BuildCharacter == nullptr) return;
	if (InteractionMode == EInteractionMode::Build)
	{
		GetTraceHit();
		//GEngine->AddOnScreenDebugMessage(-1, 5, FColor, ''');
		if (PreviewActor)
		{
			UpdatePreviewLocation();
			UpdatePreviewStatus();
		}
	}
}

void UBuildingComponent::SetSelectedActorToSpawn(const FName& Tag)
{
	const FObjectDetails* Row = ObjectsDataTable->FindRow<FObjectDetails>(Tag, "");
	if (!Row) return;
	
	SelectedActorToSpawn = *Row;
	UpdatePreviewActor();
}

void UBuildingComponent::OnInteractionModeChange(EInteractionMode NewInteractionMode)
{
	InteractionMode = NewInteractionMode;
	if (InteractionMode == EInteractionMode::Build)
	{
		UpdatePreviewActor();
	}
	else
	{
		SelectedMaterial = FMaterialType();
		SelectedActorToSpawn = FObjectDetails();
		if (PreviewActor) PreviewActor->Destroy();
		PreviewActor = nullptr;
	}
}

void UBuildingComponent::GetTraceHit()
{
	if (!PreviewActor)
	{
		return;
	}

	const FVector Start = BuildCharacter->GetActorLocation();
	const FVector End = Start + BuildCharacter->GetActorForwardVector() * 50'000.f;

	// Perform the initial trace with simple collision
	FHitResult HitResult = PerformLineTrace(Start, End, true);
	
	// Save the result for the ceiling
	TraceHitResult = HitResult;
}

FHitResult UBuildingComponent::PerformLineTrace(const FVector& Start, const FVector& End, bool bTraceComplex)
{
	FCollisionQueryParams QueryParams;
	QueryParams.bTraceComplex = bTraceComplex;
	QueryParams.AddIgnoredActor(BuildController->BuildCharacter);
	QueryParams.AddIgnoredActor(PreviewActor);

	// Ignore child actors of the PreviewActor
	TArray<AActor*> ChildActorsOfPreview;
	PreviewActor->GetAllChildActors(ChildActorsOfPreview, true);
	QueryParams.AddIgnoredActors(ChildActorsOfPreview);

	FHitResult HitResult;
	GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECollisionChannel::ECC_Visibility, QueryParams);

	return HitResult;
}


void UBuildingComponent::UpdatePreviewActor()
 {
	if (PreviewActor) PreviewActor->Destroy();

	PreviewActor = GetWorld()->SpawnActorDeferred<ABaseBuildActor>(SelectedActorToSpawn.ClassToSpawn, FTransform());

	if (!PreviewActor){return;}
	PreviewActor->FinishSpawning(FTransform());

	//GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, FString::Printf(TEXT("New Spawn")));
	LastManualRotation=FRotator::ZeroRotator;

	FVector Temp=FVector().ZeroVector;
	PreviewActor->GetActorBounds(true,Temp,PreviewBoxExtent,true);
	
	PreviewActor->Title=SelectedActorToSpawn.Title;
	
	IBuildObjectInterface::Execute_OnPreviewModeSpawnEvent(PreviewActor, PreviewActor);

	if(PreviewActor->ActorMesh->HasValidNaniteData())
		PreviewActor->ActorMesh->SetForceDisableNanite(true);
}

void UBuildingComponent::UpdatePreviewLocation()
{
	FTransform NewTransform;
	
	if (const USceneComponent* Socket = GetHitActorSocket())
	{
		NewTransform = Socket->GetComponentTransform();
	}
	else
	{
		bIsWall = FMath::Abs(TraceHitResult.ImpactNormal.Z) < 0.5f; // Narrow wall condition
		bIsCeiling = !bIsWall && TraceHitResult.ImpactNormal.Z < -0.5f;
		bIsFloor = !bIsWall && TraceHitResult.ImpactNormal.Z > 0.5f;

		/*if (GEngine)
		{
			FString Message = bIsCeiling 
				? FString::Printf(TEXT("Ceiling: %f"), TraceHitResult.ImpactNormal.Z) 
				: FString::Printf(TEXT("Not Ceiling: %f"), TraceHitResult.ImpactNormal.Z);
			FColor MessageColor = bIsCeiling ? FColor::Yellow : FColor::Blue;
			GEngine->AddOnScreenDebugMessage(-1, 1.0f, MessageColor, Message);

			UE_LOG(LogTemp, Log, TEXT( "  : %f"),TraceHitResult.ImpactNormal.Z);
		}*/
		
		FRotator TargetRotation;
		
		if (bIsWall)
		{
			FVector Forward = -TraceHitResult.ImpactNormal;
			FRotator BaseRotation = Forward.Rotation();
			BaseRotation.Pitch += 90.0f;
			TargetRotation=BaseRotation;
		}
		else if (bIsFloor)
		{
			TargetRotation = FRotator::ZeroRotator;
		}
		else if (bIsCeiling)
		{
			TargetRotation=FRotator(180,0,0);
		}
		
		NewTransform = UBuildFunctionLibrary::SnapTransformToGrid(FTransform{
			TargetRotation,
			TraceHitResult.bBlockingHit ? TraceHitResult.ImpactPoint : TraceHitResult.TraceEnd
		});
		// UE_LOG(LogTemp, Display, TEXT("Normal rotation value %s"), *TraceHitResult.ImpactNormal.Rotation().ToString());
	}

	if (!IsLocationAllowed(NewTransform.GetLocation())) return;
	PreviewActor->SetActorTransform(NewTransform);

	// Check the distance and direction between the impact point and the center of the mesh
	FVector MeshCenter = PreviewActor->GetActorLocation();
	FVector ImpactPoint = TraceHitResult.ImpactPoint;
	FVector Offset = ImpactPoint - MeshCenter;

	// Adjust the actor's location to fix the offset
	NewTransform.AddToTranslation(Offset);
	PreviewActor->SetActorTransform(NewTransform);
	PreviewActor->GetRootComponent()->AddLocalRotation(LastManualRotation);
}

void UBuildingComponent::UpdatePreviewStatus()
{
	if (PreviewActor)
	{
		IBuildObjectInterface::Execute_UpdatePreviewMaterial(PreviewActor, CanSpawnActor());
	}
}

void UBuildingComponent::StartPreviewRotation(bool bRotateLeft)
{
	if (!PreviewActor) return;
    
	FTimerDelegate PreviewRotationTimerDelegate;
	PreviewRotationTimerDelegate.BindLambda([this, bRotateLeft]() {
		float RotationAmount = BuildController->bLeftShiftDown ? 1.f : 5.f;
		
		PreviewActor->GetRootComponent()->AddLocalRotation(FRotator(0.f, bRotateLeft ? -RotationAmount : RotationAmount, 0.f));
		
		LastManualRotation +=  (FRotator(0.f, bRotateLeft ? -RotationAmount : RotationAmount, 0.f));
		bHasAppliedRotation = true;
	});
	GetWorld()->GetTimerManager().SetTimer(PreviewRotationTimerHandle, PreviewRotationTimerDelegate, 0.1f, true);
	bIsPreviewRotating = true;
	
}

void UBuildingComponent::EndPreviewRotation()
{
	GetWorld()->GetTimerManager().ClearTimer(PreviewRotationTimerHandle);
	bIsPreviewRotating = false;
}

void UBuildingComponent::ResetPreviewRotation()
{
	if (bIsPreviewRotating)
	{
		EndPreviewRotation();
		PreviewActor->SetActorRotation(FRotator());
		bIsPreviewRotating = false;
		LastManualRotation = FRotator::ZeroRotator;
		bHasAppliedRotation = false;
	}
}

USceneComponent* UBuildingComponent::GetHitActorSocket()
{
	if (CheckHitTargetHasSockets())
	{
		TArray<USceneComponent*> Sockets = IBuildObjectInterface::Execute_GetAvailableSockets(
			TraceHitResult.GetActor(), TArray{
				SelectedActorToSpawn.ObjectType.Tag, SelectedActorToSpawn.ObjectType.ParentTag
			});
		return FindClosestSocket(Sockets);
	}

	return nullptr;
}

USceneComponent* UBuildingComponent::FindClosestSocket(TArray<USceneComponent*> Sockets)
{
	float ShortestDistance = -1;
	USceneComponent* ClosestSocket = nullptr;
	for (USceneComponent* Socket : Sockets)
	{
		float Distance = UKismetMathLibrary::Vector_Distance(Socket->GetComponentLocation(), TraceHitResult.ImpactPoint);
		
		if (ClosestSocket == nullptr) // first loop
		{
			ShortestDistance = Distance;
			ClosestSocket = Socket;
		} else if (Distance < ShortestDistance)
		{
			ShortestDistance = Distance;
			ClosestSocket = Socket;
		}
	}

	return ClosestSocket;
}

bool UBuildingComponent::CheckHitTargetAcceptsTag(bool bIgnoreParent)
{
	AActor* HitActor = TraceHitResult.GetActor();

	if (HitActor)
	{
		UClass* ActorClass = HitActor->GetClass();
		if (ActorClass)
		{
			UClass* ParentClass = ActorClass->GetSuperClass();
			/*GEngine->AddOnScreenDebugMessage(2, 1.0f,FColor::Red , ParentClass->GetName());*/
			if (ParentClass && ParentClass->GetName().Contains(TEXT("Ceiling")))
			{
				// Return true only if IsCeiling is true
				if (!bIsCeiling)
				{
					return false;
				}
			}
			if (ParentClass && (ParentClass->GetName().Contains(TEXT("Floor")) ))
			{
				// Return true only if IsFloor is true
				if (!bIsFloor)
				{
					return false;
				}
			}
			if (ParentClass && (ParentClass->GetName().Contains(TEXT("Wall"))) )
			{
				// Return true only if IsWall is true
				if (!bIsWall)
				{
					return false;
				}
			}


			// Checks if Cabinet Light is placed on the Cabinet Floor or Ceiling and if it does, returns false.
			if (PreviewActor.GetClass()->GetName().Contains("CabinetLight"))
			{
				FString ParentClassName = ParentClass->GetName();
				FString CabinetName = TEXT("BaseCabinet");

				if (ParentClassName.Contains(CabinetName))
				{
					if (bIsFloor || bIsCeiling)
					{
						return false;
					}
					else
					{
						// Proceed with the original logic
						return IBuildObjectInterface::Execute_IsTagAvailableWithoutSockets(
								   HitActor, SelectedActorToSpawn.ObjectType.Tag) || 
							   (!bIgnoreParent && IBuildObjectInterface::Execute_IsTagAvailableWithoutSockets(
								   HitActor, SelectedActorToSpawn.ObjectType.ParentTag));
					}
				}
				
			}

			
			// Loops through all actors on which preview actor cannot be placed sideways.
			if (ParentClass)
			{
				FString ParentClassName = ParentClass->GetName();
				TArray<FString> AllowedNames = { TEXT("BaseShelf"), TEXT("BaseTable"), TEXT("BaseCabinet"), TEXT("SingleSeat") };

				for (const FString& Name : AllowedNames)
				{
					if (ParentClassName.Contains(Name))
					{
						if (bIsWall || bIsCeiling)
						{
							return false;
						}
					}
				}
			}
			
			
		}
			
	}
	

	// Proceed with the original logic
	return IBuildObjectInterface::Execute_IsTagAvailableWithoutSockets(
			   HitActor, SelectedActorToSpawn.ObjectType.Tag) || 
		   (!bIgnoreParent && IBuildObjectInterface::Execute_IsTagAvailableWithoutSockets(
			   HitActor, SelectedActorToSpawn.ObjectType.ParentTag));
}

bool UBuildingComponent::CheckHitTargetHasSockets()
{
	return CheckHitActorImplementsInterface() && (
		IBuildObjectInterface::Execute_IsSocketAvailable(TraceHitResult.GetActor(), SelectedActorToSpawn.ObjectType.Tag)
		|| IBuildObjectInterface::Execute_IsSocketAvailable(TraceHitResult.GetActor(),
		                                                    SelectedActorToSpawn.ObjectType.ParentTag));
}

bool UBuildingComponent::CheckHitActorImplementsInterface()
{
	if (!TraceHitResult.bBlockingHit || !TraceHitResult.GetActor()) return false;
	return TraceHitResult.GetActor()->Implements<UBuildObjectInterface>();
}


bool UBuildingComponent::CanSpawnActor()
{
	
    if (bIsClipping)
    	{
    		return false;
    	}

	if (TraceHitResult.GetActor()->GetClass()->GetName().Contains("PlotBase"))
	{
		if (SelectedActorToSpawn.FurnitureSnapLocation.Find(EFurnitureSnap::OutsideGround) > 0)
		{
			return true;
		}
	}
		
	if (CheckHitActorImplementsInterface()) {
		const bool bHitTargetHasAvailableSockets =  CheckHitTargetHasSockets() && !IBuildObjectInterface::Execute_GetAvailableSockets(
			TraceHitResult.GetActor(), TArray{
				SelectedActorToSpawn.ObjectType.Tag, SelectedActorToSpawn.ObjectType.ParentTag
			}).IsEmpty();
			
		if (SelectedActorToSpawn.bNeedsSocket) return bHitTargetHasAvailableSockets;
			
		return CheckHitTargetAcceptsTag() || bHitTargetHasAvailableSockets;
	}
	else
	{
		if(SelectedActorToSpawn.bIsHouseOrVilla)
		{
			return true;
		}
		else
		{
			return false;
		}
	}
}


void UBuildingComponent::DestroyHitActor()
{
	if (!CheckHitActorImplementsInterface()) return;
	
	TArray<FObjectBuiltData> NewObjectsBuilt;
	
	for (FObjectBuiltData Object : ObjectsBuilt)
	{
		// if (TraceHitResult.GetActor()->GetName() != Object.ID.ToString() && TraceHitResult.GetActor()->GetName() != Object.Parent.ToString())
		if (auto* BuildActor = Cast<ABaseBuildActor>(TraceHitResult.GetActor()))
		{
			if (BuildActor->UniqueId != Object.ID.ToString() && BuildActor->UniqueId != Object.Parent.ToString())
			{
				NewObjectsBuilt.Add(Object);
			}
			else
			{
				BuildController->PlotBuilderHistory->Add(Object);
				BuildController->LastInteractionState = EObjectInteractionState::Delete;
			}
		}
	}
	ObjectsBuilt = NewObjectsBuilt;
	IBuildObjectInterface::Execute_DestroyObject(TraceHitResult.GetActor(), false);
}

void UBuildingComponent::DestroyHitActor(FObjectBuiltData& ObjectDetails)
{
	TArray<FObjectBuiltData> NewObjectsBuilt;
	
	for (FObjectBuiltData Object : ObjectsBuilt)
	{
		if (ObjectDetails.ID != Object.ID.ToString() && ObjectDetails.ID != Object.Parent.ToString())
		{
			NewObjectsBuilt.Add(Object);
		}
	}
	ObjectsBuilt = NewObjectsBuilt;
	// BuildController->PlotBuilderHistory->Add(ObjectDetails);
	IBuildObjectInterface::Execute_DestroyObject(ObjectDetails.BuiltActor, false);
}

void UBuildingComponent::DestroyHitActor(AActor* ClickActor)
{
	TArray<FObjectBuiltData> NewObjectsBuilt;
	
	for (FObjectBuiltData Object : ObjectsBuilt)
	{
		// if (TraceHitResult.GetActor()->GetName() != Object.ID.ToString() && TraceHitResult.GetActor()->GetName() != Object.Parent.ToString())
		if (auto* BuildActor = Cast<ABaseBuildActor>(ClickActor))
		{
			if (BuildActor->UniqueId != Object.ID.ToString() && BuildActor->UniqueId != Object.Parent.ToString())
			{
				NewObjectsBuilt.Add(Object);
			}
			else
			{
				BuildController->PlotBuilderHistory->Add(Object);
				BuildController->LastInteractionState = EObjectInteractionState::Delete;
			}
		}
	}
	ObjectsBuilt = NewObjectsBuilt;
	IBuildObjectInterface::Execute_DestroyObject(ClickActor, false);
	BuildController->ActorToDelete = nullptr;
}


void UBuildingComponent::MoveHitActor()
{
	//GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, FString::Printf(TEXT("In Move Hit Actor")));

	if(PreviewActor)
	{
		PreviewActor->Destroy();
		PreviewActor=nullptr;	
	}
	AActor* CurrentHitActor;
	if (!IsValid(BuildControllerRef->SelectedHitActor) )
	{
		UE_LOG(LogTemp, Warning, TEXT("Invalid SelectedHitActor"));

		if (!IsValid(BuildControllerRef->SelectedHitActorParent) )
		{
			UE_LOG(LogTemp, Warning, TEXT("Invalid SelectedHitActor Parent"));
			return;
		}
		else
		{
			CurrentHitActor=BuildControllerRef->SelectedHitActorParent;
		}
		
	}
	else
	{
		CurrentHitActor=BuildControllerRef->SelectedHitActor;
	}
	//GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, FString::Printf(TEXT("Implements interface")));
	
	bMovingObject = true;
	
	if (PreviewActor)
	{
		PreviewActor->Destroy();
		OnInteractionModeChange(EInteractionMode::None);
	}
	

	ABaseBuildActor * PActor=Cast<ABaseBuildActor>(CurrentHitActor);
	if (!IsValid(PActor))
	{
		UE_LOG(LogTemp, Warning, TEXT("Invalid PActor"));
		return;
	}

	if (!PActor->Implements<UBuildObjectInterface>())
	{
		UE_LOG(LogTemp, Warning, TEXT("Actor Doesnt Implement Interface"));
		return;
	}

	const FName PreviewActorTitle = FName(*PActor->Title.ToString());
	IBuildObjectInterface::Execute_DestroyObject(PActor, true);
	IBuildObjectInterface::Execute_UnblockAllSockets(PActor);
	bool destroyed= PActor->Destroy();
	
	const FObjectDetails* Row = ObjectsDataTable->FindRow<FObjectDetails>(PreviewActorTitle, FString());
	

	SelectedActorToSpawn = *Row;
	
	UpdatePreviewActor();
	OnInteractionModeChange(EInteractionMode::Build);
	BuildController->DisplayMouseOnClassSpawnClick();
	//IBuildObjectInterface::Execute_OnSelectedObjectToMove(this, PreviewActor);
}

void UBuildingComponent::ReleaseHitActor()
{
	bMovingObject = false;
	/*
	IBuildObjectInterface::Execute_SetSpawnMaterialType(PreviewActor, SelectedMaterial.Type);
	IBuildObjectInterface::Execute_InitializeSocketStatus(PreviewActor);
	
	UpdateBuiltObjectSaveTransform(PreviewActor);
	
	PreviewActor = nullptr;
	UpdatePreviewActor();*/
}

void UBuildingComponent::UpdateBuiltObjectSaveTransform(const AActor* Actor)
{
	for (FObjectBuiltData& Object : ObjectsBuilt)
	{
		auto* BuildActor = Cast<ABaseBuildActor>(Actor);
		if (BuildActor == nullptr) continue;
		if (Object.ID.ToString() != BuildActor->UniqueId) continue;

		Object.Transform = Actor->GetActorTransform();
	}

	TArray<AActor*> ChildActors;
	Actor->GetAllChildActors(ChildActors, true);
	for (const AActor* Child : ChildActors)
	{
		UpdateBuiltObjectSaveTransform(Child);
	}
}

bool UBuildingComponent::IsLocationAllowed(const FVector& Location) const
{
	const bool bX = Location.X < AllowedToBuildSize.X / 2 && Location.X > -1 * AllowedToBuildSize.X / 2;
	const bool bY = Location.Y < AllowedToBuildSize.Y / 2 && Location.Y > -1 * AllowedToBuildSize.Y / 2;

	return bX && bY;
}

void UBuildingComponent::LoadObjectsBuilt()
{
	TMap<FName, ABaseBuildActor*> SpawnedActors;
	TArray<FObjectBuiltData> ObjectsBuiltCopy;
	ObjectsBuiltCopy.Append(ObjectsBuilt);
	ObjectsBuilt.Empty();

	// Log the number of objects to load
	UE_LOG(LogTemp, Display, TEXT("Loading %d objects built."), ObjectsBuiltCopy.Num());

	for (FObjectBuiltData Object : ObjectsBuiltCopy)
	{
		if (Object.Parent.IsNone())
		{
			FName PreviousSessionUniqueId = Object.ID;
			ABaseBuildActor* SpawnedActor = SpawnActor(Object);
			SpawnedActors.Add(PreviousSessionUniqueId, SpawnedActor);

			// Log successful spawn of a top-level object
			UE_LOG(LogTemp, Display, TEXT("Spawned top-level object: %s (ID: %s)"), *Object.ID.ToString(), *PreviousSessionUniqueId.ToString());
		}
		else
		{
			if (SpawnedActors.Find(Object.Parent))
			{
				ABaseBuildActor* ParentActor = *SpawnedActors.Find(FName(Object.Parent));
				SpawnActorAsChild(ParentActor, Object);

				// Log successful spawn of a child object
				UE_LOG(LogTemp, Display, TEXT("Spawned child object: %s (ID: %s) as child of %s"), 
											   *Object.ID.ToString(),
											   *ParentActor->GetName(), *Object.Parent.ToString());
			}
			else
			{
				// Log a warning if a child object's parent cannot be found
				UE_LOG(LogTemp, Error, TEXT("Spawned child object: %s (ID: %s) as child of %s (ID: %s)"), 
											   *Object.ID.ToString(), *Object.ID.ToString(),
											   *Object.Parent.ToString(), *Object.Parent.ToString());
			}
		}
	}
}

void UBuildingComponent::SpawnSelectedActorFromPreview()
{
	if (PreviewActor == nullptr || !CanSpawnActor()) return;
	
	FObjectBuiltData ObjectBuiltData;
	ObjectBuiltData.ClassToSpawnRowName = GetRowNameByClass(SelectedActorToSpawn.ClassToSpawn);
	ObjectBuiltData.Transform = PreviewActor->GetActorTransform();
	ObjectBuiltData.Mesh = PreviewActor->ActorMesh->GetStaticMesh();
	ObjectBuiltData.MaterialType = SelectedMaterial.Type;
	ObjectBuiltData.Title=FName(*PreviewActor->Title.ToString());
	ObjectBuiltData.bIsHouseOrVilla = SelectedActorToSpawn.bIsHouseOrVilla;
	
	if (CheckHitActorImplementsInterface())
	{
		if (IBuildObjectInterface::Execute_IsChildAvailable(TraceHitResult.GetActor(), SelectedActorToSpawn.ObjectType.Tag))
			SpawnActorAsChild(TraceHitResult.GetActor(), ObjectBuiltData);
		else SpawnActor(ObjectBuiltData);
	}
	else
	{SpawnActor(ObjectBuiltData);}
	
	LastManualRotation = FRotator::ZeroRotator;
	bHasAppliedRotation = false;
}

void UBuildingComponent::SpawnActorAsChild(AActor* Parent, FObjectBuiltData& ObjectToBuild)
{
	if (Parent == nullptr || (ObjectToBuild.ClassToSpawnRowName.IsNone() &&  ObjectToBuild.ClassToSpawnRowName.IsValid()))
	{
		UE_LOG(LogTemp, Error, TEXT("Invalid parameters in SpawnActorAsChild: Parent or ClassToSpawn is null"));
		return;
	}

	UActorComponent* Component = Parent->AddComponentByClass(UChildActorComponent::StaticClass(), false, UKismetMathLibrary::MakeRelativeTransform(ObjectToBuild.Transform, Parent->GetActorTransform()), false);

	if (Component == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to add ChildActorComponent to Parent"));
		return;
	}

	UChildActorComponent* ChildComponent = Cast<UChildActorComponent>(Component);
	if (ChildComponent == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to cast Component to UChildActorComponent"));
		return;
	}
	// UActorComponent* Component = Parent->AddComponentByClass(UChildActorComponent::StaticClass(), false, UKismetMathLibrary::MakeRelativeTransform(ObjectToBuild.Transform, Parent->GetActorTransform()), false);
	// UChildActorComponent* ChildComponent = Cast<UChildActorComponent>(Component);
	
	// ChildComponent->SetChildActorClass(ObjectToBuild.ClassToSpawn);
	
	ChildComponent->SetChildActorClass(GetRowByName(ObjectToBuild.ClassToSpawnRowName)->ClassToSpawn);

	
	if(auto* ChildBuildActor = Cast<ABaseBuildActor>(ChildComponent->GetChildActor()))
	{
		ChildBuildActor->UniqueId = FName(FGuid::NewGuid().ToString());
		ObjectToBuild.ID = FName(ChildBuildActor->UniqueId); //ChildComponent->GetName()
	}
	
	if(auto* ParentBuildActor = Cast<ABaseBuildActor>(Parent))
	{
		ObjectToBuild.Parent = FName(ParentBuildActor->UniqueId);
	}

	ObjectsBuilt.Add(ObjectToBuild);
}

ABaseBuildActor* UBuildingComponent::SpawnActor(FObjectBuiltData& ObjectToBuild, bool bAddTohHistory)
{
	// ABaseBuildActor* ActorToSpawn = GetWorld()->SpawnActorDeferred<ABaseBuildActor>(ObjectToBuild.ClassToSpawn, ObjectToBuild.Transform);
	ABaseBuildActor* ActorToSpawn = GetWorld()->SpawnActorDeferred<ABaseBuildActor>(GetRowByName(ObjectToBuild.ClassToSpawnRowName)->ClassToSpawn, ObjectToBuild.Transform);
	ActorToSpawn->FinishSpawning(ObjectToBuild.Transform);
	ActorToSpawn->UniqueId = FName(FGuid::NewGuid().ToString());
	if (IsValid(PreviewActor) && !PreviewActor->Title.IsEmpty())
	{
		ActorToSpawn->Title=PreviewActor->Title;
	}
	IBuildObjectInterface::Execute_InitializeSocketStatus(ActorToSpawn);
	IBuildObjectInterface::Execute_SetSpawnMaterialType(ActorToSpawn, ObjectToBuild.MaterialType);
	ObjectToBuild.ID = ActorToSpawn->UniqueId;
	//FName(ActorToSpawn->GetName());
	ObjectToBuild.BuiltActor = ActorToSpawn;
	
	if(bAddTohHistory)
	{
		BuildController->LastInteractionState = EObjectInteractionState::Spawn;
		BuildController->PlotBuilderHistory->Add(ObjectToBuild);
	}

	if (ObjectToBuild.bIsHouseOrVilla)
	{
		OnInteractionModeChange(EInteractionMode::View);
		ObjectToBuild.ActorParentReference = nullptr;
		OnInteractionModeChange(EInteractionMode::Build);
	}
	else
	{
		if (AActor* HitActor = TraceHitResult.GetActor())
		{
			if (AActor* ParentActor = HitActor->GetParentActor())
			{
				if (UClass* SuperClass = ParentActor->GetClass()->GetSuperClass())
				{
					FName ParentClassName = FName(*SuperClass->GetName());
            
					if (ParentClassName == "BP_Villas_C" || ParentClassName == "BP_MainHouse_C")
					{
						UE_LOG(LogTemp, Warning, TEXT("Hit Actor Parent Class: %s"), *SuperClass->GetName());
						ObjectToBuild.ActorParentReference = HitActor->GetParentActor();
					}
				}
			}
		}
	}
	ObjectsBuilt.Add(ObjectToBuild);
	IBuildObjectInterface::Execute_OnPostPreviewModeSpawnEvent(ActorToSpawn, ActorToSpawn);
	
	return ActorToSpawn;
}

void UBuildingComponent::InteractWithObject()
{
	if (auto* InteractingObject = Cast<ABaseBuildActor>(TraceHitResult.GetActor()); InteractingObject != nullptr)
	{
		const FName InteractingObjectActorTag = IBuildObjectInterface::Execute_GetActorTag(InteractingObject);
		if (const FObjectDetails* Row = ObjectsDataTable->FindRow<FObjectDetails>(InteractingObjectActorTag, FString()); Row->bInteractable)
		{
			IBuildObjectInterface::Execute_InteractWithObject(InteractingObject);
		}
	}
	
}

void UBuildingComponent::SpawnActorFromRedo(FObjectBuiltData& ObjectToBuild)
{
	// UE_LOG(LogTemp, Log, TEXT("Restored object: %s"), *ObjectToBuild.BuiltActor->GetActorLabel());
	BuildController->LastInteractionState = EObjectInteractionState::Redo;
	SpawnActor(ObjectToBuild, false);
}
void UBuildingComponent::DeSpawnActorFromUndo(FObjectBuiltData& ObjectToBuild)
{
	// UE_LOG(LogTemp, Log, TEXT("Undo object: %s"), *ObjectToBuild.BuiltActor->GetActorLabel());
	BuildController->LastInteractionState = EObjectInteractionState::Undo;
	DestroyHitActor(ObjectToBuild);
}

int32 UBuildingComponent::GetRowNumberByClass(TSubclassOf<ABaseBuildActor> ClassToSpawn)
{
	UDataTable* DataTable = ObjectsDataTable;
	
	if (!DataTable || !ClassToSpawn)
		return -1;

	const TMap<FName, uint8*>& RowMap = DataTable->GetRowMap();
	int32 RowNumber = 0;

	for (const auto& Row : RowMap)
	{
		FObjectDetails* ObjectDetails = reinterpret_cast<FObjectDetails*>(Row.Value);
		if (ObjectDetails && ObjectDetails->ClassToSpawn == ClassToSpawn)
		{
			return RowNumber;
		}
		RowNumber++;
	}

	return -1;
}

FName UBuildingComponent::GetRowNameByClass(TSubclassOf<ABaseBuildActor> ClassToSpawn)
{
	UDataTable* DataTable = ObjectsDataTable;
	
	if (!DataTable || !ClassToSpawn)
		return NAME_None;

	for (const auto& Row : DataTable->GetRowMap())
	{
		FObjectDetails* ObjectDetails = reinterpret_cast<FObjectDetails*>(Row.Value);
		if (ObjectDetails && ObjectDetails->ClassToSpawn == ClassToSpawn)
		{
			return Row.Key;
		}
	}
	return NAME_None;
}

int32 UBuildingComponent::FindRowIndexByClass(TSubclassOf<ABaseBuildActor> ClassToSpawn)
{
	UDataTable* DataTable = ObjectsDataTable;
	
	if (!DataTable || !ClassToSpawn)
		return -1;

	TArray<FObjectDetails*> AllRows;
	DataTable->GetAllRows<FObjectDetails>("", AllRows);

	for (int32 i = 0; i < AllRows.Num(); i++)
	{
		if (AllRows[i] && AllRows[i]->ClassToSpawn == ClassToSpawn)
		{
			return i;
		}
	}
	return -1;
}
FObjectDetails* UBuildingComponent::GetRowByRowNumber(int32 RowNumber)
{
	UDataTable* DataTable = ObjectsDataTable;
	
	if (!DataTable || RowNumber < 0)
		return nullptr;

	TArray<FName> RowNames = DataTable->GetRowNames();
	if (!RowNames.IsValidIndex(RowNumber)) return nullptr;
	
	return DataTable->FindRow<FObjectDetails>(RowNames[RowNumber], "");
}
FObjectDetails* UBuildingComponent::GetRowByName(FName RowName)
{
	if (!ObjectsDataTable || RowName.IsNone())
		return nullptr;
   
	return ObjectsDataTable->FindRow<FObjectDetails>(RowName, "");
}
EInteractionMode UBuildingComponent::ReturnCurrentInteractionMode()
{
	return InteractionMode;
}


bool UBuildingComponent::BoxOverlapComponents(
    const UObject* WorldContextObject, 
    const FVector BoxPos, 
    const FVector BoxExtent, 
    const FRotator BoxRotation, 
    const TArray<TEnumAsByte<EObjectTypeQuery>>& ObjectTypes, 
    UClass* ComponentClassFilter, 
    const TArray<AActor*>& ActorsToIgnore, 
    TArray<UPrimitiveComponent*>& OutComponents,
    bool bShowDebug,     // Toggle for sweeping plane visualization
    bool bShowHitPoints  // Toggle for hit point markers (both counted and ignored)
)
{
    OutComponents.Empty();

    UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
    if (!World) return false;
	
    bool bIsCeilingLight = false;

    if (PreviewActor)
    {
        UClass* SuperClass = PreviewActor->GetClass();
        while (SuperClass)
        {
            FString SuperClassName = SuperClass->GetName();
            if (SuperClassName.Contains(TEXT("CeilingLight")))
            {
                bIsCeilingLight = true;
                break;
            }
            SuperClass = SuperClass->GetSuperClass();
        }
    }

    // **CeilingLight Logic**
    if (bIsCeilingLight)
    {
        // Collision parameters
        FCollisionQueryParams Params(SCENE_QUERY_STAT(BoxOverlapComponents), false);
        Params.AddIgnoredActors(ActorsToIgnore);

        FCollisionObjectQueryParams ObjectParams;
        for (const auto& ObjectType : ObjectTypes)
        {
            const ECollisionChannel& Channel = UCollisionProfile::Get()->ConvertToCollisionChannel(false, ObjectType);
            ObjectParams.AddObjectTypesToQuery(Channel);
        }

        // Define start and end points of the sweeping pyramid
        FVector TopPoint = BoxPos + FVector(0, 0, BoxExtent.Z);  
        FVector BottomPoint = BoxPos - FVector(0, 0, BoxExtent.Z); 

        FQuat RotationQuat = BoxRotation.Quaternion();
        
        TArray<FHitResult> HitResults;
        TMap<UPrimitiveComponent*, TArray<FVector>> ComponentHitLocations;

        // **Pyramid-shaped Sweeps**
        int NumSweeps = 10;
        float HeightStep = (2 * BoxExtent.Z) / NumSweeps;

        float MinWidthX = 10.0f;  // Smallest rectangle X size at the top
        float MinWidthY = MinWidthX * (BoxExtent.Y / BoxExtent.X);  // Keep aspect ratio

        for (int i = 0; i <= NumSweeps; i++)
        {
            float Alpha = (float)i / (float)NumSweeps;  // Interpolation factor

            // Interpolating the width of the box from small (top) to large (bottom)
            float CurrentWidthX = FMath::Lerp(MinWidthX, BoxExtent.X, Alpha);
            float CurrentWidthY = FMath::Lerp(MinWidthY, BoxExtent.Y, Alpha);
            FVector CurrentBoxExtent = FVector(CurrentWidthX, CurrentWidthY, 0.1f);

            FVector CurrentSweepStart = TopPoint - FVector(0, 0, i * HeightStep);
            FVector CurrentSweepEnd = CurrentSweepStart - FVector(0, 0, HeightStep * 0.5f);

            // Perform the pyramid-like sweep
            TArray<FHitResult> TempResults;
            World->SweepMultiByObjectType(
                TempResults,
                CurrentSweepStart,
                CurrentSweepEnd,
                RotationQuat,
                ObjectParams,
                FCollisionShape::MakeBox(CurrentBoxExtent),
                Params
            );

            // **Debug visualization for shrinking boxes**
            if (bShowDebug)
            {
                DrawDebugBox(World, CurrentSweepStart, CurrentBoxExtent, RotationQuat, FColor::Yellow, false, 2.0f, 0, 2.0f);
            }

            for (const FHitResult& Hit : TempResults)
            {
                if (Hit.Component.IsValid() && (!ComponentClassFilter || Hit.Component->IsA(ComponentClassFilter)))
                {
                    UPrimitiveComponent* HitComponent = Hit.Component.Get();
                    FVector HitLocation = Hit.ImpactPoint.IsNearlyZero() ? Hit.Location : Hit.ImpactPoint;

                    // Check if we already detected this hit on the same component at the same location
                    bool bIsNewHit = true;
                    if (ComponentHitLocations.Contains(HitComponent))
                    {
                        for (const FVector& ExistingLocation : ComponentHitLocations[HitComponent])
                        {
                            if (FVector::Dist(ExistingLocation, HitLocation) < 20.0f) 
                            {
                                bIsNewHit = false;
                                break;
                            }
                        }
                    }

                    if (bIsNewHit)
                    {
                        OutComponents.Add(HitComponent);
                        ComponentHitLocations.FindOrAdd(HitComponent).Add(HitLocation);

                        // Debug hit point visualization (Valid Hits - Red)
                        if (bShowHitPoints)
                        {
                            DrawDebugBox(World, HitLocation, FVector(5.0f), FQuat::Identity, FColor::Red, false, 2.0f, 0, 2.0f);
                        }
                    }
                    else if (bShowHitPoints) 
                    {
                        DrawDebugBox(World, HitLocation, FVector(5.0f), FQuat::Identity, FColor::Blue, false, 2.0f, 0, 2.0f);
                    }
                }
            }
        }
    }
    else
    {
        // **Use simpler box overlap logic for non-CeilingLight objects**
        FCollisionQueryParams Params(SCENE_QUERY_STAT(BoxOverlapComponents), false);
        Params.AddIgnoredActors(ActorsToIgnore);

        TArray<FOverlapResult> Overlaps;
        FCollisionObjectQueryParams ObjectParams;
        for (const auto& ObjectType : ObjectTypes)
        {
            const ECollisionChannel& Channel = UCollisionProfile::Get()->ConvertToCollisionChannel(false, ObjectType);
            ObjectParams.AddObjectTypesToQuery(Channel);
        }

        World->OverlapMultiByObjectType(Overlaps, BoxPos, BoxRotation.Quaternion(), ObjectParams, FCollisionShape::MakeBox(BoxExtent), Params);

        for (const FOverlapResult& Overlap : Overlaps)
        {
            if (Overlap.Component.IsValid() && (!ComponentClassFilter || Overlap.Component->IsA(ComponentClassFilter)))
            {
                OutComponents.Add(Overlap.Component.Get());
            }
        }
    }

    return OutComponents.Num() > 0;
}
