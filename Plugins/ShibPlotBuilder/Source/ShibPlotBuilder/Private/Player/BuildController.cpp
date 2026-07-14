// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/BuildController.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Components/BuildingComponent.h"
#include "Player/BuildCharacter.h"
#include "Blueprint/UserWidget.h"
#include "Game/BuildGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "UI/InventoryWidget.h"
#include "Game/BuildSave.h"
#include "Game/PlotBuilderSubsystem.h"


ABuildController::ABuildController()
{
	BuildingComponent = CreateDefaultSubobject<UBuildingComponent>("BuildingComponent");
}

void ABuildController::InitializeUI()
{
	if (InventoryWidgetClass)
	{
		InventoryWidget = CreateWidget<UInventoryWidget>(this, InventoryWidgetClass);
		if (InventoryWidget)
		{
			InventoryWidget->InitializeInventory(this, BuildingComponent->ObjectsDataTable);
			
			InventoryWidget->AddToViewport();
			UWidgetBlueprintLibrary::SetHardwareCursor(this, EMouseCursor::Default, FName("T_CircleCursorSlate"), FVector2D(0.5f, 0.5f));
		}
	}
}

void ABuildController::SetupInputComponent()
{
	Super::SetupInputComponent();
	
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(InputComponent)) {
		// Movement
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ABuildController::Move);
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ABuildController::Look);

		// Mouse Clicks
		EnhancedInputComponent->BindAction(LeftClickAction, ETriggerEvent::Started, this, &ABuildController::OnLeftClickStarted);
		EnhancedInputComponent->BindAction(LeftClickAction, ETriggerEvent::Canceled, this, &ABuildController::OnLeftClickReleased);
		EnhancedInputComponent->BindAction(LeftClickAction, ETriggerEvent::Completed, this, &ABuildController::OnLeftClickReleased);

		// Q-E 
		
		EnhancedInputComponent->BindAction(QAction, ETriggerEvent::Started, this, &ABuildController::OnQActionStarted);
		EnhancedInputComponent->BindAction(QAction, ETriggerEvent::Canceled, this, &ABuildController::OnQActionReleased);
		EnhancedInputComponent->BindAction(QAction, ETriggerEvent::Completed, this, &ABuildController::OnQActionReleased);

		EnhancedInputComponent->BindAction(EAction, ETriggerEvent::Started, this, &ABuildController::OnEActionStarted);
		EnhancedInputComponent->BindAction(EAction, ETriggerEvent::Canceled, this, &ABuildController::OnEActionReleased);
		EnhancedInputComponent->BindAction(EAction, ETriggerEvent::Completed, this, &ABuildController::OnEActionReleased);
		
		EnhancedInputComponent->BindAction(MiddleClickAction, ETriggerEvent::Started, this, &ABuildController::OnMiddleClick);

		// Left Shift
		EnhancedInputComponent->BindAction(LeftShiftAction, ETriggerEvent::Started, this, &ABuildController::OnLeftShiftStarted);
		EnhancedInputComponent->BindAction(LeftShiftAction, ETriggerEvent::Canceled, this, &ABuildController::OnLeftShiftReleased);
		EnhancedInputComponent->BindAction(LeftShiftAction, ETriggerEvent::Completed, this, &ABuildController::OnLeftShiftReleased);

		// Left Ctrl
		EnhancedInputComponent->BindAction(LeftCtrlAction, ETriggerEvent::Started, this, &ABuildController::OnLeftCtrlStarted);
		EnhancedInputComponent->BindAction(LeftCtrlAction, ETriggerEvent::Canceled, this, &ABuildController::OnLeftCtrlReleased);
		EnhancedInputComponent->BindAction(LeftCtrlAction, ETriggerEvent::Completed, this, &ABuildController::OnLeftCtrlReleased);

		// Tab
		EnhancedInputComponent->BindAction(TabAction, ETriggerEvent::Started, this, &ABuildController::OnTabStarted);

		// Arrows
		// EnhancedInputComponent->BindAction(ArrowsAction, ETriggerEvent::Started, this, &ABuildController::OnArrowsStarted);

		// Enter / Back
		// EnhancedInputComponent->BindAction(EnterBackAction, ETriggerEvent::Started, this, &ABuildController::OnEnterBackAction);

		//Escape
		EnhancedInputComponent->BindAction(EscapeAction, ETriggerEvent::Started, this, &ABuildController::OnEscapeStarted);

		// Interact
		EnhancedInputComponent->BindAction(InteractionAction, ETriggerEvent::Started, this, &ABuildController::OnObjectInteracted);

		// GetMeshes
		EnhancedInputComponent->BindAction(MeshInteractionAction, ETriggerEvent::Started, this, &ABuildController::OnGetObjectMeshes);
	}
}

