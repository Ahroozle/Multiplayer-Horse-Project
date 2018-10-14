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
AMPHorsoCameraGuideCluster::AMPHorsoCameraGuideCluster(const FObjectInitializer& _init) : Super(_init)
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;


	RootComponent = _init.CreateDefaultSubobject<USceneComponent>(this, TEXT("RootComponent"));
}

void AMPHorsoCameraGuideCluster::OnConstruction(const FTransform & Transform)
{
	Super::OnConstruction(Transform);

#if WITH_EDITOR

	{
		Arrows.Empty();

		TArray<USceneComponent*> RetrievedChildren;
		RootComponent->GetChildrenComponents(true, RetrievedChildren);

		UArrowComponent* casted;
		for (auto *currChild : RetrievedChildren)
		{
			casted = Cast<UArrowComponent>(currChild);

			if (nullptr != casted)
			{
				Arrows.Add(casted->GetFName(), casted);

				if (nullptr != Cast<UWeightedArrowComponent>(casted))
					casted->ArrowColor = FLinearColor(.043f, .192f, 1.0f).ToFColor(false);
				else
					casted->ArrowColor = FLinearColor(1.0f, .502f, 0.0f).ToFColor(false);
			}
		}
	}

	//GetWorld()->PersistentLineBatcher->Flush();
	TArray<FBatchedLine>& WorldPersistentBatchedLines = GetWorld()->PersistentLineBatcher->BatchedLines;

	if (DrawnLineCopies.Num() > 0)
	{
		for (FBatchedLine &curr : DrawnLineCopies)
		{
			WorldPersistentBatchedLines.RemoveAllSwap(
				[&curr](FBatchedLine& a) { return a.Start == curr.Start && a.End == curr.End && a.Color == curr.Color; },
				true
			);
		}

		if(!Visualize)
			GetWorld()->PersistentLineBatcher->MarkRenderStateDirty();

		DrawnLineCopies.Empty();
	}

	if (Visualize)
	{
		int CopyStart = WorldPersistentBatchedLines.Num() - 1;

		TSet<UArrowComponent*> DoneAlready;

		for (FCameraGuideLine& currLine : GuideLines)
		{
			UArrowComponent* EndA = Arrows.FindRef(currLine.EndA);
			UArrowComponent* EndB = Arrows.FindRef(currLine.EndB);

			if (nullptr != EndA && nullptr != EndB)
			{
				DrawDebugLine(GetWorld(), EndA->GetComponentLocation(), EndB->GetComponentLocation(), FColor::Cyan, true, -1, (uint8)'\000', 10);

				UWeightedArrowComponent* castedA = Cast<UWeightedArrowComponent>(EndA);
				UWeightedArrowComponent* castedB = Cast<UWeightedArrowComponent>(EndB);

				float WeightA = (nullptr == castedA ? 1 : castedA->Weight);
				float WeightB = (nullptr == castedB ? 1 : castedB->Weight);

				float Ratio;
				if (WeightA <= WeightB)
					Ratio = (WeightA / WeightB) * 0.5f;
				else
					Ratio = 1 - ((WeightB / WeightA) * 0.5f);

				FVector TurningPoint = FMath::Lerp(EndA->GetComponentLocation(), EndB->GetComponentLocation(), Ratio);

				DrawDebugPoint(GetWorld(), TurningPoint, 20, FColor::Cyan, true);

				float ThreshDist;
				if (!DoneAlready.Contains(EndA))
				{
					ThreshDist = (nullptr == castedA ? DefaultThresholdDistance : castedA->ThresholdDistance);
					DrawDebugCircle(GetWorld(), EndA->GetComponentLocation(), ThreshDist, 50,
						FColor::Cyan, true, -1, (uint8)'\000', 5, FVector::ForwardVector, FVector::RightVector, false);
					DoneAlready.Add(EndA);
				}
				if (!DoneAlready.Contains(EndB))
				{
					ThreshDist = (nullptr == castedB ? DefaultThresholdDistance : castedB->ThresholdDistance);
					DrawDebugCircle(GetWorld(), EndB->GetComponentLocation(), ThreshDist, 50,
						FColor::Cyan, true, -1, (uint8)'\000', 5, FVector::ForwardVector, FVector::RightVector, false);
					DoneAlready.Add(EndB);
				}

			}
			else
				UStaticFuncLib::Print("AMPHorsoCameraGuideCluster::OnConstruction: Couldn't find an arrowcomponent named \'" +
				(nullptr == EndA ? currLine.EndA.ToString() : currLine.EndB.ToString()) +
					"\'! Please fix this or the Guide Line will be ignored during play.", true);
		}

		for (FCameraGuideTri& currTri : GuideMesh)
		{
			UArrowComponent* VertA = Arrows.FindRef(currTri.VertA);
			UArrowComponent* VertB = Arrows.FindRef(currTri.VertB);
			UArrowComponent* VertC = Arrows.FindRef(currTri.VertC);

			if (nullptr != VertA && nullptr != VertB && nullptr != VertC)
			{
				DrawDebugLine(GetWorld(), VertA->GetComponentLocation(), VertB->GetComponentLocation(), FColor::Orange, true, -1, (uint8)'\000', 10);
				DrawDebugLine(GetWorld(), VertB->GetComponentLocation(), VertC->GetComponentLocation(), FColor::Orange, true, -1, (uint8)'\000', 10);
				DrawDebugLine(GetWorld(), VertC->GetComponentLocation(), VertA->GetComponentLocation(), FColor::Orange, true, -1, (uint8)'\000', 10);

			}
			else
				UStaticFuncLib::Print("AMPHorsoCameraGuideCluster::OnConstruction: Couldn't find an arrowcomponent named \'" +
				(nullptr == VertA ? currTri.VertA.ToString() : (nullptr == VertB ? currTri.VertB.ToString() : currTri.VertC.ToString())) +
					"\'! Please fix this or the Guide Tri will be ignored during play.", true);
		}

		while (++CopyStart < WorldPersistentBatchedLines.Num())
			DrawnLineCopies.Add(WorldPersistentBatchedLines[CopyStart]);
	}

	TInlineComponentArray<UShapeComponent*> Shapes(this);
	for (UShapeComponent* currShape : Shapes)
		currShape->ShapeColor = FColor::Blue;

#endif
}

