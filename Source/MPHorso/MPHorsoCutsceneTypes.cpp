// Fill out your copyright notice in the Description page of Project Settings.

#include "MPHorso.h"
#include "MPHorsoCutsceneTypes.h"

#include "Runtime/UMG/Public/Animation/WidgetAnimation.h"
#include "Kismet/GameplayStatics.h"

#include "StaticFuncLib.h"
#include "MPHorsoPlayerController.h"

#include "Blueprint/WidgetLayoutLibrary.h"


void USpeechBubbleLetter::Init(const TArray<FSpeechTag>& Tags, float StartTime)
{
	for (const FSpeechTag& curr : Tags)
		HandleTag(curr);

	AnimTime = StartTime;

	if (Anims.Num() > 0)
	{
		FTimerHandle dummy;
		GetWorld()->GetTimerManager().SetTimer(dummy, this, &USpeechBubbleLetter::TickAnimations, 0.01f, true);
	}
}

void USpeechBubbleLetter::TickAnimations()
{
	FWidgetTransform RollingTransform;
	RollingTransform.Shear = RenderTransform.Shear;

	AnimTime += GetWorld()->GetDeltaSeconds();

	for (FName& curr : Anims)
		RollingTransform = HandleAnim(curr, RollingTransform);

	SetRenderTransform(RollingTransform);
}


void USpeechSoundSet::Annotate(const FString& InMessage, TArray<int>& OutSoundIndices, TArray<USoundCue*>& OutSounds)
{
	OutSoundIndices.Empty();
	OutSounds.Empty();

	// TODO maybe add some sort of constructor kind of deal that sorts the words automatically when saved or bool pressed?

	if (!IsManual)
	{
		if (WordSounds.Num() > 0)
		{
			TArray<FWordSound> WordSoundsCopy = WordSounds;
			WordSoundsCopy.Sort([](const FWordSound& a, const FWordSound& b) { return a.Word > b.Word; });

			const int MaxCharsNeeded = WordSoundsCopy[0].Word.Len();

			for (int i = 0; i < InMessage.Len() + 1; ++i)
			{
				FString Substr = InMessage.RightChop(i).Left(MaxCharsNeeded);

				FWordSound* FoundWord =
					WordSoundsCopy.FindByPredicate([&Substr](const FWordSound& a) { return Substr.StartsWith(a.Word); });

				OutSoundIndices.Add(i);

				if (nullptr != FoundWord)
				{
					OutSounds.Add(FoundWord->Sound);
					i += FoundWord->Word.Len() - 1;
				}
				else if (RandomSounds.Num() > 0)
					OutSounds.Add(RandomSounds[FMath::RandRange(0, RandomSounds.Num() - 1)]);
			}
		}
		else if (RandomSounds.Num() > 0)
		{
			for (int i = 0; i < InMessage.Len() + 1; ++i)
			{
				OutSoundIndices.Add(i);
				OutSounds.Add(RandomSounds[FMath::RandRange(0, RandomSounds.Num() - 1)]);
			}
		}
	}
	else
	{
		// TODO maybe move this out of the soundset and into the AI stuff? This won't be used by any players so it has no business being here.

		TArray<int> KeysCopy;
		ManualIndices.GenerateKeyArray(KeysCopy);
		KeysCopy.Sort();

		for (int& currInd : KeysCopy)
		{
			OutSoundIndices.Add(currInd);
			OutSounds.Add(ManualIndices[currInd]);
		}
	}
}


void USpeechBubble::NativeConstruct()
{
	Super::NativeConstruct();

	InitAnimationPieces();

	FSpeechBubbleAnim& Anim = RelevantAnims.Arriving;

	EUMGSequencePlayMode::Type PlayMode = (Anim.PlayForward ? EUMGSequencePlayMode::Forward : EUMGSequencePlayMode::Reverse);

	PlayAnimation(Anim.Animation, 0.0f, 1, PlayMode);

	if (FinishEntryBeforeSettingText)
		Anim.Animation->OnAnimationFinished.AddDynamic(this, &USpeechBubble::OnEntryAnimFinished);

	SetAlignmentInViewport(ViewportStartingPos);

	HandlePosAndScale();

	FTimerHandle dummy;
	GetWorld()->GetTimerManager().SetTimer(dummy, this, &USpeechBubble::HandlePosAndScale, 0.01f, true);
}

