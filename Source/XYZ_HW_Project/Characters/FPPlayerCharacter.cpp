// Fill out your copyright notice in the Description page of Project Settings.


#include "FPPlayerCharacter.h"

#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Controllers/HWPlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "XYZ_HW_Project/XYZ_HW_Types.h"

AFPPlayerCharacter::AFPPlayerCharacter(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	FirstPersonMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>("SkeletalMeshComponent");
	FirstPersonMeshComponent->SetupAttachment(GetCapsuleComponent());
	FirstPersonMeshComponent->SetRelativeLocation(FVector(0.0f, 0.0f, -86.0f));
	FirstPersonMeshComponent->CastShadow = false;
	FirstPersonMeshComponent->bCastDynamicShadow = false;
	FirstPersonMeshComponent->SetOnlyOwnerSee(true);
	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>("CameraComponent");
	FirstPersonCameraComponent->SetupAttachment(FirstPersonMeshComponent, SocketFPCamera);
	FirstPersonCameraComponent->bUsePawnControlRotation = true;

	GetMesh()->SetOwnerNoSee(true);
	GetMesh()->bCastHiddenShadow = true;

	CameraComponent->bAutoActivate = 0;
	SpringArmComponent->bAutoActivate = 0;
	SpringArmComponent->bUsePawnControlRotation = false;

	GetCharacterMovement()->bOrientRotationToMovement = false;
	bUseControllerRotationYaw = true;
}

void AFPPlayerCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	HWPlayerController = Cast<AHWPlayerController>(NewController);
}

bool AFPPlayerCharacter::IsFPMontagePlaying() const
{
	UAnimInstance* FPAnimInstance = FirstPersonMeshComponent->GetAnimInstance();
	return IsValid(FPAnimInstance) && FPAnimInstance->IsAnyMontagePlaying();
}

FRotator AFPPlayerCharacter::GetViewRotation() const
{
	FRotator Result = Super::GetViewRotation();
	if (IsFPMontagePlaying())
	{
		FRotator SocketRotation = FirstPersonMeshComponent->GetSocketRotation(SocketFPCamera);
		Result.Yaw = SocketRotation.Yaw;
		Result.Roll = SocketRotation.Roll;
		Result.Pitch += SocketRotation.Pitch;
	}
	return Result;
}

void AFPPlayerCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (IsFPMontagePlaying() && HWPlayerController.IsValid())
	{
		FRotator TargetControlRotation = HWPlayerController->GetControlRotation();
		TargetControlRotation.Pitch = 0.0f;
		float BlendSpeed = 300.0f;
		TargetControlRotation = FMath::RInterpTo(HWPlayerController->GetControlRotation(), TargetControlRotation, DeltaSeconds, BlendSpeed);
		HWPlayerController->SetControlRotation(TargetControlRotation);
	}
}

void AFPPlayerCharacter::OnStartCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
	Super::OnStartCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);
	const AFPPlayerCharacter* DefaultCharacter = GetDefault<AFPPlayerCharacter>(GetClass());
	FVector& FirstPersonMeshRelativeLocation = FirstPersonMeshComponent->GetRelativeLocation_DirectMutable();
	FirstPersonMeshRelativeLocation.Z = DefaultCharacter->FirstPersonMeshComponent->GetRelativeLocation().Z + HalfHeightAdjust;
	
}

void AFPPlayerCharacter::OnEndCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
	Super::OnEndCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);
	const AFPPlayerCharacter* DefaultCharacter = GetDefault<AFPPlayerCharacter>(GetClass());
	FVector& FirstPersonMeshRelativeLocation = FirstPersonMeshComponent->GetRelativeLocation_DirectMutable();
	FirstPersonMeshRelativeLocation.Z = DefaultCharacter->FirstPersonMeshComponent->GetRelativeLocation().Z;
}

