// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "ShibGameChatSubsystem.generated.h"

/**
 * 
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FChatUpdateDelegate,const FString&, PlayerName, const FString&, Message);

USTRUCT(BlueprintType)
struct FChatMessage
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	FDateTime Timestamp;
	
	UPROPERTY(BlueprintReadWrite)
	FString PlayerName = FString();
	
	UPROPERTY(BlueprintReadWrite)
	FString Message = FString();
};

UCLASS()
class SHIBMULTIPLAYER_API UShibGameChatSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	virtual void Deinitialize() override;
	
public:
	UPROPERTY(BlueprintReadWrite, Category="ChatWidget")
	APlayerController* ShibPC;
	
	FTimerHandle PlayerStateCheckTimerHandle;

	UPROPERTY(BlueprintReadWrite,Category="ChatWidget")
	bool bGameChatUIOpen;
	
	UPROPERTY(BlueprintReadWrite, Category="ChatWidget")
	UUserWidget* ChatWidgetInstance;

	UFUNCTION(BlueprintCallable, Server, Reliable)
	void SendMessage(const FString& PlayerName, const FString& Message);
	
	UFUNCTION(BlueprintCallable)
	void AddChatMessage(const FChatMessage& MessageChat);

	UPROPERTY(BlueprintReadOnly)
	TArray<FChatMessage> ChatArray;

	UPROPERTY(BlueprintAssignable)
	FChatUpdateDelegate ChatUpdateDelegate;
};
