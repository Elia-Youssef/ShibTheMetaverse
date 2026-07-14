// Copyright Shiba Inu Games LLC.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Minigames/ShibGameComponent.h"
#include "ArcheryComponent.generated.h"


class AArrowProjectile;
struct FInputActionValue;
class UInputAction;
class UInputMappingContext;
class AShibCharacterBase;
class AShibPlayerController;

UCLASS(Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class SHIBMVMAIN_API UArcheryComponent : public UShibGameComponent
{
	GENERATED_BODY()

public:
	UArcheryComponent();

	virtual bool ActivateGame() override;
	virtual bool CleanupAndDestroy() override;

	UPROPERTY(BlueprintReadOnly)
	bool bIsInitialized = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Camera")
	float FocusCameraFOV = 60.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Camera")
	float FocusArmLength = 150.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Camera")
	FVector FocusSocketOffset = FVector(0.f, 30.f, 0.f);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	UInputMappingContext* ArcheryInputContext = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	UInputAction* ShootAction = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Input")
	float MaxHoldTime = 1.5f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Projectile")
	TSubclassOf<AArrowProjectile> ProjectileClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Projectile")
	float MinProjectileVelocity = 500.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Projectile")
	float MaxProjectileVelocity = 2500.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "User Interface")
	TSubclassOf<UUserWidget> TargetUiClass;

	UPROPERTY(BlueprintReadWrite, Category = "User Interface")
	UUserWidget* TargetUI;

protected:
	virtual void BeginPlay() override;

	void OnStartShooting(const FInputActionValue& Value);
	void OnEndShooting(const FInputActionValue& Value);

	void SpawnProjectile(float InputTimeHeld);

private:
	UPROPERTY()
	AShibPlayerController* ShibPC = nullptr;

	UPROPERTY()
	AShibCharacterBase* ShibCharacter = nullptr;

	float InitialCameraFOV = 0.0f;
	float InitialSpringArmLength = 0.0f;
	FVector InitialSpringArmSocketOffset = FVector::ZeroVector;

	double InputStartTime = 0.f;
};