void AMPHorsoCameraGuideCluster::Destroyed()
{
#if WITH_EDITOR
	if (DrawnLineCopies.Num() > 0)
	{
		for (FBatchedLine &curr : DrawnLineCopies)
		{
			GetWorld()->PersistentLineBatcher->BatchedLines.RemoveAllSwap(
				[&curr](FBatchedLine& a) { return a.Start == curr.Start && a.End == curr.End && a.Color == curr.Color; },
				true
			);
		}

		GetWorld()->PersistentLineBatcher->MarkRenderStateDirty();

		DrawnLineCopies.Empty();
	}
#endif

	Super::Destroyed();
}

// Called when the game starts or when spawned
void AMPHorsoCameraGuideCluster::BeginPlay()
{
	Super::BeginPlay();

	if (Arrows.Num() < 1)
	{
		UStaticFuncLib::Print("AMPHorsoCameraGuideCluster::BeginPlay: No ArrowComponents found! Removing self from play.", true);
		Destroy();
	}
}

// Called every frame
void AMPHorsoCameraGuideCluster::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AMPHorsoCameraGuideCluster::NotifyActorBeginOverlap(AActor* OtherActor)
{
	if (!Enabled)
		return;

	AActor* OtherOwner = OtherActor->GetOwner();

	if (nullptr != OtherOwner)
	{
		APawn* CastedToPawn = Cast<APawn>(OtherOwner);

		if (nullptr != CastedToPawn && CastedToPawn->GetController() == UGameplayStatics::GetPlayerController(this, 0))
		{
			StoredObserver = CastedToPawn;

			RetrievedRotFunc = StoredObserver->FindFunction("RotCamDirect");

			if (nullptr == RetrievedRotFunc)
				UStaticFuncLib::Print("AMPHorsoCameraGuide::NotifyActorBeginOverlap: Couldn't find ObserverBP::RotCamDirect! No rotation will happen.", true);

			RetrievedNextZoom = nullptr;
			for (TFieldIterator<UFloatProperty> iter(StoredObserver->GetClass()); iter; ++iter)
			{
				if (iter->GetNameCPP() == "NextZoomFactor")
				{
					RetrievedNextZoom = *iter;
					break;
				}
			}

			if (nullptr == RetrievedNextZoom)
				UStaticFuncLib::Print("AMPHorsoCameraGuide::NotifyActorBeginOverlap: Couldn't find ObserverBP::NextZoomFactor! No zooming will happen.", true);

			GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &AMPHorsoCameraGuideCluster::GuideCamera, 0.01f, true);
		}
	}
}

