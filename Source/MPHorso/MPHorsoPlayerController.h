// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/PlayerController.h"
#include "MPHorsoCutsceneTypes.h"
#include "MPHorsoPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class MPHORSO_API AMPHorsoPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated)
		FName MyName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated)
		TArray<FName> EquippedSpells;


	UFUNCTION(BlueprintCallable, Server, Reliable, WithValidation)
		void RunOnServer(UObject* Obj, const FString& FuncToCall);
	void RunOnServer_Implementation(UObject* Obj, const FString& FuncToCall);
	bool RunOnServer_Validate(UObject* Obj, const FString& FuncToCall);

	UFUNCTION(BlueprintImplementableEvent, BlueprintPure)
		AActor* GetBody();

	UFUNCTION(BlueprintCallable, Server, Reliable, WithValidation)
		void SendChatMessage(const FString& Message);
		void SendChatMessage_Implementation(const FString& Message);
		bool SendChatMessage_Validate(const FString& Message);

	UFUNCTION(BlueprintImplementableEvent)
		void ReceiveChatMessage(const FSpeechParsedMessage& Message);
};
