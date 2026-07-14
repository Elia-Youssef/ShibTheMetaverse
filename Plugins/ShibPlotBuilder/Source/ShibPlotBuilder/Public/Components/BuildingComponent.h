// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Tools/BuildTypes.h"
#include "BuildingComponent.generated.h"

class UDataTable;
class ABuildCharacter;
class ABuildController;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SHIBPLOTBUILDER_API UBuildingComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UBuildingComponent();
	
	void Initialize(ABuildCharacter* Character, ABuildController* Controller);

	bool bIsWall;
	bool bIsCeiling;
	bool bIsFloor;

	UPROPERTY(BlueprintReadWrite)
	ABuildController * BuildControllerRef;
	
	UPROPERTY(BlueprintReadWrite)
	FVector PreviewBoxExtent;
	
	UPROPERTY(BlueprintReadWrite)
	bool bIsClipping;
	
	UFUNCTION()
	void CustomTick();

	UPROPERTY(EditDefaultsOnly)
	float TicksPerSecond = 15;

	UPROPERTY(BlueprintReadOnly, Category = "Input")
	EInteractionMode InteractionMode = EInteractionMode::Build;

	UPROPERTY(EditDefaultsOnly, Category = "Data")
	UDataTable* ObjectsDataTable;

	UPROPERTY(BlueprintReadOnly, Category = "Preview")
	bool bMovingObject = false;

	// SAVE
	UPROPERTY(BlueprintReadOnly, Category = "Save")
	TArray<FObjectBuiltData> ObjectsBuilt;
	
	UPROPERTY(BlueprintReadOnly, Category = "Save")
	TArray<FObjectBuiltData> OldObjectsBuilt;

	// INPUT
	UPROPERTY(BlueprintReadOnly, Category = "Input")
	FObjectDetails SelectedActorToSpawn;

	// Get the actor details from the data table and sets it as preview actor
	UFUNCTION(BlueprintCallable)
	void SetSelectedActorToSpawn(const FName& Tag);

	// Update UI/input for each interaction mode
	UFUNCTION(BlueprintCallable)
	void OnInteractionModeChange(EInteractionMode NewInteractionMode);

	UFUNCTION(BlueprintCallable)
	EInteractionMode ReturnCurrentInteractionMode();

	// Update the `FHitResult TraceHitResult` variable
	void GetTraceHit();

	
	FHitResult PerformLineTrace(const FVector& Start, const FVector& End, bool bTraceComplex);

	// Update preview actor to be the selected actor
	void UpdatePreviewActor();
	
	// Update the preview actor location based on the hit target
	void UpdatePreviewLocation();

	// Update the preview actor status (can/can't build) based on the hit target
	void UpdatePreviewStatus();

	// Start Preview Rotation
	void StartPreviewRotation(bool bRotateLeft);
	
	// End Preview Rotation
	void EndPreviewRotation();
	
	// Reset Preview Rotation
	void ResetPreviewRotation();

	// Get the hit actor's closest socket (can be nullptr)
	USceneComponent* GetHitActorSocket();

	// Find the closest socket
	USceneComponent* FindClosestSocket(TArray<USceneComponent*> Sockets);

	// Check if the hit target accepts the selected actor's tag or parent's tag (should be called only if no sockets are available)
	bool CheckHitTargetAcceptsTag(bool bIgnoreParent = false);

	// Check if the hit target has sockets for the selected actor's tag
	bool CheckHitTargetHasSockets();

	// Actor->Implements<Interface>()
	bool CheckHitActorImplementsInterface();

	// Check if the selected actor can be spawned based on the hit target
	bool CanSpawnActor();
	
	// Destroy Hit Actor and remove it from the ObjectsBuilt array
	void DestroyHitActor();

	void DestroyHitActor(FObjectBuiltData& ObjectDetails);

	void DestroyHitActor(AActor* ClickActor);

	// Spawn the selected actor in the preview actor's place
	void SpawnSelectedActorFromPreview();
	
	// Set preview actor to hit actor to move it
	UFUNCTION(BlueprintCallable)
	void MoveHitActor();
	
	// Leave the preview actor in place and reinitialize sockets
	UFUNCTION(BlueprintCallable)
	void ReleaseHitActor();

	// Updates the actor's transform in the BuiltObjects list
	void UpdateBuiltObjectSaveTransform(const AActor* Actor);

	// Check if given location is within allowed land
	bool IsLocationAllowed(const FVector& Location) const;

	// Loads the `ObjectsBuilt` array
	void LoadObjectsBuilt();

	// Interact
	void InteractWithObject();

	// Interact For Villas
	void InteractWithObjectWithInVillas();

	// Undo-Redo Functions
	void SpawnActorFromRedo(FObjectBuiltData& ObjectToBuild);
	void DeSpawnActorFromUndo(FObjectBuiltData& ObjectToBuild);

	// Table Utilities
	int32 GetRowNumberByClass(TSubclassOf<ABaseBuildActor> ClassToSpawn);
	FName GetRowNameByClass(TSubclassOf<ABaseBuildActor> ClassToSpawn);
	int32 FindRowIndexByClass(TSubclassOf<ABaseBuildActor> ClassToSpawn);
	FObjectDetails* GetRowByRowNumber(int32 RowNumber);
	FObjectDetails* GetRowByName(FName RowName);
	
protected:
	virtual void BeginPlay() override;

	// Spawns the actor as a child
	void SpawnActorAsChild(AActor* Parent, FObjectBuiltData& ObjectToBuild);

	// Spawns the actor
	ABaseBuildActor* SpawnActor(FObjectBuiltData& ObjectToBuild, bool bAddTohHistory = true);

	// PREVIEW
	UPROPERTY(BlueprintReadOnly, Category = "Preview")
	TObjectPtr<ABaseBuildActor> PreviewActor;
	
	FTimerHandle PreviewRotationTimerHandle;

	bool bIsPreviewRotating = false;
	bool bHasAppliedRotation = false;
	FRotator LastManualRotation;
	
	// INPUT
	UPROPERTY(BlueprintReadOnly, Category = "Input")
	FHitResult TraceHitResult;

	UPROPERTY(BlueprintReadOnly, Category = "Input")
	FMaterialType SelectedMaterial;

	// REFERENCES
	UPROPERTY(BlueprintReadOnly, Category = "Casts")
	TObjectPtr<ABuildCharacter> BuildCharacter;

	UPROPERTY(BlueprintReadOnly, Category = "Casts")
	TObjectPtr<ABuildController> BuildController;

	// LAND
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FVector2D LandSize;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float NotAllowedBorderSize;
	
	FVector2D AllowedToBuildSize;

	UFUNCTION(BlueprintCallable)
	bool BoxOverlapComponents(
	const UObject* WorldContextObject, 
	const FVector BoxPos, 
	const FVector BoxExtent, 
	const FRotator BoxRotation, 
	const TArray<TEnumAsByte<EObjectTypeQuery>>& ObjectTypes, 
	UClass* ComponentClassFilter, 
	const TArray<AActor*>& ActorsToIgnore, 
	TArray<UPrimitiveComponent*>& OutComponents,
	bool bShowDebug,     // Toggle for sweeping plane visualization
	bool bShowHitPoints  // Toggle for hit point markers
);
	
private:
	UPROPERTY()
	FTimerHandle CustomTickHandler;
	
	UPROPERTY()
	FTimerDynamicDelegate CustomTickDelegate;

	UPROPERTY()
	bool bIsOutOfBounds;
 	
};
