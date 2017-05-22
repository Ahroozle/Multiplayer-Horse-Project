// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/PlayerState.h"
#include "ChatTypes.h"
#include "MPHorsoPlayerState.generated.h"

class AMPHorsoPlayerController;
/**
 * 
 */
UCLASS()
class MPHORSO_API AMPHorsoPlayerState : public APlayerState
{
	GENERATED_BODY()

public:

	// TODO some funcs for private messaging?
	// i.e. on the owning client from the server if the message is private
	// so that no one else actually even gets the message?

	UFUNCTION(BlueprintCallable, Server, Reliable, WithValidation)
		void ServerChatSend(AMPHorsoPlayerController* Sender, const FString& NewRawMessage);
		void ServerChatSend_Implementation(AMPHorsoPlayerController* Sender, const FString& NewRawMessage);
		bool ServerChatSend_Validate(AMPHorsoPlayerController* Sender, const FString& NewRawMessage);

	UFUNCTION(NetMulticast, Reliable, WithValidation)
		void MulticastChatSend(const FChatMessage& NewMessage);
		void MulticastChatSend_Implementation(const FChatMessage& NewMessage);
		bool MulticastChatSend_Validate(const FChatMessage& NewMessage);
	
};
