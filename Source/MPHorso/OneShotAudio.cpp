// Fill out your copyright notice in the Description page of Project Settings.

#include "MPHorso.h"
#include "OneShotAudio.h"

#include "StaticFuncLib.h"
#include "MPHorsoGameInstance.h"

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

	{
		UMPHorsoGameInstance* gameInst = UStaticFuncLib::RetrieveGameInstance(this);
		if (nullptr != gameInst)
			audioComp->SetVolumeMultiplier(gameInst->MasterVolume * gameInst->SFXVolume);
		else
			UStaticFuncLib::Print("AOneShotAudio::Init: Couldn't retrieve the game instance! "
								  "Sound effect volume will not be changed to fit settings as a result.", true);
	}

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
//	UKismetSystemLibrary::DrawDebugSphere(this, GetActorLocation(), 400.0f, 4, FLinearColor::Red);
//
//}

void AOneShotAudio::Die()
{
	Destroy();
}
