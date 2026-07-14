// Copyright Shib LLC.

#pragma once

#include "CoreMinimal.h"
#include "ShibAvatarCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "ShibCharacterBase.generated.h"

class UShibGameInstance;
struct FInputActionValue;
class UInputAction;
class UInputMappingContext;
class USpringArmComponent;
class UCameraComponent;

#pragma region Enums

UENUM(BlueprintType)
enum EAnimationState
{
	AS_None				UMETA(DisplayName = "None"),
	AS_FishingStart		UMETA(DisplayName = "Fishing Start"),
	AS_FishingEnd		UMETA(DisplayName = "Fishing End"),
	AS_Dancing			UMETA(DisplayName = "Dancing"),
};

#pragma endregion Enunms

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnTravelPodOverlap, bool, bBegin, const TArray<FName>&, Levels, const TArray<FName>&, LevelsToIgnore);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAnimationStateChanged, EAnimationState, NewAnimationState);

UCLASS(Blueprintable)
class SHIBMVMAIN_API AShibCharacterBase : public AShibAvatarCharacter
{
	GENERATED_BODY()

public:
	AShibCharacterBase();

	// Default mapping contexts for this character
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TArray<TObjectPtr<UInputMappingContext>> DefaultMappingContexts;
	
	UPROPERTY(BlueprintAssignable)
	FOnTravelPodOverlap OnTravelPodOverlap;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
	class USphereComponent* VoiceSphereComponent;
	UFUNCTION()
	void OnVoiceOverlapBegin(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);
	UFUNCTION()
	void OnVoiceOverlapEnd(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
	USphereComponent* NameSphereComponent;
	UFUNCTION()
	void OnNameOverlapBegin(class UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);
	UFUNCTION()
	void OnNameOverlapEnd(class UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
	
	void OnReplicationReady();
	
	UFUNCTION(BlueprintGetter)
	bool GetIsTeleporting(){ return bIsTeleporting; }

#pragma region Animation

	UPROPERTY(ReplicatedUsing=OnRep_AnimationState)
	TEnumAsByte<EAnimationState> AnimationState = AS_None;

	UPROPERTY(BlueprintReadOnly) // Used for when exiting animation state to None
	TEnumAsByte<EAnimationState> PreviousAnimationState = AS_None;

	UFUNCTION()
	void OnRep_AnimationState();
	
	UFUNCTION(BlueprintCallable)
	void SetAnimationState(EAnimationState NewAnimationState);

	UFUNCTION(BlueprintPure)
	TEnumAsByte<EAnimationState> GetAnimationState() const;

	UPROPERTY(BlueprintAssignable)
	FOnAnimationStateChanged OnAnimationStateChanged;
	
#pragma endregion Animation
	
protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent);

	// demo
	// UFUNCTION(Reliable, Server)
	void Server_SendAvatarInfo(const FString& NewAvatarInfo);

	void ApplyAvatarDataFromString(const FString& NewAvatarData);
	
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_AvatarInfo)
	FString AvatarInfo;

	UFUNCTION()
	void OnRep_AvatarInfo();
	
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void MoveCompleted(const FInputActionValue& Value);
	void Sprint(const FInputActionValue& Value);
	
	// void Multi_SetMovementSpeed(float Speed);

	/** Handles interaction input event, Made native for added blueprint logic */
	UFUNCTION(BlueprintNativeEvent)
	void Interact();

	UFUNCTION(BlueprintImplementableEvent)
	void SetPlayerNameVisibility(AShibCharacterBase* Player, bool bShow);

public:
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Camera")
	TObjectPtr<USpringArmComponent> SpringArmComponent;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Camera")
	TObjectPtr<UCameraComponent> CameraComponent;
	
protected:
	// ===== INPUT =====

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* JumpAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* MoveAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* SprintAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* LookAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* InteractAction;

	UPROPERTY()
	UShibGameInstance* ShibGI;

	UPROPERTY(BlueprintReadWrite)
	bool bIsTeleporting = true;

	UPROPERTY(ReplicatedUsing=OnRep_bIsTalking)
	bool bIsTalking = false;

	UFUNCTION()
	void OnRep_bIsTalking();

public:
	UFUNCTION(BlueprintCallable)
	void TogglePlayerTalking(bool bNewIsTalking);

protected:
	UFUNCTION(BlueprintImplementableEvent)
	void OnPlayerIsTalking(bool bNewIsTalking);
	
#pragma region Swim

public:
	UFUNCTION(BlueprintCallable)
	void ToggleSwim(bool bNewSwim);
	
	UFUNCTION(BlueprintImplementableEvent)
	void OnToggleSwim(bool bNewSwim);
	
#pragma endregion Swim
};
