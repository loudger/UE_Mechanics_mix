// Fill out your copyright notice in the Description page of Project Settings.

#include "PlayerCharacter.h"

#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/CharacterEquipmentComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/KismetSystemLibrary.h"


APlayerCharacter::APlayerCharacter(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;
	
	SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComponent"));
	SpringArmComponent->SetupAttachment(RootComponent);
	SpringArmComponent->bUsePawnControlRotation = true;

	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	CameraComponent->SetupAttachment(SpringArmComponent);
	CameraComponent->bUsePawnControlRotation = false;

	GetCharacterMovement()->bOrientRotationToMovement = 1;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f);

	IKTraceDistance = GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	
}

void APlayerCharacter::MoveForward(float Value)
{
	Super::MoveForward(Value);
	if (HWBaseCharacterMovementComponent->IsSliding())
	{
		AddMovementInput(GetActorForwardVector(), 1);
		return;
	}
	if ((GetCharacterMovement()->IsWalking() || GetCharacterMovement()->IsFalling()) && !FMath::IsNearlyZero(Value))
	{
		FVector Forward = CameraComponent->GetForwardVector();
		FVector ForwardVector = FVector(Forward.X,Forward.Y,0.0f);
		ForwardVector.Normalize();
		AddMovementInput(ForwardVector, Value);
	}
}

void APlayerCharacter::MoveRight(float Value)
{
	Super::MoveRight(Value);
	if (HWBaseCharacterMovementComponent->IsSliding())
		return;
	if ((GetCharacterMovement()->IsWalking() || GetCharacterMovement()->IsFalling()) && !FMath::IsNearlyZero(Value))
	{
		FVector Forward = CameraComponent->GetRightVector();
		FVector ForwardVector = FVector(Forward.X,Forward.Y,0.0f);
		ForwardVector.Normalize();
		AddMovementInput(ForwardVector, Value);
	}
}

void APlayerCharacter::Turn(float Value)
{
	Super::Turn(Value);
	AddControllerYawInput(Value * GetAimTurnModifier());
}

void APlayerCharacter::LookUp(float Value)
{
	Super::LookUp(Value);
	AddControllerPitchInput(Value * GetAimLookUpModifier());
}

void APlayerCharacter::Jump()
{
	if (IsMoveInputIgnored())
		return;
	if (GetCharacterMovement()->IsCrouching())
	{
		ChangeCrouchState();
	}
	else if (!GetBaseCharacterMovementComponent()->GetIsOutOfStamina())
	{
		Super::Jump();
	}
}

void APlayerCharacter::ChangeCrouchState()
{
	if (GetCharacterMovement()->IsCrouching())
	{
		UnCrouch();
	} else
	{
		Crouch();
	}
}

void APlayerCharacter::OnStartCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
	Super::OnStartCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);
	SpringArmComponent->TargetOffset += FVector(0.0f, 0.0f, HalfHeightAdjust);
}

void APlayerCharacter::OnEndCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
	Super::OnEndCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);
	SpringArmComponent->TargetOffset -= FVector(0.0f, 0.0f, HalfHeightAdjust);
}

void APlayerCharacter::SwimForward(float Value)
{
	Super::SwimForward(Value);
	if (GetCharacterMovement()->IsSwimming() && !FMath::IsNearlyZero(Value))
	{
		FVector ForwardVector = CameraComponent->GetForwardVector();
		ForwardVector.Normalize();
		AddMovementInput(ForwardVector, Value);
	}
}

void APlayerCharacter::SwimRight(float Value)
{
	Super::SwimRight(Value);
	if (GetCharacterMovement()->IsSwimming() && !FMath::IsNearlyZero(Value))
	{
		FVector Forward = CameraComponent->GetRightVector();
		FVector ForwardVector = FVector(Forward.X,Forward.Y,0.0f);
		ForwardVector.Normalize();
		AddMovementInput(ForwardVector, Value);
	}
}

void APlayerCharacter::SwimUp(float Value)
{
	Super::SwimUp(Value);
	if (GetCharacterMovement()->IsSwimming() && !FMath::IsNearlyZero(Value))
	{
		AddMovementInput(FVector::UpVector, Value);
	}
}

void APlayerCharacter::OnStartProne(float HeightAdjust)
{
	Super::OnStartProne(HeightAdjust);
	if (IsValid(SpringArmComponent))
	{
		FVector& SpringArmRelativeLocation = SpringArmComponent->GetRelativeLocation_DirectMutable();
		SpringArmRelativeLocation.Z += HeightAdjust;
	}
}

