// Fill out your copyright notice in the Description page of Project Settings.


#include "HWBaseCharacter.h"

#include "Components/CapsuleComponent.h"
#include "Components/CharacterAttributeComponent.h"
#include "Components/CharacterEquipmentComponent.h"
#include "Curves/CurveVector.h"
#include "Engine/DamageEvents.h"
#include "XYZ_HW_Project/XYZ_HW_Types.h"
#include "XYZ_HW_Project/ActorComponents/LedgeDetectorComponent.h"
#include "XYZ_HW_Project/Actors/Interactive/InteractiveActor.h"
#include "XYZ_HW_Project/Actors/Interactive/Environment/Ladder.h"
#include "XYZ_HW_Project/Actors/Interactive/Environment/Zipline.h"

class UHWBaseCharacterMovementComponent;

void AHWBaseCharacter::TryChangeSprintState(float DeltaTime)
{
	// UE_LOG(LogTemp, Log, TEXT("MaxWalkSpeed = %f"), GetCharacterMovement()->MaxWalkSpeed)
	if (bIsSprintRequested && !HWBaseCharacterMovementComponent->IsSprinting() && CanSprint())
	{
		HWBaseCharacterMovementComponent->StartSprint();
		OnSprintStart();
	}

	if (!bIsSprintRequested && HWBaseCharacterMovementComponent->IsSprinting())
	{
		HWBaseCharacterMovementComponent->StopSprint();
		OnSprintStop();
	}
}

void AHWBaseCharacter::SetIsOutOfStamina(bool StaminaOut)
{
	HWBaseCharacterMovementComponent->SetIsOutOfStamina(StaminaOut);
	if (StaminaOut && HWBaseCharacterMovementComponent->IsSprinting())
	{
		StopSprint();
	}
}

void AHWBaseCharacter::Mantle(bool bForce)
{
	// Если персонаж лежит, сразу заканчиваем выполнение метода
	if (!(CanMantle() || bForce))
		return;
	
	FLedgeDescription LedgeDescription;
	if (!HWBaseCharacterMovementComponent->IsMantling() && LedgeDetectorComponent->DetectLedge(LedgeDescription))
	{
		ACharacter* DefaultCharacter = GetClass()->GetDefaultObject<ACharacter>();
		UCapsuleComponent* DefaultCapsuleComponent = DefaultCharacter->GetCapsuleComponent();
		
		FMantlingMovementParameters MantlingParameters;
		MantlingParameters.InitialLocation = GetActorLocation();
		MantlingParameters.InitialRotation = GetActorRotation();
		MantlingParameters.TargetLocation = LedgeDescription.Location;
		MantlingParameters.TargetRotation = LedgeDescription.Rotation;
		MantlingParameters.LedgeComponent = LedgeDescription.LedgeComponent;
		MantlingParameters.OffsetRelativeToLedge = LedgeDescription.Location - MantlingParameters.LedgeComponent->GetComponentLocation();

		if (bIsCrouched)
		{
			// Если персонаж сидит, следует приподнять его InitialLocation, а то он начнёт подниматься из под земли
			MantlingParameters.InitialLocation.Z += DefaultCapsuleComponent->GetScaledCapsuleHalfHeight()-GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
		}
		
		float MantlingHeight = (MantlingParameters.TargetLocation - MantlingParameters.InitialLocation).Z;
		const FMantlingSettings& MantlingSettings = GetMantlingSettings(MantlingHeight);

		float MinRange;
		float MaxRange;
		MantlingSettings.MantlingCurve->GetTimeRange(MinRange, MaxRange);
		MantlingParameters.Duration = MaxRange - MinRange;

		MantlingParameters.MantlingCurve = MantlingSettings.MantlingCurve;

		FVector2D SourceRange(MantlingSettings.MinHight, MantlingSettings.MaxHight);
		FVector2D TargetRange(MantlingSettings.MinHightStartTime, MantlingSettings.MaxHightStartTime);

		MantlingParameters.StartTime = FMath::GetMappedRangeValueClamped(SourceRange, TargetRange, MantlingHeight);

		MantlingParameters.InitialAnimationLocation = MantlingParameters.TargetLocation - MantlingSettings.AnimationCorrectionZ * FVector::UpVector + MantlingSettings.AnimationCorrectionXY * LedgeDescription.LedgeNormal;
		
		HWBaseCharacterMovementComponent->StartMantle(MantlingParameters);

		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
		AnimInstance->Montage_Play(MantlingSettings.MantlingMontage, 1.0f, EMontagePlayReturnType::Duration, MantlingParameters.StartTime);
		OnMantle(MantlingSettings, MantlingParameters.StartTime);
	}
}

