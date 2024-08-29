// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "XYZ_HW_Project/Actors/Equipment/Weapons/RangeWeaponItem.h"
#include "XYZ_HW_Project/Characters/HWBaseCharacter.h"
#include "CharacterEquipmentComponent.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FOnCurrentWeaponAmmoChanged, int32);


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class XYZ_HW_PROJECT_API UCharacterEquipmentComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;

	ARangeWeaponItem* GetCurrentRangeWeapon() const;

	EEquipableItemType GetCurrentEquippedWeaponType() const;

	FOnCurrentWeaponAmmoChanged OnCurrentWeaponAmmoChangedEvent;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Loadout")
	TSubclassOf<ARangeWeaponItem> SideArmClass;

private:
	void CreateLoadout();

	UFUNCTION()
	void OnCurrentWeaponAmmoChanged(int32 Ammo);
	
	TWeakObjectPtr<ARangeWeaponItem> CurrentEquippedWeapon;
	TWeakObjectPtr<AHWBaseCharacter> CachedBaseCharacter;
};
