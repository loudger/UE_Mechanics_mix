// Fill out your copyright notice in the Description page of Project Settings.


#include "HWBaseCharacterAnimInstance.h"
#include "XYZ_HW_Project/Characters/Components/CharacterEquipmentComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "XYZ_HW_Project/Characters/HWBaseCharacter.h"

void UHWBaseCharacterAnimInstance::NativeBeginPlay()
{
	Super::NativeBeginPlay();

	checkf(TryGetPawnOwner()->IsA<AHWBaseCharacter>(), TEXT("UHWBaseCharacterAnimInstance::NativeBeginPlay() TryGetPawnOwner is not AHWBaseCharacter"));
	CachedBaseCharacter = StaticCast<AHWBaseCharacter*>(TryGetPawnOwner());
}

void UHWBaseCharacterAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (!CachedBaseCharacter.IsValid())
		return;

	UHWBaseCharacterMovementComponent* CharacterMovement = CachedBaseCharacter->GetBaseCharacterMovementComponent();
	AimRotator = CachedBaseCharacter->GetBaseAimRotation();
		
	Speed = CharacterMovement->Velocity.Size();
	bIsFalling = CharacterMovement->IsFalling();
	bIsCrouch = CharacterMovement->IsCrouching();
	bIsSprinting = CharacterMovement->IsSprinting();
	bIsOutOfStamina = CharacterMovement->GetIsOutOfStamina();
	bIsSwimming = CharacterMovement->IsSwimming();
	bIsProning = CharacterMovement->IsProning();
	bIsOnLadder = CharacterMovement->IsOnLadder();
	if (bIsOnLadder)
		LadderSpeedRatio = CharacterMovement->GetLadderSpeedRation();
	bIsOnZipline = CharacterMovement->IsOnZipline();
	bIsWallRunning = CharacterMovement->IsWallRunning();
	if (bIsWallRunning)
		bIsWallRunRightWall = CharacterMovement->IsWallRunRightWall();
	bIsSliding = CharacterMovement->IsSliding();
	bIsStrafing = !CharacterMovement->bOrientRotationToMovement;
	Direction = CalculateDirection(CharacterMovement->Velocity, CachedBaseCharacter->GetActorRotation());
	const UCharacterEquipmentComponent* CharacterEquipmentComponent = CachedBaseCharacter->GetCharacterEquipmentComponent();
	CurrentEquipableItem = CharacterEquipmentComponent->GetCurrentEquippedWeaponType();
	bIsAiming = CachedBaseCharacter->IsAiming();
	
	ARangeWeaponItem* CurrentRangeWeapon = CharacterEquipmentComponent->GetCurrentRangeWeapon();
	if (IsValid(CurrentRangeWeapon))
	{
		ForeGripSocketTransform = CurrentRangeWeapon->GetForeGripTransform();
	}
}
