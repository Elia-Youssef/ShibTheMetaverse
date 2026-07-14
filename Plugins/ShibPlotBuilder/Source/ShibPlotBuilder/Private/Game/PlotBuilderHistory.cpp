
#include "Game/PlotBuilderHistory.h"


bool UPlotBuilderHistory::Redo(FObjectBuiltData& ObjectBuiltData)
{
	if (!UndoRedoStackObjectsBuilt.IsValidIndex(CurrentHistoryPosition + 1)) 
	{
		UE_LOG(LogTemp, Error, TEXT("Cannot redo: at end of history or empty stack"));
		return false;
	}
    
	CurrentHistoryPosition++;
	ObjectBuiltData = UndoRedoStackObjectsBuilt[CurrentHistoryPosition];
	UE_LOG(LogTemp, Log, TEXT("Redo change [%d]"), CurrentHistoryPosition);
	return true;
}

bool UPlotBuilderHistory::Undo(FObjectBuiltData& ObjectBuiltData, EObjectInteractionState InteractionState)
{
	if (!UndoRedoStackObjectsBuilt.IsValidIndex(CurrentHistoryPosition)) 
	{
		UE_LOG(LogTemp, Error, TEXT("Cannot undo: at beginning of history or empty stack"));
		return false;
	}

	if (InteractionState == EObjectInteractionState::Delete)
	{
		ObjectBuiltData = UndoRedoStackObjectsBuilt[CurrentHistoryPosition];
		UE_LOG(LogTemp, Log, TEXT("Undo change [%d]"), CurrentHistoryPosition);
		return true;
	}
	else
	{
		ObjectBuiltData = UndoRedoStackObjectsBuilt[CurrentHistoryPosition];
		CurrentHistoryPosition--;
		UE_LOG(LogTemp, Log, TEXT("Undo change [%d]"), CurrentHistoryPosition);
		return true;
	}
}

void UPlotBuilderHistory::Add(const FObjectBuiltData& ObjectBuiltData)
{
	if (!UndoRedoStackObjectsBuilt.IsEmpty())
	{
		if (ObjectBuiltData.ID == UndoRedoStackObjectsBuilt[CurrentHistoryPosition].ID)
		{
			UE_LOG(LogTemp, Log, TEXT("Attempting to add duplicate data, returning.."));
			return;
		}
	
		if (UndoRedoStackObjectsBuilt.Num() - 1 == CurrentHistoryPosition)
		{
			if (UndoRedoStackObjectsBuilt.Num() == CurrentHistoryPosition)
				UndoRedoStackObjectsBuilt.RemoveAt(0);
			else
				CurrentHistoryPosition++;
		}
		else
		{
			UndoRedoStackObjectsBuilt.RemoveAtSwap(CurrentHistoryPosition + 1, UndoRedoStackObjectsBuilt.Num() - CurrentHistoryPosition - 1);
			CurrentHistoryPosition++;
		}
	}
	else
	{
		CurrentHistoryPosition++;
	}
	UndoRedoStackObjectsBuilt.Add(ObjectBuiltData);
	UE_LOG(LogTemp, Log, TEXT("Added change [%d], Size of Stack [%d]"), CurrentHistoryPosition, UndoRedoStackObjectsBuilt.Num());
}

void UPlotBuilderHistory::Replace(FObjectBuiltData& ObjectBuiltData)
{
	UndoRedoStackObjectsBuilt[CurrentHistoryPosition] = ObjectBuiltData;
}

FVector UPlotBuilderHistory::ReturnActorOldLocation()
{
	return UndoRedoStackObjectsBuilt[CurrentHistoryPosition].Transform.GetLocation();
}
