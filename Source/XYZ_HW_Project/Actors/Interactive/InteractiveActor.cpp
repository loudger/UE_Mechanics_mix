// Fill out your copyright notice in the Description page of Project Settings.


#include "InteractiveActor.h"

#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "XYZ_HW_Project/Characters/HWBaseCharacter.h"


// Called when the game starts or when spawned
void AInteractiveActor::BeginPlay()
{
	Super::BeginPlay();
	if (IsValid(InteractionVolume))
	{
		InteractionVolume->OnComponentBeginOverlap.AddDynamic(this, &AInteractiveActor::OnInteractionVolumeOverlapBegin);
		InteractionVolume->OnComponentEndOverlap.AddDynamic(this, &AInteractiveActor::OnInteractionVolumeOverlapEnd);
	}
}

UBoxComponent* AInteractiveActor::GetLadderInteractionBox() const
{
	return Cast<UBoxComponent>(InteractionVolume);
}

bool AInteractiveActor::IsOverlappingCharacterCapsule(AActor* OtherActor, UPrimitiveComponent* OtherComp)
{
	AHWBaseCharacter* BaseCharacter = Cast<AHWBaseCharacter>(OtherActor);
	if (!IsValid(BaseCharacter))
		return false;
	if (Cast<UCapsuleComponent>(OtherComp) != BaseCharacter->GetCapsuleComponent())
		return false;
	return true;
}

void AInteractiveActor::OnInteractionVolumeOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                                        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!IsOverlappingCharacterCapsule(OtherActor, OtherComp))
		return;

	AHWBaseCharacter* BaseCharacter = StaticCast<AHWBaseCharacter*>(OtherActor);
	BaseCharacter->RegisterInteractiveActor(this);
}

void AInteractiveActor::OnInteractionVolumeOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (!IsOverlappingCharacterCapsule(OtherActor, OtherComp))
		return;

	AHWBaseCharacter* BaseCharacter = StaticCast<AHWBaseCharacter*>(OtherActor);
	BaseCharacter->UnRegisterInteractiveActor(this);
}
