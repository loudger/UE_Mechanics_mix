// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "XYZ_HW_Project/ActorComponents/Weapons/WeaponBarellComponent.h"
#include "XYZ_HW_Project/Actors/Equipment/EquipableItem.h"
#include "RangeWeaponItem.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FOnAmmoChanged, int32);

class UAnimMontage;

UENUM(BlueprintType)
enum class EWeaponFireMode : uint8
{
	Single,
	FullAuto
};

UCLASS(Blueprintable)
class XYZ_HW_PROJECT_API ARangeWeaponItem : public AEquipableItem
{
	GENERATED_BODY()

public:
	ARangeWeaponItem();
	virtual void BeginPlay() override;
	
	void StartFire();
	void StopFire();

	void StartAim();
	void StopAim();

	float GetAimFOV() const {return AimFOV;}
	float GetAimTurnModifier() const {return AimTurnModifier;}
	float GetAimLookUpModifier() const {return AimLookUpModifier;}
	float GetAimMovementMaxSpeed() const {return AimMovementMaxSpeed;}
	float GetCurrentBulletSpreadAngle();
	UCurveFloat* GetAimCurve() const {return AimingCurve;}

	FTransform GetForeGripTransform() const;

	int32 GetAmmo() const;
	void SetAmmo(int32 NewAmmo);
	bool CanShoot() const;

	FOnAmmoChanged OnAmmoChanged;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USkeletalMeshComponent* WeaponMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UWeaponBarellComponent* WeaponBarellComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animations | Weapon")
	UAnimMontage* WeaponFireMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animations | Character")
	UAnimMontage* CharacterFireMontage;

	// Rate of fire in rounds per minutes
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon | Parameters", meta = (ClampMin = 1.0f, UIMin = 1.0f))
	float RateOfFire = 600;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon | FireMode")
	EWeaponFireMode FireMode = EWeaponFireMode::Single;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon | Parameters", meta = (ClampMin = 0.0f, UIMin = 0.0f, ClampMax = 2.0f, UIMax = 2.0f))
	float SpreadAngle = 1.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon | Parameters | Aiming", meta = (ClampMin = 0.0f, UIMin = 0.0f, ClampMax = 2.0f, UIMax = 2.0f))
	float AimSpreadAngle = 0.25f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon | Parameters | Aiming")
	float AimMovementMaxSpeed = 200.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon | Parameters | Aiming")
	float AimFOV = 60.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon | Parameters | Aiming")
	UCurveFloat* AimingCurve;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon | Parameters | Aiming")
	float AimTurnModifier = 0.5f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon | Parameters | Aiming")
	float AimLookUpModifier = 0.5f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon |Parameters | Ammo", meta = (ClampMin = 1, UIMin = 1))
	int32 MaxAmmo = 30;
	
private:
	int32 Ammo = 0;
	
	FTimerHandle ShotTimer;

	FVector GetBulletSpreadOffset(float Angle, FRotator ShotRotation);

	float GetShotTimerInterval();

	void MakeShot();

	bool bIsAiming;
	
	float PlayAnimMontage(UAnimMontage* AnimMontage);
};
