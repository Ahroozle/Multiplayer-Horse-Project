// Fill out your copyright notice in the Description page of Project Settings.

#include "MPHorso.h"
#include "MPHorsoChatTypes.h"

#include "StaticFuncLib.h"

#include "MPHorsoGameInstance.h"

#include "MPHorsoPlayerController.h"


void UChatWidget::SendMessage(FString Message)
{
	AMPHorsoPlayerController* PC = Cast<AMPHorsoPlayerController>(GetOwningPlayer());

	if (nullptr != PC)
		PC->SendChatMessage(Message);
}

void UChatWidget::ConstructMessageLetters(const FSpeechParsedMessage& Message, TArray<UUserWidget*>& OutLetters)
{
	OutLetters.Empty();

	if (nullptr != LetterClass)
	{
		int CurrTagInd = 0;
		TArray<FSpeechTag> TagStack;

		for (auto iter = Message.Message.CreateConstIterator(); iter && *iter; ++iter)
		{
			while (Message.SpeechTags.Num() > CurrTagInd && iter.GetIndex() >= Message.SpeechTags[CurrTagInd].Index)
			{
				if (!Message.SpeechTags[CurrTagInd].Removing)
					TagStack.Push(Message.SpeechTags[CurrTagInd]);
				else
					TagStack.Pop();

				++CurrTagInd;
			}

			USpeechBubbleLetter* NewLetter = CreateWidget<USpeechBubbleLetter>(UStaticFuncLib::RetrieveGameInstance(this), LetterClass);

			NewLetter->PreInit();

			if (nullptr != NewLetter->TextBlock)
				NewLetter->TextBlock->SetText(FText::FromString(FString(1, &*iter)));

			NewLetter->LetterIndex = iter.GetIndex();
			NewLetter->Init(TagStack, 0);

			OutLetters.Add(NewLetter);
		}
	}
	else
		UStaticFuncLib::Print("UChatWidget::ConstructMessageLetters: LetterClass was null!", true);
}
