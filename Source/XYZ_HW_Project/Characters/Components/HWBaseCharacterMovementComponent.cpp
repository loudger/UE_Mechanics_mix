// Fill out your copyright notice in the Description page of Project Settings.


#include "HWBaseCharacterMovementComponent.h"

#include "Components/CapsuleComponent.h"
#include "Curves/CurveVector.h"
#include "GameFramework/Character.h"
#include "GameFramework/PhysicsVolume.h"
#include "XYZ_HW_Project/XYZ_HW_Types.h"
#include "XYZ_HW_Project/Actors/Interactive/Environment/Ladder.h"
#include "XYZ_HW_Project/Actors/Interactive/Environment/Zipline.h"
#include "XYZ_HW_Project/Characters/HWBaseCharacter.h"
#include "XYZ_HW_Project/Utils/HWDebugUtils.h"

void UHWBaseCharacterMovementComponent::PhysicsRotation(float DeltaTime)
{
	if (bForceRotation)
	{
		FRotator CurrentRotation = UpdatedComponent->GetComponentRotation(); // Normalized
		CurrentRotation.DiagnosticCheckNaN(TEXT("CharacterMovementComponent::PhysicsRotation(): CurrentRotation"));

		FRotator DeltaRot = GetDeltaRotation(DeltaTime);
		DeltaRot.DiagnosticCheckNaN(TEXT("CharacterMovementComponent::PhysicsRotation(): GetDeltaRotation"));

		const float AngleTolerance = 1e-3f;

		if (!CurrentRotation.Equals(ForceTargetRotation, AngleTolerance))
		{
			FRotator DesiredRotation = ForceTargetRotation;
			// PITCH
			if (!FMath::IsNearlyEqual(CurrentRotation.Pitch, DesiredRotation.Pitch, AngleTolerance))
			{
				DesiredRotation.Pitch = FMath::FixedTurn(CurrentRotation.Pitch, DesiredRotation.Pitch, DeltaRot.Pitch);
			}

			// YAW
			if (!FMath::IsNearlyEqual(CurrentRotation.Yaw, DesiredRotation.Yaw, AngleTolerance))
			{
				DesiredRotation.Yaw = FMath::FixedTurn(CurrentRotation.Yaw, DesiredRotation.Yaw, DeltaRot.Yaw);
			}

			// ROLL
			if (!FMath::IsNearlyEqual(CurrentRotation.Roll, DesiredRotation.Roll, AngleTolerance))
			{
				DesiredRotation.Roll = FMath::FixedTurn(CurrentRotation.Roll, DesiredRotation.Roll, DeltaRot.Roll);
			}

			// Set the new rotation.
			DesiredRotation.DiagnosticCheckNaN(TEXT("CharacterMovementComponent::PhysicsRotation(): DesiredRotation"));
			MoveUpdatedComponent( FVector::ZeroVector, DesiredRotation, /*bSweep*/ false );
		}
		else
		{
			bForceRotation = false;
		}
		return;
	}
	
	if (IsOnLadder())
		return;
	
	Super::PhysicsRotation(DeltaTime);
}

void UHWBaseCharacterMovementComponent::BeginPlay()
{
	Super::BeginPlay();
	HWBaseCharacter = Cast<AHWBaseCharacter>(GetCharacterOwner());
}

