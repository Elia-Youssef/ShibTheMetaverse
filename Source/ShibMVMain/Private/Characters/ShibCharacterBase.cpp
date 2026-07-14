// Copyright Shib LLC.

#include "Characters/ShibCharacterBase.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "ShibAvatarBuilderSubsystem.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/SphereComponent.h"
#include "ShibSave.h"
#include "ShibSaveSubsystem.h"
#include "Characters/ShibCharacterMovementComponent.h"
#include "Game/ShibGameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Player/ShibPlayerController.h"
#include "Player/ShibPlayerState.h"

AShibCharacterBase::AShibCharacterBase()
{
	PrimaryActorTick.bCanEverTick = true;

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	if(UCharacterMovementComponent* CharMoveComp = GetCharacterMovement())
	{
		CharMoveComp->bOrientRotationToMovement = true;
		CharMoveComp->RotationRate = FRotator(0.0f, 500.0f, 0.0f);
		CharMoveComp->JumpZVelocity = 500.f;
		CharMoveComp->AirControl = 0.35f;
		CharMoveComp->MaxWalkSpeed = 200.f;
		CharMoveComp->MinAnalogWalkSpeed = 20.f;
		CharMoveComp->BrakingDecelerationWalking = 2000.f;
		CharMoveComp->SetWalkableFloorAngle(60.f);
		CharMoveComp->MaxStepHeight = 90.f;
	}
	
	// NAME
	NameSphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("NameSphereComponent"));
	NameSphereComponent->SetupAttachment(RootComponent);
	NameSphereComponent->SetSphereRadius(1200.0f);
	NameSphereComponent->CanCharacterStepUpOn = ECB_No;

	NameSphereComponent->OnComponentBeginOverlap.AddDynamic(this, &AShibCharacterBase::OnNameOverlapBegin);
	NameSphereComponent->OnComponentEndOverlap.AddDynamic(this, &AShibCharacterBase::OnNameOverlapEnd);

	// VOICE
	VoiceSphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("VoiceSphereComponent"));
	VoiceSphereComponent->SetupAttachment(RootComponent);
	VoiceSphereComponent->SetSphereRadius(300.0f);
	VoiceSphereComponent->CanCharacterStepUpOn = ECB_No;

	VoiceSphereComponent->OnComponentBeginOverlap.AddDynamic(this, &AShibCharacterBase::OnVoiceOverlapBegin);
	VoiceSphereComponent->OnComponentEndOverlap.AddDynamic(this, &AShibCharacterBase::OnVoiceOverlapEnd);

	// CAMERA
	SpringArmComponent = CreateOptionalDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComponent"));
	if (IsValid(SpringArmComponent))
	{
		SpringArmComponent->SetupAttachment(RootComponent);
		SpringArmComponent->SetRelativeLocation(FVector(0.0f, 0.0f, 58.5f));
		SpringArmComponent->bUsePawnControlRotation = true;
		SpringArmComponent->bEnableCameraLag = false;
		SpringArmComponent->TargetArmLength = 400.0f;

		CameraComponent = CreateOptionalDefaultSubobject<UCameraComponent>(TEXT("CameraComponent"));
		CameraComponent->SetupAttachment(SpringArmComponent);
		CameraComponent->bUsePawnControlRotation = false;

		if (APlayerCameraManager* CameraManager = UGameplayStatics::GetPlayerCameraManager(GetWorld(), 0))
		{
			CameraManager->ViewPitchMax = 70.f; // limit camera pitch rotation to not be able to look under the character
		}
	}
}

#pragma region Animation

void AShibCharacterBase::OnRep_AnimationState()
{
	OnAnimationStateChanged.Broadcast(AnimationState);

	PreviousAnimationState = AnimationState;
}

void AShibCharacterBase::SetAnimationState(EAnimationState NewAnimationState)
{
	AnimationState = NewAnimationState;
	OnRep_AnimationState();
}

TEnumAsByte<EAnimationState> AShibCharacterBase::GetAnimationState() const
{
	return AnimationState;
}

#pragma endregion Animation

void AShibCharacterBase::BeginPlay()
{
	Super::BeginPlay();
	
	ShibGI = GetGameInstance<UShibGameInstance>();	
	
	if (ShibGI = GetGameInstance<UShibGameInstance>(); ShibGI)
	{
		// demo
		// if (IsLocallyControlled())
		// {
			ApplyAvatarDataFromString(FString());
		// }
	}
}

void AShibCharacterBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	OnAnimationStateChanged.Clear();
}

void AShibCharacterBase::OnReplicationReady()
{
	if(IsLocallyControlled() || !HasAuthority())
	{
		if (ShibGI = GetGameInstance<UShibGameInstance>(); ShibGI)
		{
			// string data is be invalid for replication, it will apply the default avatar
			Server_SendAvatarInfo(ShibGI->AvatarInfo.AvatarData.IsEmpty() ? "-" : ShibGI->AvatarInfo.AvatarData);
		}
	}
}

void AShibCharacterBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (auto* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		//Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		//Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ThisClass::Move);
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Completed, this, &ThisClass::MoveCompleted);
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Started, this, &ThisClass::Sprint);

		//Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ThisClass::Look);

		//Interacting
		EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Triggered, this, &ThisClass::Interact);
	}
}

void AShibCharacterBase::ApplyAvatarDataFromString(const FString& NewAvatarData)
{
	if (auto* AvatarSubsystem = GetGameInstance()->GetSubsystem<UShibAvatarBuilderSubsystem>())
	{
		AvatarSubsystem->LoadReplicatedAvatar(this, NewAvatarData);
	}
}

void AShibCharacterBase::Server_SendAvatarInfo(const FString& NewAvatarInfo)
{
	AvatarInfo = NewAvatarInfo;
}

void AShibCharacterBase::OnRep_AvatarInfo()
{
	ApplyAvatarDataFromString(AvatarInfo);
}

void AShibCharacterBase::Move(const FInputActionValue& Value)
{
	if (!Controller) return;

	const FVector2d MovementVector = Value.Get<FVector2d>();

	const FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);
	const FVector ForwardDirection(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X));
	const FVector RightDirection(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y));

	AddMovementInput(ForwardDirection, MovementVector.Y);
	AddMovementInput(RightDirection, MovementVector.X);
}

void AShibCharacterBase::Look(const FInputActionValue& Value)
{
	if (!Controller) return;

	bool bInvertY = false;
	float Sensitivity = 1.f;
	if (auto* Save = GetGameInstance()->GetSubsystem<UShibSaveSubsystem>())
	{
		Sensitivity = Save->ShibSave->ShibSettings.MouseSensitivity * 2; // 1 is default, sensitivity is 0 -> 1, so added value should be more/less 1, so *2
		bInvertY = Save->ShibSave->ShibSettings.bInvertYAxis;
	}

	const auto LookVector = Value.Get<FVector2d>();
	AddControllerYawInput(LookVector.X * Sensitivity);
	AddControllerPitchInput(LookVector.Y * Sensitivity * (bInvertY ? 1 : -1));
}

#pragma region Sprint

void AShibCharacterBase::MoveCompleted(const FInputActionValue& Value)
{
	if (UShibCharacterMovementComponent* ShibCharacterMovementComponent = Cast<UShibCharacterMovementComponent>(GetCharacterMovement()))
	{
		ShibCharacterMovementComponent->ToggleSprint(false);
	}
}

void AShibCharacterBase::Sprint(const FInputActionValue& Value)
{
	if (UShibCharacterMovementComponent* ShibCharacterMovementComponent = Cast<UShibCharacterMovementComponent>(GetCharacterMovement()))
	{
		ShibCharacterMovementComponent->ToggleSprint( ! ShibCharacterMovementComponent->IsSprinting());
	}
}

void AShibCharacterBase::OnRep_bIsTalking()
{
	OnPlayerIsTalking(bIsTalking);
}

void AShibCharacterBase::TogglePlayerTalking(bool bNewIsTalking)
{
	bIsTalking = bNewIsTalking;
}

void AShibCharacterBase::ToggleSwim(bool bNewSwim)
{
	if (UShibCharacterMovementComponent* ShibCharacterMovementComponent = Cast<UShibCharacterMovementComponent>(GetCharacterMovement()))
	{
		ShibCharacterMovementComponent->ToggleSwim(bNewSwim);
	}
}

