// Copyright Shiba Inu Games LLC.

#pragma once

#include "CoreMinimal.h"
#include "ShibBasePlayerState.h"
#include "Metaverse/MetaverseApisTypes.h"
#include "ShibPlayerState.generated.h"

class UShibInventory;
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAvatarInfoReplicatedDelegate, FMvAvatarDetails, AvatarInfo);

UCLASS()
class SHIBMVMAIN_API AShibPlayerState : public AShibBasePlayerState
{
	GENERATED_BODY()

public:
	AShibPlayerState();
	
	UPROPERTY(ReplicatedUsing=OnRep_Player, BlueprintReadOnly)
	FShibMvPlayer Player;
	
	UFUNCTION()
	void OnRep_Player();

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	
};