void UHWBaseCharacterMovementComponent::UpdateCharacterStateBeforeMovement(float DeltaSeconds)
{
	if (IsValid(HWBaseCharacter))
	{
		const bool bIsProning = IsProning();
		const bool bIsCrouching = IsCrouching();

		// if bIsSliding - dont do anything
		if (bIsSliding || bIsHardLanding)
		{
			bWantsToProne = false;
			bWantsToCrouch = 0;
			Super::UpdateCharacterStateBeforeMovement(DeltaSeconds);
			return;
		}
		
		// Перейти в состояние лежа можно только из состояния приседа
		if (bWantsToProne && bIsCrouching && HWBaseCharacter->CanProne())
		{
			//Если персонаж сидит и он намерен и лечь - тогда он ложится
			// GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Blue, (TEXT("0")));
			bWantsToCrouch = 0;
			UnCrouch();
			Prone();
		} else if (!bWantsToProne && bIsProning)
		{
			// Если персонаж лежит и он не намерен больше лежать, то либо садится, либо встаёт. Зависит от bWantsToCrouch
			// GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Blue, (TEXT("1")));
			if (HWBaseCharacter->CanUnProne())
			{
				UnProne();
				// if (bWantsToCrouch)
				// GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Blue, (TEXT("2")));
				Crouch();
			} else
			{
				bWantsToProne = false;
			}
		} else if (bWantsToProne && bWantsToCrouch && !bIsCrouching && bIsProning)
		{
			// Если персонаж лежит, но персонаж намерен и лечь, и сесть - тогда он садится
			// GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Blue, (TEXT("3")));
			if (HWBaseCharacter->CanUnProne())
			{
				UnProne();
				bWantsToProne = false;
				Crouch();				
			}
			else
			{
				bWantsToCrouch = false;
			}
		} else if (bWantsToProne && bWantsToCrouch && !bIsCrouching)
		{
			// Если персонаж стоит, но персонаж намерен и лечь, и сесть - тогда он садится
			bWantsToProne = false;
			Crouch();
		}
		else
		{
			Super::UpdateCharacterStateBeforeMovement(DeltaSeconds);
		}
	}
}

float UHWBaseCharacterMovementComponent::GetMaxSpeed() const
{
	if (bIsSliding)
		return SlideSpeed;
	if (bIsSprinting)
		return SprintSpeed;
	if (bIsOutOfStamina)
		return OutOfStaminaSpeed;
	if (HWBaseCharacter->bIsProne)
		return ProneSpeed;
	if (IsOnLadder())
		return ClimbingOnLadderMaxSpeed;
	if (HWBaseCharacter->IsAiming())
	{
		return HWBaseCharacter->GetAimingMovementSpeed();
	}
	return Super::GetMaxSpeed();
}

float UHWBaseCharacterMovementComponent::GetCurrentVolumeTopPlane()
{
	APhysicsVolume* Volume = GetPhysicsVolume();
	return Volume->GetActorLocation().Z + Volume->GetBounds().BoxExtent.Z * Volume->GetActorScale3D().Z;
}

void UHWBaseCharacterMovementComponent::StartSprint()
{
	bIsSprinting = true;
	bForceMaxAccel = 1;
}

void UHWBaseCharacterMovementComponent::StopSprint()
{
	bIsSprinting = false;
	bForceMaxAccel = 0;
}

bool UHWBaseCharacterMovementComponent::GetIsOutOfStamina()
{
	return bIsOutOfStamina;
}

void UHWBaseCharacterMovementComponent::SetIsOutOfStamina(bool NewStatement)
{
	bIsOutOfStamina = NewStatement;
}

bool UHWBaseCharacterMovementComponent::IsProning()
{
	if (IsValid(HWBaseCharacter))
		return HWBaseCharacter->bIsProne;
	return false;
}

void UHWBaseCharacterMovementComponent::Prone()
{
	// Получение дефолтного персонажа
	const ACharacter* DefaultCharacter = GetDefault<ACharacter>(CharacterOwner->GetClass());
	float UnscaledCapsuleHalfHeight = DefaultCharacter->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight();

	// Изменить размер капсулы	
	HWBaseCharacter->GetCapsuleComponent()->SetCapsuleSize(ProneCapsuleRadius, ProneCapsuleHalfHeight);
	
	// Переместить капсулу вниз к земле
	HWBaseCharacter->GetCapsuleComponent()->MoveComponent(
		FVector(0.0f, 0.0f, ProneCapsuleHalfHeight - UnscaledCapsuleHalfHeight),
		HWBaseCharacter->GetCapsuleComponent()->GetComponentQuat(),
		false,
		nullptr,
		EMoveComponentFlags::MOVECOMP_NoFlags,
		ETeleportType::TeleportPhysics);

	HWBaseCharacter->bIsProne = true;
	HWBaseCharacter->OnStartProne(UnscaledCapsuleHalfHeight - ProneCapsuleHalfHeight);
}

