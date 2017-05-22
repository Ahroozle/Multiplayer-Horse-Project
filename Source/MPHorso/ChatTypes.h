// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Object.h"
#include "ChatTypes.generated.h"


UENUM(BlueprintType)
enum class EChatMessageType : uint8
{
	// User-created chat message
	ChatMessageType_User		UMETA(DisplayName="User-Created Message"),

	// System Message
	ChatMessageType_Sys			UMETA(DisplayName="System Message"),

	// max
	CHATMESSAGETYPE_MAX			UMETA(Hidden)
};

USTRUCT(BlueprintType)
struct FChatMessage
{
	GENERATED_USTRUCT_BODY();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chat Messages")
		FName Sender;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chat Messages")
		EChatMessageType MessageType = EChatMessageType::ChatMessageType_User;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chat Messages")
		FName ChatChannel;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chat Messages")
		FString Message;
};

USTRUCT(BlueprintType)
struct FChatCommand
{
	GENERATED_USTRUCT_BODY();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chat Messages|Commands")
		TSubclassOf<class UChatCommandArchetype> ClassPtr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chat Messages|Commands")
		FString Parameters;
};

UCLASS(Blueprintable)
class MPHORSO_API UChatCommandArchetype : public UObject
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chat Command Archetype")
		TSet<FName> CommandNames; // holds all different acceptable ways to call the command

	// The contents don't really get used in this so it's basically a special comment of sorts
	// only Num() is called ever.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chat Command Archetype")
		TArray<FString> ArgumentNames;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chat Command Archetype")
		bool ShouldMakeSpeechBubble = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chat Command Archetype")
		bool ShouldMakeMessage = true;

	UFUNCTION(BlueprintNativeEvent, Category = "Chat Command Archetype")
		bool Execute(class AMPHorsoPlayerController* Caller, const FString& Args, UPARAM(Ref) FChatMessage& InOutMessage, FString& FailReason);
	bool Execute_Implementation(class AMPHorsoPlayerController* Caller, const FString& Args, UPARAM(Ref) FChatMessage& InOutMessage, FString& FailReason);
};

UCLASS(Blueprintable)
class MPHORSO_API UChatEmoteArchetype : public UObject
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chat Emote Archetype")
		TArray<FString> EmoteNames; // All the ways you can write this particular emote

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chat Emote Archetype")
		class UTexture* Image;
};

UCLASS()
class MPHORSO_API UChatActionsLibrary : public UObject
{
	GENERATED_BODY()

	static TArray<TSubclassOf<UChatCommandArchetype>> CommandArchetypes;
	static TArray<TSubclassOf<UChatEmoteArchetype>> EmoteArchetypes;

	static void PopulateCommandArchetypes();
	static void PopulateEmoteArchetypes();

public:

	UFUNCTION()
		static void TranslateMessage(const FString& InMessage, FChatMessage& OutMessage, FChatCommand& OutCommand);

	UFUNCTION(BlueprintPure)
		static void TimeAsString(FString& OutString);

	UFUNCTION(BlueprintPure)
		static void SplitStringWithEmotes(const FString& In, TArray<FString>& Out);

	UFUNCTION(BlueprintPure)
		static bool IsEmote(const FString& String, class UTexture*& OutEmoteImage);

};

