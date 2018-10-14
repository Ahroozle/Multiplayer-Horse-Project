// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "PlayerRespawnPointBase.generated.h"

/*
	Base class for respawn point-type things!

	These are searched for by the room streaming system
	when players enter in order to properly place them
	in the world. It is meant to be derived from so that
	respawning at certain points can yield different
	behaviors, for various polishing and gameplay reasons.
*/
UCLASS(Blueprintable)
class MPHORSO_API APlayerRespawnPointBase : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APlayerRespawnPointBase(const FObjectInitializer& _init);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintNativeEvent)
		void HandleRespawn(APawn* Player);
	void HandleRespawn_Implementation(APawn* Player);
	
};