void UHWBaseCharacterMovementComponent::UnProne()
{
	// Получение дефолтного персонажа
	ACharacter* DefaultCharacter = CharacterOwner->GetClass()->GetDefaultObject<ACharacter>();
	float UnscaledCapsuleHalfHeight = DefaultCharacter->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight();

	// Изменить размер капсулы	
	CharacterOwner->GetCapsuleComponent()->SetCapsuleSize(DefaultCharacter->GetCapsuleComponent()->GetUnscaledCapsuleRadius(), DefaultCharacter->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight());

	// Переместить капсулу повыше
	HWBaseCharacter->GetCapsuleComponent()->MoveComponent(
		FVector(0.0f, 0.0f, UnscaledCapsuleHalfHeight - ProneCapsuleHalfHeight),
		HWBaseCharacter->GetCapsuleComponent()->GetComponentQuat(),
		false,
		nullptr,
		EMoveComponentFlags::MOVECOMP_NoFlags,
		ETeleportType::TeleportPhysics);

	HWBaseCharacter->bIsProne = false;
	HWBaseCharacter->OnEndProne(UnscaledCapsuleHalfHeight - ProneCapsuleHalfHeight);
}

void UHWBaseCharacterMovementComponent::StartMantle(const FMantlingMovementParameters& MantlingParameters)
{
	// TargetLedge = MantlingDescription;
	CurrentMantlingParameters = MantlingParameters;
	SetMovementMode(MOVE_Custom, (uint8)ECustomMovementMode::CMOVE_Mantling);
}

void UHWBaseCharacterMovementComponent::EndMantle()
{
	SetMovementMode(MOVE_Walking);
}

bool UHWBaseCharacterMovementComponent::IsMantling() const
{
	return UpdatedComponent && MovementMode == MOVE_Custom && CustomMovementMode == (uint8)ECustomMovementMode::CMOVE_Mantling;
}

void UHWBaseCharacterMovementComponent::PhysMantling(float deltaTime, int32 Iterations)
{
	float ElapsedTime = GetWorld()->GetTimerManager().GetTimerElapsed(MantlingTimer) + CurrentMantlingParameters.StartTime;

	FVector MantlingCurveValue = CurrentMantlingParameters.MantlingCurve->GetVectorValue(ElapsedTime);

	float PositionAlpha = MantlingCurveValue.X;
	float XYCorrectionAlpha = MantlingCurveValue.Y;
	float ZCorrectionAlpha = MantlingCurveValue.Z;

	FVector CorrectedInitialLocation = FMath::Lerp(CurrentMantlingParameters.InitialLocation, CurrentMantlingParameters.InitialAnimationLocation, XYCorrectionAlpha);
	CorrectedInitialLocation.Z = FMath::Lerp(CurrentMantlingParameters.InitialLocation.Z, CurrentMantlingParameters.InitialAnimationLocation.Z, ZCorrectionAlpha);
			
	FVector NewLocation = FMath::Lerp(CorrectedInitialLocation, CurrentMantlingParameters.TargetLocation, PositionAlpha);
	FRotator NewRotation = FMath::Lerp(CurrentMantlingParameters.InitialRotation, CurrentMantlingParameters.TargetRotation, PositionAlpha);
	FVector DeltaLocation = NewLocation - GetActorLocation();

	// Если компонент на который персонаж взбирается двигается, то двигаем персонажа вместе с этим компонентом
	FVector CurrentOffsetRelativeToLedge = CurrentMantlingParameters.TargetLocation - CurrentMantlingParameters.LedgeComponent->GetComponentLocation();
	DeltaLocation -= CurrentOffsetRelativeToLedge - CurrentMantlingParameters.OffsetRelativeToLedge;
	FHitResult Hit;
	SafeMoveUpdatedComponent(DeltaLocation, NewRotation, false, Hit);
}

void UHWBaseCharacterMovementComponent::ResetLastWallRunSide()
{
	LastWallRunSide = EWallRunSide::None;
}

void UHWBaseCharacterMovementComponent::StartSlide()
{
	bIsSliding = true;
	
	GetWorld()->GetTimerManager().SetTimer(SlidingTimer, this, &UHWBaseCharacterMovementComponent::StopSlide, SlideMaxTime, false);
}

void UHWBaseCharacterMovementComponent::StopSlide()
{
	bIsSliding = false;

	GetWorld()->GetTimerManager().ClearTimer(SlidingTimer);
}

void UHWBaseCharacterMovementComponent::StartHardLanding()
{
	bIsHardLanding = true;
}

void UHWBaseCharacterMovementComponent::StopHardLanding()
{
	bIsHardLanding = false;
}

