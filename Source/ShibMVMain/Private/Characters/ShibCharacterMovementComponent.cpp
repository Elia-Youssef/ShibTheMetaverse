// Copyright Shiba Inu Games LLC.

#include "Characters/ShibCharacterMovementComponent.h"

#include "Characters/ShibCharacterBase.h"
#include "GameFramework/Character.h"

class AShibCharacterBase;


float UShibCharacterMovementComponent::GetMaxSpeed() const
{
	if(Safe_bWantsToSprint)
	{
		return SprintSpeed;
	}
	
	return Super::GetMaxSpeed();
}

#pragma region Flags

bool UShibCharacterMovementComponent::FSavedMove_Sy::CanCombineWith(const FSavedMovePtr& NewMove, ACharacter* InCharacter,
                                                         float MaxDelta) const
{
	FSavedMove_Sy* NewSyMove = static_cast<FSavedMove_Sy*>(NewMove.Get());

	if(Saved_bWantsToDrift != NewSyMove->Saved_bWantsToDrift)
	{
		return false;
	}
	
	if(Saved_bWantsToSwim != NewSyMove->Saved_bWantsToSwim) 
	{
		return false;
	}
	
	return FSavedMove_Character::CanCombineWith(NewMove, InCharacter, MaxDelta);
}

void UShibCharacterMovementComponent::FSavedMove_Sy::Clear()
{
	FSavedMove_Character::Clear();

	Saved_bWantsToDrift = 0;
	Saved_bWantsToSwim = 0;
}

uint8 UShibCharacterMovementComponent::FSavedMove_Sy::GetCompressedFlags() const
{
	uint8 Result = Super::GetCompressedFlags();

	if(Saved_bWantsToDrift)
		Result |= FLAG_Custom_0;

	if(Saved_bWantsToSwim)
		Result |= FLAG_Custom_1;
	
	return Result;
}

void UShibCharacterMovementComponent::FSavedMove_Sy::SetMoveFor(ACharacter* C, float InDeltaTime, FVector const& NewAccel,
	FNetworkPredictionData_Client_Character& ClientData)
{
	FSavedMove_Character::SetMoveFor(C, InDeltaTime, NewAccel, ClientData);

	if(UShibCharacterMovementComponent* SyMovement = Cast<UShibCharacterMovementComponent>(C->GetCharacterMovement()))
	{
		Saved_bWantsToDrift = SyMovement->Safe_bWantsToSprint;
		Saved_bWantsToSwim = SyMovement->Safe_bWantsToSwim;
	}
}

void UShibCharacterMovementComponent::FSavedMove_Sy::PrepMoveFor(ACharacter* C)
{
	FSavedMove_Character::PrepMoveFor(C);

	if(UShibCharacterMovementComponent* SyMovement = Cast<UShibCharacterMovementComponent>(C->GetCharacterMovement()))
	{
		SyMovement->Safe_bWantsToSprint = Saved_bWantsToDrift;
		SyMovement->Safe_bWantsToSwim = Saved_bWantsToSwim;
	}
}

UShibCharacterMovementComponent::FNetworkPredictionData_Client_Sy::FNetworkPredictionData_Client_Sy(
	const UCharacterMovementComponent& ClientMovement)
:	Super(ClientMovement)
{
}

FSavedMovePtr UShibCharacterMovementComponent::FNetworkPredictionData_Client_Sy::AllocateNewMove()
{
	return FSavedMovePtr(new FSavedMove_Sy());
}

FNetworkPredictionData_Client* UShibCharacterMovementComponent::GetPredictionData_Client() const
{
	check(PawnOwner != nullptr)

	if(ClientPredictionData == nullptr)
	{
		if(UShibCharacterMovementComponent* SyMovementComponent = const_cast<UShibCharacterMovementComponent*>(this))
		{
			SyMovementComponent->ClientPredictionData = new FNetworkPredictionData_Client_Sy(*this);
			SyMovementComponent->NetworkMaxSmoothUpdateDistance = 92.f;
			SyMovementComponent->NetworkNoSmoothUpdateDistance = 140.f;
		}
	}

	return ClientPredictionData;
}