bool AHWBaseCharacter::CanMantle() const
{
	return !GetBaseCharacterMovementComponent()->IsOnLadder() || !GetBaseCharacterMovementComponent()->IsOnZipline() || !bIsProne;
}

void AHWBaseCharacter::OnMantle(const FMantlingSettings& MantlingSettings, float MantlingAnimationStartTime)
{
}

bool AHWBaseCharacter::CanJumpInternal_Implementation() const
{
	return Super::CanJumpInternal_Implementation() && !GetBaseCharacterMovementComponent()->IsMantling();
}

float AHWBaseCharacter::GetHealthPercent() const
{
	return CharacterAttributesComponent->GetHealthPercent();
}

void AHWBaseCharacter::StartFire()
{
	ARangeWeaponItem* CurrentRangeWeapon = CharacterEquipmentComponent->GetCurrentRangeWeapon();
	if (IsValid(CurrentRangeWeapon))
	{
		CurrentRangeWeapon->StartFire();
	}
}

void AHWBaseCharacter::StopFire()
{
	ARangeWeaponItem* CurrentRangeWeapon = CharacterEquipmentComponent->GetCurrentRangeWeapon();
	if (IsValid(CurrentRangeWeapon))
	{
		CurrentRangeWeapon->StopFire();
	}
}

UCharacterEquipmentComponent* AHWBaseCharacter::GetCharacterEquipmentComponent()
{
	return CharacterEquipmentComponent;
}

void AHWBaseCharacter::RegisterInteractiveActor(AInteractiveActor* InteractiveActor)
{
	AvailableInteractiveActrors.AddUnique(InteractiveActor);
}

void AHWBaseCharacter::UnRegisterInteractiveActor(AInteractiveActor* InteractiveActor)
{
	AvailableInteractiveActrors.RemoveSingleSwap(InteractiveActor);
}

void AHWBaseCharacter::OnCharacterCapsuleHit(UPrimitiveComponent* HitComponent, AActor* OtherActor,
                                             UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	
	// Check that character has collided with vertical wall
	float DotResult = FVector::DotProduct(Hit.ImpactNormal, FVector::UpVector);
	// GEngine->AddOnScreenDebugMessage(7, 10.0f, FColor::Blue, FString::Printf(TEXT("%f"), DotResult));
	if (FMath::Abs(DotResult) > 0.5f)
		HWBaseCharacterMovementComponent->ResetLastWallRunSide();

	if (FMath::IsNearlyZero(DotResult) && CanWallRun(Hit))
		DoWallRun(Hit);
}

ALadder* AHWBaseCharacter::GetAvailableLadder()
{
	ALadder* Result = nullptr;
	for (AInteractiveActor* InteractiveActor : AvailableInteractiveActrors)
	{
		if (InteractiveActor->IsA<ALadder>())
		{
			Result = StaticCast<ALadder*>(InteractiveActor);
			break;
		}
	}
	return Result;
}

AZipline* AHWBaseCharacter::GetAvailableZipline()
{
	AZipline* Result = nullptr;
	for (AInteractiveActor* InteractiveActor : AvailableInteractiveActrors)
	{
		if(InteractiveActor->IsA<AZipline>())
		{
			Result = StaticCast<AZipline*>(InteractiveActor);
			break;
		}
	}
	return Result;
}