void UHWBaseCharacterMovementComponent::PhysCustom(float deltaTime, int32 Iterations)
{
	switch (CustomMovementMode)
	{
	case (uint8)ECustomMovementMode::CMOVE_Mantling:
		{
			PhysMantling(deltaTime, Iterations); 
			break;
		}
	case (uint8)ECustomMovementMode::CMOVE_Ladder:
		{
			PhysLadder(deltaTime, Iterations);
			break;
		}
	case (uint8)ECustomMovementMode::CMOVE_Zipline:
		{
			PhysZipline(deltaTime, Iterations);
			break;
		}
	case (uint8)ECustomMovementMode::CMOVE_WallRunning:
		{
			PhysWallRun(deltaTime, Iterations);
			break;
		}
		
	default:
		break;
	}
		
	
	Super::PhysCustom(deltaTime, Iterations);
}

void UHWBaseCharacterMovementComponent::OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode)
{
	Super::OnMovementModeChanged(PreviousMovementMode, PreviousCustomMode);
	if (MovementMode == MOVE_Swimming)
	{
		CharacterOwner->GetCapsuleComponent()->SetCapsuleSize(SwimmingCapsuleRadius, SwimmingCapsuleHalfSize);
	} else if (PreviousMovementMode == MOVE_Swimming)
	{
		ACharacter* DefaultCharacter = CharacterOwner->GetClass()->GetDefaultObject<ACharacter>();
		CharacterOwner->GetCapsuleComponent()->SetCapsuleSize(
			DefaultCharacter->GetCapsuleComponent()->GetUnscaledCapsuleRadius(),
			DefaultCharacter->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight()
			);
	}
	if (PreviousMovementMode == MOVE_Custom && PreviousCustomMode == (uint8)ECustomMovementMode::CMOVE_Ladder)
	{
		CurrentLadder = nullptr;
	}

	
	if (MovementMode == MOVE_Custom)
	{
		switch (CustomMovementMode)
		{
		case (uint8)ECustomMovementMode::CMOVE_Mantling:
			{
				GetWorld()->GetTimerManager().SetTimer(MantlingTimer, this, &UHWBaseCharacterMovementComponent::EndMantle, CurrentMantlingParameters.Duration, false);
			}
		// case (uint8)ECustomMovementMode::CMOVE_Ladder:
		// 	{
		// 	}
		default:
			break;
		}
	}
}

float UHWBaseCharacterMovementComponent::GetActorToCurrentLadderProjection(const FVector& ActorLocation) const
{
	checkf(IsValid(CurrentLadder), TEXT("UHWBaseCharacterMovementComponent::GetCharacterToCurrentLadderProjection cannot be invoked when current ladder in null"))
	FVector LadderUpVector = CurrentLadder->GetActorUpVector();
	FVector LadderToCharacterDistance = ActorLocation - CurrentLadder->GetActorLocation();
	return FVector::DotProduct(LadderUpVector, LadderToCharacterDistance);
}

void UHWBaseCharacterMovementComponent::AttachToLadder(ALadder* Ladder)
{
	CurrentLadder = Ladder;

	FRotator TargetOrientationRotation = CurrentLadder->GetActorForwardVector().ToOrientationRotator();
	TargetOrientationRotation.Yaw += 180.0f;

	const float ActorToLadderProjection = GetActorToCurrentLadderProjection(GetOwner()->GetActorLocation());
	FVector LadderUpVector = CurrentLadder->GetActorUpVector();

	FVector NewCharacterLocation = CurrentLadder->GetActorLocation() + ActorToLadderProjection * LadderUpVector + LadderToCharacterOffset * CurrentLadder->GetActorForwardVector();

	if (CurrentLadder->GetIsOnTop())
	{
		NewCharacterLocation = CurrentLadder->GetAttachFromTopAnimMontageStartingLocation();
	}
	
	GetOwner()->SetActorLocation(NewCharacterLocation);
	GetOwner()->SetActorRotation(TargetOrientationRotation);
	
	SetMovementMode(MOVE_Custom, (uint8)ECustomMovementMode::CMOVE_Ladder);
}

