// Fill out your copyright notice in the Description page of Project Settings.


#include "CharacterEquipmentComponent.h"

#include "XYZ_HW_Project/XYZ_HW_Types.h"

// Called when the game starts
void UCharacterEquipmentComponent::BeginPlay()
{
	Super::BeginPlay();

	checkf(GetOwner()->IsA<AHWBaseCharacter>(), TEXT("UCharacterEquipmentComponent::BeginPlay can be used only with AHWBaseCharacter"));
	CachedBaseCharacter = StaticCast<AHWBaseCharacter*>(GetOwner());
	CreateLoadout();
}


ARangeWeaponItem* UCharacterEquipmentComponent::GetCurrentRangeWeapon() const
{
	return CurrentEquippedWeapon.Get();
}

EEquipableItemType UCharacterEquipmentComponent::GetCurrentEquippedWeaponType() const
{
	EEquipableItemType Result = EEquipableItemType::None;
	if (CurrentEquippedWeapon.IsValid())
	{
		Result = CurrentEquippedWeapon->GetItemType();
	}
	return Result;
}

void UCharacterEquipmentComponent::CreateLoadout()
{
	if (!IsValid(SideArmClass))
		return;
	CurrentEquippedWeapon = GetWorld()->SpawnActor<ARangeWeaponItem>(SideArmClass);
	CurrentEquippedWeapon->AttachToComponent(CachedBaseCharacter->GetMesh(), FAttachmentTransformRules::KeepRelativeTransform, SocketCharacterWeapon);
	CurrentEquippedWeapon->SetOwner(CachedBaseCharacter.Get());
	CurrentEquippedWeapon->OnAmmoChanged.AddUFunction(this, FName("OnCurrentWeaponAmmoChanged"));
	OnCurrentWeaponAmmoChanged(CurrentEquippedWeapon->GetAmmo());
}

void UCharacterEquipmentComponent::OnCurrentWeaponAmmoChanged(int32 Ammo)
{
	if (OnCurrentWeaponAmmoChangedEvent.IsBound())
	{
		OnCurrentWeaponAmmoChangedEvent.Broadcast(Ammo);
	}
}