void USpeechBubble::OnEntryAnimFinished()
{
	RelevantAnims.Arriving.Animation->OnAnimationFinished.RemoveDynamic(this, &USpeechBubble::OnEntryAnimFinished);

	SetMessage(DelayedMessage, DelayedTimeout);
}

void USpeechBubble::HandlePosAndScale_Implementation()
{
	FVector OwnerLoc = OwningActor->GetActorLocation();

	FVector2D ViewportPos;
	if (IsCutsceneBubble)
		ViewportPos = UCutsceneFuncLib::GetGlidedViewportPos(this, Leaving);
	else
	{
		float BubbleDist = FMath::Clamp((OwnerLoc - RelevantPlayer->GetActorLocation()).Size() / ScalingDistFactor, 0.0f, 1.0f);

		SetRenderScale({ 1 - BubbleDist, 1 - BubbleDist });

		SetRenderTranslation({ 0, BubbleDist * ScalingDip });

		APlayerController* RelevantController = UGameplayStatics::GetPlayerController(OwningActor, 0);
		if (!UGameplayStatics::ProjectWorldToScreen(RelevantController, OwningActor->GetActorLocation() + AnchorPoint, ViewportPos))
			SetVisibility(ESlateVisibility::Collapsed);
		else if (Visibility == ESlateVisibility::Collapsed)
			SetVisibility(ESlateVisibility::Visible);
	}

	SetPositionInViewport(ViewportPos);

}

void USpeechBubble::PopulateWords()
{
	if (nullptr == LetterClass)
	{
		UStaticFuncLib::Print("USpeechBubble::PopulateWords: LetterClass was null!", true);
		return;
	}

	if (nullptr == WordClass && GroupWords)
	{
		UStaticFuncLib::Print("USpeechBubble::PopulateWords: WordClass was null!", true);
		return;
	}

	// Populating the wordbox
	{
		USpeechBubbleWord* CurrentWord = nullptr;
		if (GroupWords)
		{
			CurrentWord = CreateWidget<USpeechBubbleWord>(UStaticFuncLib::RetrieveGameInstance(this), WordClass);
			WordBox->AddChild(CurrentWord);
		}

		for (auto iter = CurrentMessage.Message.CreateConstIterator(); iter && *iter; ++iter)
		{
			USpeechBubbleLetter* NewLetter = CreateWidget<USpeechBubbleLetter>(UStaticFuncLib::RetrieveGameInstance(this), LetterClass);

			NewLetter->PreInit();

			NewLetter->LetterIndex = iter.GetIndex();

			if (nullptr != NewLetter->TextBlock)
				NewLetter->TextBlock->SetText(FText::FromString(FString(1, &*iter)));

			if (nullptr == CurrentWord)
				WordBox->AddChild(NewLetter);
			else
			{
				TArray<UUserWidget*> Dummy;
				Dummy.Add(NewLetter);
				CurrentWord->AddLetters(Dummy);
			}

			if (IsTypewriting)
				NewLetter->SetVisibility(ESlateVisibility::Hidden);

			if (*iter == TEXT(' ') && GroupWords)
			{
				CurrentWord = CreateWidget<USpeechBubbleWord>(UStaticFuncLib::RetrieveGameInstance(this), WordClass);
				WordBox->AddChild(CurrentWord);
			}

			TWSpawnedLetters.Add(NewLetter);
		}
	}

	TWIndex = 0;
	TWRollingTime = 0;

	if (IsTypewriting)
	{
		if (nullptr != TWSoundSet)
			TWSoundSet.GetDefaultObject()->Annotate(CurrentMessage.Message, TWSoundInds, TWSounds);

		GetWorld()->GetTimerManager().SetTimer(TWTimerHandle, this, &USpeechBubble::DoTypewrite, TypeDelay, true);
	}
	else
	{
		for (; TWIndex < TWSpawnedLetters.Num(); ++TWIndex)
		{
			while (CurrentMessage.SpeechTags.Num() > 0 && TWIndex >= CurrentMessage.SpeechTags[0].Index)
			{
				if (CurrentMessage.SpeechTags[0].Removing)
					TWTagStack.Pop();
				else
					TWTagStack.Push(CurrentMessage.SpeechTags[0]);

				CurrentMessage.SpeechTags.RemoveAt(0);
			}

			TWSpawnedLetters[TWIndex]->Init(TWTagStack, TWRollingTime);
		}
	}
}

