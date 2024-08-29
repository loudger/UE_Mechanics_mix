// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PlatformInvocator.h"
#include "Components/TimelineComponent.h"
#include "GameFramework/Actor.h"
#include "BasePlatform.generated.h"

UENUM()
enum class EPlatformBehavior : uint8
{
	OnDemand = 0,
	Loop
};


UCLASS()
class XYZ_HW_PROJECT_API ABasePlatform : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ABasePlatform();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EPlatformBehavior PlatformBehavior = EPlatformBehavior::OnDemand;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UStaticMeshComponent* PlatformMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Transient)
	FVector StartLocation;
	
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, meta = (MakeEditWidget))
	FVector EndLocation;

	FTimeline PlatformTimeline;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UCurveFloat* TimelineCurve;

	UFUNCTION()
	void PlatformTimeLineUpdate(const float Alpha);
	
	UFUNCTION()
	void PlatformTimeLineFinishedWithDelay();

	UFUNCTION()
	void PlatformTimeLineFinished();

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float PlatformDelay = 0.0f;

	UPROPERTY(EditAnywhere)
	APlatformInvocator* PlatformInvocator;

	UFUNCTION()
	void OnPlatformInvoked();
	UFUNCTION()
	void PlatformComeBackWithDelay();
	UFUNCTION()
	void PlatformComeBack();

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
