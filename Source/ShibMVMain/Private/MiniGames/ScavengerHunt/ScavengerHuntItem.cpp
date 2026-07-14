// Copyright Shiba Inu Games LLC.


#include "MiniGames/ScavengerHunt/ScavengerHuntItem.h"

#include "Components/SphereComponent.h"
#include "MiniGames/ScavengerHunt/ScavengerHuntComponent.h"


AScavengerHuntItem::AScavengerHuntItem()
{
	PrimaryActorTick.bCanEverTick = false;

	Sphere = CreateDefaultSubobject<USphereComponent>("Sphere");
	Sphere->SetMobility(EComponentMobility::Static);
	Sphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	Sphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	SetRootComponent(Sphere);
}

void AScavengerHuntItem::BeginPlay()
{
	Super::BeginPlay();
}

void AScavengerHuntItem::Interact_Implementation(AActor* InteractionInstigator)
{
	IInteractInterface::Interact_Implementation(InteractionInstigator);

	if (AActor* InstigatorOwner = InteractionInstigator->GetOwner()) // controller
	{
		if (auto* Component = Cast<UScavengerHuntComponent>(
			InstigatorOwner->GetComponentByClass(UScavengerHuntComponent::StaticClass())))
		{
			Component->CollectedItems.Add(Details);
			Destroy();
		}
	}
}
