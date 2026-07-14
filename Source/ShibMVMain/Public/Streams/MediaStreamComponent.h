// Copyright Shiba Inu Games LLC.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MediaStreamComponent.generated.h"

class UMediaSoundComponent;
class UShibMediaStreamer;
class UMediaPlayer;
class AMediaStreamOrigin;

UENUM(BlueprintType)
enum class EStreamerLevel : uint8
{
	SL_None = 0 UMETA(DisplayName = "None"),
	
	SL_Station = 1 UMETA(DisplayName = "Station"),
	SL_BarkPark = 2 UMETA(DisplayName = "BarkPark"),
	SL_CurrencyCanyon = 3 UMETA(DisplayName = "CurrencyCanyon"),
	SL_DefenseValley = 4 UMETA(DisplayName = "DefenseValley"),
	SL_FudGround = 5 UMETA(DisplayName = "FudGround"),
	SL_GrowthDunes = 6 UMETA(DisplayName = "GrowthDunes"),
	SL_RocketPond = 7 UMETA(DisplayName = "RocketPond"),
	SL_RyoPlaza = 8 UMETA(DisplayName = "RyoPlaza"),
	SL_TheBackyard = 9 UMETA(DisplayName = "TheBackyard"),
	SL_TheKennelClub = 10 UMETA(DisplayName = "TheKennelClub"),
	SL_Wagmi = 11 UMETA(DisplayName = "Wagmi"),
	
	SL_AlWahaTech = 12 UMETA(DisplayName = "AlWahaTech"),
	SL_DigitalCenter = 13 UMETA(DisplayName = "DigitalCenter"),
	SL_EarthCenter = 14 UMETA(DisplayName = "EarthCenter"),
	
	SL_TechTrench = 15 UMETA(DisplayName = "TechTrench"),
};

UENUM(BlueprintType)
enum class EStreamerRatio : uint8
{
	SR_None = 0 UMETA(DisplayName = "None"),
	
	SR_16x9 = 1 UMETA(DisplayName = "16:9"),
	SR_9x16 = 2 UMETA(DisplayName = "9:16"),
	SR_4x5 = 3 UMETA(DisplayName = "4:5"),
	SR_2x6 = 4 UMETA(DisplayName = "2:6"),
	SR_11x24 = 8 UMETA(DisplayName = "11:24"),
	SR_MoeiBillboard_1 = 5 UMETA(DisplayName = "MoeiBillboard_1"),
	SR_MoeiBillboard_2 = 6 UMETA(DisplayName = "MoeiBillboard_2"),
	SR_MoeiBillboard_3 = 7 UMETA(DisplayName = "MoeiBillboard_3"),
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class SHIBMVMAIN_API UMediaStreamComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UMediaStreamComponent();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EStreamerLevel StreamerLevel = EStreamerLevel::SL_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EStreamerRatio StreamerRatio = EStreamerRatio::SR_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 UniqueStreamerId = -1;

	UPROPERTY(BlueprintReadOnly)
	UShibMediaStreamer* Streamer = nullptr;

	UPROPERTY(BlueprintReadWrite)
	UMediaSoundComponent* VideoPlayerAudio = nullptr;

protected:
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintPure)
	FString GetStreamerId();
};
