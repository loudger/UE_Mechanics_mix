// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "XYZ_HW_Project/XYZ_HW_Types.h"
#include "EquipableItem.generated.h"

UCLASS(Abstract, NotBlueprintable)
class XYZ_HW_PROJECT_API AEquipableItem : public AActor
{
	GENERATED_BODY()

public:
	EEquipableItemType GetItemType() const { return EquippedItemType; }

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Equipable Item")
	EEquipableItemType EquippedItemType = EEquipableItemType::None;
};
