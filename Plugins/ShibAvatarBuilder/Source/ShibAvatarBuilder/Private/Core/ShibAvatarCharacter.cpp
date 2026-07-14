// Fill out your copyright notice in the Description page of Project Settings.


#include "ShibAvatarCharacter.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/Controller.h"
#include "ShibLibrary.h"
#include "ShibAvatarBuilderSubsystem.h"
#include "UISubsystem.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ShibAvatarCharacter)

// Sets default values
AShibAvatarCharacter::AShibAvatarCharacter()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	GetCapsuleComponent()->SetCapsuleHalfHeight(90.0f);
	GetCapsuleComponent()->SetCapsuleRadius(35.0f);

	GetMesh()->SetRelativeLocation(FVector(.0f, .0f, -89.f));
	GetMesh()->SetRelativeRotation(FRotator(.0f, -90.f, .0f));

	CloseFar = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("CloseFar"));
	CloseFar->AttachToComponent(GetCapsuleComponent(), FAttachmentTransformRules::KeepRelativeTransform);
	CloseFar->SetRelativeLocation(FVector(200.0f, 0.0f, 5.0f));
	CloseFar->SetRelativeRotation(FRotator(2.0f, -180.0f, 0.0f));
	CloseFar->ProjectionType = ECameraProjectionMode::Perspective;
	CloseFar->FOVAngle = 45.0f;

	CloseUp = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("CloseUp"));
	CloseUp->AttachToComponent(GetCapsuleComponent(), FAttachmentTransformRules::KeepRelativeTransform);
	CloseUp->SetRelativeLocation(FVector(70.013705f, 0.0f, 74.476640f));
	CloseUp->SetRelativeRotation(FRotator(-3.0f, -180.0f, 0.0f));
	CloseUp->ProjectionType = ECameraProjectionMode::Perspective;
	CloseUp->FOVAngle = 75.0f;

	LeftArm = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("LeftArm"));
	LeftArm->AttachToComponent(GetMesh(), FAttachmentTransformRules::KeepRelativeTransform, NAME_None);

	RightArm = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("RightArm"));
	RightArm->AttachToComponent(GetMesh(), FAttachmentTransformRules::KeepRelativeTransform, NAME_None);

	Back = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Back"));
	Back->AttachToComponent(GetMesh(), FAttachmentTransformRules::KeepRelativeTransform, NAME_None);

	HeadTop = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("HeadTop"));
	HeadTop->AttachToComponent(GetMesh(), FAttachmentTransformRules::KeepRelativeTransform, NAME_None);

	Neck = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Neck"));
	Neck->AttachToComponent(GetMesh(), FAttachmentTransformRules::KeepRelativeTransform, NAME_None);

	Face = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Face"));
	Face->AttachToComponent(GetMesh(), FAttachmentTransformRules::KeepRelativeTransform, NAME_None);

	Ears = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Ears"));
	Ears->AttachToComponent(GetMesh(), FAttachmentTransformRules::KeepRelativeTransform, NAME_None);

	FullBody = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FullBody"));
	FullBody->AttachToComponent(GetMesh(), FAttachmentTransformRules::KeepRelativeTransform, NAME_None);

	Shoes = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Shoes"));
	Shoes->AttachToComponent(GetMesh(), FAttachmentTransformRules::KeepRelativeTransform, NAME_None);

	Bottoms = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Bottoms"));
	Bottoms->AttachToComponent(GetMesh(), FAttachmentTransformRules::KeepRelativeTransform, NAME_None);

	Hair = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Hair"));
	Hair->AttachToComponent(GetMesh(), FAttachmentTransformRules::KeepRelativeTransform, NAME_None);

	Top = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Top"));
	Top->AttachToComponent(GetMesh(), FAttachmentTransformRules::KeepRelativeTransform, NAME_None);

	DefaultCapsuleHeight = GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight();
	DefaultMeshZ = GetBody()->GetRelativeLocation().Z;
	// TEMP FIX;
	DefaultBodyMat = GetMesh()->GetMaterial(0);
	DefaultEyeMat = GetMesh()->GetMaterial(3);
}