// void AShibCharacterBase::Sprint(const FInputActionValue& Value)
// {
// 	if (!GetCharacterMovement()) return;
//
// 	if(CheckSprintCooldown()) return; // don't proceed to RPC if on cooldown
// 	
// 	bSprinting = !bSprinting;
// 	if (bSprinting)
// 	{
// 		SpeedBeforeSprint = GetCharacterMovement()->MaxWalkSpeed;
// 		GetCharacterMovement()->MaxWalkSpeed = SprintSpeed;
// 		Server_SetMovementSpeed(SprintSpeed);
// 	}
// 	else
// 	{
// 		GetCharacterMovement()->MaxWalkSpeed = SpeedBeforeSprint;
// 		Server_SetMovementSpeed(SpeedBeforeSprint);
// 	}
// }
//
// bool AShibCharacterBase::CheckSprintCooldown()
// {
// 	if(const UWorld* World = GetWorld())
// 	{
// 		if(World->GetTimerManager().IsTimerActive(Timer_SprintCooldown))
// 		{
// 			return true;
// 		}
// 		
// 		World->GetTimerManager().SetTimer(Timer_SprintCooldown, SprintCooldown, false);
// 	}
//
// 	return false;
// }
//
// void AShibCharacterBase::Server_SetMovementSpeed_Implementation(float Speed) //TODO using a parameter makes it potentially hackable. Better to declare a var in the header
// {
// 	Multi_SetMovementSpeed(Speed);
// }
//
// void AShibCharacterBase::Multi_SetMovementSpeed_Implementation(float Speed)
// {
// 	if (GetCharacterMovement())
// 	{
// 		GetCharacterMovement()->MaxWalkSpeed = FMath::Min(SprintSpeed, Speed);
// 	}
// }

#pragma endregion Sprint

void AShibCharacterBase::Interact_Implementation()
{
	; // Implement any interaction logic if any to be executed on interaction input pressed
}

void AShibCharacterBase::OnVoiceOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
                                        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
                                        const FHitResult& SweepResult)
{
	if (!IsValid(OtherActor) || OtherActor == this || HasAuthority()) return;

	if (auto ShibCharacter = Cast<AShibCharacterBase>(OtherActor); !ShibCharacter) return;

	if (AShibPlayerController* ShibPC = Cast<AShibPlayerController>(Controller))
	{
		UE_LOG(LogTemp, Log, TEXT("%hs: Overlap Begin with %s"), __FUNCTION__, *OtherActor->GetName());

		if (AShibPlayerState* PS = Cast<AShibPlayerState>(ShibPC->PlayerState))
		{
			ShibPC->UnMuteRemotePlayer(PS->GetUniqueId().GetUniqueNetId());
		}
	}
}

void AShibCharacterBase::OnVoiceOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
                                      UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (!IsValid(OtherActor) || OtherActor == this || HasAuthority()) return;

	if (auto ShibCharacter = Cast<AShibCharacterBase>(OtherActor); !ShibCharacter) return;

	if (AShibPlayerController* ShibPC = Cast<AShibPlayerController>(Controller))
	{
		UE_LOG(LogTemp, Log, TEXT("%hs: Overlap End with %s"), __FUNCTION__,
		       *OtherActor->GetName());
		if (AShibPlayerState* PS = Cast<AShibPlayerState>(ShibPC->PlayerState))
		{
			ShibPC->MuteRemotePlayer(PS->GetUniqueId().GetUniqueNetId());
		}
	}
}

void AShibCharacterBase::OnNameOverlapBegin(class UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!IsValid(OtherActor) || OtherActor == this || HasAuthority()) return;

	AShibCharacterBase* ShibCharacter;
	if (ShibCharacter = Cast<AShibCharacterBase>(OtherActor); !ShibCharacter) return;

	SetPlayerNameVisibility(ShibCharacter, true);
}

void AShibCharacterBase::OnNameOverlapEnd(class UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (!IsValid(OtherActor) || OtherActor == this || HasAuthority()) return;

	AShibCharacterBase* ShibCharacter;
	if (ShibCharacter = Cast<AShibCharacterBase>(OtherActor); !ShibCharacter) return;
	
	SetPlayerNameVisibility(ShibCharacter, false);
}

void AShibCharacterBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AShibCharacterBase, AvatarInfo);
	DOREPLIFETIME(AShibCharacterBase, bIsTalking);
	DOREPLIFETIME(AShibCharacterBase, AnimationState);
}
