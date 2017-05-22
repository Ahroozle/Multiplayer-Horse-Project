// Fill out your copyright notice in the Description page of Project Settings.

#include "MPHorso.h"
#include "MPHorsoGameInstance.h"

#include "SkeletalSpriteAnimation.h"

#include "StaticFuncLib.h"


void UMPHorsoGameInstance::Init()
{
	GEngine->OnNetworkFailure().AddUObject(this, &UMPHorsoGameInstance::OnNetFail);
}

void UMPHorsoGameInstance::OnNetFail(UWorld *World, UNetDriver *NetDriver, ENetworkFailure::Type FailureType, const FString& ErrorString)
{
	RuntimeErrorList.Add(ErrorString);
}

//USkinAnimation* UMPHorsoGameInstance::GetSkinAnim(TSubclassOf<USkinAnimation> InstClass)
//{
//	if (UStaticFuncLib::ValidateObject(InstClass, "UMPHorsoGameInstance::GetSkinAnim: Skin animation class passed in was invalid!", true))
//	{
//		USkinAnimation* foundSkin = SkinInsts.FindRef(InstClass);
//		if (nullptr == foundSkin || !foundSkin->IsValidLowLevel())
//		{
//			foundSkin = NewObject<USkinAnimation>((UObject*)GetTransientPackage(), InstClass);
//			SkinInsts.Add(InstClass, foundSkin);
//		}
//		return foundSkin;
//	}
//	return nullptr;
//}
//
//UOffsetAnimation* UMPHorsoGameInstance::GetOffsAnim(TSubclassOf<UOffsetAnimation> InstClass)
//{
//	if (UStaticFuncLib::ValidateObject(InstClass, "UMPHorsoGameInstance::GetOffsAnim: Offset animation class passed in was invalid!", true))
//	{
//		UOffsetAnimation* foundOffs = OffsInsts.FindRef(InstClass);
//		if (nullptr == foundOffs)
//		{
//			foundOffs = NewObject<UOffsetAnimation>((UObject*)GetTransientPackage(), InstClass);
//			OffsInsts.Add(InstClass, foundOffs);
//		}
//		return foundOffs;
//	}
//	return nullptr;
//}
