// Fill out your copyright notice in the Description page of Project Settings.

#include "Plots/BasePlot.h"
#include "Game/PlotBuilderSubsystem.h"
#include "Game/ShibGameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Player/BuildController.h"
#include "Plots/PlotsOrigin.h"

ABasePlot::ABasePlot()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	bAlwaysRelevant = true;

	PlotRoot = CreateDefaultSubobject<USceneComponent>(TEXT("PlotRoot"));
	PlotRoot->SetMobility(EComponentMobility::Static);
	SetRootComponent(PlotRoot);
	
	LandingPlatform = CreateDefaultSubobject<UChildActorComponent>(TEXT("LandingPlatform"));
	LandingPlatform->SetMobility(EComponentMobility::Static);
	LandingPlatform->SetupAttachment(PlotRoot);

	// Function to call when the plot receives an update
	OnPlotUpdatingDelegate.BindDynamic(this, &ABasePlot::OnPlotUpdating);
}

void ABasePlot::CalculateCoordinatesUsingRef()
{
	AActor* OriginActor = UGameplayStatics::GetActorOfClass(this, APlotsOrigin::StaticClass());
	APlotsOrigin* OriginPlot = OriginPlot = Cast<APlotsOrigin>(OriginActor);
	if (!OriginPlot) return;

	const FVector LocationDelta = GetActorLocation() - OriginPlot->GetActorLocation();
	const FVector2D Delta2D = FVector2D(LocationDelta.X, LocationDelta.Y) + APlotsOrigin::PlotSize / 2.f;
	const int32 X = FMath::Floor(Delta2D.X / APlotsOrigin::PlotSize.X);
	const int32 Y = FMath::Floor(Delta2D.Y / APlotsOrigin::PlotSize.Y) * -1;
	
	Coordinates = FVector2D(X + OriginPlot->Coordinates.X, Y + OriginPlot->Coordinates.Y);
}

void ABasePlot::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	CalculateCoordinatesUsingRef();
}

void ABasePlot::BeginPlay()
{
	Super::BeginPlay();
}

void ABasePlot::AssemblePlot()
{
	if (!ObjectsDataTable || ServerPlotInfo.PlotData.IsEmpty()) return;
	
	if (auto* PB = GetGameInstance<UShibGameInstance>()->GetSubsystem<UPlotBuilderSubsystem>())
	{
		PB->LoadPlot(OnPlotUpdatingDelegate, ObjectsDataTable, GetActorTransform(), ServerPlotInfo.PlotData, BuiltObjects);
	}
}

void ABasePlot::AssemblePlotForPlotBuilder()
{
	// if it's a plot builder plot, then assemble the plot for Plot Builder
	if (auto* Controller = Cast<ABuildController>(UGameplayStatics::GetPlayerController(this, 0)))
	{
		Controller->InitializeBuildController(ServerPlotInfo.PlotData);
	}
}

void ABasePlot::OnPlotUpdating()
{
	MoveCharactersToLandingPlatform();
}

void ABasePlot::MoveCharactersToLandingPlatform()
{
	TArray<AActor*> FoundCharacters;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACharacter::StaticClass(), FoundCharacters);

	FVector Origin = FVector::ZeroVector;
	FVector BoxExtent = FVector::ZeroVector;

	GetActorBounds(false, Origin, BoxExtent);
	const FBox Box(Origin - BoxExtent, Origin + BoxExtent);
	
	for (AActor* FoundActor : FoundCharacters)
	{
		if (ACharacter* Character = Cast<ACharacter>(FoundActor))
		{
			FVector CharacterLocation = Character->GetActorLocation();

			// Check if the character is within bounds
			if (Box.IsInside(CharacterLocation))
			{
				Character->SetActorLocation(LandingPlatform->GetComponentLocation() + FVector(0,0,100), false, nullptr, ETeleportType::ResetPhysics);
				Character->SetActorRotation(LandingPlatform->GetComponentRotation());
			}
		}
	}
}
