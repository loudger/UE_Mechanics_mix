#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "XYZ_HW_Project/Characters/HWBaseCharacter.h"
#include "CharacterAttributeComponent.generated.h"

DECLARE_MULTICAST_DELEGATE(FOnDeathEventSignature);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnStaminaEventSignature, bool);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class XYZ_HW_PROJECT_API UCharacterAttributeComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UCharacterAttributeComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	FOnDeathEventSignature OnDeathEvent;
	FOnStaminaEventSignature OnStaminaEvent;

	bool IsAlive() const { return Health > 0.0f; }
	float GetHealthPercent() const;
	
protected:
	virtual void BeginPlay() override;

	// Health
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Health", meta = (UIMin = 0.0f))
	float MaxHealth = 100.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Stamina", meta = (UIMin =0.0f))
	float MaxStamina = 100.0f;
	
	// Stamina
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Stamina")
	float StaminaRestoreVelocity = 8.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Stamina", meta = (UIMin =0.0f))
	float SprintStaminaConsumptionVelocity = 15.0f;

	void UpdateStaminaValue(float DeltaTime);
	void CheckStaminaBorderlineState();

	// Oxygen
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Oxygen", meta = (UIMin =0.0f))
	float MaxOxygen = 50.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Oxygen")
	float OxygenRestoreVelocity = 15.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Oxygen")
	float SwimOxygenConsumptionVelocity = 2.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Oxygen")
	float OxygenPeriodTime = 2.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Oxygen")
	float OxygenDamage = 5.0f;
	
	void UpdateOxygenValue(float DeltaTime);
	void PeriodicOxygenDamage();

private:
	float Health = 0.0f;
	float Stamina = 0.0f;
	float Oxygen = 0.0f;

	FTimerHandle OxygenDamageTimer;

#if UE_BUILD_DEBUG || UE_BUILD_DEVELOPMENT
	void DebugDrawAttributes();
#endif

	UFUNCTION()
	void OnTakeAnyDamage(AActor* DamagedActor, float Damage, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);

	TWeakObjectPtr<AHWBaseCharacter> CachedBaseCharacterOwner;
};
