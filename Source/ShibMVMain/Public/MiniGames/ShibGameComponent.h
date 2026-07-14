// Copyright Shiba Inu Games LLC.

#pragma once
#include "Inventory/ShibInventory.h"

#include "ShibGameComponent.generated.h"

UCLASS(Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class SHIBMVMAIN_API UShibGameComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UShibGameComponent();

	virtual bool CanActivateGame();
	virtual bool CanDestroyGame();

	virtual bool ActivateGame();
	virtual bool CleanupAndDestroy();

	virtual bool CanPlayerStartGame();

	UShibInventory* GetShibInventory() const;

	int32 GetLootLuck() const;

protected:
	/**
	 * Component Config
	 */

	bool bCanDestroy = true;

	/**
	 * Component State
	 */

	bool bIsActive = false;
	bool bPendingDestroy = false;
};
