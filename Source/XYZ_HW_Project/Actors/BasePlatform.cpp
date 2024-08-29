// Fill out your copyright notice in the Description page of Project Settings.


#include "BasePlatform.h"

// Sets default values
ABasePlatform::ABasePlatform()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	USceneComponent* DefaultPlatformRoot = CreateDefaultSubobject<USceneComponent>(TEXT("Platform root"));
	RootComponent = DefaultPlatformRoot;

	PlatformMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Platform"));
	PlatformMesh->SetupAttachment(DefaultPlatformRoot);
}

// Called when the game starts or when spawned
void ABasePlatform::BeginPlay()
{
	Super::BeginPlay();
	StartLocation = PlatformMesh->GetRelativeLocation();

	if (IsValid(TimelineCurve))
	{
		FOnTimelineFloatStatic DelegatePlatformMovementUpdate;
		DelegatePlatformMovementUpdate.BindUObject(this, &ABasePlatform::PlatformTimeLineUpdate);
		PlatformTimeline.AddInterpFloat(TimelineCurve, DelegatePlatformMovementUpdate);

		if (PlatformBehavior==EPlatformBehavior::Loop)
		{
			// Циклически гоняем платформу туда-обратно
			FOnTimelineEvent DelegatePlatformMovementFinished;
			DelegatePlatformMovementFinished.BindUFunction(this, FName("PlatformTimeLineFinishedWithDelay"));
			PlatformTimeline.SetTimelineFinishedFunc(DelegatePlatformMovementFinished);
			PlatformTimeline.Play();
		}
		else if (PlatformBehavior==EPlatformBehavior::OnDemand)
		{
			// Начинает движение при контакте с PlatformInvocator, потом возвращается обратно
			if (IsValid(PlatformInvocator))
			{
				PlatformInvocator->OnInvocatorActivated.AddUObject(this, &ABasePlatform::OnPlatformInvoked);
			}
		}
	}
}


void ABasePlatform::PlatformTimeLineUpdate(const float Alpha)
{
	const FVector PlatformTargetLocation = FMath::Lerp(StartLocation, EndLocation, Alpha);
	PlatformMesh->SetRelativeLocation(PlatformTargetLocation);
}

void ABasePlatform::PlatformTimeLineFinishedWithDelay()
{
	FTimerHandle UnusedHandle;
	// Назначить выполнение функции с задержкой 'PlatformDelay'
	// TODO Если задержка равна 0, то не выполняется
	GetWorldTimerManager().SetTimer(UnusedHandle, this, &ABasePlatform::PlatformTimeLineFinished, PlatformDelay, false);
}

void ABasePlatform::PlatformTimeLineFinished()
{
	// Проверяем направление таймлайна
	if (PlatformTimeline.GetPlaybackPosition() == 0)
		// Если таймлайн достиг начала (после воспроизведения в обратном направлении), играем его снова
		PlatformTimeline.PlayFromStart();
	else
		// Если таймлайн достиг конца, воспроизводим его в обратном направлении
		PlatformTimeline.Reverse();
}

void ABasePlatform::OnPlatformInvoked()
{
	FOnTimelineEvent DelegatePlatformMovementFinished;
	DelegatePlatformMovementFinished.BindUFunction(this, FName("PlatformComeBackWithDelay"));
	PlatformTimeline.SetTimelineFinishedFunc(DelegatePlatformMovementFinished);
	if (PlatformTimeline.GetPlaybackPosition() == 0)
		PlatformTimeline.PlayFromStart();
}

void ABasePlatform::PlatformComeBackWithDelay()
{
	FTimerHandle UnusedHandle;
	// Назначить выполнение функции с задержкой 'PlatformDelay'
	// TODO Если задержка равна 0, то не выполняется
	GetWorldTimerManager().SetTimer(UnusedHandle, this, &ABasePlatform::PlatformComeBack, PlatformDelay, false);
}

void ABasePlatform::PlatformComeBack()
{
	// GEngine->AddOnScreenDebugMessage(1, 2.0f, FColor::Red, FString::Printf(TEXT("GetPlaybackPosition() - %f"), PlatformTimeline.GetPlaybackPosition()));
	if (PlatformTimeline.GetPlaybackPosition() == 1.0f)
	{
		PlatformTimeline.Reverse();
	}
}

// Called every frame
void ABasePlatform::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	PlatformTimeline.TickTimeline(DeltaTime);
}