void USpeechBubble::SetMessage_Implementation(const FSpeechParsedMessage& Message, float TimeoutTime)
{
	WaitTags.Empty();
	TWSpawnedLetters.Empty();
	FString PrevMessage = CurrentMessage.Message;
	CurrentMessage = Message;

	HandleMetadata(Message);

	if (TimeoutTime > FLT_EPSILON)
		GetWorld()->GetTimerManager().SetTimer(TimeoutHandle, this, &USpeechBubble::Timeout, TimeoutTime, false);

	if (!PrevMessage.IsEmpty())
	{
		FSpeechBubbleAnim& Anim = RelevantAnims.Updating;

		if (ClearTextBeforeUpdateAnim)
		{
			WordBox->ClearChildren();
			PopulateWords();
		}
		else if (nullptr != Anim.Animation)
			Anim.Animation->OnAnimationFinished.AddDynamic(this, &USpeechBubble::OnUpdateAnimFinished);

		EUMGSequencePlayMode::Type PlayMode = (Anim.PlayForward ? EUMGSequencePlayMode::Forward : EUMGSequencePlayMode::Reverse);

		PlayAnimation(Anim.Animation, 0.0f, 1, PlayMode);
	}
	else
		PopulateWords();
}

void USpeechBubble::DoTypewrite()
{
	TWRollingTime += TypeDelay;

	if (WaitTags.Num() > 0 && TWIndex >= WaitTags[0].Index)
	{
		TArray<FString> ParsedArgs;
		WaitTags[0].TagArgs.ParseIntoArray(ParsedArgs, &FString(" ")[0], false);

		if (ParsedArgs.Num() > 1 && ParsedArgs[1].IsNumeric())
		{
			FTimerManager& WorldTimerManager = GetWorld()->GetTimerManager();
			WorldTimerManager.ClearTimer(TWTimerHandle);
			WorldTimerManager.SetTimer(TWTimerHandle, this, &USpeechBubble::DoWait, FCString::Atof(*ParsedArgs[1]));
		}
		return;
	}

	if (TWIndex >= TWSpawnedLetters.Num())
	{
		GetWorld()->GetTimerManager().ClearTimer(TWTimerHandle);
		return;
	}

	while (CurrentMessage.SpeechTags.Num() > 0 && TWIndex >= CurrentMessage.SpeechTags[0].Index)
	{
		if (CurrentMessage.SpeechTags[0].Removing)
			TWTagStack.Pop();
		else
			TWTagStack.Push(CurrentMessage.SpeechTags[0]);

		CurrentMessage.SpeechTags.RemoveAt(0);
	}

	if(TWSoundInds.Num() > 0 && TWIndex >= TWSoundInds[0])
	{
		// TODO attenuation stuff
		UStaticFuncLib::PlaySound(this, OwningActor->GetActorTransform(), TWSounds[0], 1.0f);

		TWSoundInds.RemoveAt(0);
		TWSounds.RemoveAt(0);
	}

	TWSpawnedLetters[TWIndex]->Init(TWTagStack, TWRollingTime);
	TWSpawnedLetters[TWIndex]->SetVisibility(ESlateVisibility::Visible);

	++TWIndex;
}

