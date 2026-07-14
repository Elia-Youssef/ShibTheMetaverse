// Copyright Shiba Inu Games LLC.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "ShibBasePlayerState.generated.h"

/** 
* Pawn related data that we want to keep before a player is abruptly disconnected.
* The data in this struct should only include what is not already stored in the player state.
* Most, if not all the info here should come from the player's pawn.
*/
USTRUCT()
struct FDisconnectedPlayerData
{
	GENERATED_BODY()

	// Last known player position before it got disconnected 
	UPROPERTY()
	FTransform PlayerPosition;

	// ADD MORE STUFF TO KEEP IN MEMORY HERE...

};

/**
 * 
 */
UCLASS()
class SHIBMVMAIN_API AShibBasePlayerState : public APlayerState
{
	GENERATED_BODY()
	
public:
	AShibBasePlayerState();
	
	virtual void OnDeactivated() override;
	virtual void OnReactivated() override;

	// Disconnected player's stashed data, so we reapply it on reconnection
	UPROPERTY()
	FDisconnectedPlayerData DisconnectedPlayerData;

	UPROPERTY(BlueprintReadWrite)
	bool bIsReconnecting = false;
	
	UPROPERTY(BlueprintReadWrite)
	bool bIsLeavingSession = false;

protected:
	virtual void CopyProperties(APlayerState* PlayerState) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