void AFPPlayerCharacter::OnMantle(const FMantlingSettings& MantlingSettings, float MantlingAnimationStartTime)
{
	Super::OnMantle(MantlingSettings, MantlingAnimationStartTime);
	UAnimInstance* FPAnimInstance = FirstPersonMeshComponent->GetAnimInstance();
	if (IsValid(FPAnimInstance) && MantlingSettings.FPMantlingMontage)
	{
		if (HWPlayerController.IsValid())
		{
			HWPlayerController->SetIgnoreLookInput(true);
			HWPlayerController->SetIgnoreMoveInput(true);
		}
		float MontageDuration = FPAnimInstance->Montage_Play(MantlingSettings.FPMantlingMontage, 1.0f, EMontagePlayReturnType::Duration, MantlingAnimationStartTime);
		GetWorld()->GetTimerManager().SetTimer(FPMontageTimer, this, &AFPPlayerCharacter::OnFPMontageTimerElapsed, MontageDuration);
	}
}

void AFPPlayerCharacter::SetCameraLimitation(float CameraMinPitch, float CameraMaxPitch, float CameraMinYaw, float CameraMaxYaw)
{
	APlayerCameraManager* CameraManager = HWPlayerController->PlayerCameraManager;
	CameraManager->ViewPitchMin = CameraMinPitch;
	CameraManager->ViewPitchMax = CameraMaxPitch;
	CameraManager->ViewYawMin = CameraMinYaw;
	CameraManager->ViewYawMax = CameraMaxYaw;
}

void AFPPlayerCharacter::OnHardLanding(float HardLandingTime)
{
	Super::OnHardLanding(HardLandingTime);
	UAnimInstance* AnimInstance = FirstPersonMeshComponent->GetAnimInstance();
	if (IsValid(AnimInstance) && HardLandingSettings.FPHardLandingMontage)
	{
		if (HWPlayerController.IsValid())
		{
			HWPlayerController->SetIgnoreLookInput(true);
			HWPlayerController->SetIgnoreMoveInput(true);
		}
		AnimInstance->Montage_Play(HardLandingSettings.FPHardLandingMontage, 1.0f);
	}
}

void AFPPlayerCharacter::OnHardLandingElapsed()
{
	if (HWPlayerController.IsValid())
	{
		HWPlayerController->SetIgnoreLookInput(false);
		HWPlayerController->SetIgnoreMoveInput(false);
	}
	Super::OnHardLandingElapsed();
}

void AFPPlayerCharacter::OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode)
{
	Super::OnMovementModeChanged(PrevMovementMode, PreviousCustomMode);

	if (PreviousCustomMode == (uint8)ECustomMovementMode::CMOVE_Ladder || PreviousCustomMode == (uint8)ECustomMovementMode::CMOVE_Zipline)
	{
		HWPlayerController->SetIgnoreCameraPitch(false);
		bUseControllerRotationYaw = true;
		APlayerCameraManager* DefaultCameraManager = HWPlayerController->PlayerCameraManager->GetClass()->GetDefaultObject<APlayerCameraManager>();
		SetCameraLimitation(DefaultCameraManager->ViewPitchMin, DefaultCameraManager->ViewPitchMax, DefaultCameraManager->ViewYawMin, DefaultCameraManager->ViewYawMax);
	}
	
	if (GetBaseCharacterMovementComponent()->IsOnLadder())
	{
		if (!HWPlayerController.IsValid())
			return;
		HWPlayerController->SetIgnoreCameraPitch(true);
		bUseControllerRotationYaw = false;
		SetCameraLimitation(LadderCameraMinPitch, LadderCameraMaxPitch, LadderCameraMinYaw, LadderCameraMaxYaw);
	}
	if (GetBaseCharacterMovementComponent()->IsOnZipline())
	{
		if (!HWPlayerController.IsValid())
			return;
		HWPlayerController->SetIgnoreCameraPitch(true);
		bUseControllerRotationYaw = false;
		SetCameraLimitation(ZiplineCameraMinPitch, ZiplineCameraMaxPitch, ZiplineCameraMinYaw, ZiplineCameraMaxYaw);
	}
}

void AFPPlayerCharacter::OnFPMontageTimerElapsed()
{
	if (HWPlayerController.IsValid())
	{
		HWPlayerController->SetIgnoreLookInput(false);
		HWPlayerController->SetIgnoreMoveInput(false);
	}
}