// Called when the game starts or when spawned
void AShibAvatarCharacter::BeginPlay()
{
	Super::BeginPlay();
	const TObjectPtr<UUISubsystem> MyUISubsystem = GetGameInstance()->GetSubsystem<UUISubsystem>();
	SetUISubsystem(MyUISubsystem);
	const TObjectPtr<UShibAvatarBuilderSubsystem> ShibSubsystem = GetGameInstance()->GetSubsystem<
		UShibAvatarBuilderSubsystem>();
	
	GetBodyDMI();
	GetEyesDMI();
	GetLeftArmDMI();
	GetRightArmDMI();
	GetBackDMI();
	GetHeadTopDMI();
	GetNeckDMI();
	GetFaceDMI();
	GetEarsDMI();
	GetFullBodyTopDMI();
	GetFullBodyBottomDMI();
	GetShoesDMI();
	GetBottomsDMI();
	GetHairDMI();
	GetTopDMI();
}

void AShibAvatarCharacter::ResetAvatar(bool resetTextures)
{
	UE_LOG(LogShib, Log, TEXT("%hs - Resetting Avatar"), __FUNCTION__);

	SetLeftArm();
	SetRightArm();
	SetBottom();
	SetEars();
	SetFace();
	SetBack();
	SetHair();
	SetNeck();
	SetShoes();
	SetTop();
	SetFullBody();
	SetHeadTop();
	
	GetBody()->ClearMorphTargets();

	if(IsValid(DefaultBodyMat) && IsValid(DefaultEyeMat) && resetTextures)
	{
		BodyDMI = GetMesh()->CreateDynamicMaterialInstance(0,DefaultBodyMat);
		EyesDMI = GetMesh()->CreateDynamicMaterialInstance(3,DefaultEyeMat);	
	}
	else
	{
		UE_LOG(LogShib, Warning, TEXT("%hs - Default materials invalid"), __FUNCTION__);
	}
	
}

void AShibAvatarCharacter::Server_SetNewHeightFromScale(float scale)
{
	SetNewHeightFromScale(scale);
}

void AShibAvatarCharacter::SetNewHeightFromScale(float scale)
{
	// Make sure DefaultCapsule Height has been initialized first
	

	float newX = UKismetMathLibrary::MapRangeClamped(scale, 0.0f, 1.0f, 0.9f, 1.1f);
	float newY = UKismetMathLibrary::MapRangeClamped(scale, 0.0f, 1.0f, 0.9f, 1.1f);
	float newZ = UKismetMathLibrary::MapRangeClamped(scale, 0.0f, 1.0f, 0.8f, 1.2f);
	
	GetBody()->SetRelativeScale3D(FVector(newX,newY,newZ));
	
	float newHalfHeight = DefaultCapsuleHeight*newZ;
	
	// Capsule Location handled automatically when scaling down but for some reason not when scaling up
	if(newHalfHeight > GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight())
	{
		FVector CapsuleLocation = GetCapsuleComponent()->GetRelativeLocation();
		CapsuleLocation.Z += UKismetMathLibrary::Abs(newHalfHeight - DefaultCapsuleHeight);
		GetCapsuleComponent()->SetRelativeLocation(CapsuleLocation);
	}
	GetCapsuleComponent()->SetCapsuleHalfHeight(newHalfHeight);

	// Update mesh location based on new capsule location
	FVector MeshLocation = GetBody()->GetRelativeLocation();
	MeshLocation.Z = DefaultMeshZ - (newHalfHeight - DefaultCapsuleHeight);
	GetBody()->SetRelativeLocation(MeshLocation);

	// If the character has authority (like in single player mode or if we're on the server)
	// then we don't need to make a server call.
	if(!HasAuthority())
	{
		Server_SetNewHeightFromScale(scale);
	}
}

void AShibAvatarCharacter::SetCurrentMontagePosition()
{
	MontagePosition = GetMesh()->GetPosition();
}

void AShibAvatarCharacter::PlayMontageFromCurrentPosition(UAnimationAsset* AnimToPlay)
{
	GetMesh()->PlayAnimation(AnimToPlay, true);
	GetMesh()->SetPosition(MontagePosition);
}

UMaterialInstanceDynamic* AShibAvatarCharacter::GetBodyDMI()
{
	if((BodyDMI = Cast<UMaterialInstanceDynamic>(GetMesh()->GetMaterial(0))))
	{
		//UE_LOG(LogShib, Log, TEXT("%hs - GotMaterialInstance!"), __FUNCTION__);

		return BodyDMI;
	}
	
	//UE_LOG(LogShib, Log, TEXT("%hs - Creating MaterialInstance"), __FUNCTION__);

	BodyDMI = GetMesh()->CreateDynamicMaterialInstance(0, GetMesh()->GetMaterial(0));
	return BodyDMI;

}

