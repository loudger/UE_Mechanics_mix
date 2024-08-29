// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "../InteractiveActor.h"
#include "Zipline.generated.h"

/**
 * 
 */
UCLASS(Blueprintable)
class XYZ_HW_PROJECT_API AZipline : public AInteractiveActor
{
	GENERATED_BODY()

public:
	AZipline();

	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void BeginPlay() override;

	FVector GetDescentDirection();
	FVector GetTopPeakLocation() const { return TopPeakLocation; }
	FVector GetBottomPeakLocation() const { return BottomPeakLocation; }

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UStaticMeshComponent* FirstColumnMeshComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UStaticMeshComponent* SecondColumnMeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UStaticMeshComponent* CabelMeshComponent;

	class UCapsuleComponent* GetZiplineInteractionCapsule() const;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float InteractionVolumeRadius = 80.0f;

	void CalculateTopAndBottomPositions();

	FVector TopPeakLocation = FVector::ZeroVector;
	FVector BottomPeakLocation = FVector::ZeroVector;
};
