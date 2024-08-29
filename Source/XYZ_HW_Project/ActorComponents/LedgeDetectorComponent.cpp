// Fill out your copyright notice in the Description page of Project Settings.


#include "LedgeDetectorComponent.h"

#include "Components/CapsuleComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "XYZ_HW_Project/HWGameInstance.h"
#include "XYZ_HW_Project/XYZ_HW_Types.h"
#include "XYZ_HW_Project/Characters/HWBaseCharacter.h"
#include "XYZ_HW_Project/Subsystems/DebugSubsystem.h"
#include "XYZ_HW_Project/Utils/HWTraceUtils.h"


// Called when the game starts
void ULedgeDetectorComponent::BeginPlay()
{
	Super::BeginPlay();

	checkf(GetOwner()->IsA<ACharacter>(), TEXT("ULedgeDetectorComponent::BeginPlay() only a character can use this component"))
	CachedCharacterOwner = Cast<ACharacter>(GetOwner());
}

bool ULedgeDetectorComponent::DetectLedge(FLedgeDescription& LedgeDescription)
{
	UCapsuleComponent* CapsuleComponent = CachedCharacterOwner->GetCapsuleComponent();

#if ENABLE_DRAW_DEBUG
	UDebugSubsystem* DebugSubsystem = UGameplayStatics::GetGameInstance(GetWorld())->GetSubsystem<UDebugSubsystem>();
	bool bIsDebugEnabled = DebugSubsystem->IsCategoryEnabled(DebugCategoryLedgeDetection);
#else
	bool bIsDebugEnabled = false;
#endif
	float DebugDrawTime = 2.0f;
	
	float BottomZOffset = 2.0f;
	FVector CharacterBottom = CachedCharacterOwner->GetActorLocation() - (CapsuleComponent->GetScaledCapsuleHalfHeight() - BottomZOffset) * FVector::UpVector;

	FCollisionQueryParams QueryParams;
	QueryParams.bTraceComplex = true;
	QueryParams.AddIgnoredActor(CachedCharacterOwner);
	
	//1. Forward check
	FHitResult ForwardCheckHitResult;
	float ForwardCheckCapsuleRadius = CapsuleComponent->GetScaledCapsuleRadius();
	float ForwardCheckCapsuleHalfHeight = (MaximumLedgeHeight - MinimumLedgeHeight) * 0.5f;
	
	FVector ForwardStartLocation = CharacterBottom + (MinimumLedgeHeight + ForwardCheckCapsuleHalfHeight) * FVector::UpVector; 
	FVector ForwardEndLocation = ForwardStartLocation + CachedCharacterOwner->GetActorForwardVector() * ForwardCheckDistance;

	if (!HWTraceUtils::SweepCapsuleSingleByChannel(GetWorld(), ForwardCheckHitResult, ForwardStartLocation, ForwardEndLocation, ForwardCheckCapsuleRadius, ForwardCheckCapsuleHalfHeight, FQuat::Identity, ECC_Climbing, QueryParams, FCollisionResponseParams::DefaultResponseParam, bIsDebugEnabled, DebugDrawTime))
		return false;
	
	//2. Downward check
	float DepthOffset = -10.0f;
	FHitResult DownwardCheckHitResult;
	float DownwardCheckSphereRadius = CapsuleComponent->GetScaledCapsuleRadius();
	FVector DownwardStartLocation = ForwardCheckHitResult.ImpactPoint + DepthOffset * ForwardCheckHitResult.ImpactNormal;
	DownwardStartLocation.Z = CharacterBottom.Z + MaximumLedgeHeight; //+ DownwardCheckSphereRadius;
	FVector DownwardEndLocation = FVector(DownwardStartLocation.X, DownwardStartLocation.Y, CharacterBottom.Z);

	if (!HWTraceUtils::SweepSphereSingleByChannel(GetWorld(), DownwardCheckHitResult, DownwardStartLocation, DownwardEndLocation, DownwardCheckSphereRadius, ECC_Climbing, QueryParams, FCollisionResponseParams::DefaultResponseParam, bIsDebugEnabled, DebugDrawTime, FColor::Blue, FColor::Green))
		return false;
	
	//3. Overlap check
	ACharacter* DefaultCharacter = CachedCharacterOwner->GetClass()->GetDefaultObject<ACharacter>();
	if (!IsValid(DefaultCharacter))
		return false;
	UCapsuleComponent* DefaultCapsuleComponent = DefaultCharacter->GetCapsuleComponent();
	if (!IsValid(DefaultCapsuleComponent))
		return false;

	float OverlapCapsuleRadius = DefaultCapsuleComponent->GetScaledCapsuleRadius();
	float OverlapCapsuleHalfHeight = DefaultCapsuleComponent->GetScaledCapsuleHalfHeight();
	float OverlapCapsuleFloorOffset = 2.0f;
	FVector OverlapLocation = DownwardCheckHitResult.ImpactPoint + (OverlapCapsuleHalfHeight + OverlapCapsuleFloorOffset) * FVector::UpVector;

	if (HWTraceUtils::OverlapCapsuleBlockingByProfile(GetWorld(), OverlapLocation, OverlapCapsuleRadius, OverlapCapsuleHalfHeight, ForwardCheckHitResult.Component->GetComponentQuat(), CollisionProfilePawn, QueryParams, bIsDebugEnabled, DebugDrawTime, FColor::Purple))
	{
		return false;
	}
	
	//4. LedgeDescription filling
	// LedgeDescription.LedgeActor = DownwardCheckHitResult.GetActor();
	LedgeDescription.LedgeComponent = DownwardCheckHitResult.GetComponent();
	LedgeDescription.Location = OverlapLocation;
	LedgeDescription.Rotation = (ForwardCheckHitResult.ImpactNormal * FVector(-1.0f, -1.0f, 0.0f)).ToOrientationRotator();
	LedgeDescription.LedgeNormal = ForwardCheckHitResult.ImpactNormal;
	return true;
}