void UHWBaseCharacterMovementComponent::DetachFromLadder(EDetachFromLadderMethod DetachFromLadderMethod)
{
	switch (DetachFromLadderMethod)
	{
		case EDetachFromLadderMethod::JumpOff:
			{
				HWBaseCharacter->Mantle(true);
				const FVector JumpDirection = GetCurrentLadder()->GetActorForwardVector();
				Launch(JumpDirection * JumpOffLadderSpeed);
				ForceTargetRotation = JumpDirection.ToOrientationRotator();
				bForceRotation = true;
				break;
			}
		case EDetachFromLadderMethod::ReachingTheBottom:
			{
				SetMovementMode(MOVE_Falling);
				break;
			}
		case EDetachFromLadderMethod::ReachingTheTop:
			{
				HWBaseCharacter->Mantle(true);
				break;
			}
		default:
			{
				SetMovementMode(MOVE_Falling);
				break;
			}
	}
}

bool UHWBaseCharacterMovementComponent::IsOnLadder() const
{
	return UpdatedComponent && MovementMode == MOVE_Custom && CustomMovementMode == (uint8)ECustomMovementMode::CMOVE_Ladder;
}

void UHWBaseCharacterMovementComponent::PhysLadder(float DeltaTime, int32 INT32)
{
	CalcVelocity(DeltaTime, 1.0f, false, ClimbingOnLadderBrakingDeceleration);
	FVector Delta = Velocity * DeltaTime;

	if (HasAnimRootMotion())
	{
		FHitResult Hit;
		SafeMoveUpdatedComponent(Delta, GetOwner()->GetActorRotation(), false, Hit);
		return;
	}
	
	FVector NewPosition = GetActorLocation() + Delta;
	const float MewPositionProject = GetActorToCurrentLadderProjection(NewPosition);
	if (MewPositionProject < MinLadderBottomOffset)
	{
		DetachFromLadder(EDetachFromLadderMethod::ReachingTheBottom);
		return;
	}
	else if (MewPositionProject > (CurrentLadder->GetLadderHeight() - MaxLadderTopOffset))
	{
		DetachFromLadder(EDetachFromLadderMethod::ReachingTheTop);
		return;
	}
	FHitResult Hit;
	SafeMoveUpdatedComponent(Delta, GetOwner()->GetActorRotation(), true, Hit);
}

ALadder* UHWBaseCharacterMovementComponent::GetCurrentLadder() const
{
	return CurrentLadder;
}

float UHWBaseCharacterMovementComponent::GetLadderSpeedRation() const
{
	checkf(IsValid(CurrentLadder), TEXT("UHWBaseCharacterMovementComponent::GetLadderSpeedRation error"))

	FVector LadderUpVector = CurrentLadder->GetActorUpVector();
	return FVector::DotProduct(LadderUpVector, Velocity) / ClimbingOnLadderMaxSpeed;
}

bool UHWBaseCharacterMovementComponent::IsOnZipline() const
{
	return UpdatedComponent && MovementMode == MOVE_Custom && CustomMovementMode == (uint8)ECustomMovementMode::CMOVE_Zipline;
}

void UHWBaseCharacterMovementComponent::AttachToZipline(AZipline* Zipline)
{
	CurrentZipline = Zipline;
	FVector DescentDirection = Zipline->GetDescentDirection();
	// TODO Почему-то крена вперёд не происходит, это не мешает, но лучше разобраться
	HWBaseCharacter->SetActorRotation(DescentDirection.ToOrientationRotator());

	FVector ZiplineToCharacterDistance = HWBaseCharacter->GetActorLocation() - Zipline->GetTopPeakLocation();
	float CharacterToZiplineProjection = FVector::DotProduct(DescentDirection, ZiplineToCharacterDistance);
	// Move character under zipline
	HWBaseCharacter->SetActorLocation(Zipline->GetTopPeakLocation() + CharacterToZiplineProjection * DescentDirection - ZiplineOffset);

	SetMovementMode(MOVE_Custom, (uint8)ECustomMovementMode::CMOVE_Zipline);
}

void UHWBaseCharacterMovementComponent::DetachFromZipline(EDetachFromZiplineMethod DetachFromZiplineMethod)
{
	CurrentZipline = nullptr;
	// В целом сейчас поведение одинаковое на все случаи, но можно будет добавить свои фишки при разном сходе с зиплайна
	switch (DetachFromZiplineMethod)
	{
		case EDetachFromZiplineMethod::ReachingTheBottom:
			{
				Velocity = FVector::ZeroVector;
				SetMovementMode(MOVE_Falling);
				break;
			}
		case EDetachFromZiplineMethod::Fall:
			{
				Velocity = FVector::ZeroVector;
				SetMovementMode(MOVE_Falling);
				break;
			}
		default:
			{
				Velocity = FVector::ZeroVector;
				SetMovementMode(MOVE_Falling);
				break;
			}
	}
}

