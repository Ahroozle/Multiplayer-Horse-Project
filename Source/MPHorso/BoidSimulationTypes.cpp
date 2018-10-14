// Fill out your copyright notice in the Description page of Project Settings.

#include "MPHorso.h"
#include "BoidSimulationTypes.h"

#include "Kismet/GameplayStatics.h"


// Sets default values for this component's properties
UBoidSimComponent::UBoidSimComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}

// Called when the game starts
void UBoidSimComponent::BeginPlay()
{
	Super::BeginPlay();

	{
		TActorIterator<ABoidSimFlyweightActor> iter(GetWorld());
		if (iter)
			iter->AddNewBoid(this, FName());
	}
}

// Called every frame
void UBoidSimComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

}

void UBoidSimComponent::GetFlock(TArray<UBoidSimComponent*>& RetrievedFlock, FVector& AveragePosition, FVector& AverageDirection)
{
	TActorIterator<ABoidSimFlyweightActor> RetrievedFlyweight(GetWorld());

	if (RetrievedFlyweight)
	{
		FBoidFlockData* RetrievedData = RetrievedFlyweight->FlockData.Find(FlockName);

		if (nullptr != RetrievedData)
		{
			RetrievedFlock = RetrievedData->FlockMembers;
			AveragePosition = RetrievedData->AveragePosition;
			AverageDirection = RetrievedData->AverageDirection;
		}
	}
}

void UBoidSimComponent::GetLocalBoids(float SearchDistance, TArray<UBoidSimComponent*>& Flock, FVector& AveragePosition, FVector& AverageDirection)
{
	AActor* RetrievedOwner = GetOwner();
	FVector CurrentPosition = RetrievedOwner->GetActorLocation();
	AveragePosition = AverageDirection = FVector::ZeroVector;

	for (FActorIterator iter(GetWorld()); iter; ++iter)
	{
		if (RetrievedOwner != *iter)
		{
			UActorComponent* currBoid = iter->GetComponentByClass(UBoidSimComponent::StaticClass());

			if (nullptr != currBoid && FVector::Dist(iter->GetActorLocation(), CurrentPosition) <= SearchDistance)
			{
				Flock.Add(Cast<UBoidSimComponent>(currBoid));
				AveragePosition += iter->GetActorLocation();
				AverageDirection += iter->GetActorForwardVector();
			}
		}
	}

	AveragePosition /= Flock.Num();
	AverageDirection.Normalize();
}

FVector UBoidSimComponent::Align(FVector AverageFlockDirection, float Weight)
{
	return AverageFlockDirection.GetSafeNormal() * Weight;
}

FVector UBoidSimComponent::Cohese(FVector AverageFlockLocation, float Weight)
{
	return (AverageFlockLocation - GetOwner()->GetActorLocation()).GetClampedToMaxSize(1.0f) * Weight;
}

FVector UBoidSimComponent::Separate(const TArray<UBoidSimComponent*>& Flock, const float& AvoidDistance, float Weight)
{
	FVector Rolling = FVector::ZeroVector;
	FVector CurrentPosition = GetOwner()->GetActorLocation();

	for (UBoidSimComponent* currFlockmate : Flock)
	{
		if (this != currFlockmate)
		{
			FVector TowardMe = CurrentPosition - currFlockmate->GetOwner()->GetActorLocation();
			float DistBetween = TowardMe.Size();

			if (DistBetween <= AvoidDistance)
				Rolling += TowardMe.GetSafeNormal() * ((AvoidDistance - DistBetween) / AvoidDistance);
		}
	}

	return Rolling.GetClampedToMaxSize(1.0f) * Weight;
}

FVector UBoidSimComponent::Avoid(const TArray<AActor*>& ToAvoid, const float& AvoidDistance, float Weight)
{
	FVector Rolling = FVector::ZeroVector;
	FVector CurrentPosition = GetOwner()->GetActorLocation();

	for (AActor* currToAvoid : ToAvoid)
	{
		FVector ClosestPoint;
		float DistBetween =
			currToAvoid->ActorGetDistanceToCollision(CurrentPosition, ECollisionChannel::ECC_WorldStatic, ClosestPoint);

		if (0.0f <= DistBetween && DistBetween <= AvoidDistance)
			Rolling += (CurrentPosition - ClosestPoint).GetSafeNormal() * ((AvoidDistance - DistBetween) / AvoidDistance);
	}

	return Rolling.GetClampedToMaxSize(1.0f) * Weight;
}

FVector UBoidSimComponent::Attract(const TArray<AActor*>& ToAttractToward, float AttractDistance, float Weight)
{
	FVector Rolling = FVector::ZeroVector;
	FVector CurrentPosition = GetOwner()->GetActorLocation();

	for (AActor* currToAttract : ToAttractToward)
	{
		FVector ClosestPoint;
		float DistBetween =
			currToAttract->ActorGetDistanceToCollision(CurrentPosition, ECollisionChannel::ECC_WorldStatic, ClosestPoint);
		
		if (0.0f <= DistBetween && DistBetween <= AttractDistance)
			Rolling += (ClosestPoint - CurrentPosition).GetSafeNormal() * ((AttractDistance - DistBetween) / AttractDistance);
	}

	return Rolling.GetClampedToMaxSize(1.0f) * Weight;
}

FVector UBoidSimComponent::Queue(const TArray<UBoidSimComponent*>& Flock, const float& QueueRadius, float Weight, float QueueFOV_Threshold)
{
	FVector Rolling = FVector::ZeroVector;
	FVector CurrentPosition = GetOwner()->GetActorLocation();
	FVector CurrentForward = GetOwner()->GetActorForwardVector();

	for (UBoidSimComponent* currFlockmate : Flock)
	{
		if (this != currFlockmate)
		{
			FVector TowardMe = CurrentPosition - currFlockmate->GetOwner()->GetActorLocation();
			float DistBetween = TowardMe.Size();
			float TowardDot = FVector::DotProduct(TowardMe.GetSafeNormal(), currFlockmate->GetOwner()->GetActorForwardVector());

			if (DistBetween <= QueueRadius && TowardDot > 1 - QueueFOV_Threshold)
				Rolling += TowardMe.GetSafeNormal();
		}
	}

	return Rolling.GetClampedToMaxSize(1.0f) * Weight;
}


// Sets default values
ABoidSimFlyweightActor::ABoidSimFlyweightActor(const FObjectInitializer& _init) : Super(_init)
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	RootComponent = _init.CreateDefaultSubobject<USceneComponent>(this, TEXT("RootComponent"));
}

// Called when the game starts or when spawned
void ABoidSimFlyweightActor::BeginPlay()
{
	Super::BeginPlay();

}

// Called every frame
void ABoidSimFlyweightActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ABoidSimFlyweightActor::AddNewBoid(UBoidSimComponent* NewBoid, FName FlockName)
{
	FBoidFlockData& CurrentData = FlockData.FindOrAdd(FlockName);

	CurrentData.FlockMembers.AddUnique(NewBoid);

	CurrentData.AveragePosition = CurrentData.AverageDirection = FVector::ZeroVector;

	for (UBoidSimComponent* currBoid : CurrentData.FlockMembers)
	{
		CurrentData.AveragePosition += currBoid->GetOwner()->GetActorLocation();
		CurrentData.AverageDirection += currBoid->GetOwner()->GetActorForwardVector();
	}

	CurrentData.AveragePosition /= CurrentData.FlockMembers.Num();
	CurrentData.AverageDirection.Normalize();
}
