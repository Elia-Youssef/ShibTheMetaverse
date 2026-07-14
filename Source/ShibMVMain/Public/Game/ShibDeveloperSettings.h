#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "ShibDeveloperSettings.generated.h"

UCLASS(config=Editor, defaultconfig, meta=(DisplayName="Shib Editor Settings"))
class SHIBMVMAIN_API UShibDeveloperSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UPROPERTY(config, EditAnywhere, BlueprintReadOnly, Category=Configuration)
	bool bEnableTestingConfig;

	UPROPERTY(config, EditAnywhere, BlueprintReadOnly, Category=Configuration)
	FString LiveSandboxId;

	UPROPERTY(config, EditAnywhere, BlueprintReadOnly, Category=Configuration)
	FString LiveDeploymentId;

	UPROPERTY(config, EditAnywhere, BlueprintReadOnly, Category=Configuration)
	FString StageSandboxId;

	UPROPERTY(config, EditAnywhere, BlueprintReadOnly, Category=Configuration)
	FString StageDeploymentId;
	
	UPROPERTY(config, EditAnywhere, BlueprintReadOnly, Category="Editor Testing Settings", meta=(EditCondition = "bEnableTestingConfig"))
	FString AdditionalLaunchParameters;

	UPROPERTY(config, EditAnywhere, BlueprintReadOnly, Category="Editor Testing Settings", meta=(EditCondition = "bEnableTestingConfig"))
	FString ServerMapNameOverride;

	UPROPERTY(config, EditAnywhere, BlueprintReadOnly, Category="Editor Testing Settings", meta=(EditCondition = "bEnableTestingConfig"))
	FString AdditionalServerGameOption;

	UFUNCTION(BlueprintCallable, Category="ShibEditorSettings", meta=(DevelopmentOnly))
	void ApplySettings();

	UFUNCTION(BlueprintPure, Category="ShibEditorSettings", meta=(DisplayName="Get Shib Editor Settings"))
	static const UShibDeveloperSettings* Get()
	{
		return GetDefault<UShibDeveloperSettings>();
	}

private:
	void ChangeProjectSettings() const;
};