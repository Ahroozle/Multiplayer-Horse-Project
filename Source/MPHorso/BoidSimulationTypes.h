// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "BoidSimulationTypes.generated.h"


UCLASS(abstract, Blueprintable, meta = (DisplayName = "Boid Simulation Component"))
class MPHORSO_API UBoidSimComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UBoidSimComponent();

protected:

	// Called when the game starts
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// The name of the flock that this boid belongs to.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FName FlockName;


	/*
		Returns the next trajectory of the agent.
		IMPL NOTE: Should NOT return a normalized vector! Only the sum of the forces acting on the agent.
	*/
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
		FVector GetNextTrajectory();


	// Gets this boid's flock (specified by FlockName).
	UFUNCTION(BlueprintCallable, Category = "Boid Sim")
		void GetFlock(TArray<UBoidSimComponent*>& Flock, FVector& AveragePosition, FVector& AverageDirection);

	// Constructs flock data from all boids found within the specified area around this boid.
	UFUNCTION(BlueprintCallable, Category = "Boid Sim")
		void GetLocalBoids(float SearchDistance, TArray<UBoidSimComponent*>& Flock, FVector& AveragePosition, FVector& AverageDirection);

	// Provides a steering vector which moves in the same direction as surrounding flockmates.
	UFUNCTION(BlueprintCallable, Category = "Boid Sim|Common Behaviors")
		FVector Align(FVector AverageFlockDirection, float Weight);

	// Provides a steering vector which closes in on the average position of the flock.
	UFUNCTION(BlueprintCallable, Category = "Boid Sim|Common Behaviors")
		FVector Cohese(FVector AverageFlockLocation, float Weight);

	// Provides a steering vector which avoids local flockmates.
	UFUNCTION(BlueprintCallable, Category = "Boid Sim|Common Behaviors")
		FVector Separate(const TArray<UBoidSimComponent*>& Flock, const float& AvoidDistance, float Weight);

	// Provides a steering vector which avoids the given actors.
	UFUNCTION(BlueprintCallable, Category = "Boid Sim|Common Behaviors")
		FVector Avoid(const TArray<AActor*>& ToAvoid, const float& AvoidDistance, float Weight);

	// Provides a steering vector which closes in on the closest of the given actors.
	UFUNCTION(BlueprintCallable, Category = "Boid Sim|Common Behaviors")
		FVector Attract(const TArray<AActor*>& ToAttractToward, float AttractDistance, float Weight);


	// Provides a steering vector which both aligns and coheses the flock, forming a somewhat orderly line.
	UFUNCTION(BlueprintCallable, Category = "Boid Sim|Extra Behaviors", meta = (AdvancedDisplay = 3))
		FVector Queue(const TArray<UBoidSimComponent*>& Flock, const float& QueueRadius, float Weight, float QueueFOV_Threshold = 0.5f);
};


USTRUCT(BlueprintType)
struct FBoidFlockData
{
	GENERATED_USTRUCT_BODY();

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		TArray<UBoidSimComponent*> FlockMembers;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		FVector AveragePosition;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		FVector AverageDirection;
};

UCLASS(meta = (DisplayName = "Boid Simulation Flyweight Actor"))
class MPHORSO_API ABoidSimFlyweightActor : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ABoidSimFlyweightActor(const FObjectInitializer& _init);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;


	UPROPERTY(EditInstanceOnly, BlueprintReadWrite)
		TMap<FName, FBoidFlockData> FlockData;


	UFUNCTION()
		void AddNewBoid(UBoidSimComponent* NewBoid, FName FlockName);

};

//UCLASS()
//class MPHORSO_API ABoidSimulationTypes : public AActor
//{
//	GENERATED_BODY()
//	
//public:	
//	// Sets default values for this actor's properties
//	ABoidSimulationTypes();
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