void USpeechBubble::DoWait()
{
	if (WaitTags.Num() > 0 && TWIndex >= WaitTags[0].Index)
	{
		TArray<FString> ParsedArgs;
		WaitTags[0].TagArgs.ParseIntoArray(ParsedArgs, &FString(" ")[0], false);

		if (ParsedArgs.Num() > 1 && ParsedArgs[1].IsNumeric())
			TWRollingTime += FCString::Atof(*ParsedArgs[1]);

		if (ParsedArgs.Num() > 2 && ParsedArgs[2].IsNumeric())
			TypeDelay = FCString::Atof(*ParsedArgs[2]);

		GetWorld()->GetTimerManager().SetTimer(TWTimerHandle, this, &USpeechBubble::DoTypewrite, TypeDelay, true);

		WaitTags.RemoveAt(0);
	}
}

void USpeechBubble::OnUpdateAnimFinished()
{
	FSpeechBubbleAnim& Anim = RelevantAnims.Updating;

	if (nullptr == Anim.Animation)
		return;

	Anim.Animation->OnAnimationFinished.RemoveDynamic(this, &USpeechBubble::OnUpdateAnimFinished);

	WordBox->ClearChildren();

	EUMGSequencePlayMode::Type PlayMode = (Anim.PlayForward ? EUMGSequencePlayMode::Reverse : EUMGSequencePlayMode::Forward);

	PlayAnimation(Anim.Animation, 1.0f, 1, PlayMode);

	PopulateWords();
}

void USpeechBubble::Timeout()
{
	TimedOut = true;

	GetWorld()->GetTimerManager().ClearTimer(TimeoutHandle);

	FSpeechBubbleAnim& Anim = RelevantAnims.Leaving;

	EUMGSequencePlayMode::Type PlayMode = (Anim.PlayForward ? EUMGSequencePlayMode::Forward : EUMGSequencePlayMode::Reverse);

	PlayAnimation(Anim.Animation, 0.0f, 1, PlayMode);

	Anim.Animation->OnAnimationFinished.AddDynamic(this, &USpeechBubble::OnLeaveAnimFinished);


	ASpeechManager* SpeechManager = UCutsceneFuncLib::GetSpeechManager(this);

	if (nullptr != SpeechManager)
		SpeechManager->ExistingBubbles.Remove(OwningActor);
}

void USpeechBubble::OnLeaveAnimFinished()
{
	RemoveFromParent();
}


ASpeechManager::ASpeechManager(const FObjectInitializer& _init)
{
	PrimaryActorTick.bCanEverTick = true;

	bReplicates = true;

	RootComponent = _init.CreateDefaultSubobject<USceneComponent>(this, TEXT("RootComponent"));
}

void ASpeechManager::BeginPlay()
{
	Super::BeginPlay();

}

void ASpeechManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

FSpeechParsedMessage ASpeechManager::ParseTags(const FString& RawMessage)
{
	FSpeechParsedMessage OutParsed;
	TArray<int> OpenTagInds;
	for (auto iter = RawMessage.CreateConstIterator(); iter; ++iter)
	{
		if (*iter == TEXT('['))
		{
			auto furtherIter = iter;

			FString TagType, TagArgs;
			bool WritingType = true;
			while (++furtherIter && *furtherIter != TEXT(':') && *furtherIter != TEXT(']'))
			{
				if (*furtherIter == TEXT('/') || *furtherIter == TEXT('\\'))
					WritingType = false;
				else if (WritingType)
					TagType += *furtherIter;
				else
					TagArgs += *furtherIter;
			}

			if (furtherIter)
			{
				int TagInd = OutParsed.Message.Len();

				if (TagType == "MD")
					OutParsed.MetadataTags.Add({ TagInd, false, *TagType, TagArgs });
				else
				{
					OutParsed.SpeechTags.Add({ TagInd, false, *TagType, TagArgs });
					OpenTagInds.Push(OutParsed.SpeechTags.Num() - 1);
				}

				while (++iter != furtherIter);

				continue;
			}
		}
		else if (*iter == TEXT(']') && OpenTagInds.Num() > 0)
		{
			FSpeechTag NewTag = OutParsed.SpeechTags[OpenTagInds.Pop()];
			NewTag.Index = OutParsed.Message.Len();
			NewTag.Removing = true;

			OutParsed.SpeechTags.Add(NewTag);

			continue;
		}

		OutParsed.Message += *iter;
	}

	return OutParsed;
}

