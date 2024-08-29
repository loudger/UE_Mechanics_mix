// Fill out your copyright notice in the Description page of Project Settings.


#include "HWPlayerController.h"

#include "Blueprint/UserWidget.h"
#include "XYZ_HW_Project/Characters/PlayerCharacter.h"
#include "XYZ_HW_Project/Characters/Components/CharacterEquipmentComponent.h"
#include "XYZ_HW_Project/UI/PlayerHUDWidget.h"

void AHWPlayerController::SetPawn(APawn* InPawn)
{
	Super::SetPawn(InPawn);

	CachedBaseCharacter = Cast<AHWBaseCharacter>(InPawn);
	CreateAndInitializeWidgets();
}

bool AHWPlayerController::GetIgnoreCameraPitch() const
{
	return bIgnoreCameraPitch;
}

void AHWPlayerController::SetIgnoreCameraPitch(bool bIgnoreCameraPitch_In)
{
	bIgnoreCameraPitch = bIgnoreCameraPitch_In;
}

void AHWPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	InputComponent->BindAxis("MoveForward", this, &AHWPlayerController::MoveForward);
	InputComponent->BindAxis("MoveRight", this, &AHWPlayerController::MoveRight);
	InputComponent->BindAxis("Turn", this, &AHWPlayerController::Turn);
	InputComponent->BindAxis("LookUp", this, &AHWPlayerController::LookUp);
	InputComponent->BindAction("Mantle", IE_Pressed, this, &AHWPlayerController::Mantle);
	InputComponent->BindAction("Slide", IE_Pressed, this, &AHWPlayerController::Slide);
	InputComponent->BindAction("InteractWithLadder", IE_Pressed, this, &AHWPlayerController::InteractWithLadder);
	InputComponent->BindAction("InteractWithZipline", IE_Pressed, this, &AHWPlayerController::InteractWithZipline);
	InputComponent->BindAction("Jump", IE_Pressed, this, &AHWPlayerController::Jump);
	InputComponent->BindAction("Crouch", IE_Pressed, this, &AHWPlayerController::ChangeCrouchState);
	InputComponent->BindAction("Sprint", IE_Pressed, this, &AHWPlayerController::StartSprint);
	InputComponent->BindAction("Sprint", IE_Released, this, &AHWPlayerController::StopSprint);
	InputComponent->BindAxis("SwimForward", this, &AHWPlayerController::SwimForward);
	InputComponent->BindAxis("SwimRight", this, &AHWPlayerController::SwimRight);
	InputComponent->BindAxis("SwimUp", this, &AHWPlayerController::SwimUp);
	InputComponent->BindAction("Prone", IE_Pressed, this, &AHWPlayerController::ChangeProneState);
	InputComponent->BindAction("Fire", IE_Pressed, this, &AHWPlayerController::StartFire);
	InputComponent->BindAction("Fire", IE_Released, this, &AHWPlayerController::StopFire);
	InputComponent->BindAxis("ClimbLadderUp", this, &AHWPlayerController::ClimbLadderUp);
	InputComponent->BindAction("Aim", IE_Pressed, this, &AHWPlayerController::StartAiming);
	InputComponent->BindAction("Aim", IE_Released, this, &AHWPlayerController::StopAiming);
}

void AHWPlayerController::CreateAndInitializeWidgets()
{
	PlayerHUDWidget = CreateWidget<UPlayerHUDWidget>(GetWorld(), PlayerHUDWidgetClass);

	if (IsValid(PlayerHUDWidget) && CachedBaseCharacter.IsValid())
	{
		PlayerHUDWidget->AddToViewport();
		UReticleWidget* ReticleWidget = PlayerHUDWidget->GetReticleWidget();
		if (IsValid(ReticleWidget))
		{
			CachedBaseCharacter->OnAimingStateChanged.AddUFunction(ReticleWidget, FName("OnAimingStateChanged"));
		}

		UAmmoWidget* AmmoWidget = PlayerHUDWidget->GetAmmoWidget();
		if (IsValid(AmmoWidget))
		{
			UCharacterEquipmentComponent* CharacterEquipmentComponent = CachedBaseCharacter->GetCharacterEquipmentComponent();
			CharacterEquipmentComponent->OnCurrentWeaponAmmoChangedEvent.AddUFunction(AmmoWidget, FName("UpdateAmmoCount"));
		}
	}
}

