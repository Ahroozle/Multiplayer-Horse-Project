// Fill out your copyright notice in the Description page of Project Settings.

#include "MPHorso.h"
#include "KillVolumeActor.h"

#include "Components/SplineComponent.h"


// Sets default values
AKillVolumeActor::AKillVolumeActor(const FObjectInitializer& _init) : Super(_init)
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	RootComponent = _init.CreateDefaultSubobject<USceneComponent>(this, TEXT("RootComponent"));

	KillVolumesRoot = _init.CreateDefaultSubobject<USceneComponent>(this, TEXT("KillVolumes"));

	RespawnVolumesRoot = _init.CreateDefaultSubobject<USceneComponent>(this, TEXT("RespawnVolumes"));


}

// Called when the game starts or when spawned
void AKillVolumeActor::BeginPlay()
{
	Super::BeginPlay();
	
	TArray<USceneComponent*> CurrChildren;
	RespawnVolumesRoot->GetChildrenComponents(true, CurrChildren);

	for (USceneComponent* curr : CurrChildren)
	{
		if (USplineComponent* currSpline = Cast<USplineComponent>(curr))
			RetrievedRespawnSplines.Add(currSpline);
		else if (UPrimitiveComponent* currPrim = Cast<UPrimitiveComponent>(curr))
		{
			RetrievedRespawnVolumes.Add(currPrim);

			currPrim->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

			currPrim->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		}
	}


	KillVolumesRoot->GetChildrenComponents(true, CurrChildren);

	for (USceneComponent* curr : CurrChildren)
	{
		if (UPrimitiveComponent* currCasted = Cast<UPrimitiveComponent>(curr))
			currCasted->OnComponentBeginOverlap.AddDynamic(this, &AKillVolumeActor::OnKillVolumesHit);
	}
}

// Called every frame
void AKillVolumeActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AKillVolumeActor::OnKillVolumesHit_Implementation(UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (AActor* RetrievedOwner = OtherActor->GetOwner())
	{
		APawn* OwnerAsPawn = Cast<APawn>(RetrievedOwner);
		if (nullptr != OwnerAsPawn)
		{
			AController* OwnerController = OwnerAsPawn->GetController();

			if (nullptr != OwnerController && OwnerController->GetClass()->IsChildOf<APlayerController>())
				OnPlayerOOB(OwnerAsPawn, OtherActor);
		}
	}

	// TODO determine if destroyable and destroy?
}

void AKillVolumeActor::OnPlayerOOB_Implementation(APawn* Obs, AActor* Player)
{
	FVector LastContacted = Player->GetActorLocation();
	for (TFieldIterator<UStructProperty> iter(Player->GetClass()); iter; ++iter)
	{
		if (iter->GetNameCPP() == "LastContactedLocation")
		{
			(*iter)->CopyCompleteValue(&LastContacted, (*iter)->ContainerPtrToValuePtr<void>(Player));
			break;
		}
	}

	for (TFieldIterator<UStructProperty> iter(Player->GetClass()); iter; ++iter)
	{
		if (iter->GetNameCPP() == "RespLoc")
		{
			FVector DeterminedRespPoint = ClosestRespawnPoint(LastContacted);
			(*iter)->CopyCompleteValue((*iter)->ContainerPtrToValuePtr<void>(Player), &DeterminedRespPoint);
			break;
		}
	}

	if (UFunction* RevPosFunc = Obs->FindFunction("RevertPos"))
		Obs->ProcessEvent(RevPosFunc, nullptr);

}

FVector AKillVolumeActor::ClosestRespawnPoint(FVector Point)
{
	float ClosestDist = 100000000000000;
	FVector Res = Point + (FVector::UpVector * 500);

	for (UPrimitiveComponent* curr : RetrievedRespawnVolumes)
	{
		FVector NewPoint;
		float NewDist = curr->GetClosestPointOnCollision(Point, NewPoint);

		if (NewDist >= 0 && NewDist < ClosestDist)
		{
			ClosestDist = NewDist;
			Res = NewPoint;
		}
	}

	for (USplineComponent* curr : RetrievedRespawnSplines)
	{
		FVector NewPoint = curr->FindLocationClosestToWorldLocation(Point, ESplineCoordinateSpace::World);
		float NewDist = FVector::Distance(NewPoint, Point);

		if (NewDist >= 0 && NewDist < ClosestDist)
		{
			ClosestDist = NewDist;
			Res = NewPoint;
		}
	}

	return Res;
}
