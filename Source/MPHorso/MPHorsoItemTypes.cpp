// Fill out your copyright notice in the Description page of Project Settings.

#include "MPHorso.h"
#include "MPHorsoItemTypes.h"


FString UMPHorsoItemBase::GetFriendlyName_Implementation()
{
	FString RollingName;
	for (auto &currPrefix : Prefixes)
		RollingName += currPrefix.ToString() + " ";

	return RollingName + ItemName.ToString();
}