void APlayerCharacter::OnEndProne(float HeightAdjust)
{
	Super::OnEndProne(HeightAdjust);
	if (IsValid(SpringArmComponent))
	{
		FVector& SpringArmRelativeLocation = SpringArmComponent->GetRelativeLocation_DirectMutable();
		SpringArmRelativeLocation.Z -= HeightAdjust;
	}
}

void APlayerCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	AimingTimeline.TickTimeline(DeltaSeconds);

	IKRightFootOffset = FMath::FInterpTo(IKRightFootOffset, GetIKOffsetForASocket(RightFootSocketName), DeltaSeconds, 20.0f);
	IKLeftFootOffset = FMath::FInterpTo(IKLeftFootOffset, GetIKOffsetForASocket(LeftFootSocketName), DeltaSeconds, 20.0f);
	if (bActivateIKDistanceDebug)
	{
		GEngine->AddOnScreenDebugMessage(1, 2.0f, FColor::Red, FString::Printf(TEXT("IKRightFootOffset - %f"), IKRightFootOffset));
		GEngine->AddOnScreenDebugMessage(2, 2.0f, FColor::Blue, FString::Printf(TEXT("IKLeftFootOffset - %f"), IKLeftFootOffset));
	}

}

void APlayerCharacter::AimingTimelineUpdate(const float Alpha)
{
	APlayerController* PlayerController = GetController<APlayerController>();
	if (!IsValid(PlayerController))
		return;

	APlayerCameraManager* PlayerCameraManager = PlayerController->PlayerCameraManager;
	if (!IsValid(PlayerCameraManager))
		return;

	ARangeWeaponItem* CurrentRangeWeapon = GetCharacterEquipmentComponent()->GetCurrentRangeWeapon();
	if (!IsValid(CurrentRangeWeapon))
		return;

	const float CurrentFOV = FMath::Lerp(PlayerCameraManager->DefaultFOV, CurrentRangeWeapon->GetAimFOV(), Alpha);
	PlayerCameraManager->SetFOV(CurrentFOV);
}

void APlayerCharacter::OnStartAimingInternal()
{
	Super::OnStartAimingInternal();

	ARangeWeaponItem* CurrentRangeWeapon = GetCharacterEquipmentComponent()->GetCurrentRangeWeapon();
	if (!IsValid(CurrentRangeWeapon))
		return;

	FOnTimelineFloatStatic DelegateAimingUpdate;
	DelegateAimingUpdate.BindUObject(this, &APlayerCharacter::AimingTimelineUpdate);
	AimingTimeline.AddInterpFloat(CurrentRangeWeapon->GetAimCurve(), DelegateAimingUpdate);
	AimingTimeline.PlayFromStart(); 
}

void APlayerCharacter::OnStopAimingInternal()
{
	Super::OnStopAimingInternal();
	APlayerController* PlayerController = GetController<APlayerController>();
	if (!IsValid(PlayerController))
		return;

	APlayerCameraManager* PlayerCameraManager = PlayerController->PlayerCameraManager;
	if (!IsValid(PlayerCameraManager))
		return;

	ARangeWeaponItem* CurrentRangeWeapon = GetCharacterEquipmentComponent()->GetCurrentRangeWeapon();
	if (!IsValid(CurrentRangeWeapon))
		return;

	PlayerCameraManager->UnlockFOV();
}

float APlayerCharacter::GetIKOffsetForASocket(const FName& SocketName)
{
	float Result = 0.0f;

	FVector SocketLocation = GetMesh()->GetSocketLocation(SocketName);
	FVector TraceStart(SocketLocation.X, SocketLocation.Y, GetActorLocation().Z);
	FVector TraceEnd = TraceStart - IKTraceDistance * FVector::UpVector;

	FHitResult HitResult;
	ETraceTypeQuery TraceType = UEngineTypes::ConvertToTraceType(ECC_Visibility);
	if (UKismetSystemLibrary::LineTraceSingle(GetWorld(), TraceStart, TraceEnd, TraceType, true, TArray<AActor*>(), EDrawDebugTrace::ForOneFrame, HitResult, true))
	{
		Result = (TraceEnd.Z - HitResult.Location.Z);
		return Result;
	}
	if (UKismetSystemLibrary::LineTraceSingle(GetWorld(), TraceEnd, TraceEnd - IKTraceExtendDistance * FVector::UpVector, TraceType, true, TArray<AActor*>(), EDrawDebugTrace::ForOneFrame, HitResult, true))
	{
		Result = (TraceEnd.Z - HitResult.Location.Z);
		return Result;
	}
	return Result;
}


