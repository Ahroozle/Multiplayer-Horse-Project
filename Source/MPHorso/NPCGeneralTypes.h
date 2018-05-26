// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "UObject/NoExportTypes.h"

#include "Variant.h"

#include "NPCGeneralTypes.generated.h"


UENUM(BlueprintType, meta = (DisplayName = "NPC-Relevant Fact Type"))
enum class ENPCFactType : uint8
{
	Bool,
	Int,
	Float,
	Name,
	String, // NOTE: accounts for both single strings and arrays of strings. It's up to user to identify which is which.
	Vector,

	NPCFACTTYPE_MAX UMETA(Hidden)
};

USTRUCT(BlueprintType, meta = (DisplayName = "NPC-Relevant Fact"))
struct FNPCFact
{
	GENERATED_USTRUCT_BODY();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		ENPCFactType FactType;

	FVariant Data;
};

USTRUCT(BlueprintType, meta = (DisplayName = "NPC-Relevant State"))
struct FNPCRelevantState
{
	GENERATED_USTRUCT_BODY();

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		TMap<FName, FNPCFact> Facts;
};

USTRUCT(BlueprintType, meta = (DisplayName = "Editor-Friendly NPC-Relevant Fact"))
struct FNPCFactEditor
{
	GENERATED_USTRUCT_BODY();

	UPROPERTY(EditAnywhere)
		ENPCFactType FactType;

	UPROPERTY(EditAnywhere)
		FString FactValue;
};


USTRUCT(BlueprintType, meta = (DisplayName = "Editor-Friendly NPC-Relevant State"))
struct FNPCRelevantStateEditor
{
	GENERATED_USTRUCT_BODY();

	UPROPERTY(EditAnywhere)
		TMap<FName, FNPCFactEditor> Facts;

	UPROPERTY()
		FNPCRelevantState InternalState;

	void PrepForUse(bool EmptyFacts = true);
};

USTRUCT(BlueprintType, meta = (DisplayName = "NPC-Relevant State Handle", HasNativeMake = "MPHorso.UNPCGeneralFuncLib.GetStateHandle", HasNativeBreak = "MPHorso.UNPCGeneralFuncLib.BreakStateHandle"))
struct FNPCRelevantStateHandle
{
	GENERATED_USTRUCT_BODY();

	FNPCRelevantState* ToState;// = nullptr; // okay so don't do the defaulty thingy apparently because it invalidates the initializer lists.
};

/**
 * 
 */
UCLASS(meta = (DisplayName = "NPC General-Types Function Library"))
class MPHORSO_API UNPCGeneralFuncLib : public UObject
{
	GENERATED_BODY()

	static void CheckFactForSet(UPARAM(Ref) FNPCRelevantState& State, const FName& FactName, const ENPCFactType& ExpectedFactType, FString CallingFuncName);

	/*
		Gets an NPC-Relevant Fact from an NPC-Relevant State

		returns whether the fact was found or not.
	*/
	static bool GetFact(const FNPCRelevantState& State, const FName& FactName, const ENPCFactType& ExpectedFactType, FNPCFact& FoundFact, bool& WasExpectedType);

public:

	UFUNCTION(BlueprintCallable)
		static void AddOrSetBoolFact(UPARAM(Ref) FNPCRelevantState& State, FName FactName, bool Value);

	UFUNCTION(BlueprintCallable)
		static void AddOrSetIntFact(UPARAM(Ref) FNPCRelevantState& State, FName FactName, int Value);

	UFUNCTION(BlueprintCallable)
		static void AddOrSetFloatFact(UPARAM(Ref) FNPCRelevantState& State, FName FactName, float Value);

	UFUNCTION(BlueprintCallable)
		static void AddOrSetNameFact(UPARAM(Ref) FNPCRelevantState& State, FName FactName, FName Value);

	UFUNCTION(BlueprintCallable)
		static void AddOrSetStringFact(UPARAM(Ref) FNPCRelevantState& State, FName FactName, FString Value);

	UFUNCTION(BlueprintCallable)
		static void AddOrSetVectorFact(UPARAM(Ref) FNPCRelevantState& State, FName FactName, FVector Value);


	UFUNCTION(BlueprintCallable)
		static void AddOrSetArrayFact(UPARAM(Ref) FNPCRelevantState& State, FName FactName, TArray<FString> Value);

	// Returns false if a type mismatch occurs, otherwise returns true.
	UFUNCTION(BlueprintCallable)
		static bool AddEntryToArrayFact(UPARAM(Ref) FNPCRelevantState& State, FName FactName, FString NewEntry, bool AllowRepeats = false);


	/*
		Get a bool-type NPC-Relevant Fact from an NPC-Relevant State.

		Returns:
			- Retval: true if OutFact is valid, false if otherwise
			- FoundEntry: true if fact was found, false otherwise.
			- OutFact: the returned data of the fact. Invalid if retval is false.
	*/
	UFUNCTION(BlueprintCallable)
		static bool GetBoolFact(const FNPCRelevantState& State, FName FactName, bool& FoundEntry, bool& OutFact);

