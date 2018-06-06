// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "Blueprint/UserWidget.h"
#include "Runtime/UMG/Public/Components/WrapBox.h"
#include "Runtime/UMG/Public/Components/TextBlock.h"
#include "MPHorsoCutsceneTypes.generated.h"


/*
	TODO
		Remember that NPCs should NOT communicate tag data over connection.
		This would cause hilariously large, unnecessary amounts of traffic.
		Instead a SayParsed function now exists and a multicast RPC can simply
		be called over the connection to call the actual say function locally on
		every client.

	Maybe also pre-process user messages as kind-of discord markdown so that users can do
	stuff like '_this_' or '*this*' for emphasis and ':this:' for pics?

	TODO (Cutscenes and chat systems)
		-Control facial expressions w/emoticons
		-Add party support?


	/Group
		Sends only to people specified in the "group targets" tab
		in the pause menu. (tbh maybe just steal terraria's teams
		except allow them to be maybe any color that doesn't overlap
		within a range of another color?). Does not pass to bubble.

	/Whisper <Name/ID>
		Sends only to the specified person. Does not pass to bubble.

	/Hush <Name/ID>
		"blocks" specified person's messages from appearing in your feed

*/

/*
	TODO
		-Player Controller sends off its own chat stuff serverside
		-Allow player to fast forward through text, i.e. press B to unload it all without making more sound.
		 (maybe add functionality a la paper mario OG to hold B and every X time either do all text or progress?)
*/


USTRUCT(BlueprintType)
struct FSpeechTag
{
	GENERATED_USTRUCT_BODY();

	UPROPERTY(BlueprintReadWrite)
		int Index;

	UPROPERTY(BlueprintReadWrite)
		bool Removing;

	UPROPERTY(BlueprintReadWrite)
		FName TagType;

	UPROPERTY(BlueprintReadWrite)
		FString TagArgs;
};

USTRUCT()
struct FSpeechParsedMessage
{
	GENERATED_USTRUCT_BODY();


	UPROPERTY(BlueprintReadWrite)
		TArray<FSpeechTag> MetadataTags;

	UPROPERTY(BlueprintReadWrite)
		TArray<FSpeechTag> SpeechTags;

	UPROPERTY(BlueprintReadWrite)
		FString Message;
};

UCLASS(Blueprintable, abstract)
class MPHORSO_API USpeechBubbleLetter : public UUserWidget
{
	GENERATED_BODY()

protected:

public:

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		UTextBlock* TextBlock;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite)
		TArray<FName> Anims;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly)
		int LetterIndex = 0;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite)
		float AnimTime = 0;


	UFUNCTION(BlueprintImplementableEvent)
		void PreInit();

	UFUNCTION()
		void Init(const TArray<FSpeechTag>& Tags, float StartTime);

	UFUNCTION(BlueprintImplementableEvent)
		void HandleTag(const FSpeechTag& Tag);

	UFUNCTION(BlueprintImplementableEvent)
		FWidgetTransform HandleAnim(FName AnimName, const FWidgetTransform& InTransform);

	UFUNCTION()
		void TickAnimations();
};

UCLASS(Blueprintable, abstract)
class MPHORSO_API USpeechBubbleWord : public UUserWidget
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintImplementableEvent)
		void AddLetters(const TArray<UUserWidget*>& Letters);

};


USTRUCT()
struct FWordSound
{
	GENERATED_USTRUCT_BODY();

	UPROPERTY(EditAnywhere)
		FString Word;

	UPROPERTY(EditAnywhere)
		USoundCue* Sound;
};

UCLASS(Blueprintable, abstract)
class MPHORSO_API USpeechSoundSet : public UObject
{
	GENERATED_BODY()

public:

	/*
	These sounds are played on letters/strings that are
	not covered by sounds mapped within WordSounds;
	*/
	UPROPERTY(EditDefaultsOnly)
		TArray<USoundCue*> RandomSounds;

	/*
	These sounds are played at the start of the specified
	'word' (just a collection of characters).
	*/
	UPROPERTY(EditDefaultsOnly)
		TArray<FWordSound> WordSounds;