void ABuildController::BeginPlay()
{
	Super::BeginPlay();
}

void ABuildController::InitializeBuildController(const FString& Json)
{
	BuildCharacter = Cast<ABuildCharacter>(GetCharacter());
	BuildingComponent->Initialize(BuildCharacter, this);

	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		Subsystem->AddMappingContext(DefaultMappingContext, 0);
	}

	InitializeUI();
	
	UBuildSave* BuildSave = Cast<UBuildSave>(UGameplayStatics::CreateSaveGameObject(UBuildSave::StaticClass()));

	if (BuildSave)
	{
		// UE_LOG(LogTemp, Display, TEXT("Loading GameMode: UBuildSave object created successfully."));
		BuildSave->LoadGameFromString(Json);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Loading GameMode: Failed to create UBuildSave object."));
	}
	
	BuildGameMode = Cast<ABuildGameMode>(UGameplayStatics::GetGameMode(this));
	PlotBuilderHistory = NewObject<UPlotBuilderHistory>();
	
	if (BuildGameMode)
	{
		// UE_LOG(LogTemp, Display, TEXT("Loading GameMode: BuildGameMode is valid."));
		BuildGameMode->BuildSave = BuildSave;
		if (BuildGameMode->BuildSave)
		{
			// UE_LOG(LogTemp, Display, TEXT("Loading GameMode: BuildGameMode->BuildSave is valid."));

			// UE_LOG(LogTemp, Display, TEXT("Loading GameMode: BuildingComponent->ObjectsBuilt = BuildGameMode->BuildSave->ObjectsBuilt;"));
			BuildingComponent->ObjectsBuilt = BuildGameMode->BuildSave->ObjectsBuilt;
			
			// UE_LOG(LogTemp, Display, TEXT("Loading GameMode: BuildingComponent->LoadObjectsBuilt();"));
			BuildingComponent->LoadObjectsBuilt();
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Loading GameMode: BuildGameMode->BuildSave is null. Unable to load objects built."));
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Loading GameMode: BuildGameMode is null. Unable to load objects built."));
	}

	// SetShowMouseCursor(false);
	// FInputModeGameOnly InputModeGameOnly;
	// SetInputMode(InputModeGameOnly);

	BuildingComponent->OnInteractionModeChange(EInteractionMode::View);
}

void ABuildController::Move(const FInputActionValue& Value)
{
	if (bControlModeDisabled)
		return;
	
	FVector MovementVector = Value.Get<FVector>();

	const FVector ForwardVector = BuildCharacter->GetActorForwardVector();
	const FVector RightVector = BuildCharacter->GetActorRightVector();
	const FVector UpVector = BuildCharacter->GetActorUpVector();

	BuildCharacter->AddMovementInput(ForwardVector, MovementVector.Y);
	BuildCharacter->AddMovementInput(RightVector, MovementVector.X);
	BuildCharacter->AddMovementInput(UpVector, MovementVector.Z);
	// PlotBuilderHistory->Add()
}

