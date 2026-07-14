#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameUserSettings.h"
#include "ShibGameUserSettings.generated.h"

UCLASS()
class SHIBMVMAIN_API UShibGameUserSettings : public UGameUserSettings
{
	GENERATED_BODY()
	

public:
	
	virtual void ApplySettings(bool bCheckForCommandLineOverrides) override;

	virtual void LoadSettings(bool bForceReload) override;

	void EnsureMinimalGraphicsQuality();
	
};
