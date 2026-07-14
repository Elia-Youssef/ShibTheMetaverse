// Copyright Shiba Inu Games LLC.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PlotsOrigin.generated.h"

class UBoxComponent;
struct FServerPlotInfo;
class UShibAPIsSubsystem;
class ABasePlot;

UCLASS()
class SHIBMVMAIN_API APlotsOrigin : public AActor
{
	GENERATED_BODY()

	FTimerHandle TimerHandle;
	FTimerDelegate TimerDelegate;
	FVector2D AreaFrom = FVector2D::ZeroVector;
	FVector2D AreaTo = FVector2D::ZeroVector;

	UFUNCTION()
	void LoadPlotsInfo();
	
	UFUNCTION()
	void OnPlotsInfoLoaded(const TArray<FServerPlotInfo>& PlotsInfo, bool bSuccessful, bool bInfoUpdated);
	
public:
	inline static FVector2D PlotSize = FVector2D(5699.f, 5528.f);
	
	APlotsOrigin();

	UFUNCTION(BlueprintImplementableEvent)
	bool GetCurrentPlotArea(FVector2D& From, FVector2D& To);

	UFUNCTION(BlueprintCallable, Category=PlotsOrigin)
	void GetWorldBounds(FVector2D& OutMin, FVector2D& OutMax);

	UFUNCTION(BlueprintCallable, Category=PlotsOrigin)
	FVector2D ConvertWorldToMapLocation(const FVector WorldLocation, const FVector2D MapSize);
	
	// This is set in the blueprint construction script using the DT_Hubs data table
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=PlotsOrigin)
	FVector2D Coordinates;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=PlotsOrigin, meta = (ClampMin="15"))
	float UpdatePlotsRate = 60.f;

	// Looking at the map in the correct orientation, +X should point to the right.
	// If this is not the case for the current map, reverse the X axis on the Plot Origin instance.
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category=PlotsOrigin)
	bool bFlipXAxis;

	// Looking at the map in the correct orientation, +Y should point down.
	// If this is not the case for the current map, reverse the Y axis on the Plot Origin instance.
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category=PlotsOrigin)
	bool bFlipYAxis;

	// Looking at the map in the correct orientation, +X should point to the right and +Y should point down.
	// If this is not the case for the current map, reverse the X and Y axis on the Plot Origin instance. Which means Y will become X, and X will become Y.
	// FlipXAxis and bFlipYAxis now work in reverse.
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category=PlotsOrigin)
	bool bReverseAxis;

protected:
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void BeginPlay() override;

private:
	UPROPERTY(Category=PlotsOrigin, VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess = "true"))
	TObjectPtr<USceneComponent> PlotsOriginRoot;
	
	UPROPERTY(Category=PlotsOrigin, VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess = "true"))
	TObjectPtr<UBoxComponent> BoundsBox;

};
