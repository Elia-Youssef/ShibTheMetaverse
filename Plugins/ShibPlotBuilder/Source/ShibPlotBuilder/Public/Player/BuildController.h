// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BuildCharacter.h"
#include "GameFramework/PlayerController.h"
#include "InputActionValue.h"
#include "Game/PlotBuilderHistory.h"
#include "BuildController.generated.h"

class ABuildGameMode;
class UBuildSave;
class UInventoryWidget;
class UBuildingComponent;
class ABaseBuildActor;
class UInputMappingContext;
class UInputAction;

UCLASS()
class SHIBPLOTBUILDER_API ABuildController : public APlayerController
{
	GENERATED_BODY()

public:
	ABuildController();
	
	void InitializeUI();

	virtual void SetupInputComponent() override;
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void OnLeftClickStarted(const FInputActionValue& Value);
	void OnLeftClickReleased(const FInputActionValue& Value);
	void OnQActionStarted(const FInputActionValue& Value);
	void OnQActionReleased(const FInputActionValue& Value);
	void OnMiddleClick(const FInputActionValue& Value);
	void OnEActionStarted(const FInputActionValue& Value);
	void OnEActionReleased(const FInputActionValue& Value);
	void OnLeftShiftStarted(const FInputActionValue& Value);
	void OnLeftShiftReleased(const FInputActionValue& Value);
	void OnLeftCtrlStarted(const FInputActionValue& Value);
	void OnLeftCtrlReleased(const FInputActionValue& Value);
	void OnTabStarted(const FInputActionValue& Value);
	// void OnArrowsStarted(const FInputActionValue& Value);
	// void OnEnterBackAction(const FInputActionValue& InputActionValue);
	void OnEscapeStarted(const FInputActionValue& InputActionValue);
	void OnObjectInteracted(const FInputActionValue& InputActionValue);
	FHitResult ShortDetectionCast();
	void OnObjectInteractedWithInVillas(const FInputActionValue& InputActionValue);
	void OnGetObjectMeshes(const FInputActionValue& InputActionValue);

	UFUNCTION(BlueprintCallable)
	void OnSaveProgress();

	UFUNCTION(BlueprintCallable)
	bool CheckSaveStateAvailable();

	UFUNCTION(BlueprintCallable)
	void DisplayMouseOnClassSpawnClick();

	UFUNCTION(BlueprintCallable)
	void UndoItem();

	UFUNCTION(BlueprintCallable)
	void RedoItem();

	void InitializeBuildController(const FString& Json);
	
	// UFUNCTION(BlueprintCallable)
	// bool bSaveStateAvailable = false;

	UPROPERTY(BlueprintReadOnly, Category = "Casts")
	TObjectPtr<ABuildCharacter> BuildCharacter;

	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<UUserWidget> InventoryWidgetClass;

	UPROPERTY(BlueprintReadOnly, Category = "UI")
	TObjectPtr<UInventoryWidget> InventoryWidget;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputMappingContext* DefaultMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* MoveAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* LookAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* LeftClickAction;
	
	// UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	// UInputAction* RightClickAction;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* MiddleClickAction;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* LeftShiftAction;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* LeftCtrlAction;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* TabAction;
	
	// UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	// UInputAction* ArrowsAction;
	
	// UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	// UInputAction* EnterBackAction;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* EscapeAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* InteractionAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* MeshInteractionAction;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UBuildingComponent> BuildingComponent;

	bool bLeftShiftDown = false;
	bool bLeftCtrlDown = false;
	bool bShowMouseCursor = false;
	bool bControlModeDisabled = true;
	AActor* ActorToDelete = nullptr;

	UPROPERTY(BlueprintReadWrite)
	AActor* SelectedHitActorParent = nullptr;

	UPROPERTY(BlueprintReadWrite)
	AActor* SelectedHitActor = nullptr;

	UPROPERTY(BlueprintReadOnly)
	UPlotBuilderHistory* PlotBuilderHistory;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EObjectInteractionState LastInteractionState = EObjectInteractionState::Spawn;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* QAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* EAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	USoundBase* Music;

	UFUNCTION(BlueprintCallable)
	void SelectObjectToDelete(bool bShiftStatus, AActor* HitActorParent, AActor* HitActor);

	UFUNCTION(BlueprintCallable)
	void DisableControlMode();

	UFUNCTION(BlueprintCallable)
	void SelectedHitActors(AActor* HitActorParent, AActor* HitActor);

	UFUNCTION(BlueprintCallable)
	FSelectedHitActors RetrunSelectedHitActors();
	
protected:
	virtual void BeginPlay() override;

	UPROPERTY()
	ABuildGameMode* BuildGameMode;

};
