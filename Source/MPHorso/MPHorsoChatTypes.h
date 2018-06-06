// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Blueprint/UserWidget.h"

#include "MPHorsoCutsceneTypes.h"

#include "MPHorsoChatTypes.generated.h"


/*
	TODO maybe add setting for enabling/disabling tag effects in chat?
*/

UCLASS()
class MPHORSO_API UChatWidget : public UUserWidget
{
	GENERATED_BODY()

public:

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		TSubclassOf<USpeechBubbleLetter> LetterClass;


	UFUNCTION(BlueprintCallable)
		void SendMessage(FString Message);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
		void ReceiveMessage(const FSpeechParsedMessage& Message);

	UFUNCTION(BlueprintCallable)
		void ConstructMessageLetters(const FSpeechParsedMessage& Message, TArray<UUserWidget*>& OutLetters);
};

/**
 * 
 */
//UCLASS()
//class MPHORSO_API UMPHorsoChatTypes : public UUserWidget
//{
//	GENERATED_BODY()
//	
//	
//	
//	
//};
