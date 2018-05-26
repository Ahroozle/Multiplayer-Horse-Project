// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "Blueprint/UserWidget.h"
#include "Runtime/UMG/Public/Components/WrapBox.h"
#include "Runtime/UMG/Public/Components/TextBlock.h"
#include "MPHorsoCutsceneTypes.generated.h"


/*
	TODO
		Remember that NPCs should NOt communicate tag data over connection.
		This would cause hilariously large, unnecessary amounts of traffic.
		Instead somehow store NPC dialogue in an array, already parsed, and
		pass around indices into that array.


	Transcribed from board:

	CutsceneStart
		->Lock player movement
		->Black bars (if necessary)

	Cutscene Body
		->Messages/Dialog
		->Camera movements
		->Entity movements

	Cutscene End
		-> Kill all remaining messages
		-> Kill black bars (if necessary)
		-> Unlock player movement

	TODO (Cutscenes and chat systems)
		-Control facial expressions w/emoticons
		-Add party support?


	SPEECH MANAGER TAGS
		[MD/(Name) (Args)]
		Metadata
			-Bubble Type
				[MD/B (BubbleTypeName)]


	BUBBLE TAGS
		[MD/(Name) (Args)]
		Metadata
			-Typewrite
				[MD/TW (SoundSetName) (TypeDelay)]
			-Wait
				[MD/W (Seconds)]


	LETTER TAGS
		[FX/(Name) (Args):(Text)]
		Text Effects, i.e.
			-Anims
				-Shake
				-Roll
			-Emph/Italics

		[P/(Name)]
		Places picture in text.

		[E/(Name)]
		Switches emote. Lasts until next emote change or end of text.
		Only changes in real time if typewriting, else only the last
		emote triggers.

		[C/(Color)]
		Colors text.


	/Group
		Sends only to people specified in the "group targets" tab
		in the pause menu.

	/Whisper <Name/ID>
		Sends only to the specified person

	/Hush <Name/ID>
		"blocks" specified person's messages from appearing in your feed







*/

/*
	TODO
		-Player Controller sends off its own chat stuff serverside
		-Tag args set once at beginning in letter and never need to be parsed again
		-Deal with Sound Sets, somehow (do they go on the *bubble* or the *letter*? probably bubble)
		-Typewrite should probably actually be done at the bubble level, and is therefore distinct from
		 other text effects. How do I do this kind of shit? How can I let the player control it? *Should*
		 I allow players to control it? (probably)
		-Make 'RawSay' function for inputting things that are already baked, i.e. things like
		 'baked' word sounds for NPC dialogue and stuff.
		-Allow player to fast forward through text, i.e. press B to unload it all without making more sound.
		 (maybe add functionality a la paper mario to hold B and every X time either do all text or progress?)
*/


USTRUCT()
struct FSpeechTag
{
	GENERATED_USTRUCT_BODY();

	UPROPERTY(BlueprintReadOnly)
		int Index;

	UPROPERTY(BlueprintReadOnly)
		bool Removing;

	UPROPERTY(BlueprintReadOnly)
		FName TagType;

	UPROPERTY(BlueprintReadOnly)
		FString TagArgs;
};

USTRUCT()
struct FSpeechParsedMessage
{
	GENERATED_USTRUCT_BODY();


	UPROPERTY(BlueprintReadOnly)
		TArray<FSpeechTag> MetadataTags;

	UPROPERTY(BlueprintReadOnly)
		TArray<FSpeechTag> SpeechTags;

	UPROPERTY(BlueprintReadOnly)
		FString Message;
};

UCLASS(Blueprintable, abstract)
class MPHORSO_API USpeechBubbleLetter : public UUserWidget
{
	GENERATED_BODY()

protected:

public:

	UPROPERTY(EditDefaultsOnly)
		UTextBlock* TextBlock;

	UPROPERTY(VisibleDefaultsOnly)
		TArray<FName> Anims;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly)
		int LetterIndex;

	UPROPERTY()
		float AnimTime = 0;

	UFUNCTION()
		void Init(const TArray<FSpeechTag>& Tags, const FString& Text);

	UFUNCTION(BlueprintImplementableEvent)
		void HandleTag(const FSpeechTag& Tag);

	UFUNCTION(BlueprintImplementableEvent)
		void HandleAnim(FName AnimName);

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


UENUM()
enum class ESpeechBubbleAnim : uint8
{
	Arriving,
	Updating,
	Leaving,
	MAX
};

