// Fill out your copyright notice in the Description page of Project Settings.

#include "MPHorso.h"
#include "ChatTypes.h"

#include "MPHorsoPlayerController.h"

#include "StaticFuncLib.h"

#include "MPHorsoGameInstance.h"


//void UChatActionsLibrary::TranslateMessage(const FString& InMessage, FChatMessage& OutMessage, FChatCommand& OutCommand)
//{
//	if (InMessage.StartsWith("/"))
//	{
//		// Check for command stuff
//		if (CommandArchetypes.Num() < 1)
//			PopulateCommandArchetypes();
//
//		FString CommandHold, SplitLeft, SplitRight;
//
//		if (!InMessage.Split(" ", &CommandHold, &SplitRight))
//			CommandHold = InMessage;
//
//		FName CommandCast = *(CommandHold.ToLower());
//		for (auto &curr : CommandArchetypes)
//		{
//			if (curr.GetDefaultObject()->CommandNames.Contains(CommandCast))
//			{
//				OutCommand.ClassPtr = curr;
//
//				if (!SplitRight.IsEmpty())
//				{
//					FString ArgString;
//					for (int currArg = 0; currArg < curr.GetDefaultObject()->ArgumentNames.Num(); ++currArg)
//					{
//						if (SplitRight.Split(" ", &SplitLeft, &SplitRight))
//						{
//							ArgString += SplitLeft + " ";
//						}
//						else
//						{
//							ArgString = SplitRight;
//							break;
//						}
//					}
//					OutCommand.Parameters = ArgString.TrimTrailing();
//				}
//
//				break;
//			}
//		}
//		
//		if (nullptr == OutCommand.ClassPtr)
//		{
//			for (auto &curr : CommandArchetypes)
//			{
//				if (curr->GetName().Contains("_Invalid"))
//				{
//					OutCommand.ClassPtr = curr;
//					OutCommand.Parameters = CommandHold;
//
//					break;
//				}
//			}
//		}
//
//		//OutMessage.Message = SplitRight;
//		return;
//	}
//
//	//OutMessage.ChatChannel = "General";
//	//OutMessage.Message = InMessage;
//}

void UChatActionsLibrary::TimeAsString(FString& OutString)
{
	int32 Year, Month, Day, DayOfWeek, Hour, Minute, Second, Millisecond;
	FPlatformTime::SystemTime(Year, Month, DayOfWeek, Day, Hour, Minute, Second, Millisecond);

	OutString = FString::Printf(TEXT("[ %02d:%02d:%02d ] "), Hour, Minute, Second);
}

void UChatActionsLibrary::ParseTags(const FString& InMessage, TArray<FDeconstructedTagData>& OutDeconstructed)
{
	TArray<FName> CurrTags;
	TArray<FString> CurrTagParams;

	for (auto iter = InMessage.CreateConstIterator(); iter; ++iter)
	{

		switch (*iter)
		{
		case TEXT('['): // start of tag
			{
				auto furtheriter = iter;
				FString TagRaw;
				++furtheriter;
				while (furtheriter && (*furtheriter != ':' && *furtheriter != '[' && *furtheriter != ']'))
				{
					TagRaw += *furtheriter;
					++furtheriter;
				}

				if (furtheriter && *furtheriter == ':' && !TagRaw.IsEmpty())
				{
					// tag is properly defined enough, split and push.

					FString LHS, RHS;
					if (!TagRaw.Split("/", &LHS, &RHS))
						LHS = TagRaw;

					CurrTags.Push(*LHS);
					CurrTagParams.Push(RHS);

					while (iter != furtheriter)
						++iter;
				}
				else
				{
					// invalid tag, just consider this sequence a normal word.

					FDeconstructedTagData NewDecon;

					while ((iter + 1) != furtheriter)
					{
						NewDecon.Data += *iter;
						++iter;
					}
					NewDecon.Data += *iter;

					NewDecon.Tags = CurrTags;
					NewDecon.TagParams = CurrTagParams;

					OutDeconstructed.Add(NewDecon);
				}
			}

			break;

		case TEXT(']'): // end of tag
			{
				if (CurrTags.Num() > 0)
				{
					CurrTags.Pop();
					CurrTagParams.Pop();
				}
				else
				{
					FDeconstructedTagData NewDecon;
					NewDecon.Data += *iter;

					NewDecon.Tags = CurrTags;
					NewDecon.TagParams = CurrTagParams;

					OutDeconstructed.Add(NewDecon);
				}

			}
			break;

		default: // other
			{
				// new actual deconstructed data to create

				FDeconstructedTagData NewDecon;

				auto furtheriter = iter;
				while (furtheriter && (*furtheriter != '[' && *furtheriter != ']'))
				{
					NewDecon.Data += *furtheriter;
					++furtheriter;
				}

				NewDecon.Tags = CurrTags;
				NewDecon.TagParams = CurrTagParams;

				OutDeconstructed.Add(NewDecon);

				while ((iter + 1) != furtheriter)
					++iter;
			}
			break;
		}

	}

}

void UChatActionsLibrary::ApplyTags(UObject* Caller, const FString& Message, TArray<UUserWidget*>& ConstructedWords, bool PerLetterWords)
{
	UMPHorsoGameInstance* GameInst = UStaticFuncLib::RetrieveGameInstance(Caller);

	if (nullptr != GameInst)
	{
		TArray<FDeconstructedTagData> Decon;
		ParseTags(Message, Decon);

		GameInst->ChatTagBlock.GetDefaultObject()->Apply(Caller, Decon, GameInst->DefaultChatWordType, ConstructedWords, PerLetterWords);
	}
	else
		UStaticFuncLib::Print("UChatActionsLibrary::ApplyTags: Couldn't retrieve the gameinstance!", true);
}

int UChatActionsLibrary::Roll(int NumDie, int Sidedness)
{
	int RollingRes = 0;

	while (--NumDie >= 0)
		RollingRes += FMath::RandRange(1, Sidedness);

	return RollingRes;
}
