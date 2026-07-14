#include "Game/ShibDeveloperSettings.h"

void UShibDeveloperSettings::ApplySettings()
{
	ChangeProjectSettings();
}

void UShibDeveloperSettings::ChangeProjectSettings() const
{
	if (!GConfig)
	{
		UE_LOG(LogTemp, Error, TEXT("GConfig is null, cannot apply runtime settings."));
		return;
	}
	
	if (bEnableTestingConfig)
	{
		GConfig->SetString(TEXT("EpicOnlineServices"), TEXT("SandboxId"), *StageSandboxId, GEngineIni);
		GConfig->SetString(TEXT("EpicOnlineServices"), TEXT("DeploymentId"), *StageDeploymentId, GEngineIni);
	}
	else
	{
		GConfig->SetString(TEXT("EpicOnlineServices"), TEXT("SandboxId"), *LiveSandboxId, GEngineIni);
		GConfig->SetString(TEXT("EpicOnlineServices"), TEXT("DeploymentId"), *LiveDeploymentId, GEngineIni);
	}

	// Flush changes in DefaultEngine.ini
	GConfig->Flush(false, GEngineIni);

	UE_LOG(LogTemp, Warning, TEXT("Project Settings applied to: %s"), *GEditorPerProjectIni);
}