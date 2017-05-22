// Fill out your copyright notice in the Description page of Project Settings.

#include "MPHorso.h"
#include "SkeletalSpriteBodyComponent.h"

#include "SkeletalSpriteFlipbookComponent.h"

#include "SkeletalSpriteAnimation.h"

//#include "UnrealNetwork.h"
#include "StaticFuncLib.h"


// Sets default values for this component's properties
USkeletalSpriteBodyComponent::USkeletalSpriteBodyComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	// ...
}

// Called when the game starts
void USkeletalSpriteBodyComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...

	TArray<USceneComponent*> AllChildren;
	GetChildrenComponents(true, AllChildren);

	for (auto *currChild : AllChildren)
	{
		if (currChild->GetClass() == USkeletalSpriteFlipbookComponent::StaticClass())
		{
			USkeletalSpriteFlipbookComponent* currCasted = Cast<USkeletalSpriteFlipbookComponent>(currChild);
			Bones.Add(currChild->GetFName(), currCasted);
			currCasted->OriginalRelative = currChild->GetRelativeTransform();

			//UStaticFuncLib::Print(currChild->GetName(), true);
		}
	}
	
}


// Called every frame
void USkeletalSpriteBodyComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}


void USkeletalSpriteBodyComponent::PlayAnimation(TSubclassOf<class USkinAnimation> SpriteAnim, TSubclassOf<class UOffsetAnimation> OffsAnim)
{
	USkinAnimation* SkinInst = nullptr;
	if (nullptr != SpriteAnim)
		SkinInst = SpriteAnim.GetDefaultObject();//UStaticFuncLib::GetSkinAnim(this, SpriteAnim);
	UOffsetAnimation* OffsInst = nullptr;
	if (nullptr != OffsAnim)
		OffsInst = OffsAnim.GetDefaultObject();//UStaticFuncLib::GetOffsAnim(this, OffsAnim);

	int maxInd = FMath::Max((SkinInst == nullptr? 0 : SkinInst->Data.Num()), (OffsInst == nullptr ? 0 : OffsInst->Data.Num()));
	
	USkeletalSpriteFlipbookComponent* currFlip;
	for (int currInd = 0; currInd < maxInd; ++currInd)
	{
		if (nullptr != SkinInst && currInd < SkinInst->Data.Num())
		{
			if (nullptr != (currFlip = Bones.FindRef(SkinInst->Data[currInd].BoneName)))
				currFlip->ChangeSkinAnim(SkinInst, currInd);
			else
				UStaticFuncLib::Print("USkeletalSpriteBodyComponent::PlayAnimation: (Skin) Couldn't find a bone named \'" + SkinInst->Data[currInd].BoneName.ToString() + "\'!", true);
		}

		if (nullptr != OffsInst && currInd < OffsInst->Data.Num())
		{
			if (nullptr != (currFlip = Bones.FindRef(OffsInst->Data[currInd].BoneName)))
				currFlip->ChangeOffsetAnim(OffsInst, currInd);
			else
				UStaticFuncLib::Print("USkeletalSpriteBodyComponent::PlayAnimation: (Offset) Couldn't find a bone named \'" + OffsInst->Data[currInd].BoneName.ToString() + "\'!", true);
		}
	}
}
