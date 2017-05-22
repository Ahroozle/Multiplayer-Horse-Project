// Fill out your copyright notice in the Description page of Project Settings.

#include "MPHorso.h"
#include "MPHorsoPlayerController.h"
#include "UnrealNetwork.h"


void AMPHorsoPlayerController::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMPHorsoPlayerController, MyName);
	DOREPLIFETIME(AMPHorsoPlayerController, EquippedSpells);
	DOREPLIFETIME(AMPHorsoPlayerController, OpenChannels);

}

void AMPHorsoPlayerController::PassMessage_Implementation(const FChatMessage& Msg) {}

void AMPHorsoPlayerController::PassToPersonalBubble_Implementation(const FChatMessage& Msg) {}
