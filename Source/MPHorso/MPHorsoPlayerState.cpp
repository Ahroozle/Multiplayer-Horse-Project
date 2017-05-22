// Fill out your copyright notice in the Description page of Project Settings.

#include "MPHorso.h"
#include "MPHorsoPlayerState.h"

#include "MPHorsoPlayerController.h"

#include "StaticFuncLib.h"


void AMPHorsoPlayerState::ServerChatSend_Implementation(AMPHorsoPlayerController* Sender, const FString& NewRawMessage)
{
	bool shouldBubble = true;
	bool shouldMessage = true;

	FChatMessage Message;
	FChatCommand Command;

	UChatActionsLibrary::TranslateMessage(NewRawMessage, Message, Command);

	if (nullptr != Sender)
		Message.Sender = Sender->MyName;

	if (nullptr != Command.ClassPtr/*->IsValidLowLevel()*/)
	{
		shouldBubble = Command.ClassPtr.GetDefaultObject()->ShouldMakeSpeechBubble;
		shouldMessage = Command.ClassPtr.GetDefaultObject()->ShouldMakeMessage;

		FString FailedReason;
		
		bool CommandSucceeded = Command.ClassPtr.GetDefaultObject()->Execute(Sender, Command.Parameters, Message, FailedReason);

		if (!CommandSucceeded)
		{
			// TODO : DISPLAY FAIL REASON SOMEHOW
			UStaticFuncLib::Print("Command Failed! Reason: " + FailedReason, true);

			if(Command.ClassPtr->GetName().Contains("_Invalid"))
				UStaticFuncLib::Print("(Note: Did you make sure to add it to the appropriate MPHorsoGameInstanceBP array?)", true);
		}
	}

	if(nullptr != Sender && shouldBubble)
		Sender->PassToPersonalBubble(Message);

	if (shouldMessage)
		MulticastChatSend(Message);
}

bool AMPHorsoPlayerState::ServerChatSend_Validate(AMPHorsoPlayerController* Sender, const FString& NewRawMessage) { return true; }

void AMPHorsoPlayerState::MulticastChatSend_Implementation(const FChatMessage& NewMessage)
{

	AMPHorsoPlayerController* currCon;
	for (FConstPlayerControllerIterator iter = GetWorld()->GetPlayerControllerIterator(); iter; ++iter)
	{
		if (nullptr != (currCon = Cast<AMPHorsoPlayerController>(*iter)))
		{
			if (currCon->OpenChannels.Contains(NewMessage.ChatChannel))
				currCon->PassMessage(NewMessage);
		}
	}
}

bool AMPHorsoPlayerState::MulticastChatSend_Validate(const FChatMessage& NewMessage) { return true; }