USTRUCT()
struct FSpeechBubbleAnim
{
	GENERATED_USTRUCT_BODY();

	UPROPERTY(EditAnywhere)
		UWidgetAnimation* Animation;

	UPROPERTY(EditAnywhere)
		bool PlayForward = true;

	UPROPERTY(EditAnywhere)
		bool ClearBefore = false;
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

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite)
		UWrapBox* WordBox;

	UPROPERTY(EditDefaultsOnly)
		FSpeechBubbleAnim RelevantAnimations[ESpeechBubbleAnim::MAX];

	UPROPERTY(BlueprintReadOnly)
		bool Leaving = false;

	UPROPERTY(EditDefaultsOnly)
		float TimeoutDuration = 5;

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

	/*
		Should this speech bubble put letters into words first,
		or just directly insert them into the WordBox? This affects
		wrapping.
	*/
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite)
		bool GroupWords = true;

	// The soundset to be used in conjunction with typewriting functionality.
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly)
		TSubclassOf<USpeechSoundSet> TWSoundSet;

	/*
		The delay used for the typewrite effect; A letter appears every this many seconds.
		If this number is <= 0 then no typewrite effect will happen and all letters will
		be visible immediately.
	*/
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly)
		float TypeDelay;


	UFUNCTION(BlueprintNativeEvent)
		void HandlePosAndScale();
	void HandlePosAndScale_Implementation();
	
	UFUNCTION(BlueprintImplementableEvent)
		void HandleMetadata(const FSpeechParsedMessage& Message);

	UFUNCTION(BlueprintNativeEvent) // does this need to be bp-implementable?
		void SetMessage(/*TODO ARGS*/);
	void SetMessage_Implementation(/*TODO ARGS*/);

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
	UPROPERTY()
		TSubclassOf<USpeechBubble> Normal;

	// The type of bubble this is when the player is in a cutscene.
	UPROPERTY()
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


	UPROPERTY(EditDefaultsOnly)
		TSubclassOf<UCutsceneBars> CutsceneBarsType;

	UPROPERTY(EditDefaultsOnly)
		TMap<FName, FBubbleType> SpeechBubbleTypes;

	UPROPERTY(EditDefaultsOnly)
		TSubclassOf<USpeechChoice> SpeechChoiceType;

	UPROPERTY()
		UCutsceneBars* CurrentBars;
	
	UPROPERTY(VisibleAnywhere)
		TMap<AActor*, USpeechBubble*> ExistingBubbles;
	
	UFUNCTION(BlueprintImplementableEvent)
		void HandleCommand(UObject* Caller, const FString& RawCommand, FString& LeftoverMessage);

	UFUNCTION()
		FSpeechParsedMessage ParseTags(const FString& RawMessage);

	UFUNCTION(BlueprintImplementableEvent)
		void HandleMetadata(const FSpeechParsedMessage& Message);

	UFUNCTION(NetMulticast, Reliable)
		void StartCutscene(const TArray<APlayerController*>& AffectedPlayers);
	void StartCutscene_Implementation(const TArray<APlayerController*>& AffectedPlayers);

	UFUNCTION(NetMulticast, Reliable)
		void SpawnBars(const TArray<APlayerController*>& AffectedPlayers);
	void SpawnBars_Implementation(const TArray<APlayerController*>& AffectedPlayers);

	UFUNCTION(NetMulticast, Reliable)
		void MakeNewBubble(AActor* Speaker, const FSpeechParsedMessage& Message, float TimeoutTime = 5.0f);
	void MakeNewBubble_Implementation(AActor* Speaker, const FSpeechParsedMessage& Message, float TimeoutTime = 5.0f);

	UFUNCTION(NetMulticast, Reliable)
		void DestroyBubble(AActor* Speaker, bool Immediate = false);
	void DestroyBubble_Implementation(AActor* Speaker, bool Immediate = false);

	UFUNCTION(NetMulticast, Reliable)
		void KillBars(const TArray<APlayerController*>& AffectedPlayers);
	void KillBars_Implementation(const TArray<APlayerController*>& AffectedPlayers);

	UFUNCTION(NetMulticast, Reliable)
		void EndCutscene(const TArray<APlayerController*>& AffectedPlayers);
	void EndCutscene_Implementation(const TArray<APlayerController*>& AffectedPlayers);

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

		TODO
			-How determine what NPCs may be talking in a cutscene? It can't be just one because that limits the options drastically.
			-How puppeteer NPCs? Just use Ruleset? (probably)
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