void ASpeechManager::StartCutscene_Implementation(const TArray<APlayerController*>& AffectedPlayers)
{
	APlayerController* FocusController = UGameplayStatics::GetPlayerController(this, 0);
	if (!UKismetSystemLibrary::IsDedicatedServer(this) && AffectedPlayers.Contains(FocusController))
	{
		InCutscene = true;

		StopPlayer(FocusController->GetPawn());
	}
}

void ASpeechManager::SpawnBars_Implementation(const TArray<APlayerController*>& AffectedPlayers)
{
	if (!UKismetSystemLibrary::IsDedicatedServer(this) && AffectedPlayers.Contains(UGameplayStatics::GetPlayerController(this, 0)))
	{
		if (nullptr != CurrentBars)
			CurrentBars->RemoveFromParent();

		CurrentBars = CreateWidget<UCutsceneBars>(UStaticFuncLib::RetrieveGameInstance(this), CutsceneBarsType);

		CurrentBars->AddToViewport();
	}
}

void ASpeechManager::MakeNewBubble_Implementation(AActor* Speaker, const FSpeechParsedMessage& Message, float TimeoutTime)
{
	USpeechBubble* Found = ExistingBubbles.FindRef(Speaker);

	if (nullptr == Found)
	{
		if (InCutscene)
			NextBubbleType = SpeechBubbleTypes["Default"].Cutscene;
		else
			NextBubbleType = SpeechBubbleTypes["Default"].Normal;

		HandleMetadata(Message);

		USpeechBubble* NewBubble = CreateWidget<USpeechBubble>(UStaticFuncLib::RetrieveGameInstance(this), NextBubbleType);

		NewBubble->PreInit();

		if (NewBubble->FinishEntryBeforeSettingText)
		{
			NewBubble->DelayedMessage = Message;
			NewBubble->DelayedTimeout = TimeoutTime;
		}
		else
			NewBubble->SetMessage(Message, TimeoutTime);

		NewBubble->OwningActor = Speaker;
		NewBubble->RelevantPlayer = UGameplayStatics::GetPlayerPawn(this, 0);

		NewBubble->AddToViewport();

		ExistingBubbles.Add(Speaker, NewBubble);

	}
	else
		Found->SetMessage(Message, TimeoutTime);
}

void ASpeechManager::PassToPlayerChats_Implementation(AActor* Sender, const TArray<FString>& Receivers, const FSpeechParsedMessage& Message)
{
	if (!UKismetSystemLibrary::IsDedicatedServer(this))
	{
		FSpeechParsedMessage FinalMessage = Message;

		FString SpeakerPrepend = "{" + GetSpeakerName(Sender) + "} ";

		FinalMessage.Message = SpeakerPrepend + FinalMessage.Message;

		for (FSpeechTag& currTag : FinalMessage.SpeechTags)
			currTag.Index += SpeakerPrepend.Len();

		APlayerController* RelevantController = UGameplayStatics::GetPlayerController(this, 0);
		if (Receivers.Num() < 1 || Receivers.Contains(GetSpeakerName(RelevantController)))
		{
			AMPHorsoPlayerController* CastedRelevant = Cast<AMPHorsoPlayerController>(RelevantController);
			
			if (nullptr != CastedRelevant)
				CastedRelevant->ReceiveChatMessage(FinalMessage);
		}
	}
}

