// Copyright Shiba Inu Games LLC.

#pragma once

#include "Engine/DeveloperSettingsBackedByCVars.h"
#include "UObject/SoftObjectPath.h"
#include "ShibLoadingScreenConfig.generated.h"

class UObject;

USTRUCT(BlueprintType)
struct SHIBASYNCLOADINGSCREEN_API FShibMapsWithCustomLoadingScreen
{
	GENERATED_BODY()
	
public:
	FShibMapsWithCustomLoadingScreen()
		: MapName()
		, bHasFadeIn()
		, FadeInWarmupDuration()
		, FadeInDuration(){};

	FShibMapsWithCustomLoadingScreen(FString NewMapName)
	{
		MapName = NewMapName;
	}
	
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "ShibLoadingScreen")
	FString MapName = TEXT("None");
	
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category="ShibLoadingScreen")
	FSlateBrush MapBrush;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category="ShibLoadingScreen|Config")
	FText LoadingScreenText;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "ShibLoadingScreen")
	bool bHasFadeIn=false;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "ShibLoadingScreen", meta = (EditCondition = "bHasFadeIn", ClampMin = "0", ForceUnits=s))
	float FadeInWarmupDuration=0.f;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "ShibLoadingScreen", meta = (EditCondition = "bHasFadeIn", ClampMin = "0", ForceUnits=s))
	float FadeInDuration=0.f;

	bool operator==(const FShibMapsWithCustomLoadingScreen& Other) const
	{
		return MapName == Other.MapName;
	}
};

/**
 * 
 */
UCLASS(Config=Game, defaultconfig, meta=(DisplayName="Shib Loading Screen"))
class SHIBASYNCLOADINGSCREEN_API UShibLoadingScreenConfig : public UDeveloperSettingsBackedByCVars
{
	GENERATED_BODY()

public:
	UShibLoadingScreenConfig();
	
public:	
	UPROPERTY(config, EditAnywhere, BlueprintReadOnly, Category=Configuration)
	TArray<FShibMapsWithCustomLoadingScreen> MapsWithCustomLoadingScreen;
	
	// The widget to load for the loading screen.
	UPROPERTY(config, EditAnywhere, Category=Display, meta=(MetaClass="/Script/UMG.UserWidget"))
	FSoftClassPath LoadingScreenWidget;

	UPROPERTY(config, EditAnywhere, BlueprintReadOnly, Category=Display)
	FSlateBrush LoadingScreenDefaultBrush;

	// The z-order of the loading screen widget in the viewport stack
	UPROPERTY(config, EditAnywhere, Category=Display)
	int32 LoadingScreenZOrder = 10000;

	// How long to hold the loading screen up after other loading finishes (in seconds) to
	// try to give texture streaming a chance to avoid blurriness
	//
	// Note: This is not normally applied in the editor for iteration time, but can be 
	// enabled via HoldLoadingScreenAdditionalSecsEvenInEditor
 	UPROPERTY(config, EditAnywhere, Category=Configuration, meta=(ClampMin = "0", ForceUnits=s, ConsoleVariable="ShibLoadingScreen.HoldLoadingScreenAdditionalSecs"))
	float HoldLoadingScreenAdditionalSecs = 2.0f;

	// The interval in seconds beyond which the loading screen is considered permanently hung (if non-zero).
 	UPROPERTY(config, EditAnywhere, Category=Configuration, meta=(ClampMin = "0", ForceUnits=s))
	float LoadingScreenHeartbeatHangDuration = 0.0f;

	// The interval in seconds between each log of what is keeping a loading screen up (if non-zero).
 	UPROPERTY(config, EditAnywhere, Category=Configuration, meta=(ClampMin = "0", ForceUnits=s))
	float LogLoadingScreenHeartbeatInterval = 5.0f;

	// When true, the reason the loading screen is shown or hidden will be printed to the log every frame.
	UPROPERTY(Transient, EditAnywhere, Category=Debugging, meta=(ConsoleVariable="ShibLoadingScreen.LogLoadingScreenReasonEveryFrame"))
	bool LogLoadingScreenReasonEveryFrame = false;

	// Force the loading screen to be displayed (useful for debugging)
	UPROPERTY(Transient, EditAnywhere, Category=Debugging, meta=(ConsoleVariable="ShibLoadingScreen.AlwaysShow"))
	bool ForceLoadingScreenVisible = false;

	// Should we apply the additional HoldLoadingScreenAdditionalSecs delay even in the editor
	// (useful when iterating on loading screens)
	UPROPERTY(Transient, EditAnywhere, Category=Debugging)
	bool HoldLoadingScreenAdditionalSecsEvenInEditor = true;

	// Should we apply the additional HoldLoadingScreenAdditionalSecs delay even in the editor
	// (useful when iterating on loading screens)
	UPROPERTY(config, EditAnywhere, Category=Debugging)
	bool ForceTickLoadingScreenEvenInEditor = true;

	UFUNCTION(BlueprintPure, Category="ShibLoadingScreen", meta=(DisplayName="Get Loading Screen Settings"))
	static const UShibLoadingScreenConfig* Get()
	{
		return GetDefault<UShibLoadingScreenConfig>();
	}
};
