// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

//#include "UObject/NoExportTypes.h"
#include "Components/ActorComponent.h"
#include "RuleTypes.generated.h"

/*
	Outlines for tomorrow:

	QUERY CONSTRUCTION:
		On event:
			1. Event retrieves a copy of the World State Query
			2. Event adds 'Concept' fact (and other facts?) to Query Copy
			3. Event calls NPC's Respond(), passes Query Copy
			4. NPC adds 'Who' fact to Query Copy
			5. NPC adds 'Memories' (multiple facts) to Query Copy
			6. NPC tests Rules with Query to get Response
			7. NPC executes Response.

*/

USTRUCT(BlueprintType)
struct FRuleQuery
{
	GENERATED_USTRUCT_BODY();

	//UPROPERTY(/*BlueprintReadWrite*/)
		TMap<FName, FVariant> Facts;
};

USTRUCT(BlueprintType)
struct FRule
{
	GENERATED_USTRUCT_BODY();

	UPROPERTY(EditAnywhere)
		TArray<FString> Criterion; // Names of criterion functions

	UPROPERTY(EditAnywhere)
		FString Response; // Name of response function
};

UCLASS(Blueprintable)
class MPHORSO_API URuleBlock : public UObject
{
	GENERATED_BODY()
	
public:


	UPROPERTY(EditDefaultsOnly, Category = "Rule System|Rule Block")
		TArray<FRule> Rules;


	/*
		this gets called when the game instance instantiates
		one of these in order to sort the rules list.
	*/
	UFUNCTION(BlueprintNativeEvent, Category = "Rule System|Rule Block")
		void Prep();
	void Prep_Implementation();

	/*
		NOTE:
		This class is *intended* for you to make your own
		functions and then specify their names in the Rules
		Array!

		Remember:
			- Criteria are:
				bool Func(const FRuleQuery& Query);
			- Responses are:
				void Func(URuleComponent* Invoker);
	*/

	/*
		Example Criteria Function!
		
		NOTE: Returns true. Otherwise does nothing.
		Is used for comparison to check if a found
		criteria function is the correct function
		type.
	*/
	UFUNCTION(BlueprintCallable, Category = "Rule System|Rule Block")
		bool ExampleCriteriaFunction(const FRuleQuery& Query) { return false; }

	/*
		Example Response Function!

		NOTE: Does nothing. Is used for
		comparison to check if a found response
		function is the correct function type.
	*/
	UFUNCTION(BlueprintCallable, Category = "Rule System|Rule Block")
		void ExampleResponseFunction(class URuleComponent* Invoker) {}

	/*
		This is basically a criteria for the whole block!
		If this fails the block is completely ignored.
		Use this for things like making certain blocks only
		happen in specific regions or circumstances!
	*/
	UFUNCTION(BlueprintNativeEvent, Category = "Rule System|Rule Block")
		bool BlockIsRelevant(const FRuleQuery& Query);
	bool BlockIsRelevant_Implementation(const FRuleQuery& Query) { return true; }

	UFUNCTION()
		bool Respond(const FRuleQuery& Query, class URuleComponent* Invoker);
	
};

UCLASS(BlueprintType)
class MPHORSO_API URuleComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	URuleComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:

	/*
		The name of the NPC! These should be unique or close to
		unique to avoid weird saving conflicts and such.
	*/
	UPROPERTY(EditAnywhere, Category = "Rule System|Rule Component")
		FName NPC_Name;

	/*
		These are used to ask the game instance for an instantiation
		of the class to use!

		NOTE: These go in order when checking! This was done so that
		you could make certain rule blocks have priority over others
		if you wanted to. If you want a rule from a lower priority
		block to happen, *do not implement it in a higher priority
		block*.
	*/
	UPROPERTY(EditAnywhere, Category = "Rule System|Rule Component")
		TArray<TSubclassOf<URuleBlock>> Rules;

	// Pointers
	UPROPERTY()
		TArray<URuleBlock*> RuleInsts;

	/*
		The memories of this NPC! These get saved for each individual NPC
		as well as are appended to the rule query when Respond() is called.
	*/
	UPROPERTY(BlueprintReadWrite)
		FRuleQuery Memories;
	
	UFUNCTION(BlueprintCallable, Category = "Rule System|Rule Component")
		void Respond(UPARAM(Ref) FRuleQuery& Query);

};

