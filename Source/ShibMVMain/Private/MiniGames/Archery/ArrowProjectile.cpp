// Copyright Shiba Inu Games LLC.


#include "MiniGames/Archery/ArrowProjectile.h"

#include "Components/BoxComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "MiniGames/Archery/ArcheryTarget.h"


AArrowProjectile::AArrowProjectile()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	BoxComponent = CreateDefaultSubobject<UBoxComponent>("BoxComponent");
	SetRootComponent(BoxComponent);
	BoxComponent->InitBoxExtent(FVector(30.f, 2.f, 2.f));
	BoxComponent->SetCollisionResponseToAllChannels(ECR_Block);
	BoxComponent->SetCollisionResponseToChannel(ECC_Visibility, ECR_Ignore);
	BoxComponent->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	BoxComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>("ProjectileMovement");
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->InitialSpeed = 500.f;
	ProjectileMovement->MaxSpeed = 500.f;
	ProjectileMovement->ProjectileGravityScale = 0.5f;
}

void AArrowProjectile::BeginPlay()
{
	Super::BeginPlay();

	BoxComponent->OnComponentHit.AddDynamic(this, &AArrowProjectile::OnComponentHit);

	FTimerHandle DestroyHandle;
	GetWorldTimerManager().SetTimer(DestroyHandle, [this]()
	{
		Destroy();
	}, 8.f, false);
}

void AArrowProjectile::OnComponentHit(UPrimitiveComponent* HitComponent, AActor* OtherActor,
                                      UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (!HasAuthority()) return;
	
	auto* Target = Cast<AArcheryTarget>(OtherActor);
	if (!IsValid(Target)) return;

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("Score: %d"), Target->Score));
	}

	Target->DestroyTarget();
	Destroy();
}
