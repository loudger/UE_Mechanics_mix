// Fill out your copyright notice in the Description page of Project Settings.


#include "Ladder.h"

#include "Components/BoxComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "XYZ_HW_Project/XYZ_HW_Types.h"

ALadder::ALadder()
{
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("LadderRoot"));
	
	LeftRailMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LeftRail"));
	LeftRailMeshComponent->SetupAttachment(RootComponent);

	RightRailMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RightRail"));
	RightRailMeshComponent->SetupAttachment(RootComponent);

	StepsMeshComponent = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("Steps"));
	StepsMeshComponent->SetupAttachment(RootComponent);

	InteractionVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("InteractionVolume"));
	InteractionVolume->SetupAttachment(RootComponent);
	InteractionVolume->SetCollisionProfileName(CollisionProfilePawnInteractionVolume);
	InteractionVolume->SetGenerateOverlapEvents(true);

	TopInteractionVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("TopInteractionVolume"));
	TopInteractionVolume->SetupAttachment(RootComponent);
	TopInteractionVolume->SetCollisionProfileName(CollisionProfilePawnInteractionVolume);
	TopInteractionVolume->SetGenerateOverlapEvents(true);
}

void ALadder::BeginPlay()
{
	Super::BeginPlay();
	TopInteractionVolume->OnComponentBeginOverlap.AddDynamic(this, &ALadder::OnInteractionVolumeOverlapBegin);
	TopInteractionVolume->OnComponentEndOverlap.AddDynamic(this, &ALadder::OnInteractionVolumeOverlapEnd);
}

void ALadder::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	LeftRailMeshComponent->SetRelativeLocation(FVector(0.0f, -LadderWidth * 0.5f, 0.0f));
	RightRailMeshComponent->SetRelativeLocation(FVector(0.0f, LadderWidth * 0.5f, 0.0f));
	
	UStaticMesh* LeftRailMesh = LeftRailMeshComponent->GetStaticMesh();
	if (IsValid(LeftRailMesh))
	{
		float MeshHeight = LeftRailMesh->GetBoundingBox().GetSize().Z;
		if (!FMath::IsNearlyZero(MeshHeight))
		{
			LeftRailMeshComponent->SetRelativeScale3D(FVector(1.0f, 1.0f, LadderHeight / MeshHeight));
		}
	}

	UStaticMesh* RightRailMesh = RightRailMeshComponent->GetStaticMesh();
	if (IsValid(RightRailMesh))
	{
		float MeshHeight = RightRailMesh->GetBoundingBox().GetSize().Z;
		if (!FMath::IsNearlyZero(MeshHeight))
		{
			RightRailMeshComponent->SetRelativeScale3D(FVector(1.0f, 1.0f, LadderHeight / MeshHeight));
		}
	}

	UStaticMesh* StepMesh = StepsMeshComponent->GetStaticMesh();
	if (IsValid(StepMesh))
	{
		float MeshWidth = StepMesh->GetBoundingBox().GetSize().Y;
		if (!FMath::IsNearlyZero(MeshWidth))
		{
			StepsMeshComponent->SetRelativeScale3D(FVector(1.0f, LadderWidth / MeshWidth, 1.0f));
		}
	}

	StepsMeshComponent->ClearInstances();
	uint32 StepsCount = FMath::FloorToInt((LadderHeight - BottomStepOffset) / StepsInterval);
	for (uint32 i = 0; i < StepsCount; ++i)
	{
		FTransform InstanseTransform(FVector(1.0f, 1.0f, BottomStepOffset + i * StepsInterval));
		StepsMeshComponent->AddInstance(InstanseTransform);
	}

	UBoxComponent* InteractionBox = GetLadderInteractionBox();
	if (!IsValid(InteractionBox))
		return;
	
	float BoxDepthExtent = InteractionBox->GetUnscaledBoxExtent().X;
	InteractionBox->SetBoxExtent(FVector(BoxDepthExtent, LadderWidth * 0.5f, LadderHeight * 0.5f));
	InteractionBox->SetRelativeLocation(FVector(BoxDepthExtent, 0.0f, LadderHeight * 0.5f));

	FVector TopBoxExtent = TopInteractionVolume->GetUnscaledBoxExtent();
	TopInteractionVolume->SetBoxExtent(FVector(TopBoxExtent.X, LadderWidth*0.5f, TopBoxExtent.Z));
	TopInteractionVolume->SetRelativeLocation(FVector(-TopBoxExtent.X, 0.0f, LadderHeight + TopBoxExtent.Z));
}

UAnimMontage* ALadder::GetAttachFromTopAnimMontage() const
{
	return AttachFromTopAnimMontage;
}

FVector ALadder::GetAttachFromTopAnimMontageStartingLocation() const
{
	FRotator OrientationRotation = GetActorForwardVector().ToOrientationRotator();
	FVector Offset = OrientationRotation.RotateVector(AttachFromTopAnimMontageInitialOffset);
	FVector LadderTop = GetActorLocation() + GetActorUpVector() * LadderHeight;
	return LadderTop + Offset;
}

void ALadder::OnInteractionVolumeOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                              UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnInteractionVolumeOverlapBegin(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep,
	                                       SweepResult);

	if (!IsOverlappingCharacterCapsule(OtherActor, OtherComp))
		return;
	if (OverlappedComponent == TopInteractionVolume)
		bIsOnTop = true;
}

void ALadder::OnInteractionVolumeOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	Super::OnInteractionVolumeOverlapEnd(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex);

	if (!IsOverlappingCharacterCapsule(OtherActor, OtherComp))
		return;
	if (OverlappedComponent == TopInteractionVolume)
		bIsOnTop = false;
}
