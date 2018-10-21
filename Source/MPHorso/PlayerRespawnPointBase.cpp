// Fill out your copyright notice in the Description page of Project Settings.

#include "MPHorso.h"
#include "PlayerRespawnPointBase.h"


// Sets default values
APlayerRespawnPointBase::APlayerRespawnPointBase(const FObjectInitializer& _init) : Super(_init)
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	RootComponent = _init.CreateDefaultSubobject<USceneComponent>(this, TEXT("RootComponent"));
}

// Called when the game starts or when spawned
void APlayerRespawnPointBase::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void APlayerRespawnPointBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void APlayerRespawnPointBase::HandleRespawn_Implementation(APawn* Player)
{
	Player->SetActorLocation(GetActorLocation(), false, nullptr, ETeleportType::TeleportPhysics);
}
