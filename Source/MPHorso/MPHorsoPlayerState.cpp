// Fill out your copyright notice in the Description page of Project Settings.

#include "MPHorso.h"
#include "MPHorsoPlayerState.h"

#include "MPHorsoPlayerController.h"

#include "StaticFuncLib.h"

#include "MPHorsoGameInstance.h"


void AMPHorsoPlayerState::ServerChatSend_Implementation(AMPHorsoPlayerController* Sender, const FString& NewRawMessage)
{
	FString EndMessage;
	FString BubbleMessage;
	bool shouldPropagate = true; // TODO change to enum representing party or something?

	UMPHorsoGameInstance* GameInst = UStaticFuncLib::RetrieveGameInstance(Sender);

	if (nullptr != GameInst)
	{
		if (UChatActionsLibrary::IsCommand(NewRawMessage))
		{
			if (!GameInst->ChatCommandBlock.GetDefaultObject()->Apply(Sender, NewRawMessage, EndMessage, BubbleMessage, shouldPropagate))
			{
				FString temp;
				if (!NewRawMessage.Split(" ", &temp, nullptr))
					temp = NewRawMessage;

				// failure case and stuff
				EndMessage = "[c/ff0000: The command \'" + temp + "\' wasn't found.]";

				shouldPropagate = false;
			}
		}
		else
		{
			BubbleMessage = NewRawMessage;
			EndMessage = "[n:" + Sender->MyName.ToString() + "] " + NewRawMessage;
		}
	}

	if(!BubbleMessage.IsEmpty())
		Sender->PassToPersonalBubble(BubbleMessage);

	if (!EndMessage.IsEmpty())
	{
		if (shouldPropagate)
			MulticastChatSend(Sender, EndMessage);
		else
			ClientMessageSend(Sender, EndMessage);//Sender->PassMessage(EndMessage);
	}
}

bool AMPHorsoPlayerState::ServerChatSend_Validate(AMPHorsoPlayerController* Sender, const FString& NewRawMessage) { return true; }

void AMPHorsoPlayerState::MulticastChatSend_Implementation(AMPHorsoPlayerController* Sender, const FString& NewMessage)
{

	AMPHorsoPlayerController* currCon;
	for (FConstPlayerControllerIterator iter = GetWorld()->GetPlayerControllerIterator(); iter; ++iter)
	{
		if (nullptr != (currCon = Cast<AMPHorsoPlayerController>(*iter)))
		{
			currCon->PassMessage(/*Sender*/currCon, NewMessage);
			//if (currCon->OpenChannels.Contains(NewMessage.ChatChannel))
			//	currCon->PassMessage(NewMessage);
		}
	}
}

bool AMPHorsoPlayerState::MulticastChatSend_Validate(AMPHorsoPlayerController* Sender, const FString& NewMessage) { return true; }

void AMPHorsoPlayerState::ClientMessageSend_Implementation(AMPHorsoPlayerController* Sender, const FString& NewMessage)
{
	Sender->PassMessage(Sender, NewMessage);
}

bool AMPHorsoPlayerState::ClientMessageSend_Validate(AMPHorsoPlayerController* Sender, const FString& NewMessage) { return true; }
