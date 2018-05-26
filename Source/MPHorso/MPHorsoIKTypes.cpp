// Fill out your copyright notice in the Description page of Project Settings.

#include "MPHorso.h"
#include "MPHorsoIKTypes.h"

#include "StaticFuncLib.h"


// Sets default values for this component's properties
UMPHorsoIKRoot::UMPHorsoIKRoot()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}

// Called when the game starts
void UMPHorsoIKRoot::BeginPlay()
{
	Super::BeginPlay();

	// ...

	InitChains();

	OriginalRelativePosition = RelativeLocation;

	FTimerHandle dummyHandle;
	GetWorld()->GetTimerManager().SetTimer(dummyHandle, this, &UMPHorsoIKRoot::MonitorChains, .01f, true);

}

// Called every frame
void UMPHorsoIKRoot::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UMPHorsoIKRoot::InitChains()
{
	// Get leaf children
	TArray<USceneComponent*> ChildQueue;
	GetChildrenComponents(true, ChildQueue);
	ChildQueue = ChildQueue.FilterByPredicate([](USceneComponent* const a) { return a->GetNumChildrenComponents() <= 0; });

	// Make an IK chain for each leaf child that exists
	const USceneComponent* MyAttachParent = GetAttachParent();
	for (USceneComponent* curr : ChildQueue)
	{
		FMPHorsoIKChain& currChain = Chains[Chains.AddDefaulted()];

		currChain.Joints[currChain.Joints.AddDefaulted()].SceneComponent = curr;

		USceneComponent* currParent = curr->GetAttachParent();
		USceneComponent* currPrevious = curr;
		while (MyAttachParent != currParent)
		{
			FMPHorsoIKJoint& currJoint = currChain.Joints[currChain.Joints.AddDefaulted()];
			currJoint.SceneComponent = currParent;
			currJoint.Forward = currPrevious->GetComponentLocation() - currParent->GetComponentLocation();

			currPrevious = currParent;
			currParent = currParent->GetAttachParent();
		}

		//currChain.PreviousTargetTransform = (nullptr == Target ? FTransform::Identity : Target->GetComponentTransform());
	}
}

void UMPHorsoIKRoot::MonitorChains()
{
	if (nullptr == Target)
		return;

	int NumUpdated = 0;
	for (FMPHorsoIKChain& curr : Chains)
	{
		if (!Target->GetComponentTransform().Equals(curr.PreviousTargetTransform))
		{
			curr.Joints[0].SceneComponent->SetWorldTransform(Target->GetComponentTransform());
			UpdateChain(curr);
			curr.PreviousTargetTransform = Target->GetComponentTransform();
			++NumUpdated;
		}
	}

	if (Anchored && NumUpdated > 0)
		SetRelativeLocation(OriginalRelativePosition);

}

void UMPHorsoIKRoot::UpdateChain(FMPHorsoIKChain& Chain)
{
	TArray<FTransform> HeldTransforms;
	HeldTransforms.Add(Chain.Joints[0].SceneComponent->GetComponentTransform());

	TArray<FMPHorsoIKJoint>& Joints = Chain.Joints;
	for (int Ind = 1; Ind < Joints.Num(); ++Ind)
	{
		FMPHorsoIKJoint& Curr = Joints[Ind];
		FMPHorsoIKJoint& Next = Joints[Ind - 1];

		FMPHorsoIKAlignAxes* CurrAlignAxes = SpecialAlignAxes.Find(Curr.SceneComponent->GetFName());
		if (nullptr == CurrAlignAxes)
			CurrAlignAxes = &DefaultAlignAxes;

		FVector ToCurr = (Next.SceneComponent->GetComponentLocation() - Curr.SceneComponent->GetComponentLocation());
		ToCurr.Normalize();

		ToCurr *= Curr.Forward.Size();
		Curr.SceneComponent->SetWorldLocation(Next.SceneComponent->GetComponentLocation() - ToCurr);

		Curr.Forward = ToCurr;

		if (EAxis::None != CurrAlignAxes->ForwardAxis &&
			EAxis::None != CurrAlignAxes->UpAxis &&
			CurrAlignAxes->ForwardAxis != CurrAlignAxes->UpAxis)
		{
			FRotator NewRot;

			FMatrix(*TransformFunc)(const FVector&, const FVector&) = nullptr;

			switch (CurrAlignAxes->ForwardAxis)
			{
			default:
			case EAxis::X:
				if (EAxis::Y == CurrAlignAxes->UpAxis)
					TransformFunc = &FRotationMatrix::MakeFromXY;
				else
					TransformFunc = &FRotationMatrix::MakeFromXZ;
				break;
			case EAxis::Y:
				if (EAxis::X == CurrAlignAxes->UpAxis)
					TransformFunc = &FRotationMatrix::MakeFromYX;
				else
					TransformFunc = &FRotationMatrix::MakeFromYZ;
				break;
			case EAxis::Z:
				if (EAxis::X == CurrAlignAxes->UpAxis)
					TransformFunc = &FRotationMatrix::MakeFromZX;
				else
					TransformFunc = &FRotationMatrix::MakeFromZY;
				break;
			}

			NewRot = TransformFunc(ToCurr * (CurrAlignAxes->NegateForward ? -1 : 1),
				CurrAlignAxes->WorldUp * (CurrAlignAxes->NegateForward ? -1 : 1)).Rotator();

			Curr.SceneComponent->SetWorldRotation(NewRot);

		}

		HeldTransforms.Add(Curr.SceneComponent->GetComponentTransform());
	}

	for (int i = HeldTransforms.Num() - 1; i > 0; --i)
		Joints[i].SceneComponent->SetWorldTransform(HeldTransforms[i]);


	FMPHorsoIKJoint& Head = Joints[0];
	FMPHorsoIKJoint& Scnd = Joints[1];

	Head.SceneComponent->SetWorldLocation(Scnd.SceneComponent->GetComponentLocation() + Scnd.Forward);
}

