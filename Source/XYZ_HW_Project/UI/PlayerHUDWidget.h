// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AmmoWidget.h"
#include "ReticleWidget.h"
#include "Blueprint/UserWidget.h"
#include "PlayerHUDWidget.generated.h"

/**
 * 
 */
UCLASS()
class XYZ_HW_PROJECT_API UPlayerHUDWidget : public UUserWidget
{
	GENERATED_BODY()

public:

	UReticleWidget* GetReticleWidget();

	UAmmoWidget* GetAmmoWidget();

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Widget Names")
	FName ReticleWidgetName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Widget Names")
	FName AmmoWidgetName;
	
	UFUNCTION(BlueprintCallable)
	float GetHealthPercent() const;
	
};
