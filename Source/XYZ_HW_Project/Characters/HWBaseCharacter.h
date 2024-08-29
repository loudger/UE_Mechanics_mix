// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Components/HWBaseCharacterMovementComponent.h"
#include "HWBaseCharacter.generated.h"

class AInteractiveActor;
class UCharacterEquipmentComponent;


DECLARE_MULTICAST_DELEGATE_OneParam(FOnAimingStateChanged, bool)


USTRUCT(BlueprintType)
struct FHardLandingSettings
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	class UAnimMontage* HardLandingMontage = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	class UAnimMontage* FPHardLandingMontage = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float MinHeightFalling = 1000.0f;
};


USTRUCT(BlueprintType)
struct FMantlingSettings
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	class UAnimMontage* MantlingMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	class UAnimMontage* FPMantlingMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	class UCurveVector* MantlingCurve;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (ClampMin = 0.0f, UIMin = 0.0f))
	float AnimationCorrectionXY = 65.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (ClampMin = 0.0f, UIMin = 0.0f))
	float AnimationCorrectionZ = 200.0f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (ClampMin = 0.0f, UIMin = 0.0f))
	float MaxHight = 200.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (ClampMin = 0.0f, UIMin = 0.0f))
	float MinHight = 100.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (ClampMin = 0.0f, UIMin = 0.0f))
	float MaxHightStartTime = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (ClampMin = 0.0f, UIMin = 0.0f))
	float MinHightStartTime = 0.5f;
};


class UHWBaseCharacterMovementComponent;

UCLASS(Abstract, NotBlueprintable)
class XYZ_HW_PROJECT_API AHWBaseCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AHWBaseCharacter(const FObjectInitializer& ObjectInitializer);

	virtual void BeginPlay() override;

	virtual void Tick(float DeltaTime) override;
	
	virtual void MoveForward(float Value);
	virtual void MoveRight(float Value) {}
	virtual void Turn(float Value) {}
	virtual void LookUp(float Value) {}
	virtual void Jump() override;

	bool IsForwardMoveActivated();
	bool IsSwimmingUnderWater();

	// Crouch Methods
	virtual void ChangeCrouchState() {}

	// Sprint Methods
	virtual void StartSprint();
	virtual void StopSprint();
	virtual bool CanSprint();
	void TryChangeSprintState(float DeltaTime);

	// Prone Methods
	virtual void ChangeProneState();
	virtual bool CanProne();
	virtual bool CanUnProne(); 
	virtual void Prone();
	virtual void UnProne();
	virtual void OnStartProne(float HeightAdjust);
	virtual void OnEndProne(float HeightAdjust);

	UPROPERTY(BlueprintReadOnly)
	bool bIsProne = false;

	// Stamina Methods
	void SetIsOutOfStamina(bool StaminaOut);
	
	// Swim methods
	virtual void SwimForward(float Value) {};
	virtual void SwimRight(float Value) {};
	virtual void SwimUp(float Value) {};

	// Mantle | Ledge
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character Movement")
	class ULedgeDetectorComponent* LedgeDetectorComponent;
	virtual void Mantle(bool bForce = false);
	virtual bool CanMantle() const;
	virtual void OnMantle(const FMantlingSettings& MantlingSettings, float MantlingAnimationStartTime);

	virtual bool CanJumpInternal_Implementation() const override;

	// ClimbLadder methods
	void ClimbLadderUp(float Value);
	void InteractWithLadder();
	class ALadder* GetAvailableLadder();

	// Zipline methods
	void InteractWithZipline();
	class AZipline* GetAvailableZipline();

	// WallRun methods
	void DoWallRun(const FHitResult& Hit);
	bool CanWallRun(const FHitResult& Hit);

	// Slide methods
	void Slide();

	// LandingMethods
	virtual void Falling() override;
	virtual void Landed(const FHitResult& Hit) override;
	virtual void NotifyJumpApex() override;
	virtual void OnHardLanding(float HardLandingTime);
	virtual void OnHardLandingElapsed();

	// Health and Death
	virtual void OnDeath();
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character| Animations")
	UAnimMontage* OnDeathAnimMontage;
	UFUNCTION(BlueprintCallable)
	float GetHealthPercent() const;

	// Weapon
	virtual void StartFire();
	virtual void StopFire();
	UCharacterEquipmentComponent* GetCharacterEquipmentComponent();
	
	// Common
	void RegisterInteractiveActor(AInteractiveActor* InteractiveActor);
	void UnRegisterInteractiveActor(AInteractiveActor* InteractiveActor);
	FORCEINLINE UHWBaseCharacterMovementComponent* GetBaseCharacterMovementComponent() const {return HWBaseCharacterMovementComponent; }

	// Aiming
	void StartAiming();
	void StopAiming();
	bool IsAiming() const;
	float GetAimingMovementSpeed() const;
	float GetAimTurnModifier();
	float GetAimLookUpModifier();

	FOnAimingStateChanged OnAimingStateChanged;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Character")
	void OnStartAiming();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Character")
	void OnStopAiming();
	
	UFUNCTION()
	void OnCharacterCapsuleHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

protected:
	bool bIsSprintRequested = false;

	UFUNCTION(BlueprintImplementableEvent, Category = "Character | Movement")
	void OnSprintStart();

	UFUNCTION(BlueprintImplementableEvent, Category = "Character | Movement")
	void OnSprintStop();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stamina settings")
	float MaxStamina = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stamina settings")
	float StaminaRestoreVelocity = 20.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stamina settings")
	float SprintStaminaConsumptionVelocity = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stamina settings")
	float CurrentStamina;
	
	UPROPERTY(BlueprintReadOnly, Category = "Character | Movement")
	TObjectPtr <UHWBaseCharacterMovementComponent> HWBaseCharacterMovementComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character | Components")
	TObjectPtr <class UCharacterAttributeComponent> CharacterAttributesComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character | Movement | Mantling")
	FMantlingSettings HighMantleSettings;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character | Movement | Mantling")
	FMantlingSettings LowMantleSettings;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character | Movement | Mantling", meta = (ClampMin = 0.0f, UIMin = 0.0f))
	float LowMantleMaxHeight = 125.0f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character | Movement | Landing")
	FHardLandingSettings HardLandingSettings;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character | Attributes")
	class UCurveFloat* FallDamageCurve;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character | Components")
	class UCharacterEquipmentComponent* CharacterEquipmentComponent;

	virtual void OnStartAimingInternal();
	virtual void OnStopAimingInternal();

	virtual void AimingTimelineUpdate(const float Alpha);

private:
	const FMantlingSettings GetMantlingSettings(float LedgeHeight) const;

	TArray<AInteractiveActor*> AvailableInteractiveActrors;

	float ForwardInputValue = 0.0f;

	FTimerHandle LandingTimer;
	FTimerHandle DeathMontageTimer;

	float FallingApexPosition = 0.0f;

	bool bIsAiming = false;
	float CurrentAimingMovementSpeed;

	// void EnableRagdoll();
 };
