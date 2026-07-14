#include "Game/ShibGameUserSettings.h"

void UShibGameUserSettings::ApplySettings(bool bCheckForCommandLineOverrides)
{
	EnsureMinimalGraphicsQuality();
	
	Super::ApplySettings(bCheckForCommandLineOverrides);
}

void UShibGameUserSettings::LoadSettings(bool bForceReload)
{
	EnsureMinimalGraphicsQuality();
	
	Super::LoadSettings(bForceReload);
}

void UShibGameUserSettings::EnsureMinimalGraphicsQuality()
{
	// Ensure reflection and global illumination are at least 2
	if (GetReflectionQuality() < 2)
	{
		SetReflectionQuality(2);
	}

	if (GetGlobalIlluminationQuality() < 2)
	{
		SetGlobalIlluminationQuality(2);
	}
}
