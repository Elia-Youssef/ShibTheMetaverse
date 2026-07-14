// Copyright Shib LLC.

#pragma once

#include "CoreMinimal.h"
#include "ShibBaseGameMode.h"
#include "ShibGameMode.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogShibGameMode, Log, All);

struct FOnlineError;
class AShibPlayerController;
struct FVoiceAdminChannelCredentials;

/**
 * This GM adds the multiplayer functionalities on top of `AShibBaseGameMode`
 */
UCLASS()
class SHIBMVMAIN_API AShibGameMode : public AShibBaseGameMode
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void AutoJoinPlayerToVoiceChannel(APlayerController* NewPlayer);
	void NotifyClientsOfNewPlayer(FUniqueNetIdPtr NewPlayerID);
	
protected:
	virtual void BeginPlay() override;
	virtual void Logout(AController* Exiting) override;
	virtual void OnPostLogin(AController* NewPlayer) override;
	virtual void HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer) override;
	virtual void HandleSeamlessTravelPlayer(AController*& C) override;

	virtual void RestartPlayer(AController* NewPlayer) override;

	UFUNCTION(BlueprintCallable)
	virtual void UnregisterPlayer(APlayerController* InPlayerController);

private:
	FString PublicVoiceChannelName = FString("ChillingCentral");
	
	void IssuedCredentialsForPlayer(const FOnlineError& Result, const FUniqueNetId& LocalUserId,
	                                const TArray<FVoiceAdminChannelCredentials>& Credentials, FString InChannelName,
	                                AShibPlayerController* NewPlayer);
};