	UFUNCTION()
		void Annotate(const FString& InMessage, TArray<int>& OutSoundIndices, TArray<USoundCue*>& OutSounds);
};


USTRUCT()
struct FSpeechBubbleAnim
{
	GENERATED_USTRUCT_BODY();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		UWidgetAnimation* Animation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		bool PlayForward = true;
};

USTRUCT()
struct FSpeechBubbleAnimBundle
{
	GENERATED_USTRUCT_BODY();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FSpeechBubbleAnim Arriving;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FSpeechBubbleAnim Updating;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FSpeechBubbleAnim Leaving;
};

UCLASS(Blueprintable, abstract)
class MPHORSO_API USpeechBubble : public UUserWidget
{
	GENERATED_BODY()

protected:

	virtual void NativeConstruct() override;

public:

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly)
		AActor* OwningActor;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly)
		AActor* RelevantPlayer;

	UPROPERTY(EditDefaultsOnly)
		TSubclassOf<USpeechBubbleLetter> LetterClass;

	UPROPERTY(EditDefaultsOnly)
		TSubclassOf<USpeechBubbleWord> WordClass;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite)
		UWrapBox* WordBox;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		FSpeechBubbleAnimBundle RelevantAnims;

	UPROPERTY(EditDefaultsOnly)
		bool FinishEntryBeforeSettingText = false;

	UPROPERTY(EditDefaultsOnly)
		bool ClearTextBeforeUpdateAnim = false;

	UPROPERTY(BlueprintReadOnly)
		bool Leaving = false;

	UPROPERTY()
		bool TimedOut = false;

	UPROPERTY(EditDefaultsOnly)
		FVector2D ViewportStartingPos = { 0.5f, 0.5f };

	// Anchor point of the bubble, relative to the owning actor.
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		FVector AnchorPoint = { 0, 0, 150 };

	// The factor governing the bubble's scaling behavior, in unreal units away from the player.
	UPROPERTY(EditDefaultsOnly, meta = (ValueMin = "1.0"))
		float ScalingDistFactor = 5000;

	// How far down does the bubble dip downward to keep vertical position as the speaker moves away?
	UPROPERTY(EditDefaultsOnly)
		float ScalingDip = 50;

	UPROPERTY(EditDefaultsOnly)
		bool IsCutsceneBubble = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		bool IsTypewriting = false;

	// The soundset to be used in conjunction with typewriting functionality.
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite)
		TSubclassOf<USpeechSoundSet> TWSoundSet;

	/*
		The delay used for the typewrite effect; A letter appears every this many seconds.
		If this number is <= 0 then no typewrite effect will happen and all letters will
		be visible immediately.
	*/
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite)
		float TypeDelay = 0.03;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite)
		TArray<FSpeechTag> WaitTags;

	/*
		Should this speech bubble put letters into words first,
		or just directly insert them into the WordBox? This affects
		wrapping.
	*/
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite)
		bool GroupWords = true;

	UPROPERTY(BlueprintReadOnly)
		FSpeechParsedMessage CurrentMessage;

	UPROPERTY(BlueprintReadOnly)
		TArray<int> TWSoundInds;

	UPROPERTY(BlueprintReadOnly)
		TArray<USoundCue*> TWSounds;

	UPROPERTY(BlueprintReadOnly)
		int TWIndex;

	UPROPERTY(BlueprintReadOnly)
		FTimerHandle TWTimerHandle;

	UPROPERTY(BlueprintReadOnly)
		TArray<FSpeechTag> TWTagStack;

	UPROPERTY()
		TArray<USpeechBubbleLetter*> TWSpawnedLetters;

	UPROPERTY()
		float TWRollingTime = 0;

	UPROPERTY()
		FTimerHandle TimeoutHandle;

	UPROPERTY()
		FSpeechParsedMessage DelayedMessage;

	UPROPERTY()
		float DelayedTimeout;


	UFUNCTION(BlueprintImplementableEvent)
		void PreInit();

	UFUNCTION(BlueprintImplementableEvent)
		void InitAnimationPieces();
	
	UFUNCTION()
		void OnEntryAnimFinished();

	UFUNCTION(BlueprintNativeEvent)
		void HandlePosAndScale();
	void HandlePosAndScale_Implementation();
	
	UFUNCTION(BlueprintImplementableEvent)
		void HandleMetadata(const FSpeechParsedMessage& Message);

	UFUNCTION()
		void PopulateWords();

	UFUNCTION(BlueprintNativeEvent)
		void SetMessage(const FSpeechParsedMessage& Message, float TimeoutTime = 5);
	void SetMessage_Implementation(const FSpeechParsedMessage& Message, float TimeoutTime = 5);

	UFUNCTION()
		void DoTypewrite();

	UFUNCTION()
		void DoWait();

	UFUNCTION()
		void OnUpdateAnimFinished();

	UFUNCTION()
		void Timeout();

	UFUNCTION()
		void OnLeaveAnimFinished();
};