	/*
		Get an int-type NPC-Relevant Fact from an NPC-Relevant State.

		Returns:
		- Retval: true if OutFact is valid, false if otherwise
		- FoundEntry: true if fact was found, false otherwise.
		- OutFact: the returned data of the fact. Invalid if retval is false.
	*/
	UFUNCTION(BlueprintCallable)
		static bool GetIntFact(const FNPCRelevantState& State, FName FactName, bool& FoundEntry, int& OutFact);

	/*
		Get a float-type NPC-Relevant Fact from an NPC-Relevant State.

		Returns:
		- Retval: true if OutFact is valid, false if otherwise
		- FoundEntry: true if fact was found, false otherwise.
		- OutFact: the returned data of the fact. Invalid if retval is false.
	*/
	UFUNCTION(BlueprintCallable)
		static bool GetFloatFact(const FNPCRelevantState& State, FName FactName, bool& FoundEntry, float& OutFact);

	/*
		Get a name-type NPC-Relevant Fact from an NPC-Relevant State.

		Returns:
		- Retval: true if OutFact is valid, false if otherwise
		- FoundEntry: true if fact was found, false otherwise.
		- OutFact: the returned data of the fact. Invalid if retval is false.
	*/
	UFUNCTION(BlueprintCallable)
		static bool GetNameFact(const FNPCRelevantState& State, FName FactName, bool& FoundEntry, FName& OutFact);

	/*
		Get a string-type NPC-Relevant Fact from an NPC-Relevant State.

		Returns:
		- Retval: true if OutFact is valid, false if otherwise
		- FoundEntry: true if fact was found, false otherwise.
		- OutFact: the returned data of the fact. Invalid if retval is false.
	*/
	UFUNCTION(BlueprintCallable)
		static bool GetStringFact(const FNPCRelevantState& State, FName FactName, bool& FoundEntry, FString& OutFact);

	/*
		Get a vector-type NPC-Relevant Fact from an NPC-Relevant State.

		Returns:
		- Retval: true if OutFact is valid, false if otherwise
		- FoundEntry: true if fact was found, false otherwise.
		- OutFact: the returned data of the fact. Invalid if retval is false.
	*/
	UFUNCTION(BlueprintCallable)
		static bool GetVectorFact(const FNPCRelevantState& State, FName FactName, bool& FoundEntry, FVector& OutFact);


	UFUNCTION(BlueprintCallable)
		static bool GetArrayFact(const FNPCRelevantState& State, FName FactName, bool& FoundEntry, TArray<FString>& OutFact);


	UFUNCTION(BlueprintCallable)
		static void RemoveFact(UPARAM(Ref) FNPCRelevantState& State, FName FactName);

	UFUNCTION(BlueprintCallable)
		static bool RemoveArrayFactEntry(UPARAM(Ref) FNPCRelevantState& State, FName FactName, FString Entry);

	UFUNCTION(BlueprintCallable)
		static FNPCRelevantState JoinStates(const FNPCRelevantState& Base, const FNPCRelevantState& Append);


	UFUNCTION(BlueprintPure, meta = (DisplayName = "Make NPC-Relevant State Handle", NativeMakeFunc))
		static FNPCRelevantStateHandle GetStateHandle(UPARAM(Ref) FNPCRelevantState& State) { return { &State }; }

	UFUNCTION(BlueprintPure, meta = (DisplayName = "Break NPC-Relevant State Handle", NativeBreakFunc))
		static void BreakStateHandle(const FNPCRelevantStateHandle& InHandle, UPARAM(DisplayName = "State (Copy)") FNPCRelevantState& State) { State = *InHandle.ToState; }

	UFUNCTION(BlueprintCallable)
		static void AddOrSetBoolFactFromHandle(UPARAM(Ref) FNPCRelevantStateHandle& Handle, FName FactName, bool Value);

	UFUNCTION(BlueprintCallable)
		static void AddOrSetIntFactFromHandle(UPARAM(Ref) FNPCRelevantStateHandle& Handle, FName FactName, int Value);

	UFUNCTION(BlueprintCallable)
		static void AddOrSetFloatFactFromHandle(UPARAM(Ref) FNPCRelevantStateHandle& Handle, FName FactName, float Value);

	UFUNCTION(BlueprintCallable)
		static void AddOrSetNameFactFromHandle(UPARAM(Ref) FNPCRelevantStateHandle& Handle, FName FactName, FName Value);

	UFUNCTION(BlueprintCallable)
		static void AddOrSetStringFactFromHandle(UPARAM(Ref) FNPCRelevantStateHandle& Handle, FName FactName, FString Value);

	UFUNCTION(BlueprintCallable)
		static void AddOrSetVectorFactFromHandle(UPARAM(Ref) FNPCRelevantStateHandle& Handle, FName FactName, FVector Value);
	

	UFUNCTION(BlueprintPure, meta = (DisplayName = "Get NPC-Relevant State from Editor-Friendly Version"))
		static void GetStateFromEditorFriendly(UPARAM(Ref) FNPCRelevantStateEditor& In, FNPCRelevantState& Out);

};
