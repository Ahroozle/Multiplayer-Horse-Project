// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "OneShotAudio.generated.h"


UCLASS()
class MPHORSO_API AOneShotAudio : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AOneShotAudio(const FObjectInitializer& _init);

protected:
	// Called when the game starts or when spawned
	//virtual void BeginPlay() override;

public:	
	// Called every frame
	//virtual void Tick(float DeltaTime) override;

	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "One-Shot Audio")
		UAudioComponent* audioComp;

	UFUNCTION()
		void Init(USoundCue* sound, float pitch);
	
	UFUNCTION()
		void Die();
};
