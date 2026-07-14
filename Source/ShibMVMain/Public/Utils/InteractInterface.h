#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "InteractInterface.generated.h"

UINTERFACE(Blueprintable)
class SHIBMVMAIN_API UInteractInterface : public UInterface
{
	GENERATED_BODY()
};

class SHIBMVMAIN_API IInteractInterface
{
	GENERATED_BODY()

public:
	/**
	 * Called when a user approaches this actor and interacts with it.
	 * @param Instigator Points to the actor interacting with this actor (can be invalid).
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void Interact(AActor* Instigator = nullptr);
};
