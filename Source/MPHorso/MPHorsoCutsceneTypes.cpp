// Fill out your copyright notice in the Description page of Project Settings.

#include "MPHorso.h"
#include "MPHorsoCutsceneTypes.h"

#include "Runtime/UMG/Public/Animation/WidgetAnimation.h"

#include "StaticFuncLib.h"
#include "Kismet/GameplayStatics.h"
#include "Blueprint/WidgetLayoutLibrary.h"


void USpeechBubbleLetter::Init(const TArray<FSpeechTag>& Tags, const FString& Text)
{
	for (const FSpeechTag& curr : Tags)
		HandleTag(curr);

	if (nullptr != TextBlock)
		TextBlock->SetText(FText::FromString(Text));

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
		HandleAnim(curr);
}


void USpeechSoundSet::Annotate(const FString& InMessage, TArray<int>& OutSoundIndices, TArray<USoundCue*>& OutSounds)
{
	OutSoundIndices.Empty();
	OutSounds.Empty();

	// TODO
	//		Most letters matched is the Word Sound that wins. Otherwise if no matches just do a random sound.
	//		The sound index is pretty much the index of the first letter of the 'word' the sound is intended to
	//		play on.
}


void USpeechBubble::NativeConstruct()
{
	Super::NativeConstruct();

	FSpeechBubbleAnim& Anim = RelevantAnimations[(int)ESpeechBubbleAnim::Arriving];

	EUMGSequencePlayMode::Type PlayMode = (Anim.PlayForward ? EUMGSequencePlayMode::Forward : EUMGSequencePlayMode::Reverse);

	PlayAnimation(Anim.Animation, 0.0f, 1, PlayMode);

	SetPositionInViewport(ViewportStartingPos);

	FTimerHandle dummy;
	GetWorld()->GetTimerManager().SetTimer(dummy, this, &USpeechBubble::HandlePosAndScale, 0.01f, true);
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
		UGameplayStatics::ProjectWorldToScreen(RelevantController, OwningActor->GetActorLocation() + AnchorPoint, ViewportPos);
	}

	SetPositionInViewport(ViewportPos);

}

void USpeechBubble::SetMessage_Implementation(/*TODO ARGS*/)
{
	// TODO

	FTimerHandle dummy;
	GetWorld()->GetTimerManager().SetTimer(dummy, this, &USpeechBubble::Timeout, TimeoutDuration, false);

	if (!IsAnimationPlaying(RelevantAnimations[(int)ESpeechBubbleAnim::Arriving].Animation))
	{
		FSpeechBubbleAnim& Anim = RelevantAnimations[(int)ESpeechBubbleAnim::Updating];

		EUMGSequencePlayMode::Type PlayMode = (Anim.PlayForward ? EUMGSequencePlayMode::Forward : EUMGSequencePlayMode::Reverse);

		PlayAnimation(Anim.Animation, 0.0f, 1, PlayMode);
	}
}

void USpeechBubble::Timeout()
{
	// TODO hold up, this isn't net-capable or even cutscene-capable stuff. How do I deal with this?
	TimedOut = true;

	FSpeechBubbleAnim& Anim = RelevantAnimations[(int)ESpeechBubbleAnim::Leaving];

	EUMGSequencePlayMode::Type PlayMode = (Anim.PlayForward ? EUMGSequencePlayMode::Forward : EUMGSequencePlayMode::Reverse);

	PlayAnimation(Anim.Animation, 0.0f, 1, PlayMode);

	Anim.Animation->OnAnimationFinished.AddDynamic(this, &USpeechBubble::OnLeaveAnimFinished);
}

void USpeechBubble::OnLeaveAnimFinished()
{
	// TODO Maybe fire 'died' event and collapse instead of removing from parent?

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

	// TODO?
}

void ASpeechManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// TODO?
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
				int TagInd = 0;

				if (TagType == "MD")
					OutParsed.MetadataTags.Add({ TagInd, false, *TagType, TagArgs });
				else
				{
					TagInd = OutParsed.Message.Len();
					OutParsed.SpeechTags.Add({ TagInd, false, *TagType, TagArgs });
					OpenTagInds.Push(OutParsed.SpeechTags.Num() - 1);
				}

				while (++iter != furtherIter);

				continue;
			}
		}
		else if (*iter == TEXT(']') && OpenTagInds.Num() > 0)
		{
			int RemTagInd = OutParsed.SpeechTags.Add(OutParsed.SpeechTags[OpenTagInds.Pop()]);

			FSpeechTag& RemTag = OutParsed.SpeechTags[RemTagInd];
			RemTag.Index = OutParsed.Message.Len();
			RemTag.Removing = true;

			continue;
		}

		OutParsed.Message += *iter;
	}

	return OutParsed;
}

void ASpeechManager::StartCutscene_Implementation(const TArray<APlayerController*>& AffectedPlayers)
{
	if (!UKismetSystemLibrary::IsDedicatedServer(this) && AffectedPlayers.Contains(UGameplayStatics::GetPlayerController(this, 0)))
	{
		// TODO
	}
}

void ASpeechManager::SpawnBars_Implementation(const TArray<APlayerController*>& AffectedPlayers)
{
	if (!UKismetSystemLibrary::IsDedicatedServer(this) && AffectedPlayers.Contains(UGameplayStatics::GetPlayerController(this, 0)))
	{
		// TODO
		if (nullptr != CurrentBars)
			CurrentBars->RemoveFromParent();

		CurrentBars = CreateWidget<UCutsceneBars>(UStaticFuncLib::RetrieveGameInstance(this), CutsceneBarsType);

		CurrentBars->AddToViewport();
	}
}

void ASpeechManager::MakeNewBubble_Implementation(AActor* Speaker, const FSpeechParsedMessage& Message, float TimeoutTime)
{
	// TODO
}

void ASpeechManager::DestroyBubble_Implementation(AActor* Speaker, bool Immediate)
{
	// TODO
}

void ASpeechManager::KillBars_Implementation(const TArray<APlayerController*>& AffectedPlayers)
{
	if (!UKismetSystemLibrary::IsDedicatedServer(this) && AffectedPlayers.Contains(UGameplayStatics::GetPlayerController(this, 0)))
	{
		// TODO
		if (nullptr != CurrentBars)
		{
			CurrentBars->Die();
			CurrentBars = nullptr;
		}
	}
}

void ASpeechManager::EndCutscene_Implementation(const TArray<APlayerController*>& AffectedPlayers)
{
	if (!UKismetSystemLibrary::IsDedicatedServer(this) && AffectedPlayers.Contains(UGameplayStatics::GetPlayerController(this, 0)))
	{
		// TODO
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
			if (Message.StartsWith("/"))
				SpeechManager->HandleCommand(Speaker, Message, Message);

			if (!Message.IsEmpty())
			{
				FSpeechParsedMessage Parsed = SpeechManager->ParseTags(Message);

				SpeechManager->MakeNewBubble(Speaker, Parsed, TimeoutTime);
			}
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
		UWidgetAnimation* FocusAnimation =
			TargetWidget->RelevantAnimations[(int)(Leaving ? ESpeechBubbleAnim::Leaving : ESpeechBubbleAnim::Arriving)].Animation;
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
