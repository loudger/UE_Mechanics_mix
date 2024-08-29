// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "XYZ_HW_Project/ActorComponents/LedgeDetectorComponent.h"
#include "HWBaseCharacterMovementComponent.generated.h"

class AZipline;

USTRUCT()
struct FMantlingMovementParameters
{
	GENERATED_BODY()
	
	FVector InitialLocation = FVector::ZeroVector;
	FRotator InitialRotation = FRotator::ZeroRotator;

	FVector TargetLocation = FVector::ZeroVector;
	FRotator TargetRotation = FRotator::ZeroRotator;

	FVector InitialAnimationLocation = FVector::ZeroVector;

	float Duration = 1.0f;
	float StartTime = 0.0f;

	UPROPERTY()
	TObjectPtr <UPrimitiveComponent> LedgeComponent;
	
	// Запоминаем относительное положение персонажа относительно уступа на который он взбирается
	FVector OffsetRelativeToLedge = FVector::ZeroVector; 

	UPROPERTY()
	UCurveVector* MantlingCurve;
};

UENUM()
enum class EWallRunSide : uint8
{
	Right = 0,
	Left,
	None,
};

USTRUCT()
struct FWallRunningParameters
{
	GENERATED_BODY()

	FVector InitialLocation = FVector::ZeroVector;
	FRotator InitialRotation = FRotator::ZeroRotator;

	FVector TargetLocation = FVector::ZeroVector;
	FRotator TargetRotation = FRotator::ZeroRotator;

	FVector RunDirection = FVector::ZeroVector;
	FVector WallNormal = FVector::ZeroVector;
	FVector LaunchVector = FVector::ZeroVector;
	EWallRunSide WallSide = EWallRunSide::None;
};


UENUM()
enum class ECustomMovementMode : uint8
{
	CMOVE_None = 0 UMETA(DisplayName = "None"),
	CMOVE_Mantling UMETA(DisplayName = "Mantling"),
	CMOVE_Ladder UMETA(DisplayName = "Ladder"),
	CMOVE_Zipline UMETA(DisplayName = "Zipline"),
	CMOVE_WallRunning UMETA(DisplayName = "WallRunning"),
	CMOVE_Max UMETA(Hidden)
};

UENUM()
enum class EDetachFromLadderMethod : uint8
{
	Fall = 0,
	ReachingTheTop,
	ReachingTheBottom,
	JumpOff,
};

UENUM()
enum class EDetachFromZiplineMethod : uint8
{
	Fall = 0,
	ReachingTheBottom,
};


/**
 * 
 */
UCLASS()
class XYZ_HW_PROJECT_API UHWBaseCharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

public:
	virtual void PhysicsRotation(float DeltaTime) override;
	
	virtual void BeginPlay() override;

	virtual void UpdateCharacterStateBeforeMovement(float DeltaSeconds) override;

	virtual float GetMaxSpeed() const override;

	float GetCurrentVolumeTopPlane();

	// Sprint Methods
	FORCEINLINE bool IsSprinting() { return bIsSprinting; }
	virtual void StartSprint();
	virtual void StopSprint();

	// Stamina Methods
	UFUNCTION(BlueprintCallable, BlueprintPure)
	bool GetIsOutOfStamina();

	UFUNCTION(BlueprintCallable)
	void SetIsOutOfStamina(bool NewStatement);

	// Prone Methods
	UFUNCTION(BlueprintCallable, BlueprintPure)
	bool IsProning();

	UFUNCTION(BlueprintCallable)
	void Prone();

	UFUNCTION(BlueprintCallable)
	void UnProne();

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bWantsToProne = false;

	// Mantle Methods
	void StartMantle(const FMantlingMovementParameters& MantlingParameters);
	void EndMantle();
	bool IsMantling() const;
	void PhysMantling(float deltaTime, int32 Iterations);

	// Ladder Methods
	void AttachToLadder(class ALadder* Ladder);
	void DetachFromLadder(EDetachFromLadderMethod DetachFromLadderMethod = EDetachFromLadderMethod::Fall);
	bool IsOnLadder() const;
	void PhysLadder(float DeltaTime, int32 INT32);
	class ALadder* GetCurrentLadder() const;
	float GetLadderSpeedRation() const;

	// Zipline Methods
	bool IsOnZipline() const;
	void AttachToZipline(AZipline* Zipline);
	void DetachFromZipline(EDetachFromZiplineMethod DetachFromZiplineMethod = EDetachFromZiplineMethod::Fall);
	void PhysZipline(float DeltaTime, int32 INT32);
	class AZipline* GetCurrentZipline() const;

	// Wallrun Methods
	bool IsWallRunning() const;
	bool IsWallRunRightWall();
	void PhysWallRun(float DeltaTime, int32 INT32);
	void StartWallRunning(FWallRunningParameters WallRunningParameters);
	void EndWallRunning(bool JumpFromWall = false);
	float GetWallRunMaxDistance() const { return WallRunMaxDistance; }
	EWallRunSide GetLastWallRunSide() const { return LastWallRunSide; }
	void ResetLastWallRunSide();
	float GetWallRunOffset() const { return WallRunOffset; }

	// Slide Methods
	bool IsSliding() const { return bIsSliding; }
	void StartSlide();
	void StopSlide();

	// Landing Methods
	bool IsHardLanding() const { return bIsHardLanding; }
	void StartHardLanding();
	void StopHardLanding();
	