void AMPHorsoCameraGuideCluster::NotifyActorEndOverlap(AActor* OtherActor)
{
	AActor* OtherOwner = OtherActor->GetOwner();

	if (nullptr != OtherOwner && OtherOwner == StoredObserver)
		GetWorld()->GetTimerManager().ClearTimer(TimerHandle);
}

FCameraGuideLine* AMPHorsoCameraGuideCluster::GetClosestGuideLine(const FVector& Pos, FVector& ClosestSegPoint, float& BestDist)
{
	FCameraGuideLine* BestLine = nullptr;
	BestDist = 10000000000000;
	for (FCameraGuideLine &curr : GuideLines)
	{
		UArrowComponent* EndA = Arrows.FindRef(curr.EndA);
		UArrowComponent* EndB = Arrows.FindRef(curr.EndB);

		if (nullptr != EndA && nullptr != EndB)
		{
			FVector ClosestPoint =
				UKismetMathLibrary::FindClosestPointOnSegment(Pos, EndA->GetComponentLocation(), EndB->GetComponentLocation());

			float Dist = (ClosestPoint - Pos).Size();

			if (Dist < BestDist)
			{
				BestLine = &curr;
				BestDist = Dist;
				ClosestSegPoint = ClosestPoint;
			}
		}
		else
			UStaticFuncLib::Print("AMPHorsoCameraGuideCluster::GetClosestGuideLine: Couldn't find an arrowcomponent named \'" +
								  (nullptr == EndA ? curr.EndA.ToString() : curr.EndB.ToString()) +
								  "\'! Skipping this guide line.", true);

	}

	return BestLine;
}

FCameraGuideTri* AMPHorsoCameraGuideCluster::GetClosestGuideTri(const FVector& Pos, FVector& ClosestTriPoint, float& BestDist)
{
	FCameraGuideTri* BestTri = nullptr;
	BestDist = 10000000000000;
	for (FCameraGuideTri &curr : GuideMesh)
	{
		UArrowComponent* VertA = Arrows.FindRef(curr.VertA);
		UArrowComponent* VertB = Arrows.FindRef(curr.VertB);
		UArrowComponent* VertC = Arrows.FindRef(curr.VertC);

		if (nullptr != VertA && nullptr != VertB && nullptr != VertC)
		{
			FVector ClosestPoint = 
				FMath::ClosestPointOnTriangleToPoint(Pos,
					VertA->GetComponentLocation(), VertB->GetComponentLocation(), VertC->GetComponentLocation());

			float Dist = (ClosestPoint - Pos).Size();

			if (Dist < BestDist)
			{
				BestTri = &curr;
				BestDist = Dist;
				ClosestTriPoint = ClosestPoint;
			}
		}
		else
			UStaticFuncLib::Print("AMPHorsoCameraGuideCluster::GetClosestGuideTri: Couldn't find an arrowcomponent named \'" +
			(nullptr == VertA ? curr.VertA.ToString() : (nullptr == VertB ? curr.VertB.ToString() : curr.VertC.ToString())) +
								  "\'! Skipping this guide tri.", true);
	}
	return BestTri;
}

