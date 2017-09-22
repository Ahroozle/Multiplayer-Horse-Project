// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "MPHorsoWeatherTypes.generated.h"

/*
	Weather base class! I'll build upon this
	meaningfully soon.

	Generally weather actors should be cylindrical triggers
	that, on being entered, determine what the current
	weather should be like based on its own attributes, as
	well as the current attributes of the room. It also spawns
	a single particle effect that follows the client player around
	if they're in it and does not continue outside of the trigger
	zone. This visual effect follows the player all the time but
	may be deactivated when not visible by the player.
*/
UCLASS()
class MPHORSO_API AWeatherActor : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AWeatherActor(const FObjectInitializer& _init);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