UCLASS(Blueprintable, abstract)
class MPHORSO_API UCutsceneBars : public UUserWidget
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintImplementableEvent)
		void Die();
};


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPlayerChoseNotify, const FString&, Chosen);

UCLASS(Blueprintable, abstract)
class MPHORSO_API USpeechChoice : public UUserWidget
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintCallable)
		FPlayerChoseNotify OnPlayerChose;

	UFUNCTION(BlueprintImplementableEvent)
		void SetChoices(const TArray<FString>& Choices);
};


USTRUCT()
struct FBubbleType
{
	GENERATED_USTRUCT_BODY();

	// The type of bubble this is during normal play.
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		TSubclassOf<USpeechBubble> Normal;

	// The type of bubble this is when the player is in a cutscene.
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		TSubclassOf<USpeechBubble> Cutscene;
};

UCLASS(Blueprintable, abstract)
class MPHORSO_API ASpeechManager : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ASpeechManager(const FObjectInitializer& _init);

	//virtual void OnConstruction(const FTransform& Transform) override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;


	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		TSubclassOf<UCutsceneBars> CutsceneBarsType;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		TMap<FName, FBubbleType> SpeechBubbleTypes;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		TSubclassOf<USpeechChoice> SpeechChoiceType;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly)
		UCutsceneBars* CurrentBars;
	
	UPROPERTY(BlueprintReadWrite)
		TSubclassOf<USpeechBubble> NextBubbleType;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		TMap<AActor*, USpeechBubble*> ExistingBubbles;
	
	// Determines if the client this instance of the manager is on is in a cutscene or not
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly)
		bool InCutscene = false;


	UFUNCTION(BlueprintImplementableEvent)
		void HandleCommand(UObject* Caller, const FString& RawCommand, FString& LeftoverMessage,
			TArray<FString>& OutRecipients, bool& OutPassToBubble);

	UFUNCTION()
		FSpeechParsedMessage ParseTags(const FString& RawMessage);

	UFUNCTION(BlueprintImplementableEvent, BlueprintPure, meta = (DisplayName = "Speaker Is A Player"))
		bool SpeakerIsAPlayer(AActor* Speaker);

	UFUNCTION(BlueprintImplementableEvent, BlueprintPure)
		FString GetSpeakerName(AActor* Speaker);

	UFUNCTION(BlueprintImplementableEvent, BlueprintPure)
		bool PlayerIsModerator(AActor* Player);

	UFUNCTION(BlueprintImplementableEvent)
		void HandleMetadata(const FSpeechParsedMessage& Message);

	UFUNCTION(NetMulticast, Reliable)
		void StartCutscene(const TArray<APlayerController*>& AffectedPlayers);
	void StartCutscene_Implementation(const TArray<APlayerController*>& AffectedPlayers);

	UFUNCTION(BlueprintImplementableEvent)
		void StopPlayer(APawn* PlayerPawn);

	UFUNCTION(NetMulticast, Reliable)
		void SpawnBars(const TArray<APlayerController*>& AffectedPlayers);
	void SpawnBars_Implementation(const TArray<APlayerController*>& AffectedPlayers);

	UFUNCTION(NetMulticast, Reliable)
		void MakeNewBubble(AActor* Speaker, const FSpeechParsedMessage& Message, float TimeoutTime = 5.0f);
	void MakeNewBubble_Implementation(AActor* Speaker, const FSpeechParsedMessage& Message, float TimeoutTime = 5.0f);

	UFUNCTION(NetMulticast, Reliable)
		void PassToPlayerChats(AActor* SendingPlayer, const TArray<FString>& Recipients, const FSpeechParsedMessage& Message);
	void PassToPlayerChats_Implementation(AActor* Sender, const TArray<FString>& Receivers, const FSpeechParsedMessage& Message);

	UFUNCTION(NetMulticast, Reliable)
		void DestroyBubble(AActor* Speaker, bool Immediate = false);
	void DestroyBubble_Implementation(AActor* Speaker, bool Immediate = false);

	UFUNCTION(NetMulticast, Reliable)
		void KillBars(const TArray<APlayerController*>& AffectedPlayers);
	void KillBars_Implementation(const TArray<APlayerController*>& AffectedPlayers);

	UFUNCTION(NetMulticast, Reliable)
		void EndCutscene(const TArray<APlayerController*>& AffectedPlayers);
	void EndCutscene_Implementation(const TArray<APlayerController*>& AffectedPlayers);

	UFUNCTION(BlueprintImplementableEvent)
		void UnstopPlayer(APawn* PlayerPawn);

};


