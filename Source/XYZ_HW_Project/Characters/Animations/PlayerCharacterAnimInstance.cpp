// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerCharacterAnimInstance.h"
#include "Field/FieldSystemNodes.h"

void UPlayerCharacterAnimInstance::NativeBeginPlay()
{
	Super::NativeBeginPlay();
	checkf(TryGetPawnOwner()->IsA<APlayerCharacter>(), TEXT("UPlayerCharacterAnimInstance must be use only by APlayerCharacter"));
	CachedPlayerCharacter = StaticCast<APlayerCharacter*>(TryGetPawnOwner());
}

void UPlayerCharacterAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (!CachedPlayerCharacter.IsValid())
		return;

	RightFootEffectorLocation = FVector(CachedPlayerCharacter->GetIKRightFootOffset(), 0.0f, 0.0f);
	LeftFootEffectorLocation = FVector(-CachedPlayerCharacter->GetIKLeftFootOffset(), 0.0f, 0.0f);

	
}