void AHWBaseCharacter::DoWallRun(const FHitResult& Hit)
{

	FWallRunningParameters WallRunningParameters;

	WallRunningParameters.InitialLocation = GetActorLocation();
	WallRunningParameters.InitialRotation = GetActorRotation();
	WallRunningParameters.WallNormal = Hit.ImpactNormal;

	float WallRunMaxDistance = HWBaseCharacterMovementComponent->GetWallRunMaxDistance();
	FVector WallRunDirection = FVector::ZeroVector;

	FVector FromCharacterToWallVector = FVector::ZeroVector;
	// Determine wall side and RunDirection
	if (FVector::DotProduct(GetActorRightVector(), Hit.ImpactNormal) > 0)
	{
		WallRunningParameters.WallSide = EWallRunSide::Left;
		FromCharacterToWallVector = - GetOwner()->GetActorRightVector();
		WallRunDirection = FVector::CrossProduct(Hit.ImpactNormal, GetActorUpVector());
		if (HWBaseCharacterMovementComponent->GetLastWallRunSide() == EWallRunSide::Left)
			return;
	}
	else
	{
		WallRunningParameters.WallSide = EWallRunSide::Right;
		FromCharacterToWallVector = GetOwner()->GetActorRightVector();
		WallRunDirection = FVector::CrossProduct(GetActorUpVector(), Hit.ImpactNormal);
		if (HWBaseCharacterMovementComponent->GetLastWallRunSide() == EWallRunSide::Right)
			return;
	}
	WallRunningParameters.RunDirection = WallRunDirection;

	// Check if wall is WallRunnable
	FHitResult LineTraceHit;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);
	if (!GetWorld()->LineTraceSingleByChannel(LineTraceHit, GetActorLocation(), GetActorLocation() + FromCharacterToWallVector * (HWBaseCharacterMovementComponent->GetWallRunOffset() + 50.0f), ECC_WallRunnable, QueryParams))
		return;

	// Do sweep and determine TargetLocation
	FHitResult SweepHitResult;
	const FCollisionShape CollisionShape = FCollisionShape::MakeCapsule(GetCapsuleComponent()->GetScaledCapsuleRadius(), GetCapsuleComponent()->GetScaledCapsuleHalfHeight());

	FVector WallRunMaxTargetLocation = WallRunningParameters.InitialLocation + WallRunDirection * WallRunMaxDistance;
	bool bResult = GetWorld()->SweepSingleByChannel(SweepHitResult, WallRunningParameters.InitialLocation, WallRunMaxTargetLocation, FQuat::Identity, ECC_Visibility, CollisionShape);
	if (bResult)
	{
		WallRunningParameters.TargetLocation = SweepHitResult.Location;
		// DrawDebugCapsule(GetWorld(), SweepHitResult.Location, GetCapsuleComponent()->GetScaledCapsuleHalfHeight(), GetCapsuleComponent()->GetScaledCapsuleRadius(), FQuat::Identity, FColor::Cyan, false, 20.0f);
	} else
	{
		WallRunningParameters.TargetLocation = WallRunMaxTargetLocation;
		// DrawDebugCapsule(GetWorld(), WallRunMaxTargetLocation, GetCapsuleComponent()->GetScaledCapsuleHalfHeight(), GetCapsuleComponent()->GetScaledCapsuleRadius(), FQuat::Identity, FColor::Black, false, 20.0f);
	}
	WallRunningParameters.TargetRotation = GetActorRotation();
	WallRunningParameters.LaunchVector = (Hit.ImpactNormal + WallRunDirection + FVector::UpVector).GetSafeNormal();

	// Call StartWallRunning
	HWBaseCharacterMovementComponent->StartWallRunning(WallRunningParameters);
}

bool AHWBaseCharacter::CanWallRun(const FHitResult& Hit)
{
	// Can't start WallRun while already WallRunning
	if (HWBaseCharacterMovementComponent->IsWallRunning())
		return false;
	if (!HWBaseCharacterMovementComponent->IsFalling())
		return false;
	if (!IsForwardMoveActivated())
		return false;
	return true;
}

void AHWBaseCharacter::Slide()
{
	if (HWBaseCharacterMovementComponent->IsSprinting() && HWBaseCharacterMovementComponent->MovementMode == MOVE_Walking && !HWBaseCharacterMovementComponent->IsSliding())
	{
		HWBaseCharacterMovementComponent->StartSlide();
		// GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Blue, (TEXT("0")));
	}
}