UCLASS()
class MPHORSO_API UCutsceneFuncLib : public UObject
{
	GENERATED_BODY()
	
public:

	// Gets the world's speech manager, if it exists.
	UFUNCTION(BlueprintPure, meta = (WorldContext = "WorldContext"))
		static ASpeechManager* GetSpeechManager(UObject* WorldContext);

	/*
		Client-side; specifies that the client is in a cutscene
		and therefore certain bubbles will be cutscene bubbles.
	*/
	UFUNCTION(BlueprintCallable)
		static void StartCutscene(TArray<APlayerController*> AffectedPlayers);
	
	/*
		Client-side; Spawns the cutscene bars.
	*/
	UFUNCTION(BlueprintCallable)
		static void SpawnBars(TArray<APlayerController*> AffectedPlayers);

	/*
		Spawns a speech bubble for the speaker, or updates
		any preexisting one the speaker has, with the given message.

		Spawns the proper bubble based on client settings (i.e. in a
		cutscene or otherwise, tags/commands in the message, etc.)
	*/
	UFUNCTION(BlueprintCallable)
		static void Say(AActor* Speaker, FString Message, float TimeoutTime = 5.0f);

	/*
		Variant of Say() which takes in already-parsed messages for various uses.
	*/
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Say Pre-Parsed"))
		static void SayParsed(AActor* Speaker, FSpeechParsedMessage Message, float TimeoutTime = 5.0f);

	/*
		Kills any speech bubbles the actor currently has, with the option
		to let it animate out or simply cut out abruptly for specific situations.
	*/
	UFUNCTION(BlueprintCallable)
		static void Silence(AActor* Speaker, bool Immediate = false);

	/*
		Client-side; Kills any cutscene bars present.
	*/
	UFUNCTION(BlueprintCallable)
		static void KillBars(TArray<APlayerController*> AffectedPlayers);

	/*
		Client-side; specifies that the client has finished being
		in a cutscene and that speech bubbles spawned afterward
		will spawn as regular bubbles.
	*/
	UFUNCTION(BlueprintCallable)
		static void EndCutscene(TArray<APlayerController*> AffectedPlayers);


	/*
		Intended for use in conjunction with large cutscene bubbles; Glides the
		bubble from the speaker's position, over to the center of the screen, and vice
		versa, during their respective animations.
	*/
	UFUNCTION(BlueprintPure)
		static FVector2D GetGlidedViewportPos(USpeechBubble* TargetWidget, bool Leaving = false);


	// Misc other

	UFUNCTION(BlueprintCallable)
		static int Roll(int NumDie, int Sidedness);
};

/**
 * 
 */
//UCLASS()
//class MPHORSO_API UMPHorsoCutsceneTypes : public UObject
//{
//	GENERATED_BODY()
//	
//	
//	
//	
//};
