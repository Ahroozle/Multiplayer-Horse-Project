// Fill out your copyright notice in the Description page of Project Settings.

#include "MPHorso.h"
#include "MPHorsoGameInstance.h"

#include "SkeletalSpriteAnimation.h"

#include "StaticFuncLib.h"

#include "MPHorsoSaveGameTypes.h"


void UMPHorsoGameInstance::Init()
{
	GEngine->OnNetworkFailure().AddUObject(this, &UMPHorsoGameInstance::OnNetFail);
}

void UMPHorsoGameInstance::OnNetFail(UWorld *World, UNetDriver *NetDriver, ENetworkFailure::Type FailureType, const FString& ErrorString)
{
	RuntimeErrorList.Add(ErrorString);
}

ERaceType UMPHorsoGameInstance::GetRace()
{
	if (nullptr != CharacterSave)
		return CharacterSave->Race;

	else return ERaceType::Race_EarthP;
}

void UMPHorsoGameInstance::GetColorSchemeAsArrays(TArray<FName>& OutParts, TArray<FLinearColor>& OutColors)
{
	if (nullptr != CharacterSave)
	{
		CharacterSave->ColorScheme.GenerateKeyArray(OutParts);
		CharacterSave->ColorScheme.GenerateValueArray(OutColors);
	}
}

FName UMPHorsoGameInstance::GetLocalPlayerName()
{
	if (nullptr != CharacterSave)
		return *CharacterSave->CharacterName;

	else return FName(EName::NAME_None);
}
