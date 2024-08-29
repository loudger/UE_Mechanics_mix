// Fill out your copyright notice in the Description page of Project Settings.


#include "CharacterAttributeComponent.h"

#include "Components/CapsuleComponent.h"
#include "Engine/DamageEvents.h"
#include "Kismet/GameplayStatics.h"
#include "XYZ_HW_Project/XYZ_HW_Types.h"
#include "XYZ_HW_Project/Characters/HWBaseCharacter.h"
#include "XYZ_HW_Project/Subsystems/DebugSubsystem.h"
#include "XYZ_HW_Project/Utils/HWDebugUtils.h"

UCharacterAttributeComponent::UCharacterAttributeComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UCharacterAttributeComponent::BeginPlay()
{
	Super::BeginPlay();

	checkf(MaxHealth > 0.0f, TEXT("MaxHealth must me more than 0.0"));
	checkf(GetOwner()->IsA<AHWBaseCharacter>(), TEXT("UCharacterAttributeComponent::BeginPlay. UCharacterAttributeComponent Can be used only with AHWBaseCharacter"))
	// CachedBaseCharacterOwner = StaticCast<AHWBaseCharacter*>(GetOwner());
	CachedBaseCharacterOwner = Cast<AHWBaseCharacter>(GetOwner());
	Health = MaxHealth;
	Stamina = MaxStamina;
	Oxygen = MaxOxygen;
	CachedBaseCharacterOwner->OnTakeAnyDamage.AddDynamic(this, &UCharacterAttributeComponent::OnTakeAnyDamage);
}

float UCharacterAttributeComponent::GetHealthPercent() const
{
	return Health / MaxHealth;
}

void UCharacterAttributeComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	UpdateStaminaValue(DeltaTime);
	UpdateOxygenValue(DeltaTime);
#if UE_BUILD_DEBUG || UE_BUILD_DEVELOPMENT
	DebugDrawAttributes();
#endif
	
}

void UCharacterAttributeComponent::UpdateStaminaValue(float DeltaTime)
{
	// UE_LOG(LogDamage, Warning, TEXT("IsSprinting %i"),  CachedBaseCharacterOwner->GetBaseCharacterMovementComponent()->IsSprinting());
	if (CachedBaseCharacterOwner->GetBaseCharacterMovementComponent()->IsSprinting())
	{
		Stamina -= SprintStaminaConsumptionVelocity * DeltaTime;
		Stamina = FMath::Clamp(Stamina, 0.0f, MaxStamina);
		CheckStaminaBorderlineState();
	}
	else if (!FMath::IsNearlyEqual(Stamina, MaxStamina))
	{
		Stamina += StaminaRestoreVelocity * DeltaTime;
		Stamina = FMath::Clamp(Stamina, 0.0f, MaxStamina);
		CheckStaminaBorderlineState();
	}
}

void UCharacterAttributeComponent::CheckStaminaBorderlineState()
{
	// Trigger event if stamina runs out
	if (FMath::IsNearlyZero(Stamina))
	{
		if (OnStaminaEvent.IsBound())
		{
			OnStaminaEvent.Broadcast(true);
		}
	}
	// Trigger event if stamina full
	else if (FMath::IsNearlyEqual(Stamina, MaxStamina))
	{
		if (OnStaminaEvent.IsBound())
		{
			OnStaminaEvent.Broadcast(false);
		}
	}
}

void UCharacterAttributeComponent::UpdateOxygenValue(float DeltaTime)
{
	if (CachedBaseCharacterOwner->IsSwimmingUnderWater())
	{
		Oxygen = FMath::Clamp(Oxygen + DeltaTime * OxygenRestoreVelocity, 0.0f, MaxOxygen);
	} else
	{
		Oxygen = FMath::Clamp(Oxygen - DeltaTime * SwimOxygenConsumptionVelocity, 0.0f, MaxOxygen);
		if (FMath::IsNearlyZero(Oxygen) && !GetWorld()->GetTimerManager().IsTimerActive(OxygenDamageTimer))
			GetWorld()->GetTimerManager().SetTimer(OxygenDamageTimer, this, &UCharacterAttributeComponent::PeriodicOxygenDamage, OxygenPeriodTime, true);
	}
}

void UCharacterAttributeComponent::PeriodicOxygenDamage()
{
	if (Oxygen > 0 && GetWorld()->GetTimerManager().IsTimerActive(OxygenDamageTimer))
	{
		GetWorld()->GetTimerManager().ClearTimer(OxygenDamageTimer);
	}
	else
	{
		APlayerController* Controller = CachedBaseCharacterOwner->GetController<APlayerController>();
		if (!IsValid(Controller))
			return;
		CachedBaseCharacterOwner->TakeDamage(OxygenDamage, FDamageEvent(), Controller, CachedBaseCharacterOwner.Get());
	}
}

#if UE_BUILD_DEBUG || UE_BUILD_DEVELOPMENT
void UCharacterAttributeComponent::DebugDrawAttributes()
{
	UDebugSubsystem* DebugSubsystem = UGameplayStatics::GetGameInstance(GetWorld())->GetSubsystem<UDebugSubsystem>();
	if (!DebugSubsystem->IsCategoryEnabled(DebugCategoryCharacterAttributes))
		return;
	FVector TextLocation = CachedBaseCharacterOwner->GetActorLocation() + FVector::UpVector * (CachedBaseCharacterOwner->GetCapsuleComponent()->GetScaledCapsuleHalfHeight());
	DrawDebugString(GetWorld(), TextLocation + FVector::UpVector * 5.0f, FString::Printf(TEXT("HEALTH: %.2f"), Health), nullptr, FColor::Green, 0.0f, true);
	DrawDebugString(GetWorld(), TextLocation + FVector::UpVector * 12.0f, FString::Printf(TEXT("STAMINA: %.2f"), Stamina), nullptr, FColor::Blue, 0.0f, true);
	DrawDebugString(GetWorld(), TextLocation + FVector::UpVector * 20.0f, FString::Printf(TEXT("OXYGEN: %.2f"), Oxygen), nullptr, FColor::Purple, 0.0f, true);
}
#endif

void UCharacterAttributeComponent::OnTakeAnyDamage(AActor* DamagedActor, float Damage, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser)
{
	if (!IsAlive())
		return;
	
	UE_LOG(LogDamage, Warning, TEXT("UCharacterAttributeComponent::OnTakeAnyDamage %s received %.2f amount of damage from %s"), *CachedBaseCharacterOwner->GetName(), Damage, *DamageCauser->GetName());
	Health = FMath::Clamp(Health - Damage, 0.0f, MaxHealth);

	if (Health <= 0.0f)
	{
		UE_LOG(LogDamage, Warning, TEXT("UCharacterAttributeComponent::OnTakeAnyDamage Character %s is killed by an actor %s"), *CachedBaseCharacterOwner->GetName(), *DamageCauser->GetName());
		if (OnDeathEvent.IsBound())
		{
			OnDeathEvent.Broadcast();
		}
	}
}



