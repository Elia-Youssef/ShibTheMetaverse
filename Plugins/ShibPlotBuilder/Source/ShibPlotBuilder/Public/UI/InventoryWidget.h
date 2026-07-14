// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Tools/BuildTypes.h"
#include "InventoryWidget.generated.h"

class UDataTable;
class ABuildController;
class UButton;

UCLASS()
class SHIBPLOTBUILDER_API UInventoryWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void InitializeInventory(ABuildController* InController, UDataTable* InObjectsTable);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	EWidgetMode WidgetMode = EWidgetMode::None;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FVector2D ItemWidgetSize;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FVector2D ItemWidgetPadding;

	UPROPERTY(BlueprintReadOnly)
	TArray<FWidgetItem> ItemsDetails;

	UPROPERTY(BlueprintReadWrite)
	bool bIsMenuExpanded = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int32 MenuMaxColumns = 5;
	
	UPROPERTY(BlueprintReadWrite)
	TArray<UUserWidget*> MenuItems;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int32 ExpandedMenuMaxColumns = 10;
	
	UPROPERTY(BlueprintReadWrite)
	TArray<UUserWidget*> ExpandedMenuItems;

	UPROPERTY(BlueprintReadWrite)
	UUserWidget* SelectedItemWidget;

	UPROPERTY(BlueprintReadWrite)
	FWidgetItem SelectedItem;

	UPROPERTY(BlueprintReadOnly)
	TArray<FName> ParentsTree;

	UFUNCTION(BlueprintCallable)
	void SetWidgetMode(EWidgetMode InWidgetMode, bool bClickedAgain);

	UFUNCTION(BlueprintImplementableEvent)
	void UpdateItemsWidgets();
	
	UFUNCTION(BlueprintImplementableEvent)
	void UpdateHighlight(FItemArrowsAvailability ItemArrowsAvailability, FVector2D Location);
	
	UFUNCTION(BlueprintCallable)
	FItemArrowsAvailability GetAvailableArrows();

	void OnSelect(const FVector& Vector);

	UFUNCTION(BlueprintCallable)
	void OnHover(UUserWidget* Widget);

	UFUNCTION(BlueprintCallable, BlueprintPure)
	int32 GetNumberOfRows(bool bExpanded) const;

	UFUNCTION(BlueprintCallable, BlueprintPure)
	TArray<FWidgetItem> GetItemsInRow(const int32 RowIndex, const bool bExpanded);

	FVector2D GetNewHighlightLocation() const;
	void LoadBuildItems();
	void OnEnter();
	void OnBack();

protected:
	TObjectPtr<ABuildController> Controller;
	TObjectPtr<UDataTable> ObjectsTable;
};
