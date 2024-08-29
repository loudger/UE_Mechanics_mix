// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerHUDWidget.h"

#include "Blueprint/WidgetTree.h"
#include "XYZ_HW_Project/Characters/HWBaseCharacter.h"

UReticleWidget* UPlayerHUDWidget::GetReticleWidget()
{
	return WidgetTree->FindWidget<UReticleWidget>(ReticleWidgetName);
}

UAmmoWidget* UPlayerHUDWidget::GetAmmoWidget()
{
	return WidgetTree->FindWidget<UAmmoWidget>(AmmoWidgetName);
}

float UPlayerHUDWidget::GetHealthPercent() const
{
	float Result = 1.0f;
	APawn* Pawn = GetOwningPlayerPawn();
	AHWBaseCharacter* BaseCharacter = Cast<AHWBaseCharacter>(Pawn);
	if (IsValid(BaseCharacter))
	{
		Result = BaseCharacter->GetHealthPercent();
	}
	return Result;
}