void AHWBaseCharacter::Falling()
{
	Super::Falling();
	HWBaseCharacterMovementComponent->bNotifyApex = true;
}

void AHWBaseCharacter::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);
	float FallHeight = FallingApexPosition - GetActorLocation().Z;
	
	if (FallHeight > HardLandingSettings.MinHeightFalling)
	{
		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
		float HardLandingTime = AnimInstance->Montage_Play(HardLandingSettings.HardLandingMontage, 1.0f, EMontagePlayReturnType::Duration);
		OnHardLanding(HardLandingTime);
	}
	if (IsValid(FallDamageCurve))
	{
		float DamageAmount = FallDamageCurve->GetFloatValue(FallHeight);
		TakeDamage(DamageAmount, FDamageEvent(), GetController(), Hit.GetActor());
	}
}

void AHWBaseCharacter::OnHardLanding(float HardLandingTime)
{
	HWBaseCharacterMovementComponent->StartHardLanding();
	GetController()->SetIgnoreMoveInput(true);
	GetWorld()->GetTimerManager().SetTimer(LandingTimer, this, &AHWBaseCharacter::OnHardLandingElapsed, HardLandingTime, false);
}

void AHWBaseCharacter::OnHardLandingElapsed()
{
	GetController()->SetIgnoreMoveInput(false);
	HWBaseCharacterMovementComponent->StopHardLanding();
}

void AHWBaseCharacter::OnDeath()
{
	if (IsValid(OnDeathAnimMontage))
	{
		PlayAnimMontage(OnDeathAnimMontage);
	}
	GetCharacterMovement()->DisableMovement();
}

void AHWBaseCharacter::NotifyJumpApex()
{
	Super::NotifyJumpApex();
	FallingApexPosition = GetActorLocation().Z;
}

void AHWBaseCharacter::OnStartAimingInternal()
{
	if (OnAimingStateChanged.IsBound())
	{
		OnAimingStateChanged.Broadcast(true);
	}
}

void AHWBaseCharacter::OnStopAimingInternal()
{
	if (OnAimingStateChanged.IsBound())
	{
		OnAimingStateChanged.Broadcast(false);
	}
}


void AHWBaseCharacter::AimingTimelineUpdate(const float Alpha)
{
}

const FMantlingSettings AHWBaseCharacter::GetMantlingSettings(float LedgeHeight) const
{
	return LedgeHeight > LowMantleMaxHeight ? HighMantleSettings : LowMantleSettings;
}

// void AHWBaseCharacter::EnableRagdoll()
// {
// 	GetMesh()->SetCollisionProfileName(CollisionProfileRagdoll);
// 	GetMesh()->SetSimulatePhysics(true);
// }

void AHWBaseCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	TryChangeSprintState(DeltaTime);
}

void AHWBaseCharacter::MoveForward(float Value)
{
	ForwardInputValue = Value;
}

AHWBaseCharacter::AHWBaseCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UHWBaseCharacterMovementComponent>(ACharacter::CharacterMovementComponentName))
{
	HWBaseCharacterMovementComponent = Cast<UHWBaseCharacterMovementComponent>(GetCharacterMovement());
	CurrentStamina = MaxStamina;

	LedgeDetectorComponent = CreateDefaultSubobject<ULedgeDetectorComponent>(TEXT("LedgeDetector"));

	CharacterAttributesComponent = CreateDefaultSubobject<UCharacterAttributeComponent>("CharacterAttributesComponent");

	CharacterEquipmentComponent = CreateDefaultSubobject<UCharacterEquipmentComponent>("CharacterEquipmentComponent");
}

void AHWBaseCharacter::BeginPlay()
{
	Super::BeginPlay();

	GetCapsuleComponent()->OnComponentHit.AddDynamic(this, &AHWBaseCharacter::OnCharacterCapsuleHit);

	CharacterAttributesComponent->OnDeathEvent.AddUObject(this, &AHWBaseCharacter::OnDeath);
	CharacterAttributesComponent->OnStaminaEvent.AddUObject(this, &AHWBaseCharacter::SetIsOutOfStamina);	
}

