// Fill out your copyright notice in the Description page of Project Settings.

#include "MPHorso.h"
#include "OneShotAudio.h"

//#include "Kismet/KismetSystemLibrary.h"


// Sets default values
AOneShotAudio::AOneShotAudio(const FObjectInitializer& _init) : Super(_init)
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	USceneComponent* currRoot = RootComponent = _init.CreateDefaultSubobject<USceneComponent>(this, TEXT("RootComponent"));

	audioComp = _init.CreateDefaultSubobject<UAudioComponent>(this, TEXT("AudioComp"));

	FAttachmentTransformRules AttachRules(EAttachmentRule::KeepRelative, false);
	audioComp->AttachToComponent(currRoot, AttachRules);
	audioComp->SetRelativeLocation(FVector::ZeroVector);
}

void AOneShotAudio::Init(USoundCue* sound, float pitch)
{
	if (nullptr == sound)
	{
		Die();
		return;
	}

	audioComp->SetSound(sound);
	audioComp->SetPitchMultiplier(pitch);
	audioComp->Play();

	audioComp->OnAudioFinished.AddDynamic(this, &AOneShotAudio::Die);

	//UKismetSystemLibrary::DrawDebugSphere(this, GetActorLocation(), 100.0f, 4, FLinearColor::Red, 1.0f);
}

//// Called when the game starts or when spawned
//void AOneShotAudio::BeginPlay()
//{
//	Super::BeginPlay();
//	
//}
//
//// Called every frame
//void AOneShotAudio::Tick(float DeltaTime)
//{
//	Super::Tick(DeltaTime);
//
//}

void AOneShotAudio::Die()
{
	Destroy();
}
