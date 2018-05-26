// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "UObject/NoExportTypes.h"
#include "NPCGeneralTypes.h"
#include "NPCRuleset.generated.h"


USTRUCT(BlueprintType, meta = (DisplayName = "NPC Rule"))
struct FNPCRule
{
	GENERATED_USTRUCT_BODY();

	/*
		Names of all the criterion functions that
		this rule uses for its validity check.
	*/
	UPROPERTY(EditAnywhere)
		TArray<FName> Criterion;

	// Name of the response function this rule calls.
	UPROPERTY(EditAnywhere)
		FName Response;
};

// Timer for NPC to forget facts! Will only ever apply to the NPC's Memory State.
USTRUCT(BlueprintType, meta = (DisplayName = "NPC Forget Timer"))
struct FNPCForgetTimer
{
	GENERATED_USTRUCT_BODY();

	// Name of the target fact
	UPROPERTY(EditAnywhere)
		FName FactName;

	// Type of the target fact
	UPROPERTY(EditAnywhere)
		ENPCFactType FactType;

	/*
		Entry within the target fact to remove.
		Only applies to string array-type facts.
	*/
	UPROPERTY(EditAnywhere)
		FString EntryToRemove;

	/*
		The amount of time left before the NPC
		"forgets" this fact, in seconds.
		If removing from a string array, only
		removes the entry specified.
		Otherwise removes entire fact.
	*/
	UPROPERTY(EditAnywhere)
		float Lifetime;
};

UCLASS(Blueprintable)
class MPHORSO_API UNPCRuleBlock : public UObject
{
	GENERATED_BODY()

public:

	UPROPERTY(EditDefaultsOnly)
		TArray<FNPCRule> Rules;


	/*
		Called upon instantiation in
		order to sort the rules list.
	*/
	UFUNCTION(BlueprintNativeEvent)
		void Prep();
	void Prep_Implementation();
	
	/*
		Example criteria function.

		Returns false, does nothing.

		Is used to compare against criteria
		functions specified by the user in
		order to determine validity.
	*/
	UFUNCTION(BlueprintCallable)
		bool ExampleCriteriaFunction(const FNPCRelevantState& QueryState) { return false; }

	/*
		Example response function.

		Does nothing.

		Is used to compare against response
		functions specified by the user in
		order to determine validity.
	*/
	UFUNCTION(BlueprintCallable)
		void ExampleResponseFunction(FName InvokerName) {}

	/*
		A check for the whole block, before it's used.
		If this is false, the block is skipped.
	*/
	UFUNCTION(BlueprintNativeEvent, Category = "Rule System|Rule Block")
		bool BlockIsRelevant(const FNPCRelevantState& QueryState);
	bool BlockIsRelevant_Implementation(const FNPCRelevantState& QueryState) { return true; }

	UFUNCTION()
		bool Respond(const FNPCRelevantState& QueryState, FName InvokerName);

};

UCLASS(Blueprintable)
class MPHORSO_API UNPCRuleset : public UObject
{
	GENERATED_BODY()

	UPROPERTY()
		TArray<UNPCRuleBlock*> RuleBlockInsts;

public:

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly)
		FNPCRelevantStateHandle MemoryStateHandle;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly)
		FNPCRelevantStateHandle PersonalityStateHandle;

	/*
		Used to request an instantiated version from the game instance.

		These go in order, so put any rule blocks you want to "override"
		other ruleblocks *before* them in the array.
	*/
	UPROPERTY(EditDefaultsOnly)
		TArray<TSubclassOf<UNPCRuleBlock>> RuleBlocks;
	
	/*
		Called before use in order to populate RuleBlockInsts.
	*/
	UFUNCTION()
		void Prep();

	/*
		Gathers all the relevant states and checks the ruleblocks for the
		most appropriate rule and runs its response.

		Returns true if a rule was found, false otherwise.

		Takes the state constructed specifically for this query; the ruleset
		will find all the other required queries itself.
	*/
	UFUNCTION(BlueprintCallable)
		bool Respond(const FNPCRelevantState& QuerySpecificState, FName InvokerName);

};


UCLASS(meta = (DisplayName = "NPC Rule Function Library"))
class MPHORSO_API UNPCRuleFuncLib : public UObject
{
	GENERATED_BODY()

public:

	// Gets or Creates a static Ruleblock
	UFUNCTION(BlueprintPure, meta = (DisplayName = "Get NPC Ruleblock", WorldContext = "WorldContext"))
		static UNPCRuleBlock* GetRuleBlock(UObject* WorldContext, const TSubclassOf<UNPCRuleBlock>& RuleBlockClass);

	// TODO:
	//		-Helper functions to acquire/modify/whatever NPC attributes (i.e. Memories, etc.) with just FName,
	//		 for use while within a response func.

	UFUNCTION(BlueprintPure, meta = (DisplayName = "Get NPC-Relevant World State", WorldContext = "WorldContext"))
		static bool GetWorldState(UObject* WorldContext, FNPCRelevantStateHandle& WorldStateHandle);

	UFUNCTION(BlueprintPure, meta = (DisplayName = "Get NPC Memories", WorldContext = "WorldContext"))
		static bool GetNPCMemories(UObject* WorldContext, FName NPCName, FNPCRelevantStateHandle& RetrievedMemories);

	UFUNCTION(BlueprintPure, meta = (DisplayName = "Get NPC Personality", WorldContext = "WorldContext"))
		static bool GetNPCPersonality(UObject* WorldContext, FName NPCName, FNPCRelevantStateHandle& RetrievedPersonality);

};
