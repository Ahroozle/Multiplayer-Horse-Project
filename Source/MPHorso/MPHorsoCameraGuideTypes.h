// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "MPHorsoCameraGuideTypes.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent), BlueprintType)
class MPHORSO_API UWeightedArrowComponent : public UArrowComponent
{
	GENERATED_BODY()

public:
		UWeightedArrowComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
		float Weight = 1;


};

UCLASS(BlueprintType)
class MPHORSO_API AMPHorsoCameraGuide : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AMPHorsoCameraGuide(const FObjectInitializer& _init);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;
	virtual void NotifyActorEndOverlap(AActor* OtherActor) override;

	void GuideCamera();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		TArray<UArrowComponent*> Arrows;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		APawn* StoredObserver;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		FTimerHandle TimerHandle;

};

//UCLASS()
//class MPHORSO_API AMPHorsoCameraGuideTypes : public AActor
//{
//	GENERATED_BODY()
//	
//public:	
//	// Sets default values for this actor's properties
//	AMPHorsoCameraGuideTypes();
//
//protected:
//	// Called when the game starts or when spawned
//	virtual void BeginPlay() override;
//
//public:	
//	// Called every frame
//	virtual void Tick(float DeltaTime) override;
//
//	
//	
//};