void AMPHorsoCameraGuideCluster::GetFinalRotAndScaleFromLine(UArrowComponent* InStart, UArrowComponent* InEnd,
															 FVector Pos, FRotator& OutFinalRot, FVector& OutFinalScale)
{
	UWeightedArrowComponent* castedStart = Cast<UWeightedArrowComponent>(InStart);
	UWeightedArrowComponent* castedEnd = Cast<UWeightedArrowComponent>(InEnd);

	float FromStartDist = (Pos - InStart->GetComponentLocation()).Size();
	float WholeDist = (InEnd->GetComponentLocation() - InStart->GetComponentLocation()).Size();

	float DistRatio = FromStartDist / WholeDist;

	float StartWeight = (nullptr == castedStart ? 1 : castedStart->Weight);
	float EndWeight = (nullptr == castedEnd ? 1 : castedEnd->Weight);

	float TurningPoint;
	if (StartWeight <= EndWeight)
		TurningPoint = (StartWeight / EndWeight) * 0.5f;
	else
		TurningPoint = 1 - ((EndWeight / StartWeight) * 0.5f);

	FRotator MidRot = FMath::Lerp(InStart->GetComponentRotation(), InEnd->GetComponentRotation(), 0.5f);
	FVector MidScl = FMath::Lerp(InStart->GetComponentScale(), InEnd->GetComponentScale(), 0.5f);

	float FinalRatio;
	if (DistRatio < TurningPoint)
	{
		float ThresholdFromA = (nullptr == castedStart ? DefaultThresholdDistance / WholeDist : castedStart->ThresholdDistance / WholeDist);
		FinalRatio = FMath::Max(0.0f, (DistRatio - ThresholdFromA)) / (TurningPoint - ThresholdFromA);

		OutFinalRot = FMath::Lerp(InStart->GetComponentRotation(), MidRot, FinalRatio);
		OutFinalScale = FMath::Lerp(InStart->GetComponentScale(), MidScl, FinalRatio);
	}
	else
	{
		float ThresholdFromB = (nullptr == castedEnd ? DefaultThresholdDistance / WholeDist : castedEnd->ThresholdDistance / WholeDist);
		FinalRatio = FMath::Min(1.0f, (DistRatio - TurningPoint) / (1 - TurningPoint - ThresholdFromB));

		OutFinalRot = FMath::Lerp(MidRot, InEnd->GetComponentRotation(), FinalRatio);
		OutFinalScale = FMath::Lerp(MidScl, InEnd->GetComponentScale(), FinalRatio);
	}
}

void AMPHorsoCameraGuideCluster::GetFinalRotAndScaleFromMesh(UArrowComponent* InVertA, UArrowComponent* InVertB, UArrowComponent* InVertC,
															 FVector Pos, FRotator& OutFinalRot, FVector& OutFinalScale)
{
	FVector BaryWeights = 
		FMath::ComputeBaryCentric2D(Pos, InVertA->GetComponentLocation(), InVertB->GetComponentLocation(), InVertC->GetComponentLocation());

	OutFinalRot = (InVertA->GetComponentRotation() * BaryWeights.X) +
				  (InVertB->GetComponentRotation() * BaryWeights.Y) +
				  (InVertC->GetComponentRotation() * BaryWeights.Z);
	
	OutFinalScale = (InVertA->GetComponentScale() * BaryWeights.X) +
					(InVertB->GetComponentScale() * BaryWeights.Y) +
					(InVertC->GetComponentScale() * BaryWeights.Z);
}

void AMPHorsoCameraGuideCluster::GuideCamera()
{
	FRotator ResultRot;
	FVector ResultScale;

	TArray<FName> GrabbedKeys;
	Arrows.GetKeys(GrabbedKeys);

	if (Arrows.Num() < 2)
	{
		ResultRot = Arrows[GrabbedKeys[0]]->GetComponentRotation();
		ResultScale = Arrows[GrabbedKeys[0]]->GetComponentScale();
	}
	else
	{
		FVector ObsLoc = StoredObserver->GetActorLocation();

		FVector ClosestLinePt;
		float BestLineDist;
		FCameraGuideLine* BestGuideLine = GetClosestGuideLine(ObsLoc, ClosestLinePt, BestLineDist);

		FVector ClosestTriPt;
		float BestTriDist;
		FCameraGuideTri* BestGuideTri = GetClosestGuideTri(ObsLoc, ClosestTriPt, BestTriDist);

		if (BestLineDist < BestTriDist)
		{
			//DrawDebugPoint(GetWorld(), ClosestLinePt, 10, FColor::Orange);

			GetFinalRotAndScaleFromLine(Arrows[BestGuideLine->EndA], Arrows[BestGuideLine->EndB], ClosestLinePt, ResultRot, ResultScale);
		}
		else
		{
			//DrawDebugPoint(GetWorld(), ClosestTriPt, 10, FColor::Orange);

			GetFinalRotAndScaleFromMesh(Arrows[BestGuideTri->VertA], Arrows[BestGuideTri->VertB], Arrows[BestGuideTri->VertC],
										ClosestTriPt, ResultRot, ResultScale);
		}


	}

	FRotator FinalRot = FMath::RInterpTo(StoredObserver->GetActorRotation(), ResultRot, GetWorld()->GetDeltaSeconds(), 4.0f);

	if (nullptr != RetrievedRotFunc)
	{
		FRotator Parm = FinalRot;

		StoredObserver->ProcessEvent(RetrievedRotFunc, &Parm);
	}

	StoredObserver->SetActorRotation(FinalRot);


	if (nullptr != RetrievedNextZoom)
	{
		float CurrZoomVal = RetrievedNextZoom->GetPropertyValue_InContainer(StoredObserver);
		float NewZoomVal = FMath::FInterpTo(CurrZoomVal, ResultScale.GetMax(), GetWorld()->GetDeltaSeconds(), 4);

		RetrievedNextZoom->SetPropertyValue_InContainer(StoredObserver, NewZoomVal);
	}


	if (Arrows.Num() < 2 && Arrows[GrabbedKeys[0]]->GetComponentRotation().Equals(StoredObserver->GetActorRotation(), 0.01f))
		GetWorld()->GetTimerManager().ClearTimer(TimerHandle);
}