void ABuildController::Look(const FInputActionValue& Value)
{
	// if (bShowMouseCursor) return;

	if (bControlModeDisabled)
		return;
	
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	BuildCharacter->AddControllerYawInput(LookAxisVector.X);
	BuildCharacter->AddControllerPitchInput(LookAxisVector.Y);
}

void ABuildController::OnLeftClickStarted(const FInputActionValue& Value)
{
	if (bControlModeDisabled)
		return;
	
	if (BuildingComponent->InteractionMode == EInteractionMode::Build)
	{
		// if (bLeftShiftDown) BuildingComponent->DestroyHitActor();
		
		if (bLeftShiftDown && ActorToDelete != nullptr)
		BuildingComponent->DestroyHitActor(ActorToDelete);
		else if(!bLeftShiftDown)
		{
			if (bLeftCtrlDown) BuildingComponent->MoveHitActor();
			else BuildingComponent->SpawnSelectedActorFromPreview();
		}
	}
	else
	{
		// if (bLeftShiftDown)
		// 	BuildingComponent->DestroyHitActor();
		if (bLeftShiftDown && ActorToDelete != nullptr)
			BuildingComponent->DestroyHitActor(ActorToDelete);
	}
}

void ABuildController::OnLeftClickReleased(const FInputActionValue& Value)
{
	if (bControlModeDisabled)
		return;
	
	if (BuildingComponent->bMovingObject)
	{
		BuildingComponent->ReleaseHitActor();
	}
}

void ABuildController::OnQActionStarted(const FInputActionValue& Value)
{
	if (bControlModeDisabled)
		return;
	
	BuildingComponent->StartPreviewRotation(true);
}

void ABuildController::OnQActionReleased(const FInputActionValue& Value)
{
	if (bControlModeDisabled)
		return;
	
	BuildingComponent->EndPreviewRotation();
}

void ABuildController::OnEActionStarted(const FInputActionValue& Value)
{
	if (bControlModeDisabled)
		return;
	
	BuildingComponent->StartPreviewRotation(false);
}

void ABuildController::OnEActionReleased(const FInputActionValue& Value)
{
	if (bControlModeDisabled)
		return;
	
	BuildingComponent->EndPreviewRotation();
}

void ABuildController::OnMiddleClick(const FInputActionValue& Value)
{
	if (bControlModeDisabled)
		return;
	
	BuildingComponent->ResetPreviewRotation();
}

void ABuildController::OnLeftShiftStarted(const FInputActionValue& Value)
{
	// bLeftShiftDown = true;
}

void ABuildController::OnLeftShiftReleased(const FInputActionValue& Value)
{
	// bLeftShiftDown = false;
}

void ABuildController::OnLeftCtrlStarted(const FInputActionValue& Value)
{
	bLeftCtrlDown = true;
}

void ABuildController::OnLeftCtrlReleased(const FInputActionValue& Value)
{
	bLeftCtrlDown = false;
}

void ABuildController::OnTabStarted(const FInputActionValue& Value)
{
	if (bControlModeDisabled)
		return;
	
	bShowMouseCursor = !bShowMouseCursor;
	
	SetShowMouseCursor(bShowMouseCursor);

	if(BuildingComponent->InteractionMode == EInteractionMode::Build)
		BuildingComponent->OnInteractionModeChange(EInteractionMode::View);
	else
		BuildingComponent->OnInteractionModeChange(EInteractionMode::Build);
	
	if (bShowMouseCursor)
	{
		FInputModeGameAndUI InputModeGameAndUI;
		InputModeGameAndUI.SetLockMouseToViewportBehavior(EMouseLockMode::LockInFullscreen);
		SetInputMode(InputModeGameAndUI);
	} else
	{
		FInputModeGameOnly InputModeGameOnly;
		SetInputMode(InputModeGameOnly);
	}
}

