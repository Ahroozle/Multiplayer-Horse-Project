// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "CullZoneVolume.generated.h"

/*
	When the Cull Zone is put into culling mode, everything within it has its
	max draw distance set to 1 UU, essentially rendering it invisible without causing
	it to stop ticking or do weird stuff.
*/
UCLASS(Blueprintable)
class MPHORSO_API ACullZoneVolume : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ACullZoneVolume(const FObjectInitializer& _init);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;

	virtual void NotifyActorEndOverlap(AActor* OtherActor) override;

	UPROPERTY(VisibleInstanceOnly)
		bool IsCulling = false;

	UFUNCTION(BlueprintCallable)
		void SetIsCulling(bool NewCulling);

	UFUNCTION()
		void UpdateOverlappingActors();
	
};
