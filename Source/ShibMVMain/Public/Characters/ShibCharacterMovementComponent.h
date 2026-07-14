// Copyright Shib LLC.

#pragma once
#include "GameFramework/CharacterMovementComponent.h"
#include "ShibCharacterMovementComponent.generated.h"

// This is a performance friendly non dynamic delegate so it can not be used in blueprints
// DECLARE_MULTICAST_DELEGATE(FOnCharacterIsMoving);

UCLASS()
class UShibCharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()
public:
	
	// // Movement delegates
	// FOnCharacterIsMoving OnCharacterIsMoving;

	virtual float GetMaxSpeed() const override;

#pragma region Flags

	class FSavedMove_Sy : public FSavedMove_Character
	{
		typedef FSavedMove_Character Super;

		uint8 Saved_bWantsToDrift:1;
		uint8 Saved_bWantsToSwim:1;

		virtual bool CanCombineWith(const FSavedMovePtr& NewMove, ACharacter* InCharacter, float MaxDelta) const override;
		virtual void Clear() override;
		virtual uint8 GetCompressedFlags() const override;
		virtual void SetMoveFor(ACharacter* C, float InDeltaTime, FVector const& NewAccel, FNetworkPredictionData_Client_Character& ClientData) override;
		virtual void PrepMoveFor(ACharacter* C) override;
	};
	
	class FNetworkPredictionData_Client_Sy : public FNetworkPredictionData_Client_Character
	{
	public:
		FNetworkPredictionData_Client_Sy(const UCharacterMovementComponent& ClientMovement);

		typedef FNetworkPredictionData_Client_Character Super;
		
		virtual FSavedMovePtr AllocateNewMove() override;
	};

public:
	virtual FNetworkPredictionData_Client* GetPredictionData_Client() const override;
	
protected:
	virtual void UpdateFromCompressedFlags(uint8 Flags) override;

#pragma endregion Flags

#pragma region CustomMovement
	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	virtual void OnMovementUpdated(float DeltaSeconds, const FVector& OldLocation, const FVector& OldVelocity) override;

	virtual void SetMovementMode(EMovementMode NewMovementMode, uint8 NewCustomMode) override;

	bool IsMovementMode(EMovementMode NewMovementMode) const;
	
#pragma region Sprint

public:
	void ToggleSprint(bool bNewSprint);

	bool IsSprinting() const;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Custom Movement")
	float SprintSpeed = 700.f;
	
private:
	
	bool bWantsToSprint = false;
	bool Safe_bWantsToSprint; // There are two variables, not to be mistaken for each other, called Saved and Safe

#pragma endregion Sprint

#pragma region Swim

public:
	void ToggleSwim(bool bNewSwim);
	
	bool bWantsToSwim = false;
private:
	bool Safe_bWantsToSwim; // There are two variables, not to be mistaken for each other, called Saved and Safe

#pragma endregion Swim

#pragma endregion CustomMovement
	
	
};
