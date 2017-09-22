// Fill out your copyright notice in the Description page of Project Settings.

#include "MPHorso.h"
#include "RuleTypes.h"

#include "StaticFuncLib.h"

#include "MPHorsoGameInstance.h"


void URuleBlock::Prep_Implementation()
{
	// sort rules from most criterion to least
	Rules.Sort([](const FRule& a, const FRule& b) { return a.Criterion.Num() > b.Criterion.Num(); });
}

bool URuleBlock::Respond(const FRuleQuery& Query, URuleComponent* Invoker)
{
	struct CritParms
	{
		const FRuleQuery& Arg1;
		bool RetVal;
	};

	UFunction* ComparisonCrit = this->FindFunction("ExampleCriteriaFunction");
	UFunction* ComparisonResp = this->FindFunction("ExampleResponseFunction");

	if (!UStaticFuncLib::ValidateObject(ComparisonCrit, "URuleBlock::Respond: Criteria Comparison Function Not Found!", true) ||
		!UStaticFuncLib::ValidateObject(ComparisonResp, "URuleBlock::Respond: Response Comparison Function Not Found!", true))
		return false;

	UFunction* CurrCrit;
	for (auto &currRule : Rules)
	{
		bool CritSuccess = true;
		for (auto &currCriteria : currRule.Criterion)
		{
			CurrCrit = this->FindFunction(*currCriteria);

			if (nullptr != CurrCrit)
			{
				if (CurrCrit->IsSignatureCompatibleWith(ComparisonCrit))
				{
					CritParms parms = { Query, false };
					this->ProcessEvent(CurrCrit, &parms);

					CritSuccess = parms.RetVal;

					if (!CritSuccess)
						break;
				}
				else
					UStaticFuncLib::Print("URuleBlock::Respond: Criteria \'" + currCriteria + "\' has the incorrect function "
										  "signature! Criteria functions must be of type 'bool Func(const FRuleQuery& Query)' "
										  "to be processed.");
			}
			else
				UStaticFuncLib::Print("URuleBlock::Respond: Couldn't find Criteria \'" + currCriteria + "\'!", true);
		}

		if (CritSuccess)
		{
			UFunction* retrievedResponse = this->FindFunction(*currRule.Response);

			if (nullptr != retrievedResponse)
			{
				if (retrievedResponse->IsSignatureCompatibleWith(ComparisonResp))
				{
					this->ProcessEvent(retrievedResponse, Invoker);
				}
				else
					UStaticFuncLib::Print("URuleBlock::Respond: Response \'" + currRule.Response + "\' has the incorrect function "
										  "signature! Response functions must be of type 'void Func(URuleComponent* Invoker)' "
										  "to be processed.");
			}
			else
				UStaticFuncLib::Print("URuleBlock::Respond: Couldn't find Response \'" + currRule.Response + "\'!", true);

			return true;
		}
	}

	return false;
}


URuleComponent::URuleComponent()
{
	// TODO
}

void URuleComponent::BeginPlay()
{
	UMPHorsoGameInstance* gameInst = UStaticFuncLib::RetrieveGameInstance(this);
	if (UStaticFuncLib::ValidateObject(gameInst, "URuleComponent::BeginPlay: Couldn't retrieve the Game Instance!", true))
	{
		for (auto &currToRetrieve : Rules)
			RuleInsts.Add(gameInst->GetRuleBlock(currToRetrieve));
	}
}

void URuleComponent::Respond(UPARAM(Ref) FRuleQuery& Query)
{
	UStaticRuleLib::SetNameInQuery(Query, "Who", NPC_Name);

	UStaticRuleLib::JoinQueries(Query, Memories);

	for (auto *currBlock : RuleInsts)
	{
		if (currBlock->BlockIsRelevant(Query))
		{
			if (currBlock->Respond(Query, this))
				return;
		}
	}
}


void UStaticRuleLib::SetBoolInQuery(UPARAM(Ref) FRuleQuery& Query, FName FactName, bool FactValue)
{
	Query.Facts.Add(FactName, FVariant(FactValue));
}

void UStaticRuleLib::SetIntInQuery(UPARAM(Ref) FRuleQuery& Query, FName FactName, int FactValue)
{
	Query.Facts.Add(FactName, FVariant(FactValue));
}

void UStaticRuleLib::SetFloatInQuery(UPARAM(Ref) FRuleQuery& Query, FName FactName, float FactValue)
{
	Query.Facts.Add(FactName, FVariant(FactValue));
}

void UStaticRuleLib::SetNameInQuery(UPARAM(Ref) FRuleQuery& Query, FName FactName, FName FactValue)
{
	Query.Facts.Add(FactName, FVariant(FactValue));
}

void UStaticRuleLib::SetStringInQuery(UPARAM(Ref) FRuleQuery& Query, FName FactName, FString FactValue)
{
	Query.Facts.Add(FactName, FVariant(FactValue));
}

void UStaticRuleLib::SetVectorInQuery(UPARAM(Ref) FRuleQuery& Query, FName FactName, FVector FactValue)
{
	Query.Facts.Add(FactName, FVariant(FactValue));
}

void UStaticRuleLib::GetBoolFromQuery(UPARAM(Ref) FRuleQuery& Query, FName FactName, bool& FactExists, bool& FactValue)
{
	FVariant* found = Query.Facts.Find(FactName);
	FactExists = nullptr != found;

	if (FactExists)
		FactValue = *found;
	else
		FactValue = false;
}

void UStaticRuleLib::GetIntFromQuery(UPARAM(Ref) FRuleQuery& Query, FName FactName, bool& FactExists, int& FactValue)
{
	FVariant* found = Query.Facts.Find(FactName);
	FactExists = nullptr != found;

	if (FactExists)
		FactValue = *found;
	else
		FactValue = 0;
}

void UStaticRuleLib::GetFloatFromQuery(UPARAM(Ref) FRuleQuery& Query, FName FactName, bool& FactExists, float& FactValue)
{
	FVariant* found = Query.Facts.Find(FactName);
	FactExists = nullptr != found;

	if (FactExists)
		FactValue = *found;
	else
		FactValue = 0.0f;
}

void UStaticRuleLib::GetNameFromQuery(UPARAM(Ref) FRuleQuery& Query, FName FactName, bool& FactExists, FName& FactValue)
{
	FVariant* found = Query.Facts.Find(FactName);
	FactExists = nullptr != found;

	if (FactExists)
		FactValue = *found;
	else
		FactValue = NAME_None;
}

void UStaticRuleLib::GetStringFromQuery(UPARAM(Ref) FRuleQuery& Query, FName FactName, bool& FactExists, FString& FactValue)
{
	FVariant* found = Query.Facts.Find(FactName);
	FactExists = nullptr != found;

	if (FactExists)
		FactValue = FString(*found);
	else
		FactValue = "";
}

void UStaticRuleLib::GetVectorFromQuery(UPARAM(Ref) FRuleQuery& Query, FName FactName, bool& FactExists, FVector& FactValue)
{
	FVariant* found = Query.Facts.Find(FactName);
	FactExists = nullptr != found;

	if (FactExists)
		FactValue = *found;
	else
		FactValue = FVector::ZeroVector;
}

void UStaticRuleLib::RemoveFactFromQuery(UPARAM(Ref) FRuleQuery& Query, FName FactName)
{
	Query.Facts.Remove(FactName);
}

void UStaticRuleLib::JoinQueries(UPARAM(Ref) FRuleQuery& Source, const FRuleQuery& ToAppend)
{
	Source.Facts.Append(ToAppend.Facts);
}
