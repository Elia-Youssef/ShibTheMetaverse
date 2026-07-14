// Copyright Shiba Inu Games LLC.

#include "Plots/PlotsOrigin.h"

#include "EngineUtils.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Metaverse/MetaverseApisSubsystem.h"
#include "Plots/BasePlot.h"

APlotsOrigin::APlotsOrigin()
{
	PrimaryActorTick.bCanEverTick = false;
	
	PlotsOriginRoot = CreateDefaultSubobject<USceneComponent>(TEXT("PlotsOriginRoot"));
	PlotsOriginRoot->SetMobility(EComponentMobility::Type::Movable);
	SetRootComponent(PlotsOriginRoot);

	BoundsBox = CreateDefaultSubobject<UBoxComponent>(TEXT("WorldMapBounds"));
	BoundsBox->SetupAttachment(PlotsOriginRoot);
	BoundsBox->SetCollisionEnabled(ECollisionEnabled::Type::NoCollision);
	BoundsBox->CanCharacterStepUpOn = ECB_No;
	BoundsBox->SetGenerateOverlapEvents(false);
	BoundsBox->SetMobility(EComponentMobility::Type::Movable);
}

void APlotsOrigin::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	TArray<AActor*> OutActors;
	UGameplayStatics::GetAllActorsOfClass(this, ABasePlot::StaticClass(), OutActors);
	for (auto* Actor : OutActors)
	{
		if (auto* Plot = Cast<ABasePlot>(Actor))
			Plot->CalculateCoordinatesUsingRef();
	}
}

void APlotsOrigin::BeginPlay()
{
	Super::BeginPlay();

	// Always make sure this actor is centered in the world
	SetActorLocation(FVector(0,0,0),false,nullptr);

	if (HasAuthority())
	{
		TimerDelegate.BindUObject(this, &APlotsOrigin::LoadPlotsInfo);
		GetWorldTimerManager().SetTimer(TimerHandle, TimerDelegate, UpdatePlotsRate, true, 0.f);
	}

	FVector MinBounds(FLT_MAX, FLT_MAX, 0);
	FVector MaxBounds(-FLT_MAX, -FLT_MAX, 0);

	// Iterate through all Plots in the world
	for (TActorIterator<ABasePlot> It(GetWorld()); It; ++It)
	{
		FVector Origin, Extent;
		(*It)->GetActorBounds(true, Origin, Extent);

		FVector ActorMin = Origin - Extent;
		FVector ActorMax = Origin + Extent;

		// Track the min/max X & Y values across all actors
		MinBounds.X = FMath::Min(MinBounds.X, ActorMin.X);
		MinBounds.Y = FMath::Min(MinBounds.Y, ActorMin.Y);

		MaxBounds.X = FMath::Max(MaxBounds.X, ActorMax.X);
		MaxBounds.Y = FMath::Max(MaxBounds.Y, ActorMax.Y);
	}

	// Calculate Center & Size
	FVector Center = (MinBounds + MaxBounds) * 0.5f;
	FVector Extents = (MaxBounds - MinBounds) * 0.5f;

	// Apply to Box Collision
	BoundsBox->SetRelativeLocation(Center);
	BoundsBox->SetBoxExtent(Extents);
}

void APlotsOrigin::LoadPlotsInfo()
{
	if (AreaFrom.IsZero() || AreaTo.IsZero()) // if we don't know our area yet
	{
		if (!GetCurrentPlotArea(AreaFrom, AreaTo)) return; // try getting the area
	}
	
	if (auto* MetaverseApisSubsystem = GetGameInstance()->GetSubsystem<UMetaverseApisSubsystem>())
	{
		MetaverseApisSubsystem->OnGetAllPlotsInfoDelegate.AddUniqueDynamic(this, &ThisClass::OnPlotsInfoLoaded);
		MetaverseApisSubsystem->GetAllPlotsInfo(AreaFrom, AreaTo); // get the plot data in this area
	}
}

void APlotsOrigin::OnPlotsInfoLoaded(const TArray<FServerPlotInfo>& PlotsInfo, bool bSuccessful, bool bInfoUpdated)
{
	if (auto MetaverseApisSubsystem = GetGameInstance()->GetSubsystem<UMetaverseApisSubsystem>())
	{
		MetaverseApisSubsystem->OnGetAllPlotsInfoDelegate.RemoveDynamic(this, &ThisClass::OnPlotsInfoLoaded);
	}
	
	if (!bSuccessful) return;

	TArray<AActor*> OutActors;
	UGameplayStatics::GetAllActorsOfClass(this, ABasePlot::StaticClass(), OutActors);

	for (AActor* Actor : OutActors)
	{
		auto* Plot = Cast<ABasePlot>(Actor);
		if (!Plot) continue;

		for (const FServerPlotInfo& PlotInfo : PlotsInfo)
		{
			if (Plot->Coordinates.X == PlotInfo.X && Plot->Coordinates.Y == PlotInfo.Y)
			{
				Plot->ServerPlotInfo = PlotInfo;
				Plot->AssemblePlot();
				break;
			}
		}
	}
}

void APlotsOrigin::GetWorldBounds(FVector2D& OutMin, FVector2D& OutMax)
{
	if (!BoundsBox) return;

	// Get the world-space center of the box
	FVector Center = BoundsBox->GetComponentLocation();

	// Get the scaled extents (half-size of the box in world space)
	FVector Extents = BoundsBox->GetScaledBoxExtent();

	// Calculate Min/Max X and Y values
	OutMin = FVector2D(Center.X - Extents.X, Center.Y - Extents.Y);
	OutMax = FVector2D(Center.X + Extents.X, Center.Y + Extents.Y);
}

FVector2D APlotsOrigin::ConvertWorldToMapLocation(const FVector WorldLocation, const FVector2D MapSize)
{
	FVector2D WorldMin, WorldMax;
	GetWorldBounds(WorldMin, WorldMax); // Get correct world bounds from `BoundsBox`

	float MapX = FMath::GetMappedRangeValueClamped(
		FVector2D(WorldMin.X, WorldMax.X),
		FVector2D(0, MapSize.X),
		WorldLocation.X
	);

	float MapY = FMath::GetMappedRangeValueClamped(
		FVector2D(WorldMin.Y, WorldMax.Y),
		FVector2D(0, MapSize.Y),
		WorldLocation.Y
	);

	if (bReverseAxis) // Reverse X and Y?
	{
		return FVector2D(bFlipYAxis?MapSize.Y-MapY:MapY, bFlipXAxis?MapSize.X-MapX:MapX); // Flip Y and X axis?
	}

	return FVector2D(bFlipXAxis?MapSize.X-MapX:MapX,bFlipYAxis?MapSize.Y-MapY:MapY); // Flip X and Y axis?
}