void AHWBaseCharacter::ChangeProneState()
{
	if (GetBaseCharacterMovementComponent()->IsProning())
	{
		UnProne();
	} else
	{
		Prone();
	}
}

bool AHWBaseCharacter::CanProne()
{
	return true;
}

bool AHWBaseCharacter::CanUnProne()
{
	// Проверка на пересечение с другими объемами
	FCollisionQueryParams QueryParams(NAME_None, false, this); // this означает игнорирование актора, который выполняет запрос
	FVector UpLocation = GetActorLocation() + FVector(0.0f, 0.0f, 50.0f);
	FCollisionShape MyColShape = FCollisionShape::MakeCapsule(20.0f, 40.0f);

	bool bHasBlocking = GetWorld()->OverlapBlockingTestByChannel(
		UpLocation,
		FQuat::Identity,
		ECollisionChannel::ECC_Visibility,
		MyColShape,
		QueryParams
	);
	return !bHasBlocking;
}

void AHWBaseCharacter::Prone()
{
	HWBaseCharacterMovementComponent->bWantsToProne = true;
}

void AHWBaseCharacter::UnProne()
{
	HWBaseCharacterMovementComponent->bWantsToProne = false;
}

void AHWBaseCharacter::OnStartProne(float HeightAdjust)
{
	const ACharacter* DefaultChar = GetDefault<ACharacter>(GetClass());
	if (GetMesh() && DefaultChar->GetMesh())
	{
		FVector& MeshRelativeLocation = GetMesh()->GetRelativeLocation_DirectMutable();
		MeshRelativeLocation.Z = DefaultChar->GetMesh()->GetRelativeLocation().Z + HeightAdjust;

		// Не понял зачем нужно также переопределять BaseTranslationOffset, без него отлично работает
		// BaseTranslationOffset.Z = MeshRelativeLocation.Z + HeightAdjust;
	}
}

void AHWBaseCharacter::OnEndProne(float HeightAdjust)
{
	const ACharacter* DefaultChar = GetDefault<ACharacter>(GetClass());
	if (GetMesh() && DefaultChar->GetMesh())
	{
		FVector& MeshRelativeLocation = GetMesh()->GetRelativeLocation_DirectMutable();
		MeshRelativeLocation.Z = DefaultChar->GetMesh()->GetRelativeLocation().Z - HeightAdjust;
	}
}

void AHWBaseCharacter::StartAiming()
{
	ARangeWeaponItem* CurrentRangeWeapon = GetCharacterEquipmentComponent()->GetCurrentRangeWeapon();
	if (!IsValid(CurrentRangeWeapon))
		return;

	bIsAiming = true;
	CurrentAimingMovementSpeed = CurrentRangeWeapon->GetAimMovementMaxSpeed();
	CurrentRangeWeapon->StartAim();
	OnStartAiming();
}

void AHWBaseCharacter::StopAiming()
{
	if (!bIsAiming)
	{
		return;
	}
	bIsAiming = false;
	CurrentAimingMovementSpeed = 0.0f;
	ARangeWeaponItem* CurrentRangeWeapon = GetCharacterEquipmentComponent()->GetCurrentRangeWeapon();
	if (IsValid(CurrentRangeWeapon))
		CurrentRangeWeapon->StopAim();
	OnStopAiming();
}

bool AHWBaseCharacter::IsAiming() const
{
	return bIsAiming;
}

float AHWBaseCharacter::GetAimingMovementSpeed() const
{
	return CurrentAimingMovementSpeed;
}

float AHWBaseCharacter::GetAimTurnModifier()
{
	float DefaultResult = 1;

	if (!IsAiming())
		return DefaultResult;
	
	if (!IsValid(CharacterEquipmentComponent))
		return DefaultResult;
	
	ARangeWeaponItem* CurrentRangeWeapon = CharacterEquipmentComponent->GetCurrentRangeWeapon();
	if (!IsValid(CurrentRangeWeapon))
		return DefaultResult;

	return CurrentRangeWeapon->GetAimTurnModifier();
}