void UHWBaseCharacterMovementComponent::PhysZipline(float DeltaTime, int32 INT32)
{
	AZipline* Zipline = GetCurrentZipline();
	// Спуск со скоростью по направлению вниз
	FVector DeltaLocation = Zipline->GetDescentDirection() * DeltaTime * ZiplineSpeed;
	FHitResult Hit;
	SafeMoveUpdatedComponent(DeltaLocation, Zipline->GetDescentDirection().ToOrientationRotator(), false, Hit);
	// Спрыгиваем с зиплайна если спустились до конца
	if ((GetOwner()->GetActorLocation() - Zipline->GetBottomPeakLocation()).Size() < DistanceBeforeJumpOffFromZipline)
	{
		DetachFromZipline(EDetachFromZiplineMethod::ReachingTheBottom);
	}
}

AZipline* UHWBaseCharacterMovementComponent::GetCurrentZipline() const
{
	return CurrentZipline;
}

bool UHWBaseCharacterMovementComponent::IsWallRunning() const
{
	return UpdatedComponent && MovementMode == MOVE_Custom && CustomMovementMode == (uint8)ECustomMovementMode::CMOVE_WallRunning;
}

bool UHWBaseCharacterMovementComponent::IsWallRunRightWall()
{
	return CurrentWallRunningParameters.WallSide == EWallRunSide::Right;
}

void UHWBaseCharacterMovementComponent::PhysWallRun(float DeltaTime, int32 INT32)
{
	// TargetLocation reached
	if ((GetOwner()->GetActorLocation() - CurrentWallRunningParameters.TargetLocation).Size() < 50.0f)
		EndWallRunning(false);
	// forward button released
	if (!HWBaseCharacter->IsForwardMoveActivated())
		EndWallRunning(false);
	
	FVector FromCharacterToWallVector = FVector::ZeroVector;
	if (CurrentWallRunningParameters.WallSide == EWallRunSide::Right)
	{
		FromCharacterToWallVector = GetOwner()->GetActorRightVector();
	} else if (CurrentWallRunningParameters.WallSide == EWallRunSide::Left)
	{
		FromCharacterToWallVector = - GetOwner()->GetActorRightVector();
	}

	// Check if wall end
	FHitResult LineTraceHit;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(GetOwner());
	if (!GetWorld()->LineTraceSingleByChannel(LineTraceHit, GetOwner()->GetActorLocation(), GetOwner()->GetActorLocation() + FromCharacterToWallVector * (WallRunOffset + 50.0f), ECC_WallRunnable, QueryParams))
		EndWallRunning();
	// move character
	FVector DeltaLocation = CurrentWallRunningParameters.RunDirection * DeltaTime * WallRunSpeed;
	FHitResult MoveHit;
	SafeMoveUpdatedComponent(DeltaLocation, CurrentWallRunningParameters.RunDirection.ToOrientationRotator(), false, MoveHit);
}

void UHWBaseCharacterMovementComponent::StartWallRunning(FWallRunningParameters WallRunningParameters)
{
	CurrentWallRunningParameters = WallRunningParameters;
	// character shift 
	GetOwner()->SetActorLocation(GetOwner()->GetActorLocation() + WallRunningParameters.WallNormal * WallRunOffset);
	LastWallRunSide = WallRunningParameters.WallSide;
	SetMovementMode(MOVE_Custom, (uint8)ECustomMovementMode::CMOVE_WallRunning);
}

void UHWBaseCharacterMovementComponent::EndWallRunning(bool JumpFromWall)
{
	if (JumpFromWall)
	{
		SetMovementMode(MOVE_Falling);
		GetOwner()->SetActorRotation(CurrentWallRunningParameters.LaunchVector.ToOrientationRotator());
		Launch(CurrentWallRunningParameters.LaunchVector * WallRunLaunchStartSpeed);
	} else
	{
		SetMovementMode(MOVE_Falling);
	}
}
