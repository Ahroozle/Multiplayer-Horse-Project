// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Object.h"
#include "Blueprint/UserWidget.h"
#include "ChatTypes.generated.h"

/*
	TODO :
		commands to create:
			/me
				'/me <text>' ==> '(Name) <text>'
				third-person-ifies text, as well as italicizes.

			/p, /party, /team (or whatever)
				'/p <text>'
				limits receivers to party-only
				( this may or may not get impl'd depending on whether or not parties become a thing.
				not such a hot idea to steal *everything* from terraria :^) )

			/players, /playing
				displays list of all players currently present, client-side only of course.

			/roll
				'/roll' - ripped from terraria directly, rolls 1d100
				'/roll <X>d<Y>' - rolls X amount of dice with Y sides and adds the results together

		also could be useful to impl some of these ( http://terraria.gamepedia.com/Server#List_of_console_commands )
		or similar if the need ever arises, as well as make some simple administration functionalities like ops, bans,
		kicks, etc.

		tags to create:
			color, c
				[c/<hex>:<text>] - colors <text> based on the provided <hex>

			emph, e
				[e:<text>] - emphasizes <text>.

			name, n
				[n:<name>] - writes the text as a name. This includes their respective team/party color (if teams/parties are impl'd)

		also add support for nesting tags maybe?

*/

/*
	Represents a collection of commands, but instead
	of storing tangible objects it uses a function to
	determine the effects of each.
*/
UCLASS(Blueprintable, abstract)
class MPHORSO_API UChatCommandBlock : public UObject
{
	GENERATED_BODY()

public:

	/*
		Applies command 'Cmd' called by 'Caller'.

		returns:
			retval: success
			OutMsg: The modified message, if applicable. If this is empty, the message is never sent.
			OutForBubble: The message that should be given to the player's bubble. If this is empty the bubble is never given the message.
			Prop: Should this command's message propagate?
	*/
	UFUNCTION(BlueprintNativeEvent, Category = "Chat Command Block")
		bool Apply(UObject* Caller, const FString& Cmd, FString& OutMsg, FString& OutForBubble, bool& Prop);
	bool Apply_Implementation(UObject* Caller, const FString& Cmd, FString& OutMsg, FString& OutForBubble, bool& Prop) { return false; }
};

UCLASS(Blueprintable, abstract)
class MPHORSO_API UChatMessageAnim : public UObject
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Chat Message Animation")
		void EvaluateAtTime(float Time, const FWidgetTransform& CurrentTransform, FWidgetTransform& Result);
	void EvaluateAtTime_Implementation(float Time, const FWidgetTransform& CurrentTransform, FWidgetTransform& Result) {}
};

UCLASS(Blueprintable, abstract)
class MPHORSO_API UChatWord : public UUserWidget
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Chat Word")
		UImage* AddImage(UTexture* Img);
	UImage* AddImage_Implementation(UTexture* Img) { return nullptr; }

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Chat Word")
		UTextBlock* AddText(const FString& Text);
	UTextBlock* AddText_Implementation(const FString& Text) { return nullptr; }

	/*
		NOTE: the return is a dummy return,
			  it's only here to make sure
			  that the function registers
			  as a function and not an
			  event.
	*/
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Chat Word")
		bool SetOpacity(float NewOpacity);
	bool SetOpacity_Implementation(float NewOpacity) { return false; }

	/*
		NOTE: the return is a dummy return,
		it's only here to make sure
		that the function registers
		as a function and not an
		event.
	*/
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Chat Word")
		bool AddAnimation(TSubclassOf<UChatMessageAnim> Anim);
	bool AddAnimation_Implementation(TSubclassOf<UChatMessageAnim> Anim) { return false; }
};

USTRUCT(BlueprintType)
struct FDeconstructedTagData
{
	GENERATED_USTRUCT_BODY();

	UPROPERTY(BlueprintReadWrite)
		TArray<FName> Tags;
	UPROPERTY(BlueprintReadWrite)
		TArray<FString> TagParams;

	UPROPERTY(BlueprintReadWrite)
		FString Data;

};

/*
	Represents a collection of tags but, just like
	the UChatCommandBlock, stored as a function rather
	than several objects.

	Tag format follows the terraria format ( http://terraria.gamepedia.com/Chat#Tags ) currently for convenience

	i.e.
	[tag:text]
	[tag/options:text]

	or in deconstructed tag terms:
	[tag/params:data]
*/
UCLASS(Blueprintable, abstract)
class MPHORSO_API UChatTagBlock : public UObject
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintNativeEvent, Category = "Chat Tag Block")
		void Apply(UObject* Caller, const TArray<FDeconstructedTagData>& Message, TSubclassOf<UChatWord> WordClass, TArray<UUserWidget*>& ConstructedWords, bool PerLetterWords);
	void Apply_Implementation(UObject* Caller, const TArray<FDeconstructedTagData>& Message, TSubclassOf<UChatWord> WordClass, TArray<UUserWidget*>& ConstructedWords, bool PerLetterWords) {}

};

UCLASS()
class MPHORSO_API UChatActionsLibrary : public UObject
{
	GENERATED_BODY()

public:

	UFUNCTION()
		static bool IsCommand(const FString& Msg) { return Msg.StartsWith("/"); }

	UFUNCTION(BlueprintPure)
		static void TimeAsString(FString& OutString);

	UFUNCTION()
		static void ParseTags(const FString& InMessage, TArray<FDeconstructedTagData>& OutDeconstructed);
	UFUNCTION(BlueprintCallable)
		static void ApplyTags(UObject* Caller, const FString& Message, TArray<UUserWidget*>& ConstructedWords, bool PerLetterWords = true);

	UFUNCTION(BlueprintCallable)
		static int Roll(int NumDie, int Sidedness);
};