float AHWBaseCharacter::GetAimLookUpModifier()
{
	float DefaultResult = 1;
	if (!IsAiming())
		return DefaultResult;
	
	if (!IsValid(CharacterEquipmentComponent))
		return DefaultResult;
	
	ARangeWeaponItem* CurrentRangeWeapon = CharacterEquipmentComponent->GetCurrentRangeWeapon();
	if (!IsValid(CurrentRangeWeapon))
		return DefaultResult;

	return CurrentRangeWeapon->GetAimLookUpModifier();
}

void AHWBaseCharacter::OnStartAiming_Implementation()
{
	OnStartAimingInternal();
}

void AHWBaseCharacter::OnStopAiming_Implementation()
{
	OnStopAimingInternal();
}

void AHWBaseCharacter::StartSprint()
{
	if (GetBaseCharacterMovementComponent()->IsProning())
		return;
	bIsSprintRequested = true;
	if (bIsCrouched)
	{
		UnCrouch();
	}
}

void AHWBaseCharacter::StopSprint()
{
	bIsSprintRequested = false;
}

bool AHWBaseCharacter::CanSprint()
{
	// Не может бегать в состояннии OutOfStamina
	return !HWBaseCharacterMovementComponent->GetIsOutOfStamina();
}

void AHWBaseCharacter::Jump()
{
	if (bIsProne)
	{
		// Если персонаж ползёт, тогда при прыжке он просто встаёт
		HWBaseCharacterMovementComponent->bWantsToProne = false;
		HWBaseCharacterMovementComponent->bWantsToCrouch = 0;
	} else if (HWBaseCharacterMovementComponent->IsWallRunning())
	{
		HWBaseCharacterMovementComponent->EndWallRunning(true);
	} else if (HWBaseCharacterMovementComponent->IsSliding())
	{
		// Do nothing while Sliding
	} else
	{
		Super::Jump();
	}
}

bool AHWBaseCharacter::IsForwardMoveActivated()
{
	if (ForwardInputValue > 0.1f)
		return true;
	return false;
}

bool AHWBaseCharacter::IsSwimmingUnderWater()
{
	if (GetBaseCharacterMovementComponent()->IsSwimming())
	{
		FVector HeaderLocation = GetMesh()->GetSocketLocation(BoneHead);
		float VolumeTopPlane = GetBaseCharacterMovementComponent()->GetCurrentVolumeTopPlane();
		if (HeaderLocation.Z >= VolumeTopPlane)
			return true;
		return false;
	}
	return true;
}

void AHWBaseCharacter::ClimbLadderUp(float Value)
{
	if (FMath::IsNearlyZero(Value))
		return;

	if (GetBaseCharacterMovementComponent()->IsOnLadder())
	{
		const FVector LadderUpVector = GetBaseCharacterMovementComponent()->GetCurrentLadder()->GetActorUpVector();
		AddMovementInput(LadderUpVector, Value);
	}
}

void AHWBaseCharacter::InteractWithLadder()
{
	if (HWBaseCharacterMovementComponent->IsOnLadder())
	{
		HWBaseCharacterMovementComponent->DetachFromLadder(EDetachFromLadderMethod::JumpOff);
	} else
	{
		ALadder* Ladder = GetAvailableLadder();
		if (IsValid(Ladder))
		{
			if (GetAvailableLadder()->GetIsOnTop())
			{
				// Anim couldn't be imported correctly from UE4 to UE5, looks ugly
				// PlayAnimMontage(GetAvailableLadder()->GetAttachFromTopAnimMontage());
			}
			HWBaseCharacterMovementComponent->AttachToLadder(Ladder);			
		}
	}
}

void AHWBaseCharacter::InteractWithZipline()
{
	if (HWBaseCharacterMovementComponent->IsOnZipline())
	{
		HWBaseCharacterMovementComponent->DetachFromZipline(EDetachFromZiplineMethod::Fall);
	} else
	{
		AZipline* Zipline = GetAvailableZipline();
		if (IsValid(Zipline))
		{
			HWBaseCharacterMovementComponent->AttachToZipline(Zipline);
		}
	}
}