// void ABuildController::OnArrowsStarted(const FInputActionValue& Value)
// {
// 	const FVector MovementVector = Value.Get<FVector>();
//
// 	InventoryWidget->OnSelect(MovementVector);
// }
//
// void ABuildController::OnEnterBackAction(const FInputActionValue& Value)
// {
// 	const float EnterBackValue = Value.Get<float>();
//
// 	if (EnterBackValue > 0)
// 	{
// 		InventoryWidget->OnEnter();
// 	} else
// 	{
// 		InventoryWidget->OnBack();
// 	}
// }

void ABuildController::OnEscapeStarted(const FInputActionValue& Value)
{
	if (bControlModeDisabled)
		return;
	
	UE_LOG(LogTemp, Log, TEXT("OnEscapeStarted quit build mode"));
	
	BuildingComponent->ResetPreviewRotation();
	OnTabStarted(FInputActionValue(true));
}


FHitResult ABuildController::ShortDetectionCast()
{
	FHitResult TraceHitResult;
	const FVector Start = BuildCharacter->GetActorLocation();
	const FVector End = Start + BuildCharacter->GetActorForwardVector() * 300.0f;
	FCollisionQueryParams QueryParams = FCollisionQueryParams();
	QueryParams.AddIgnoredActor(BuildCharacter);
	GetWorld()->LineTraceSingleByChannel(TraceHitResult, Start, End, ECollisionChannel::ECC_Visibility, QueryParams);
	return TraceHitResult;
}

void ABuildController::OnObjectInteracted(const FInputActionValue& InputActionValue)
{
	if (const FHitResult TraceHitResult = ShortDetectionCast(); TraceHitResult.GetActor() != nullptr)
	{
		IBuildObjectInterface::Execute_InteractWithObjectWithInVillas(TraceHitResult.GetActor());
	}
}


void ABuildController::OnGetObjectMeshes(const FInputActionValue& InputActionValue)
{
	const FHitResult TraceHitResult = ShortDetectionCast();
	FName HitActorTag = IBuildObjectInterface::Execute_GetActorTag(TraceHitResult.GetActor());
	if (auto* HitComponent = Cast<UStaticMeshComponent>(TraceHitResult.GetComponent()))
	{
		if (HitComponent->ComponentTags.Num())
		{
			if (FObjectDetails* ActorRow = BuildingComponent->ObjectsDataTable->FindRow<FObjectDetails>(HitActorTag, ""))
			{
				UE_LOG(LogTemp, Log, TEXT("Hit Component Tag: %s"), *HitComponent->ComponentTags[0].ToString());
				if (ActorRow->ObjectPossibleMaterials.Contains(HitComponent->ComponentTags[0]))
				{
					const TArray<UMaterialInstance*> MaterialInstances = ActorRow->ObjectPossibleMaterials[HitComponent->ComponentTags[0]].PossibleMaterial;
					IBuildObjectInterface::Execute_ReturnSelectedMeshMaterials(TraceHitResult.GetActor(), MaterialInstances);
					
					// Write a proper function for this.
					// int32 GetRandomMaterial = FMath::RandRange(0, ActorRow->ObjectPossibleMaterials[HitComponent->ComponentTags[0]].PossibleMaterial.Num() - 1);
					// HitComponent->SetMaterial(0, ActorRow->ObjectPossibleMaterials[HitComponent->ComponentTags[0]].PossibleMaterial[GetRandomMaterial]);
				}
			}
		}
	}
}

void ABuildController::OnSaveProgress()
{
	if (BuildGameMode)
	{
		BuildingComponent->OldObjectsBuilt.Empty();
		BuildingComponent->OldObjectsBuilt = BuildingComponent->ObjectsBuilt;//.Append(BuildGameMode->BuildSave->ObjectsBuilt);

		UE_LOG(LogTemp, Log, TEXT("BuildingComponent->ObjectsBuilt -- OnSaveProgress %d"), BuildingComponent->ObjectsBuilt.Num());
		UE_LOG(LogTemp, Log, TEXT("BuildingComponent->OldObjectsBuilt -- OnSaveProgress %d"), BuildingComponent->OldObjectsBuilt.Num());
		
		FString SaveString = BuildGameMode->BuildSave->SaveGame(BuildingComponent->ObjectsBuilt);
		if (SaveString.IsEmpty()) return;
		
		if (auto* GI = GetGameInstance())
		{
			if (auto* PbSubsystem = GI->GetSubsystem<UPlotBuilderSubsystem>())
			{
				PbSubsystem->OnPlotBuilderSave.Broadcast(SaveString);
			}
		}
	}
}

