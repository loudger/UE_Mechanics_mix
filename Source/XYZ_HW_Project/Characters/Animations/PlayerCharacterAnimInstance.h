// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "HWBaseCharacterAnimInstance.h"
#include "../PlayerCharacter.h"
#include "PlayerCharacterAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class XYZ_HW_PROJECT_API UPlayerCharacterAnimInstance : public UHWBaseCharacterAnimInstance
{
	GENERATED_BODY()

public:
	virtual void NativeBeginPlay() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character | IK Settings")
	FVector RightFootEffectorLocation = FVector::ZeroVector;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character | IK Settings")
	FVector LeftFootEffectorLocation = FVector::ZeroVector;

	FVector RightFootEffectorLocationSPECIAL;

private:
	TWeakObjectPtr<APlayerCharacter> CachedPlayerCharacter;
};
