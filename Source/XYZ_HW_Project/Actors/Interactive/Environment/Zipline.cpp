// Fill out your copyright notice in the Description page of Project Settings.


#include "Zipline.h"

#include "Components/CapsuleComponent.h"
#include "XYZ_HW_Project/XYZ_HW_Types.h"

AZipline::AZipline()
{
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("LadderRoot"));
	
	FirstColumnMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FirstColumnMeshComponent"));
	FirstColumnMeshComponent->SetupAttachment(RootComponent);
	SecondColumnMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SecondColumnMeshComponent"));
	SecondColumnMeshComponent->SetupAttachment(RootComponent);

	CabelMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CabelMeshComponent"));
	CabelMeshComponent->SetupAttachment(RootComponent);
	CabelMeshComponent->SetCollisionProfileName(CollisionProfileSpectator);

	InteractionVolume = CreateDefaultSubobject<UCapsuleComponent>(TEXT("InteractionVolume"));
	InteractionVolume->SetupAttachment(RootComponent);
	InteractionVolume->SetCollisionProfileName(CollisionProfilePawnInteractionVolume);
	InteractionVolume->SetGenerateOverlapEvents(true);
}

void AZipline::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	CalculateTopAndBottomPositions();

	float CabelPartLength = 0.0f;
	UStaticMesh* CabelStaticMesh = CabelMeshComponent->GetStaticMesh();
	if (IsValid(CabelStaticMesh))
	{
		CabelPartLength = CabelStaticMesh->GetBoundingBox().GetSize().Z;
	}

	// determining new cabel location
	FVector CabelLocation = (TopPeakLocation + BottomPeakLocation) * 0.5f;
	CabelMeshComponent->SetWorldLocation(CabelLocation);

	//determining new cabel rotation
	FRotator CabelRotation = (TopPeakLocation - BottomPeakLocation).ToOrientationRotator();
	CabelRotation.Pitch += 90.0f;
	CabelMeshComponent->SetWorldRotation(CabelRotation);

	//determining new cabel length
	float DistanceBetweenColumns = (TopPeakLocation - BottomPeakLocation).Size();
	if (!FMath::IsNearlyZero(CabelPartLength))
		CabelMeshComponent->SetRelativeScale3D(FVector(1.0f, 1.0f, DistanceBetweenColumns / CabelPartLength));

	UCapsuleComponent* ZiplineCapsuleComponent = GetZiplineInteractionCapsule();
	if (IsValid(ZiplineCapsuleComponent))
	{
		ZiplineCapsuleComponent->SetWorldLocation(CabelLocation);
		ZiplineCapsuleComponent->SetWorldRotation(CabelRotation);
		ZiplineCapsuleComponent->SetCapsuleSize(InteractionVolumeRadius, DistanceBetweenColumns * 0.5f);
	}
}

void AZipline::BeginPlay()
{
	Super::BeginPlay();
	CalculateTopAndBottomPositions();
}

FVector AZipline::GetDescentDirection()
{
	return (BottomPeakLocation - TopPeakLocation).GetSafeNormal();
}

void AZipline::CalculateTopAndBottomPositions()
{
	FVector FirstColumnLocation = FirstColumnMeshComponent->GetComponentLocation();
	FVector SecondColumnLocation = SecondColumnMeshComponent->GetComponentLocation();
	
	float FirstColumnHeight = 0.0f;
	float SecondColumnHeight = 0.0f;
	UStaticMesh* FirstColumnStaticMesh = FirstColumnMeshComponent->GetStaticMesh();
	if (IsValid(FirstColumnStaticMesh))
	{
		FirstColumnHeight = FirstColumnStaticMesh->GetBoundingBox().GetSize().Z;
	}
	UStaticMesh* SecondColumnStaticMesh = SecondColumnMeshComponent->GetStaticMesh();
	if (IsValid(SecondColumnMeshComponent))
	{
		SecondColumnHeight = SecondColumnStaticMesh->GetBoundingBox().GetSize().Z;
	}
	if ((FirstColumnLocation.Z + FirstColumnHeight) >= (SecondColumnLocation.Z + SecondColumnHeight))
	{
		TopPeakLocation = FirstColumnLocation + FVector::UpVector * FirstColumnHeight;
		BottomPeakLocation = SecondColumnLocation + FVector::UpVector * SecondColumnHeight;
	}else
	{
		BottomPeakLocation = FirstColumnLocation + FVector::UpVector * FirstColumnHeight;
		TopPeakLocation = SecondColumnLocation + FVector::UpVector * SecondColumnHeight;
	}
}

UCapsuleComponent* AZipline::GetZiplineInteractionCapsule() const
{
	UCapsuleComponent* CapsuleComponent = Cast<UCapsuleComponent>(InteractionVolume);
	return CapsuleComponent;
}
