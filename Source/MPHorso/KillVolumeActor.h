// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "KillVolumeActor.generated.h"


UCLASS()
class MPHORSO_API AKillVolumeActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AKillVolumeActor(const FObjectInitializer& _init);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// This is where you attach all the volumes meant to kill players/entities/whatever ends up OOB.
	UPROPERTY(BlueprintReadOnly)
		USceneComponent* KillVolumesRoot;

	/*
		This is where you attach the volumes that determine where players will respawn after they
		hit one of the kill volumes! The closest point out of all the volumes wins.

		NOTE: Spline components are also supported for respawn purposes.
	*/
	UPROPERTY(BlueprintReadOnly)
		USceneComponent* RespawnVolumesRoot;

	UPROPERTY(BlueprintReadOnly)
		TArray<UPrimitiveComponent*> RetrievedRespawnVolumes;

	UPROPERTY(BlueprintReadOnly)
		TArray<class USplineComponent*> RetrievedRespawnSplines;
	

	UFUNCTION(BlueprintNativeEvent)
		void OnKillVolumesHit(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	void OnKillVolumesHit_Implementation(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION(BlueprintNativeEvent)
		void OnPlayerOOB(APawn* Obs, AActor* Player);
	void OnPlayerOOB_Implementation(APawn* Obs, AActor* Player);

	/*
		TODO determine how to efficiently deal with the player's respawn behavior.
			 My thoughts are that the kill volume should handle it so that the player
			 class doesn't get bloated and so that we can create kill volume classes
			 for multiple types of OOB (i.e. falling into water as well as falling out
			 of the world, etc.)
	*/

	UFUNCTION(BlueprintPure)
		FVector ClosestRespawnPoint(FVector Point);

};
