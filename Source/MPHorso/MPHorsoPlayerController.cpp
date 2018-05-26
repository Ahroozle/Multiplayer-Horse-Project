// Fill out your copyright notice in the Description page of Project Settings.

#include "MPHorso.h"
#include "MPHorsoPlayerController.h"
#include "UnrealNetwork.h"


void AMPHorsoPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty> & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMPHorsoPlayerController, MyName);
	DOREPLIFETIME(AMPHorsoPlayerController, EquippedSpells);
	DOREPLIFETIME(AMPHorsoPlayerController, OpenChannels);

}

void AMPHorsoPlayerController::PassMessage_Implementation(AMPHorsoPlayerController* Sender, const FString& Msg) {}

void AMPHorsoPlayerController::PassToPersonalBubble_Implementation(const FString& Msg) {}


void AMPHorsoPlayerController::RunOnServer_Implementation(UObject* Obj, const FString& FuncToCall)
{
	UFunction* FoundFunc = Obj->FindFunction(*FuncToCall);

	if (nullptr != FoundFunc)
		Obj->ProcessEvent(FoundFunc, nullptr);
}

bool AMPHorsoPlayerController::RunOnServer_Validate(UObject* Obj, const FString& FuncToCall) { return true; }