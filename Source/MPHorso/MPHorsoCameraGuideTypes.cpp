// Fill out your copyright notice in the Description page of Project Settings.

#include "MPHorso.h"
#include "MPHorsoCameraGuideTypes.h"

#include "StaticFuncLib.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"


UWeightedArrowComponent::UWeightedArrowComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	// ...
}

// Called when the game starts
void UWeightedArrowComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...

}


// Called every frame
void UWeightedArrowComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}


// Sets default values
AMPHorsoCameraGuide::AMPHorsoCameraGuide(const FObjectInitializer& _init) : Super(_init)
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;


	RootComponent = _init.CreateDefaultSubobject<USceneComponent>(this, TEXT("RootComponent"));
}

// Called when the game starts or when spawned
void AMPHorsoCameraGuide::BeginPlay()
{
	Super::BeginPlay();
	
	TArray<USceneComponent*> RetrievedChildren;
	RootComponent->GetChildrenComponents(true, RetrievedChildren);

	UArrowComponent* casted;
	for (auto *currChild : RetrievedChildren)
	{
		casted = Cast<UArrowComponent>(currChild);

		if (nullptr != casted)
			Arrows.Add(casted);
	}

	if (Arrows.Num() < 1)
	{
		UStaticFuncLib::Print("AMPHorsoCameraGuide::BeginPlay: No ArrowComponents found! Removing self from play.", true);
		Destroy();
	}
}

// Called every frame
void AMPHorsoCameraGuide::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AMPHorsoCameraGuide::NotifyActorBeginOverlap(AActor* OtherActor)
{
	AActor* OtherOwner = OtherActor->GetOwner();

	if (nullptr != OtherOwner)
	{
		APawn* CastedToPawn = Cast<APawn>(OtherOwner);

		if (nullptr != CastedToPawn && CastedToPawn->GetController() == UGameplayStatics::GetPlayerController(this, 0))
		{
			StoredObserver = CastedToPawn;

			GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &AMPHorsoCameraGuide::GuideCamera, 0.01f, true);
		}
	}
}

void AMPHorsoCameraGuide::NotifyActorEndOverlap(AActor* OtherActor)
{
	AActor* OtherOwner = OtherActor->GetOwner();

	if (nullptr != OtherOwner && OtherOwner == StoredObserver)
		GetWorld()->GetTimerManager().ClearTimer(TimerHandle);
}

void AMPHorsoCameraGuide::GuideCamera()
{
	FRotator ResultRot;
	FVector ResultScale;

	if (Arrows.Num() < 2)
	{
		ResultRot = Arrows[0]->GetComponentRotation();
		ResultScale = Arrows[0]->GetComponentScale();
	}
	else
	{
		FVector ObsLoc = StoredObserver->GetActorLocation();

		auto Lamb = [&ObsLoc](UArrowComponent& a, UArrowComponent& b)
		{
			return FVector::Dist(a.GetComponentLocation(), ObsLoc) < FVector::Dist(b.GetComponentLocation(), ObsLoc);
		};

		Arrows.Sort(Lamb);

		FVector StartLoc = Arrows[0]->GetComponentLocation();
		FVector EndLoc = Arrows[1]->GetComponentLocation();

		FVector ClosestPoint = UKismetMathLibrary::FindClosestPointOnSegment(ObsLoc, StartLoc, EndLoc);

		UWeightedArrowComponent* castedStart = Cast<UWeightedArrowComponent>(Arrows[0]);
		UWeightedArrowComponent* castedEnd = Cast<UWeightedArrowComponent>(Arrows[1]);

		// TODO FIX HOW WEIGHTING WORKS

		float fromstartdist = (ClosestPoint - StartLoc).Size();
		float wholedist = (EndLoc - StartLoc).Size();
		float startweight = (nullptr == castedStart ? 1 : FMath::Max(castedStart->Weight, 0.00001f));
		float endweight = (nullptr == castedEnd ? 1 : FMath::Max(castedEnd->Weight, 0.00001f));

		float Ratio = FMath::Clamp((fromstartdist / wholedist) * (endweight / startweight), 0.0f, 1.0f);

		ResultRot = UKismetMathLibrary::REase(Arrows[0]->GetComponentRotation(),
											  Arrows[1]->GetComponentRotation(),
											  Ratio,
											  true,
											  EEasingFunc::Type::EaseInOut
											 );

		ResultScale = UKismetMathLibrary::VEase(Arrows[0]->GetComponentScale(),
												Arrows[1]->GetComponentScale(),
												Ratio,
												EEasingFunc::Type::EaseInOut
											   );

	}

	FRotator FinalRot = FMath::RInterpTo(StoredObserver->GetActorRotation(), ResultRot, GetWorld()->GetDeltaSeconds(), 4.0f);

	UFunction* Rotfunc = StoredObserver->FindFunction("RotCamDirect");

	if (nullptr != Rotfunc)
	{
		FRotator Parm = FinalRot;
		
		StoredObserver->ProcessEvent(Rotfunc, &Parm);
	}
	else
		UStaticFuncLib::Print("AMPHorsoCameraGuide::GuideCamera: Couldn't find ObserverBP::RotCamDirect! No rotation will happen.");

	StoredObserver->SetActorRotation(FinalRot);



	for (TFieldIterator<UFloatProperty> iter(StoredObserver->GetClass()); iter; ++iter)
	{
		//UStaticFuncLib::Print(iter->GetNameCPP(), true);

		if (iter->GetNameCPP() == "NextZoomFactor")
		{
			float CurrZoomVal = iter->GetPropertyValue_InContainer(StoredObserver);
			float NewZoomVal = FMath::FInterpTo(CurrZoomVal, ResultScale.GetMax(), GetWorld()->GetDeltaSeconds(), 4);

			iter->SetPropertyValue_InContainer(StoredObserver, NewZoomVal);
			break;
		}
	}


	if (Arrows.Num() < 2 && Arrows[0]->GetComponentRotation().Equals(StoredObserver->GetActorRotation(),0.01f))
		GetWorld()->GetTimerManager().ClearTimer(TimerHandle);
}
