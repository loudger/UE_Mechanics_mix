// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "../HWBaseCharacter.h"
#include "HWPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class XYZ_HW_PROJECT_API AHWPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	virtual void SetPawn(APawn* InPawn) override;

	bool GetIgnoreCameraPitch() const;
	void SetIgnoreCameraPitch(bool bIgnoreCameraPitch_In);

protected:
	void MoveForward(float X);
	void MoveRight(float X);
	void Turn(float X);
	void LookUp(float X);
	void Mantle();
	void Jump();
	void ChangeCrouchState();
	void StartSprint();
	void StopSprint();
	void SwimForward(float X);
	void SwimRight(float X);
	void SwimUp(float X);
	void ChangeProneState();
	void InteractWithLadder();
	void ClimbLadderUp(float X);
	void InteractWithZipline();
	void Slide();
	void StartFire();
	void StopFire();
	void StartAiming();
	void StopAiming();
	
	virtual void SetupInputComponent() override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Widgets")
	TSubclassOf<class UPlayerHUDWidget> PlayerHUDWidgetClass;

private:
	void CreateAndInitializeWidgets();
	
	TSoftObjectPtr<AHWBaseCharacter> CachedBaseCharacter;

	UPlayerHUDWidget* PlayerHUDWidget = nullptr;

	bool bIgnoreCameraPitch = false;
	
};