// Sets default values
AMPHorsoCameraConformer::AMPHorsoCameraConformer(const FObjectInitializer& _init) : Super(_init)
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	RootComponent = _init.CreateDefaultSubobject<USceneComponent>(this, TEXT("RootComponent"));

}

// Called when the game starts or when spawned
void AMPHorsoCameraConformer::BeginPlay()
{
	Super::BeginPlay();

}

void AMPHorsoCameraConformer::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	for (auto &currComp : this->GetComponents())
	{
		if (*currComp->GetName() == ConformComponentName)
		{
			ConformShape = Cast<UShapeComponent>(currComp);

			if (nullptr == ConformShape)
				UStaticFuncLib::Print("MPHorsoCameraConformer::OnConstruction: Found Component \'" +
									  ConformComponentName.ToString() +
									  "\', but it wasn't a ShapeComponent!", 
									  true);

			return;
		}
	}

	UStaticFuncLib::Print("MPHorsoCameraConformer::OnConstruction: Couldn't find a component named \'" +
						  ConformComponentName.ToString() +
						  "\'!",
						  true);

#if WITH_EDITOR

	TInlineComponentArray<UShapeComponent*> Shapes(this);
	for (UShapeComponent* currShape : Shapes)
		currShape->ShapeColor = FColor::Blue;

#endif

}

// Called every frame
void AMPHorsoCameraConformer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AMPHorsoCameraConformer::NotifyActorBeginOverlap(AActor* OtherActor)
{
	AActor* OtherOwner = OtherActor->GetOwner();

	if (nullptr != OtherOwner)
	{
		APawn* CastedToPawn = Cast<APawn>(OtherOwner);

		if (nullptr != CastedToPawn && CastedToPawn->GetController() == UGameplayStatics::GetPlayerController(this, 0))
		{
			StoredObserver = CastedToPawn;

			RetrievedConformTo = nullptr;
			for (TFieldIterator<UObjectProperty> iter(StoredObserver->GetClass()); iter; ++iter)
			{
				//UStaticFuncLib::Print(iter->GetNameCPP(), true);

				if (iter->GetNameCPP() == "ConformTo")
				{
					RetrievedConformTo = *iter;
					break;
				}
			}

			if (nullptr != RetrievedConformTo)
				RetrievedConformTo->SetPropertyValue_InContainer(StoredObserver, ConformShape);
			else
				UStaticFuncLib::Print("AMPHorsoCameraGuide::NotifyActorBeginOverlap: Couldn't find ObserverBP::NextZoomFactor! No zooming will happen.", true);

		}
	}
}

void AMPHorsoCameraConformer::NotifyActorEndOverlap(AActor* OtherActor)
{
	AActor* OtherOwner = OtherActor->GetOwner();

	if (nullptr != OtherOwner && OtherOwner == StoredObserver)
	{
		if (nullptr != RetrievedConformTo)
			RetrievedConformTo->SetPropertyValue_InContainer(StoredObserver, nullptr);
	}
}