bool ABuildController::CheckSaveStateAvailable()
{
	int32 ObjectsBuiltSize = BuildingComponent->ObjectsBuilt.Num();
	int32 OldObjectsBuiltSize = BuildingComponent->OldObjectsBuilt.Num();

	UE_LOG(LogTemp, Log, TEXT("BuildingComponent->ObjectsBuilt -- CheckSaveStateAvailable %d"), BuildingComponent->ObjectsBuilt.Num());
	UE_LOG(LogTemp, Log, TEXT("BuildingComponent->OldObjectsBuilt -- CheckSaveStateAvailable %d"), BuildingComponent->OldObjectsBuilt.Num());
	
	if (OldObjectsBuiltSize < ObjectsBuiltSize || ObjectsBuiltSize > OldObjectsBuiltSize)
	{
		UE_LOG(LogTemp, Log, TEXT("Update on deployed objects where found, exit save menu to be shown"));
		return true;
	}
	return false;
}

void ABuildController::DisplayMouseOnClassSpawnClick()
{
	bShowMouseCursor = !bShowMouseCursor;
	
	SetShowMouseCursor(false);
	FInputModeGameOnly InputModeGameOnly;
	SetInputMode(InputModeGameOnly);
}

bool UPlotBuilderHistory::CanRedo() const
{
	return (!UndoRedoStackObjectsBuilt.IsEmpty() && CurrentHistoryPosition < UndoRedoStackObjectsBuilt.Num() - 1);
}

void ABuildController::RedoItem()
{
	if (PlotBuilderHistory->UndoRedoStackObjectsBuilt.IsEmpty())
	{
		UE_LOG(LogTemp, Error, TEXT("Cannot redo: History stack is empty"));
		return;
	}

	if (!PlotBuilderHistory->UndoRedoStackObjectsBuilt.IsValidIndex(PlotBuilderHistory->CurrentHistoryPosition + 1))
	{
		UE_LOG(LogTemp, Error, TEXT("Cannot redo: No more actions to redo"));
		return;
	}

	FObjectBuiltData ObjectBuiltData;
	if (PlotBuilderHistory->Redo(ObjectBuiltData))
	{
		if (ensure(BuildingComponent))
		{
			ObjectBuiltData.InteractionState = EObjectInteractionState::Redo;
			BuildingComponent->SpawnActorFromRedo(ObjectBuiltData);
			ObjectBuiltData.Transform.SetLocation(PlotBuilderHistory->ReturnActorOldLocation());
			PlotBuilderHistory->Replace(ObjectBuiltData);
			UE_LOG(LogTemp, Log, TEXT("Successfully redid action to position [%d]"), PlotBuilderHistory->CurrentHistoryPosition);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("BuildingComponent is invalid"));
		}
	}
}

