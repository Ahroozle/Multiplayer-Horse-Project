// Fill out your copyright notice in the Description page of Project Settings.

#include "MPHorso.h"
#include "ChatTypes.h"

#include "MPHorsoPlayerController.h"

#include "StaticFuncLib.h"

bool UChatCommandArchetype::Execute_Implementation(AMPHorsoPlayerController* Caller, const FString& Args, UPARAM(Ref) FChatMessage& InOutMessage, FString& FailReason)
{
	return false;
}


TArray<TSubclassOf<UChatCommandArchetype>> UChatActionsLibrary::CommandArchetypes;
TArray<TSubclassOf<UChatEmoteArchetype>> UChatActionsLibrary::EmoteArchetypes;

void UChatActionsLibrary::PopulateCommandArchetypes()
{
	for (TObjectIterator<UClass> iter; iter; ++iter)
	{
		if (iter->IsChildOf(UChatCommandArchetype::StaticClass()) &&
			*iter != UChatCommandArchetype::StaticClass() &&
			!iter->GetName().StartsWith("Skel_"))
		{
			//UStaticFuncLib::Print(iter->GetName(), true);
			CommandArchetypes.Add(*iter);
		}
	}
}

void UChatActionsLibrary::PopulateEmoteArchetypes()
{
	for (TObjectIterator<UClass> iter; iter; ++iter)
	{
		if (iter->IsChildOf(UChatEmoteArchetype::StaticClass()) &&
			*iter != UChatEmoteArchetype::StaticClass() &&
			!iter->GetName().StartsWith("Skel_"))
		{
			//UStaticFuncLib::Print(iter->GetName(), true);
			EmoteArchetypes.Add(*iter);
		}
	}
}

void UChatActionsLibrary::TranslateMessage(const FString& InMessage, FChatMessage& OutMessage, FChatCommand& OutCommand)
{
	if (InMessage.StartsWith("/"))
	{
		// Check for command stuff
		if (CommandArchetypes.Num() < 1)
			PopulateCommandArchetypes();

		FString CommandHold, SplitLeft, SplitRight;

		if (!InMessage.Split(" ", &CommandHold, &SplitRight))
			CommandHold = InMessage;

		FName CommandCast = *(CommandHold.ToLower());
		for (auto &curr : CommandArchetypes)
		{
			if (curr.GetDefaultObject()->CommandNames.Contains(CommandCast))
			{
				OutCommand.ClassPtr = curr;

				if (!SplitRight.IsEmpty())
				{
					FString ArgString;
					for (int currArg = 0; currArg < curr.GetDefaultObject()->ArgumentNames.Num(); ++currArg)
					{
						if (SplitRight.Split(" ", &SplitLeft, &SplitRight))
						{
							ArgString += SplitLeft + " ";
						}
						else
						{
							ArgString = SplitRight;
							break;
						}
					}
					OutCommand.Parameters = ArgString.TrimTrailing();
				}

				break;
			}
		}
		
		if (nullptr == OutCommand.ClassPtr)
		{
			for (auto &curr : CommandArchetypes)
			{
				if (curr->GetName().Contains("_Invalid"))
				{
					OutCommand.ClassPtr = curr;
					OutCommand.Parameters = CommandHold;

					break;
				}
			}
		}

		OutMessage.Message = SplitRight;
		return;
	}

	OutMessage.ChatChannel = "General";
	OutMessage.Message = InMessage;
}

void UChatActionsLibrary::TimeAsString(FString& OutString)
{
	int32 Year, Month, Day, DayOfWeek, Hour, Minute, Second, Millisecond;
	FPlatformTime::SystemTime(Year, Month, DayOfWeek, Day, Hour, Minute, Second, Millisecond);

	OutString = FString::Printf(TEXT("[ %02d:%02d:%02d ] "), Hour, Minute, Second);
}

void UChatActionsLibrary::SplitStringWithEmotes(const FString& In, TArray<FString>& Out)
{
	// PER-CHARACTER VERSION (DEPRECATED)

	//int firstind;
	//if (In.FindChar(':',firstind))
	//{
	//	TArray<FString> PreFeed;
	//	In.ParseIntoArray(PreFeed,TEXT(":"), false);

	//	// the structure of sentences makes it so that when split
	//	// and including empty strings, every even-numbered piece
	//	// starting from 1 is an emote

	//	int currCount = 1;
	//	for (auto &currPiece : PreFeed)
	//	{
	//		if (currCount % 2)
	//		{
	//			if (!currPiece.IsEmpty())
	//			{
	//				for (auto &currChar : currPiece.GetCharArray())
	//					Out.Add(FString(1, &currChar));

	//				Out.RemoveAt(Out.Num() - 1, 1); // remove null terminator
	//			}
	//		}
	//		else
	//			Out.Add(":" + currPiece.ToLower() + ":");

	//		++currCount;
	//	}
	//}
	//else
	//{
	//	for (auto &curr : In.GetCharArray())
	//		Out.Add(FString(1, &curr));

	//	Out.RemoveAt(Out.Num() - 1, 1); // remove null terminator
	//}

	//PER-WORD VERSION

	int firstind;
	if (In.FindChar(':',firstind))
	{
		TArray<FString> PreFeed;
		In.ParseIntoArray(PreFeed,TEXT(":"), false);

		// the structure of sentences makes it so that when split
		// and including empty strings, every even-numbered piece
		// starting from 1 is an emote

		int currCount = 1;
		for (auto &currPiece : PreFeed)
		{
			if (currCount % 2)
			{
				if (!currPiece.IsEmpty())
					Out.Add(currPiece);
			}
			else
				Out.Add(":" + currPiece.ToLower() + ":");

			++currCount;
		}
	}
	else
	{
		Out.Add(In);
	}
}

bool UChatActionsLibrary::IsEmote(const FString& String, class UTexture*& OutEmoteImage)
{
	if (String.StartsWith(":"))
	{
		if (EmoteArchetypes.Num() < 1)
			PopulateEmoteArchetypes();

		for (auto &curr : EmoteArchetypes)
		{
			if (curr.GetDefaultObject()->EmoteNames.Contains(String))
			{
				OutEmoteImage = curr.GetDefaultObject()->Image;
				return true;
			}
		}

		UStaticFuncLib::Print("Couldn't find an emote named \'" + String + "\'!\n(Note: Did you make sure to add it to the appropriate MPHorsoGameInstanceBP array?)", true);
	}

	return false;
}
