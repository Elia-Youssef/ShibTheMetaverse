// Fill out your copyright notice in the Description page of Project Settings.


#include "ShibUiBasePage.h"

void UShibUiBasePage::NativeConstruct()
{
	Super::NativeConstruct();

	OnPageChangeDelegate.AddUniqueDynamic(this, &ThisClass::OnPageChange);
}

void UShibUiBasePage::NativeDestruct()
{
	Super::NativeDestruct();

	GetWorld()->GetTimerManager().ClearTimer(LifespanHandle);
	LifespanDelegate.Unbind();
	OnPageChangeDelegate.Clear();
}

void UShibUiBasePage::Internal_ConstructPage(ENavigationDirection NavigationDirection)
{
	SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	ConstructPage(NavigationDirection);
}

void UShibUiBasePage::Internal_DestroyPage(ENavigationDirection NavigationDirection, bool bRemoveFromParent)
{
	SetVisibility(ESlateVisibility::HitTestInvisible);
	
	DestroyPage(NavigationDirection);

	if (bRemoveFromParent)
	{
		LifespanDelegate.BindLambda([this]()
		{
			RemoveFromParent();
		});
		GetWorld()->GetTimerManager().SetTimer(LifespanHandle, LifespanDelegate, 1, false, PageLifespan);
	}
}
