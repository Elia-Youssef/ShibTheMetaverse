// Copyright Shib LLC.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Metaverse/MetaverseApisTypes.h"
#include "ShibGameInstance.generated.h"

class UMetaverseApisSubsystem;
class UPlotBuilderSubsystem;
class UShibAPIsSubsystem;
class UShibAvatarBuilderSubsystem;
class UShibSaveSubsystem;

UCLASS()
class SHIBMVMAIN_API UShibGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	virtual void Init() override;

	UFUNCTION(BlueprintCallable)
	void InitializeUser();

	virtual void Shutdown() override;

	UFUNCTION(BlueprintCallable, Category="Travel Info")
	void SetTravelInfo(const FName LevelKeyInfo, const FName TeleporterTagInfo);

	UPROPERTY(BlueprintReadOnly, Category="Travel Info")
	FName TeleporterTag = FName("Default");

	UPROPERTY(BlueprintReadOnly)
	FMvAvatarDetails AvatarInfo;

	UPROPERTY(BlueprintReadWrite, Category="Travel Info")
	FServerPlotInfo SelectedPlotToView;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	bool bRunBenchmarkInStation = true;

protected:
	/** Flag indicating that a game error has occurred */
	UPROPERTY(BlueprintReadOnly, Category = "Network")
	bool bIsGameError;

	/** Error msg associated with any game error */
	UPROPERTY(BlueprintReadOnly, Category = "Network")
	FString GameErrorMsg;

private:
	UPROPERTY()
	TObjectPtr<UShibAPIsSubsystem> APIsSubsystem;
	UPROPERTY()
	TObjectPtr<UMetaverseApisSubsystem> MetaverseApisSubsystem;
	UPROPERTY()
	TObjectPtr<UShibAvatarBuilderSubsystem> AvatarBuildSubsystem;
	UPROPERTY()
	TObjectPtr<UPlotBuilderSubsystem> PlotBuilderSubsystem;

	UFUNCTION()
	void OnPlotBuilderSave(const FString& Json);
	UFUNCTION()
	void OnAvatarBuilderSave(const FString& Json);

	UFUNCTION()
	void OnGetMvPlayer(FShibMvPlayer Player, bool bSuccessful);

	void HandleNetworkFailure(UWorld* World, UNetDriver* NetDriver, ENetworkFailure::Type FailureType,
	                          const FString& ErrorString);
	void HandleTravelFailure(UWorld* InWorld, ETravelFailure::Type FailureType, const FString& ErrorString);

public:
	UFUNCTION(BlueprintCallable)
	void ResetErrorWarnings();
};
