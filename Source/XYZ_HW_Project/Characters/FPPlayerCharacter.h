// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PlayerCharacter.h"
#include "FPPlayerCharacter.generated.h"

/**
 * 
 */
UCLASS()
class XYZ_HW_PROJECT_API AFPPlayerCharacter : public APlayerCharacter
{
	GENERATED_BODY()
public:
	AFPPlayerCharacter(const FObjectInitializer& ObjectInitializer);
	virtual void PossessedBy(AController* NewController) override;
	virtual FRotator GetViewRotation() const override;
	virtual void Tick(float DeltaSeconds) override;

	virtual void OnStartCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;
	virtual void OnEndCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;

	virtual void OnMantle(const FMantlingSettings& MantlingSettings, float MantlingAnimationStartTime) override;
	void SetCameraLimitation(float CameraMinPitch, float CameraMaxPitch, float CameraMinYaw, float CameraMaxYaw);

	virtual void OnHardLanding(float HardLandingTime) override;
	virtual void OnHardLandingElapsed() override;
	
	virtual void OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode) override;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character | First person")
	class USkeletalMeshComponent* FirstPersonMeshComponent = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character | First person")
    class UCameraComponent* FirstPersonCameraComponent = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character | First person | Camera| Ladder | Pitch", meta=(UIMin=-89.0f, UIMax=89.0f))
	float LadderCameraMinPitch = -60.0f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character | First person | Camera| Ladder | Pitch", meta=(UIMin=-89.0f, UIMax=89.0f))
	float LadderCameraMaxPitch = 80.0f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character | First person | Camera| Ladder | Yaw", meta=(UIMin=0.0f, UIMax=359.0f))
	float LadderCameraMinYaw = 5.0f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character | First person | Camera| Ladder | Yaw", meta=(UIMin=0.0f, UIMax=359.0f))
	float LadderCameraMaxYaw = 175.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character | First person | Camera| Ladder | Pitch", meta=(UIMin=-89.0f, UIMax=89.0f))
	float ZiplineCameraMinPitch = -89.0f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character | First person | Camera| Ladder | Pitch", meta=(UIMin=-89.0f, UIMax=89.0f))
	float ZiplineCameraMaxPitch = 89.0f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character | First person | Camera| Ladder | Yaw", meta=(UIMin=0.0f, UIMax=359.0f))
	float ZiplineCameraMinYaw = 0.0f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character | First person | Camera| Ladder | Yaw", meta=(UIMin=0.0f, UIMax=359.0f))
	float ZiplineCameraMaxYaw = 180.0f;

private:
	FTimerHandle FPMontageTimer;

	void OnFPMontageTimerElapsed();
	bool IsFPMontagePlaying() const;

	TWeakObjectPtr<class AHWPlayerController> HWPlayerController;
};
