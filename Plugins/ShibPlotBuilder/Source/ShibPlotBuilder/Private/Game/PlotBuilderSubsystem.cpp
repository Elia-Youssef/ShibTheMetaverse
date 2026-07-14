// Copyright Shiba Inu Games LLC.

#include "Game/PlotBuilderSubsystem.h"
#include "Game/BuildSave.h"
#include "Tools/BuildTypes.h"

void UPlotBuilderSubsystem::LoadPlot(const FOnPlotUpdating& UpdateCallback, const UDataTable* ObjectsDataTable,
	const FTransform& Origin, const FString& Json, TArray<FObjectBuiltData>& ExistingObjects)
{
	TArray<FObjectBuiltData> Objects = UBuildSave::DeserializeJsonToObjectBuiltData(Json);

	// Flag to know when things are changing on the plot
	bool bPlotIsDirty=false;

	// try spawning loaded objects
	for (FObjectBuiltData& Object : Objects)
	{
		// if the object is already spawned
		if (const int32 i = ExistingObjects.Find(Object); i != INDEX_NONE)
		{
			// set the objects built actor pointer to the existing actor
			Object.BuiltActor = ExistingObjects[i].BuiltActor;
			continue;
		};
		// if not, spawn it, setting the built actor pointer
		
		const FObjectDetails* Row = ObjectsDataTable->FindRow<FObjectDetails>(Object.ClassToSpawnRowName, "");
		if (!Row) continue;

		// set relative transform to the origin
		FTransform SpawnAt;
		SpawnAt.SetLocation(Object.Transform.GetLocation() + Origin.GetLocation());
		SpawnAt.SetRotation(Object.Transform.GetRotation() * Origin.GetRotation());
		SpawnAt.SetScale3D(Object.Transform.GetScale3D());

		if (!bPlotIsDirty) // Notify when things are changing for the first time on the plot
		{
			bPlotIsDirty = true;
			UpdateCallback.ExecuteIfBound();
		}

		Object.BuiltActor = GetWorld()->SpawnActorDeferred<ABaseBuildActor>(Row->ClassToSpawn, SpawnAt);
		Object.BuiltActor->FinishSpawning(SpawnAt);
	}

	for (FObjectBuiltData& ExistingObject : ExistingObjects)
	{
		// if there's an existing object, but is not in the loaded array, then it should be destroyed
		if (const int32 i = Objects.Find(ExistingObject); i == INDEX_NONE)
		{
			if (ExistingObject.BuiltActor)
			{
				if (!bPlotIsDirty) // Notify when things are changing for the first time on the plot
				{
					bPlotIsDirty = true;
					UpdateCallback.ExecuteIfBound();
				}
				
				ExistingObject.BuiltActor->Destroy();
			}
		}
	}

	// update the existing objects array with the newly loaded array
	ExistingObjects = Objects;
}