protected:
	virtual void PhysCustom(float deltaTime, int32 Iterations) override;
	
	virtual void OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode) override;
	float GetActorToCurrentLadderProjection(const FVector& ActorLocation) const;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character movement | Sprint", meta = (ClampMin=0.0f, UIMin=0.0f))
	float SprintSpeed = 1200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character movement | Prone", meta = (ClampMin=0.0f, UIMin=0.0f))
	float ProneSpeed = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character movement | Prone", meta = (ClampMin=0.0f, UIMin=0.0f))
	float CurrentProneSpeed = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character movement | OutOfStamina", meta = (ClampMin=0.0f, UIMin=0.0f))
	float OutOfStaminaSpeed = 300.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Character movement | Swimming", meta = (ClampMin=0.0f, UIMin=0.0f))
	float SwimmingCapsuleRadius = 60.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Character movement | Swimming", meta = (ClampMin=0.0f, UIMin=0.0f))
	float SwimmingCapsuleHalfSize = 60.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character movement | OutOfStamina")
	bool bIsOutOfStamina = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character movement | Prone")
	float ProneCapsuleRadius = 20.0f; // Радиус капсулы в состоянии лежа
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character movement | Prone")
	float ProneCapsuleHalfHeight = 20.0f; // Половина высоты капсулы в состоянии лежа.

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Character movement | Ladder", meta = (ClampMin=0.0f, UIMin=0.0f))
	float ClimbingOnLadderMaxSpeed = 200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Character movement | Ladder", meta = (ClampMin=0.0f, UIMin=0.0f))
	float ClimbingOnLadderBrakingDeceleration = 2048.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Character movement | Ladder", meta = (ClampMin=0.0f, UIMin=0.0f))
	float LadderToCharacterOffset = 60.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Character movement | Ladder", meta = (ClampMin=0.0f, UIMin=0.0f))
	float MaxLadderTopOffset = 90.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Character movement | Ladder", meta = (ClampMin=0.0f, UIMin=0.0f))
	float MinLadderBottomOffset = 70.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Character movement | Ladder", meta = (ClampMin=0.0f, UIMin=0.0f))
	float JumpOffLadderSpeed = 500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Character movement | Zipline", meta = (ClampMin=0.0f, UIMin=0.0f))
	FVector ZiplineOffset = FVector(0.0f, 0.0f,118.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character movement | Zipline", meta = (ClampMin=0.0f, UIMin=0.0f))
	float ZiplineSpeed = 500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character movement | Zipline", meta = (ClampMin=0.0f, UIMin=0.0f))
	float DistanceBeforeJumpOffFromZipline = 150.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character movement | WallRun", meta = (ClampMin=0.0f, UIMin=0.0f))
	float WallRunMaxDistance = 1500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character movement | WallRun", meta = (ClampMin=0.0f, UIMin=0.0f))
	float WallRunSpeed = 750.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Character movement | WallRun", meta = (ClampMin=0.0f, UIMin=0.0f))
	float WallRunOffset = 30.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Character movement | WallRun", meta = (ClampMin=0.0f, UIMin=0.0f))
	float WallRunLaunchStartSpeed = 800.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Character movement | Slide", meta = (ClampMin=0.0f, UIMin=0.0f))
	float SlideSpeed = 1000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Character movement | Slide", meta = (ClampMin=0.0f, UIMin=0.0f))
	float SlideCaspsuleHalfHeight = 60.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Character movement | Slide", meta = (ClampMin=0.0f, UIMin=0.0f))
	float SlideMaxTime = 1.5f;
	
private:
	bool bIsSprinting = false;
	bool bIsSliding = false;
	bool bIsHardLanding = false;

	UPROPERTY()
	FTimerHandle MantlingTimer;

	float TargetMantlingTime = 1.0f;

	UPROPERTY()
	FMantlingMovementParameters CurrentMantlingParameters;

	UPROPERTY()
	TObjectPtr<class AHWBaseCharacter> HWBaseCharacter;

	UPROPERTY()
	TObjectPtr<ALadder> CurrentLadder = nullptr;

	UPROPERTY()
	TObjectPtr<AZipline> CurrentZipline = nullptr;

	FRotator ForceTargetRotation = FRotator::ZeroRotator;
	bool bForceRotation = false;

	// FWallRunningParameters не оборачивается в TObjectPtr, нужно ли как-то эту структуру обезопасить, чтобы garbage collector её точно чистил?
	UPROPERTY()
	FWallRunningParameters CurrentWallRunningParameters;

	EWallRunSide LastWallRunSide = EWallRunSide::None;

	FTimerHandle SlidingTimer;
};
