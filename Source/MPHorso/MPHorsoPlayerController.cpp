// Fill out your copyright notice in the Description page of Project Settings.

#include "MPHorso.h"
#include "MPHorsoPlayerController.h"
#include "UnrealNetwork.h"


void AMPHorsoPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty> & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMPHorsoPlayerController, MyName);
	DOREPLIFETIME(AMPHorsoPlayerController, EquippedSpells);

}

void AMPHorsoPlayerController::RunOnServer_Implementation(UObject* Obj, const FString& FuncToCall)
{
	UFunction* FoundFunc = Obj->FindFunction(*FuncToCall);

	if (nullptr != FoundFunc)
		Obj->ProcessEvent(FoundFunc, nullptr);
}

bool AMPHorsoPlayerController::RunOnServer_Validate(UObject* Obj, const FString& FuncToCall) { return true; }


void AMPHorsoPlayerController::SendChatMessage_Implementation(const FString& Message) { UCutsceneFuncLib::Say(GetBody(), Message); }

bool AMPHorsoPlayerController::SendChatMessage_Validate(const FString& Message) { return true; }
