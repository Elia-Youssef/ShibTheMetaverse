// Fill out your copyright notice in the Description page of Project Settings.

#pragma once


#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "UObject/Class.h"
#include "UObject/Interface.h"
#include "UObject/Object.h"
#include "Tools/BuildTypes.h"
#include "PlotBuilderHistory.generated.h"

UCLASS()
class SHIBPLOTBUILDER_API UPlotBuilderHistory : public UObject
{
	GENERATED_BODY()
	
public:

	int32 CurrentHistoryPosition = -1;
	int32 MaxHistoryPosition = 10;
	
	TArray<FObjectBuiltData> UndoRedoStackObjectsBuilt;

	bool Undo(FObjectBuiltData& ObjectBuiltData, EObjectInteractionState InteractionState);
	bool Redo(FObjectBuiltData& ObjectBuiltData);
	void Replace(FObjectBuiltData& ObjectBuiltData);
	void Add(const FObjectBuiltData& ObjectBuiltData);
	bool CanRedo() const;
	UE::Math::TVector<double> ReturnActorOldLocation();
};