UCLASS()
class MPHORSO_API UStaticRuleLib : public UObject
{
	GENERATED_BODY()
	
public:
	
	/*
		TODO: MAKE FUNCTIONS TO ADD FACTS TO QUERIES FOR ALL RELEVANT TYPES

		i.e.
			-ints
			-floats
			-vectors (?)
			-names
			-etc.

			as well as just regular casts as well for comparisons.

		also make a way to append queries to each other.
	*/

	UFUNCTION(BlueprintCallable, Category = "Rule System|Static Rule Library")
		static void SetBoolInQuery(UPARAM(Ref) FRuleQuery& Query, FName FactName, bool FactValue);

	UFUNCTION(BlueprintCallable, Category = "Rule System|Static Rule Library")
		static void SetIntInQuery(UPARAM(Ref) FRuleQuery& Query, FName FactName, int FactValue);

	UFUNCTION(BlueprintCallable, Category = "Rule System|Static Rule Library")
		static void SetFloatInQuery(UPARAM(Ref) FRuleQuery& Query, FName FactName, float FactValue);

	UFUNCTION(BlueprintCallable, Category = "Rule System|Static Rule Library")
		static void SetNameInQuery(UPARAM(Ref) FRuleQuery& Query, FName FactName, FName FactValue);

	UFUNCTION(BlueprintCallable, Category = "Rule System|Static Rule Library")
		static void SetStringInQuery(UPARAM(Ref) FRuleQuery& Query, FName FactName, FString FactValue);

	UFUNCTION(BlueprintCallable, Category = "Rule System|Static Rule Library")
		static void SetVectorInQuery(UPARAM(Ref) FRuleQuery& Query, FName FactName, FVector FactValue);


	UFUNCTION(BlueprintPure, Category = "Rule System|Static Rule Library")
		static void GetBoolFromQuery(UPARAM(Ref) FRuleQuery& Query, FName FactName, bool& FactExists, bool& FactValue);

	UFUNCTION(BlueprintPure, Category = "Rule System|Static Rule Library")
		static void GetIntFromQuery(UPARAM(Ref) FRuleQuery& Query, FName FactName, bool& FactExists, int& FactValue);

	UFUNCTION(BlueprintPure, Category = "Rule System|Static Rule Library")
		static void GetFloatFromQuery(UPARAM(Ref) FRuleQuery& Query, FName FactName, bool& FactExists, float& FactValue);

	UFUNCTION(BlueprintPure, Category = "Rule System|Static Rule Library")
		static void GetNameFromQuery(UPARAM(Ref) FRuleQuery& Query, FName FactName, bool& FactExists, FName& FactValue);

	UFUNCTION(BlueprintPure, Category = "Rule System|Static Rule Library")
		static void GetStringFromQuery(UPARAM(Ref) FRuleQuery& Query, FName FactName, bool& FactExists, FString& FactValue);

	UFUNCTION(BlueprintPure, Category = "Rule System|Static Rule Library")
		static void GetVectorFromQuery(UPARAM(Ref) FRuleQuery& Query, FName FactName, bool& FactExists, FVector& FactValue);


	UFUNCTION(BlueprintCallable, Category = "Rule System|Static Rule Library")
		static void RemoveFactFromQuery(UPARAM(Ref) FRuleQuery& Query, FName FactName);


	UFUNCTION(BlueprintCallable, Category = "Rule System|Static Rule Library")
		static void JoinQueries(UPARAM(Ref) FRuleQuery& Source, const FRuleQuery& ToAppend);

};