void UShibCharacterMovementComponent::UpdateFromCompressedFlags(uint8 Flags)
{
	Super::UpdateFromCompressedFlags(Flags);

	Safe_bWantsToSprint = (Flags & FSavedMove_Character::FLAG_Custom_0) != 0;
	Safe_bWantsToSwim = (Flags & FSavedMove_Character::FLAG_Custom_1) != 0;
}

#pragma endregion Flags

#pragma region CustomMove

void UShibCharacterMovementComponent::OnMovementUpdated(float DeltaSeconds, const FVector& OldLocation, const FVector& OldVelocity)
{
	// Called on everyone but flags are always false for remote clients
	Super::OnMovementUpdated(DeltaSeconds, OldLocation, OldVelocity);

	if(bWantsToSwim && ! IsMovementMode(MOVE_Flying))
	{
		SetMovementMode(MOVE_Flying, 0);
		if(AShibCharacterBase* ShibCharacterBase = Cast<AShibCharacterBase>(GetOwner())) //TODO could be a delegate
		{
			ShibCharacterBase->OnToggleSwim(true);
		}
	}
	else if(IsMovementMode(MOVE_Flying) && ! bWantsToSwim)
	{
		SetMovementMode(MOVE_Walking, 0);
		if(AShibCharacterBase* ShibCharacterBase = Cast<AShibCharacterBase>(GetOwner())) //TODO could be a delegate
		{
			ShibCharacterBase->OnToggleSwim(false);
		}
	}

	// if( ! Velocity.IsZero())
	// {
	// 	OnCharacterIsMoving.Broadcast();
	// }

#pragma region Debug

	// FString MovementModeString;
	//
	// switch (MovementMode)
	// {
	// case MOVE_Walking:
	// 	MovementModeString = TEXT("Walking");
	// 	break;
	// case MOVE_NavWalking:
	// 	MovementModeString = TEXT("NavWalking");
	// 	break;
	// case MOVE_Falling:
	// 	MovementModeString = TEXT("Falling");
	// 	break;
	// case MOVE_Swimming:
	// 	MovementModeString = TEXT("Swimming");
	// 	break;
	// case MOVE_Flying:
	// 	MovementModeString = TEXT("Flying");
	// 	break;
	// case MOVE_Custom:
	// 	MovementModeString = TEXT("Custom");
	// 	break;
	// case MOVE_None:
	// 	MovementModeString = TEXT("None");
	// 	break;
	// default:
	// 	MovementModeString = TEXT("Unknown");
	// 	break;
	// }
	//
	// if( ! Safe_bWantsToDrift && MovementModeString == "Custom")
	// {
	// 	UE_LOG(LogTemp, Warning, TEXT("ERROR: Movement Mode is CUSTOM but FLAG doesn't want to drift"));
	// }
	// else
	// {
	// 	UE_LOG(LogTemp, Warning, TEXT("Movement Mode: %s"), *MovementModeString);
	// }

#pragma endregion Debug
}

void UShibCharacterMovementComponent::SetMovementMode(EMovementMode NewMovementMode, uint8 NewCustomMode)
{
	Super::SetMovementMode(NewMovementMode, NewCustomMode);
}

bool UShibCharacterMovementComponent::IsMovementMode(EMovementMode NewMovementMode) const
{
	return MovementMode == NewMovementMode;
}

#pragma region Sprint

void UShibCharacterMovementComponent::ToggleSprint(bool bNewSprint)
{
	bWantsToSprint = bNewSprint;
}

bool UShibCharacterMovementComponent::IsSprinting() const
{
	return bWantsToSprint;
}

#pragma endregion Sprint

#pragma region Swim

void UShibCharacterMovementComponent::ToggleSwim(bool bNewSwim)
{
	bWantsToSwim = bNewSwim;
}

#pragma endregion Swim

void UShibCharacterMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                                    FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	Safe_bWantsToSprint = bWantsToSprint;
	Safe_bWantsToSwim = bWantsToSwim;
}

#pragma endregion CustomMove
