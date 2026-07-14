// Copyright Shiba Inu Games LLC.


#include "MiniGames/Archery/ArcherySpot.h"


AArcherySpot::AArcherySpot()
{
	PrimaryActorTick.bCanEverTick = false;

}

void AArcherySpot::BeginPlay()
{
	Super::BeginPlay();
	
}

void AArcherySpot::Interact_Implementation(AActor* InteractionInstigator)
{
	IInteractInterface::Interact_Implementation(InteractionInstigator);

	// TODO: enable the component here
}
