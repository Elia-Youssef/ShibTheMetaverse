// Copyright Shiba Inu Games LLC.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ShibGamesManager.generated.h"


class UShibGameComponent;
class AShibPlayerController;

UCLASS(Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class SHIBMVMAIN_API UShibGamesManager : public UActorComponent
{
	GENERATED_BODY()

public:
	UShibGamesManager();

	UFUNCTION(BlueprintCallable)
	bool StartGame(TSubclassOf<UShibGameComponent> GameComponentClass);
	
	UFUNCTION(BlueprintCallable)
	bool StopGame();
	
	UFUNCTION(BlueprintPure)
	bool CanDestroyCurrentGame();

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY()
	UShibGameComponent* CurrentGame = nullptr;
	
	UPROPERTY()
	AShibPlayerController* ShibPC = nullptr;
};