void AHWPlayerController::MoveForward(float Value)
{
	if (CachedBaseCharacter.IsValid())
	{
		CachedBaseCharacter->MoveForward(Value);
	}
}

void AHWPlayerController::MoveRight(float Value)
{
	if (CachedBaseCharacter.IsValid())
	{
		CachedBaseCharacter->MoveRight(Value);
	}
}

void AHWPlayerController::Turn(float Value)
{
	if (CachedBaseCharacter.IsValid())
	{
		CachedBaseCharacter->Turn(Value);
	}
}

void AHWPlayerController::LookUp(float Value)
{
	if (CachedBaseCharacter.IsValid())
	{
		CachedBaseCharacter->LookUp(Value);
	}
}

void AHWPlayerController::Jump()
{
	if (CachedBaseCharacter.IsValid())
	{
		CachedBaseCharacter->Jump();
	}
}

void AHWPlayerController::ChangeCrouchState()
{
	if (CachedBaseCharacter.IsValid())
	{
		CachedBaseCharacter->ChangeCrouchState();
	}
}

void AHWPlayerController::StartSprint()
{
	if (CachedBaseCharacter.IsValid())
	{
		CachedBaseCharacter->StartSprint();
	}
}

void AHWPlayerController::StopSprint()
{
	if (CachedBaseCharacter.IsValid())
	{
		CachedBaseCharacter->StopSprint();
	}
}

void AHWPlayerController::SwimForward(float Value)
{
	if (CachedBaseCharacter.IsValid())
	{
		CachedBaseCharacter->SwimForward(Value);
	}
}

void AHWPlayerController::SwimRight(float Value)
{
	if (CachedBaseCharacter.IsValid())
	{
		CachedBaseCharacter->SwimRight(Value);
	}
}

void AHWPlayerController::SwimUp(float Value)
{
	if (CachedBaseCharacter.IsValid())
	{
		CachedBaseCharacter->SwimUp(Value);
	}
}

void AHWPlayerController::ChangeProneState()
{
	if (CachedBaseCharacter.IsValid())
	{
		CachedBaseCharacter->ChangeProneState();
	}
}

void AHWPlayerController::Mantle()
{
	if (CachedBaseCharacter.IsValid())
	{
		CachedBaseCharacter->Mantle();
	}
}

void AHWPlayerController::InteractWithLadder()
{
	if (CachedBaseCharacter.IsValid())
	{
		CachedBaseCharacter->InteractWithLadder();
	}
}

void AHWPlayerController::ClimbLadderUp(float Value)
{
	if (CachedBaseCharacter.IsValid())
	{
		CachedBaseCharacter->ClimbLadderUp(Value);
	}
}

void AHWPlayerController::InteractWithZipline()
{
	if (CachedBaseCharacter.IsValid())
	{
		CachedBaseCharacter->InteractWithZipline();
	}
}

void AHWPlayerController::Slide()
{
	if (CachedBaseCharacter.IsValid())
	{
		CachedBaseCharacter->Slide();
	}
}

void AHWPlayerController::StartFire()
{
	if (CachedBaseCharacter.IsValid())
	{
		CachedBaseCharacter->StartFire();
	}
}

void AHWPlayerController::StopFire()
{
	if (CachedBaseCharacter.IsValid())
	{
		CachedBaseCharacter->StopFire();
	}
}

void AHWPlayerController::StartAiming()
{
	if (CachedBaseCharacter.IsValid())
	{
		CachedBaseCharacter->StartAiming();
	}
}

void AHWPlayerController::StopAiming()
{
	if (CachedBaseCharacter.IsValid())
	{
		CachedBaseCharacter->StopAiming();
	}
}
