// Fill out your copyright notice in the Description page of Project Settings.

#include "MPHorso.h"
#include "CullZoneVolume.h"


// Sets default values
ACullZoneVolume::ACullZoneVolume(const FObjectInitializer& _init) : Super(_init)
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	RootComponent = _init.CreateDefaultSubobject<USceneComponent>(this, TEXT("RootComponent"));
}

// Called when the game starts or when spawned
void ACullZoneVolume::BeginPlay()
{
	Super::BeginPlay();

}

// Called every frame
void ACullZoneVolume::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ACullZoneVolume::NotifyActorBeginOverlap(AActor* OtherActor)
{
	TInlineComponentArray<UPrimitiveComponent*> Prims(OtherActor);
	for (UPrimitiveComponent* currPrim : Prims)
		currPrim->SetCachedMaxDrawDistance(IsCulling ? 1.0f : 0.0f);
}

void ACullZoneVolume::NotifyActorEndOverlap(AActor* OtherActor)
{
	TInlineComponentArray<UPrimitiveComponent*> Prims(OtherActor);
	for (UPrimitiveComponent* currPrim : Prims)
		currPrim->SetCachedMaxDrawDistance(0.0f);
}

void ACullZoneVolume::SetIsCulling(bool NewCulling)
{
	IsCulling = NewCulling;

	UpdateOverlappingActors();
}

void ACullZoneVolume::UpdateOverlappingActors()
{
	TArray<AActor*> Overlapped;
	GetOverlappingActors(Overlapped);

	for (AActor* currActor : Overlapped)
	{
		TInlineComponentArray<UPrimitiveComponent*> Prims(currActor);
		for (UPrimitiveComponent* currPrim : Prims)
			currPrim->SetCachedMaxDrawDistance(IsCulling ? 1.0f : 0.0f);
	}
}