void ASpeechManager::DestroyBubble_Implementation(AActor* Speaker, bool Immediate)
{
	USpeechBubble* Found = ExistingBubbles.FindRef(Speaker);

	if (nullptr == Found)
	{
		if (Immediate)
			Found->RemoveFromParent();
		else
			Found->Timeout();

		ExistingBubbles.Remove(Speaker);
	}
}

void ASpeechManager::KillBars_Implementation(const TArray<APlayerController*>& AffectedPlayers)
{
	if (!UKismetSystemLibrary::IsDedicatedServer(this) && AffectedPlayers.Contains(UGameplayStatics::GetPlayerController(this, 0)))
	{
		if (nullptr != CurrentBars)
		{
			CurrentBars->Die();
			CurrentBars = nullptr;
		}
	}
}

void ASpeechManager::EndCutscene_Implementation(const TArray<APlayerController*>& AffectedPlayers)
{
	APlayerController* FocusController = UGameplayStatics::GetPlayerController(this, 0);
	if (!UKismetSystemLibrary::IsDedicatedServer(this) && AffectedPlayers.Contains(FocusController))
	{
		UnstopPlayer(FocusController->GetPawn());

		InCutscene = false;
	}
}


ASpeechManager* UCutsceneFuncLib::GetSpeechManager(UObject* WorldContext)
{
	for (TActorIterator<ASpeechManager> Iter(WorldContext->GetWorld()); Iter; ++Iter)
	{
		if (!(*Iter)->IsPendingKill())
			return *Iter;
	}

	return nullptr;
}

void UCutsceneFuncLib::StartCutscene(TArray<APlayerController*> AffectedPlayers)
{
	if (AffectedPlayers.Num() > 0 && UKismetSystemLibrary::IsServer(AffectedPlayers[0]))
	{
		ASpeechManager* SpeechManager = GetSpeechManager(AffectedPlayers[0]);

		if (nullptr != SpeechManager)
			SpeechManager->StartCutscene(AffectedPlayers);
		else
			UStaticFuncLib::Print("UCutsceneFuncLib::StartCutscene: Couldn't get the Speech Manager!", true);
	}
}

void UCutsceneFuncLib::SpawnBars(TArray<APlayerController*> AffectedPlayers)
{
	if (AffectedPlayers.Num() > 0 && UKismetSystemLibrary::IsServer(AffectedPlayers[0]))
	{
		ASpeechManager* SpeechManager = GetSpeechManager(AffectedPlayers[0]);

		if (nullptr != SpeechManager)
			SpeechManager->SpawnBars(AffectedPlayers);
		else
			UStaticFuncLib::Print("UCutsceneFuncLib::SpawnBars: Couldn't get the Speech Manager!", true);
	}
}

void UCutsceneFuncLib::Say(AActor* Speaker, FString Message, float TimeoutTime)
{
	if (UKismetSystemLibrary::IsServer(Speaker))
	{
		ASpeechManager* SpeechManager = GetSpeechManager(Speaker);

		if (nullptr != SpeechManager)
		{
			bool PassToBubble = true;
			TArray<FString> Recipients;

			if (Message.RemoveFromStart("/"))
				SpeechManager->HandleCommand(Speaker, Message, Message, Recipients, PassToBubble);

			if (!Message.IsEmpty())
			{
				FSpeechParsedMessage Parsed = SpeechManager->ParseTags(Message);

				if (PassToBubble)
					SpeechManager->MakeNewBubble(Speaker, Parsed, TimeoutTime);

				if (SpeechManager->SpeakerIsAPlayer(Speaker))
					SpeechManager->PassToPlayerChats(Speaker, Recipients, Parsed);
			}
		}
		else
			UStaticFuncLib::Print("UCutsceneFuncLib::Say: Couldn't get the Speech Manager!", true);
	}
}