UMaterialInstanceDynamic* AShibAvatarCharacter::GetEyesDMI()
{
	if (!IsValid(EyesDMI)) EyesDMI = GetMesh()->CreateDynamicMaterialInstance(3, GetMesh()->GetMaterial(3));
	return EyesDMI;
}

UMaterialInstanceDynamic* AShibAvatarCharacter::GetLeftArmDMI()
{
	if (!IsValid(LeftArmDMI)) LeftArmDMI = LeftArm->CreateDynamicMaterialInstance(0, LeftArm->GetMaterial(0));
	return LeftArmDMI;
}

UMaterialInstanceDynamic* AShibAvatarCharacter::GetRightArmDMI()
{
	if (!IsValid(RightArmDMI)) RightArmDMI = RightArm->CreateDynamicMaterialInstance(0, RightArm->GetMaterial(0));
	return RightArmDMI;
}

UMaterialInstanceDynamic* AShibAvatarCharacter::GetBackDMI()
{
	if (!IsValid(BackDMI)) BackDMI = Back->CreateDynamicMaterialInstance(0, Back->GetMaterial(0));
	return BackDMI;
}

UMaterialInstanceDynamic* AShibAvatarCharacter::GetHeadTopDMI()
{
	if (!IsValid(HeadTopDMI)) HeadTopDMI = HeadTop->CreateDynamicMaterialInstance(0, HeadTop->GetMaterial(0));
	return HeadTopDMI;
}

UMaterialInstanceDynamic* AShibAvatarCharacter::GetNeckDMI()
{
	if (!IsValid(NeckDMI)) NeckDMI = Neck->CreateDynamicMaterialInstance(0, Neck->GetMaterial(0));
	return NeckDMI;
}

UMaterialInstanceDynamic* AShibAvatarCharacter::GetFaceDMI()
{
	if (!IsValid(FaceDMI)) FaceDMI = Face->CreateDynamicMaterialInstance(0, Face->GetMaterial(0));
	return FaceDMI;
}

UMaterialInstanceDynamic* AShibAvatarCharacter::GetEarsDMI()
{
	if (!IsValid(EarsDMI)) EarsDMI = Ears->CreateDynamicMaterialInstance(0, Ears->GetMaterial(0));
	return EarsDMI;
}

UMaterialInstanceDynamic* AShibAvatarCharacter::GetFullBodyTopDMI()
{
	if (!IsValid(FullBodyTopDMI)) FullBodyTopDMI = FullBody->CreateDynamicMaterialInstance(0, FullBody->GetMaterial(0));
	return FullBodyTopDMI;
}

UMaterialInstanceDynamic* AShibAvatarCharacter::GetFullBodyBottomDMI()
{
	if (!IsValid(FullBodyBottomDMI)) FullBodyBottomDMI = FullBody->CreateDynamicMaterialInstance( 1, FullBody->GetMaterial(1));
	return FullBodyBottomDMI;
}

UMaterialInstanceDynamic* AShibAvatarCharacter::GetShoesDMI()
{
	if (!IsValid(ShoesDMI)) ShoesDMI = Shoes->CreateDynamicMaterialInstance(0, Shoes->GetMaterial(0));
	return ShoesDMI;
}

UMaterialInstanceDynamic* AShibAvatarCharacter::GetBottomsDMI()
{
	if (!IsValid(BottomsDMI)) BottomsDMI = Bottoms->CreateDynamicMaterialInstance(0, Bottoms->GetMaterial(0));
	return BottomsDMI;
}

UMaterialInstanceDynamic* AShibAvatarCharacter::GetHairDMI()
{
	if (!IsValid(HairDMI)) HairDMI = Hair->CreateDynamicMaterialInstance(0, Hair->GetMaterial(0));
	return HairDMI;
}

UMaterialInstanceDynamic* AShibAvatarCharacter::GetTopDMI()
{
	if (!IsValid(TopDMI)) TopDMI = Top->CreateDynamicMaterialInstance(0, Top->GetMaterial(0));
	return TopDMI;
}
