// Copyright Shiba Inu Games LLC.


#include "MiniGames/Archery/ArcheryComponent.h"

#include "EnhancedInputComponent.h"
#include "Camera/CameraComponent.h"
#include "Characters/ShibCharacterBase.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "MiniGames/Archery/ArrowProjectile.h"
#include "Player/ShibPlayerController.h"


UArcheryComponent::UArcheryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

bool UArcheryComponent::ActivateGame()
{
	if (!Super::ActivateGame())
	{
		return false;
	}

	if (bIsInitialized || !IsValid(ShibPC) || !IsValid(ShibCharacter))
	{
		return false;
	}

	bIsInitialized = true;

	// origin values
	InitialSpringArmLength = ShibCharacter->SpringArmComponent->TargetArmLength;
	InitialSpringArmSocketOffset = ShibCharacter->SpringArmComponent->SocketOffset;

	// update the values
	ShibCharacter->SpringArmComponent->TargetArmLength = FocusArmLength;
	ShibCharacter->SpringArmComponent->SocketOffset = FocusSocketOffset;
	ShibCharacter->SpringArmComponent->bEnableCameraLag = false;
	ShibCharacter->bUseControllerRotationYaw = true;

	// update contexts
	ShibPC->ToggleMappingContexts(true, {ArcheryInputContext});

	// ui
	if (TargetUiClass)
	{
		TargetUI = CreateWidget<UUserWidget>(ShibPC, TargetUiClass);
		TargetUI->AddToViewport();
	}

	return true;
}

bool UArcheryComponent::CleanupAndDestroy()
{
	if (!Super::CleanupAndDestroy())
	{
		return false;
	}

	if (!bIsInitialized || !IsValid(ShibPC) || !IsValid(ShibCharacter))
	{
		return false;
	}
	bIsInitialized = false;

	ShibCharacter->SpringArmComponent->TargetArmLength = InitialSpringArmLength;
	ShibCharacter->SpringArmComponent->SocketOffset = InitialSpringArmSocketOffset;
	ShibCharacter->SpringArmComponent->bEnableCameraLag = true;
	ShibCharacter->bUseControllerRotationYaw = false;

	ShibPC->ToggleMappingContexts(false, {ArcheryInputContext});

	if (TargetUI)
	{
		TargetUI->RemoveFromParent();
	}

	return true;
}

void UArcheryComponent::BeginPlay()
{
	Super::BeginPlay();

	ShibPC = Cast<AShibPlayerController>(GetOwner());
	if (!ShibPC) return;

	ShibCharacter = Cast<AShibCharacterBase>(ShibPC->GetPawn());
	if (!ShibCharacter) return;

	if (auto* IC = Cast<UEnhancedInputComponent>(ShibPC->InputComponent))
	{
		if (!ShootAction) return;
		IC->BindAction(ShootAction, ETriggerEvent::Started, this, &ThisClass::OnStartShooting);
		IC->BindAction(ShootAction, ETriggerEvent::Canceled, this, &ThisClass::OnEndShooting);
		IC->BindAction(ShootAction, ETriggerEvent::Completed, this, &ThisClass::OnEndShooting);
	}
}

void UArcheryComponent::OnStartShooting(const FInputActionValue& Value)
{
	InputStartTime = FPlatformTime::Seconds();

	InitialCameraFOV = ShibCharacter->CameraComponent->FieldOfView;
	ShibCharacter->CameraComponent->SetFieldOfView(FocusCameraFOV);
}

void UArcheryComponent::OnEndShooting(const FInputActionValue& Value)
{
	float ElapsedTime = FPlatformTime::Seconds() - InputStartTime;
	ShibCharacter->CameraComponent->SetFieldOfView(InitialCameraFOV);
	SpawnProjectile(ElapsedTime);
}

void UArcheryComponent::SpawnProjectile(float InputTimeHeld)
{
	FActorSpawnParameters SpawnParams;
	AArrowProjectile* Projectile = GetWorld()->SpawnActorDeferred<AArrowProjectile>(
		ProjectileClass, ShibCharacter->CameraComponent->GetComponentTransform());

	if (!Projectile) return;

	const float TimeHeld = FMath::Min(InputTimeHeld, MaxHoldTime);
	const float TimeHeldPercentage = TimeHeld / MaxHoldTime;

	const float VelocityDiff = MaxProjectileVelocity - MinProjectileVelocity;
	const float InputVelocity = VelocityDiff * TimeHeldPercentage;

	const float FinalVelocity = MinProjectileVelocity + InputVelocity;

	Projectile->ProjectileMovement->InitialSpeed = FinalVelocity;
	Projectile->ProjectileMovement->MaxSpeed = FinalVelocity;

	Projectile->FinishSpawning(ShibCharacter->CameraComponent->GetComponentTransform());
}