void UCutsceneFuncLib::SayParsed(AActor* Speaker, FSpeechParsedMessage Message, float TimeoutTime)
{
	if (UKismetSystemLibrary::IsServer(Speaker))
	{
		ASpeechManager* SpeechManager = GetSpeechManager(Speaker);

		if (nullptr != SpeechManager)
		{
			if (!Message.Message.IsEmpty())
				SpeechManager->MakeNewBubble(Speaker, Message, TimeoutTime);
		}
		else
			UStaticFuncLib::Print("UCutsceneFuncLib::Say: Couldn't get the Speech Manager!", true);
	}
}

void UCutsceneFuncLib::Silence(AActor* Speaker, bool Immediate)
{
	if (UKismetSystemLibrary::IsServer(Speaker))
	{
		ASpeechManager* SpeechManager = GetSpeechManager(Speaker);

		if (nullptr != SpeechManager)
			SpeechManager->DestroyBubble(Speaker, Immediate);
		else
			UStaticFuncLib::Print("UCutsceneFuncLib::Silence: Couldn't get the Speech Manager!", true);
	}
}

void UCutsceneFuncLib::KillBars(TArray<APlayerController*> AffectedPlayers)
{
	if (AffectedPlayers.Num() > 0 && UKismetSystemLibrary::IsServer(AffectedPlayers[0]))
	{
		ASpeechManager* SpeechManager = GetSpeechManager(AffectedPlayers[0]);

		if (nullptr != SpeechManager)
			SpeechManager->KillBars(AffectedPlayers);
		else
			UStaticFuncLib::Print("UCutsceneFuncLib::KillBars: Couldn't get the Speech Manager!", true);
	}
}

void UCutsceneFuncLib::EndCutscene(TArray<APlayerController*> AffectedPlayers)
{
	if (AffectedPlayers.Num() > 0 && UKismetSystemLibrary::IsServer(AffectedPlayers[0]))
	{
		ASpeechManager* SpeechManager = GetSpeechManager(AffectedPlayers[0]);

		if (nullptr != SpeechManager)
			SpeechManager->EndCutscene(AffectedPlayers);
		else
			UStaticFuncLib::Print("UCutsceneFuncLib::EndCutscene: Couldn't get the Speech Manager!", true);
	}
}

FVector2D UCutsceneFuncLib::GetGlidedViewportPos(USpeechBubble* TargetWidget, bool Leaving)
{
	if (nullptr != TargetWidget)
	{
		UWidgetAnimation* FocusAnimation;
		if (Leaving)
			FocusAnimation = TargetWidget->RelevantAnims.Leaving.Animation;
		else
			FocusAnimation = TargetWidget->RelevantAnims.Arriving.Animation;

		AActor* OwnActor = TargetWidget->OwningActor;

		float StartTime = FocusAnimation->GetStartTime();
		float Alpha = TargetWidget->GetAnimationCurrentTime(FocusAnimation) - StartTime;
		Alpha /= (FocusAnimation->GetEndTime() - StartTime);


		FVector2D FromPos;
		APlayerController* RelevantController = UGameplayStatics::GetPlayerController(OwnActor, 0);
		UGameplayStatics::ProjectWorldToScreen(RelevantController, OwnActor->GetActorLocation(), FromPos);

		return FMath::Lerp(FromPos, UWidgetLayoutLibrary::GetViewportSize(OwnActor) / 2, Alpha);
	}
	else
		UStaticFuncLib::Print("UCutsceneFuncLib::GetGlidedViewportPos: TargetWidget was null!", true);

	return { 0, 0 };
}

int UCutsceneFuncLib::Roll(int NumDie, int Sidedness)
{
	int RollingRes = 0;

	while (--NumDie >= 0)
		RollingRes += FMath::RandRange(1, Sidedness);

	return RollingRes;
}