void ABuildController::UndoItem()
{
	if (PlotBuilderHistory->UndoRedoStackObjectsBuilt.IsEmpty())
	{
		UE_LOG(LogTemp, Error, TEXT("Cannot undo: History stack is empty"));
		return;
	}

	if (!PlotBuilderHistory->UndoRedoStackObjectsBuilt.IsValidIndex(PlotBuilderHistory->CurrentHistoryPosition))
	{
		UE_LOG(LogTemp, Error, TEXT("Cannot undo: Invalid history position [%d]"), PlotBuilderHistory->CurrentHistoryPosition);
		return;
	}

	FObjectBuiltData ObjectBuiltData;
	if (PlotBuilderHistory->Undo(ObjectBuiltData, LastInteractionState))
	{
		if (ensure(BuildingComponent))
		{
			if (LastInteractionState == EObjectInteractionState::Delete)
			{
				BuildingComponent->SpawnActorFromRedo(ObjectBuiltData);
				PlotBuilderHistory->Replace(ObjectBuiltData);
				ObjectBuiltData.Transform.SetLocation(PlotBuilderHistory->ReturnActorOldLocation());
				UE_LOG(LogTemp, Log, TEXT("PlotBuilderHistory->Replace [%p]"), ObjectBuiltData.BuiltActor);
				// LastInteractionState = EObjectInteractionState::Spawn;
			}
			else if (LastInteractionState != EObjectInteractionState::Spawn)
			{
				BuildingComponent->DeSpawnActorFromUndo(ObjectBuiltData);
			}
			
			ObjectBuiltData.InteractionState = EObjectInteractionState::Undo;
			UE_LOG(LogTemp, Log, TEXT("Successfully undid action at position [%d]"), PlotBuilderHistory->CurrentHistoryPosition + 1);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("BuildingComponent is invalid"));
		}
	}
}
void ABuildController::SelectObjectToDelete(bool bShiftStatus, AActor* HitActorParent, AActor* HitActor)
{
	bLeftShiftDown = bShiftStatus;
	BuildingComponent->OnInteractionModeChange(EInteractionMode::View);
	
	if (HitActorParent != nullptr)
	{
		if (!HitActorParent->IsA(ABaseBuildActor::StaticClass()))
		{
			HitActorParent = nullptr;
			ActorToDelete = nullptr;
			return;
		}
		else
		{
			ActorToDelete = HitActorParent;
		}
	}
	else if(HitActor != nullptr)
	{
		if(!HitActor->IsA(ABaseBuildActor::StaticClass()))
		{
			HitActor = nullptr;
			ActorToDelete = nullptr;
			return;
		}
		else
		{
			ActorToDelete = HitActor;
		}
	}
	
	if (ActorToDelete != nullptr)
	{
		if (ActorToDelete->IsA(ABaseBuildActor::StaticClass()))
		{
			TArray<FObjectBuiltData> ObjectsData;
			TArray<AActor*> ActorsToDelete;
			ObjectsData.Empty();
			ActorsToDelete.Empty();
			ActorsToDelete.Add(ActorToDelete);
			ObjectsData.Append(BuildingComponent->ObjectsBuilt);

			for (int32 i = 0; i < ObjectsData.Num(); i++)
			{
				
				if (ObjectsData[i].ActorParentReference != nullptr && 
					ObjectsData[i].ActorParentReference == ActorToDelete && 
					ObjectsData[i].BuiltActor != nullptr)
				{
					UE_LOG(LogTemp, Log, TEXT("Deleting Objects: Destroying built actor associated with parent: %s"), *ObjectsData[i].BuiltActor->GetName());
					ActorsToDelete.Add(ObjectsData[i].BuiltActor);
				}
			}
			for (AActor* Actor : ActorsToDelete)
			{
				if (IsValid(Actor))
				{
					BuildingComponent->DestroyHitActor(Actor);
				}
			}
		}
	}
	
}

void ABuildController::DisableControlMode()
{
	bControlModeDisabled = false;
}

void ABuildController::SelectedHitActors(AActor* HitActorParent, AActor* HitActor)
{
	SelectedHitActorParent = HitActorParent;
	SelectedHitActor = HitActor;
}

FSelectedHitActors ABuildController::RetrunSelectedHitActors()
{
	FSelectedHitActors ReturnSelectedActors;
	ReturnSelectedActors.SelectedHitActorRef = SelectedHitActor;
	ReturnSelectedActors.SelectedHitActorParentRef = SelectedHitActorParent;
	return ReturnSelectedActors;
}

 
