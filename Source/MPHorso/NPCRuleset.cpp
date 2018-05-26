// Fill out your copyright notice in the Description page of Project Settings.

#include "MPHorso.h"
#include "NPCRuleset.h"

#include "StaticFuncLib.h"
#include "MPHorsoGameInstance.h"

#include "MPHorsoSaveGameTypes.h"


void UNPCRuleBlock::Prep_Implementation()
{
	// sort rules from most criterion to least
	Rules.Sort([](const FNPCRule& a, const FNPCRule& b) { return a.Criterion.Num() > b.Criterion.Num(); });
}

bool UNPCRuleBlock::Respond(const FNPCRelevantState& QueryState, FName InvokerName)
{
	struct CritParms
	{
		const FNPCRelevantState& Arg1;
		bool RetVal;
	};

	UFunction* ComparisonCrit = this->FindFunction("ExampleCriteriaFunction");
	UFunction* ComparisonResp = this->FindFunction("ExampleResponseFunction");

	if (!UStaticFuncLib::ValidateObject(ComparisonCrit, "URuleBlock::Respond: Criteria Comparison Function Not Found!", true) ||
		!UStaticFuncLib::ValidateObject(ComparisonResp, "URuleBlock::Respond: Response Comparison Function Not Found!", true))
		return false;

	UFunction* CurrCrit;
	for (FNPCRule &currRule : Rules)
	{
		bool CritSuccess = true;
		for (FName &currCriteria : currRule.Criterion)
		{
			CurrCrit = this->FindFunction(currCriteria);

			if (nullptr != CurrCrit)
			{
				if (CurrCrit->IsSignatureCompatibleWith(ComparisonCrit))
				{
					CritParms parms = { QueryState, false };
					this->ProcessEvent(CurrCrit, &parms);

					CritSuccess = parms.RetVal;

					if (!CritSuccess)
						break;
				}
				else
					UStaticFuncLib::Print("UNPCRuleBlock::Respond: Criteria \'" + currCriteria.ToString() + "\' has the incorrect function "
						"signature! Criteria functions must be of type 'bool Func(const FNPCRelevantState& QueryState)' "
						"to be processed.");
			}
			else
				UStaticFuncLib::Print("UNPCRuleBlock::Respond: Couldn't find Criteria \'" + currCriteria.ToString() + "\'!", true);
		}

		if (CritSuccess)
		{
			UFunction* retrievedResponse = this->FindFunction(currRule.Response);

			if (nullptr != retrievedResponse)
			{
				if (retrievedResponse->IsSignatureCompatibleWith(ComparisonResp))
				{
					this->ProcessEvent(retrievedResponse, &InvokerName);
				}
				else
					UStaticFuncLib::Print("UNPCRuleBlock::Respond: Response \'" + currRule.Response.ToString() + "\' has the incorrect function "
						"signature! Response functions must be of type 'void Func(FName InvokerName)' "
						"to be processed.");
			}
			else
				UStaticFuncLib::Print("UNPCRuleBlock::Respond: Couldn't find Response \'" + currRule.Response.ToString() + "\'!", true);

			return true;
		}
	}

	return false;
}


void UNPCRuleset::Prep()
{
	for (TSubclassOf<UNPCRuleBlock> &curr : RuleBlocks)
		RuleBlockInsts.AddUnique(UNPCRuleFuncLib::GetRuleBlock(this, curr));
}

bool UNPCRuleset::Respond(const FNPCRelevantState& QuerySpecificState, FName InvokerName)
{
	// NOTE: The Query-Specific state should contain facts like the 'Concept' and general other things involved in just the current event
	//		 that is in need of a response.

	FNPCRelevantStateHandle WorldStateHandle;
	UNPCRuleFuncLib::GetWorldState(this, WorldStateHandle);

	FNPCRelevantState Conglom = *WorldStateHandle.ToState;
	if (nullptr != PersonalityStateHandle.ToState)
		Conglom = UNPCGeneralFuncLib::JoinStates(Conglom, *PersonalityStateHandle.ToState);
	if (nullptr != MemoryStateHandle.ToState)
		Conglom = UNPCGeneralFuncLib::JoinStates(Conglom, *MemoryStateHandle.ToState);
	Conglom = UNPCGeneralFuncLib::JoinStates(Conglom, QuerySpecificState);

	UNPCGeneralFuncLib::AddOrSetNameFact(Conglom, "Who", InvokerName);

	for (UNPCRuleBlock *curr : RuleBlockInsts)
	{
		if (curr->BlockIsRelevant(Conglom))
		{
			if (curr->Respond(Conglom, InvokerName))
				return true;
		}
	}

	return false;
}


UNPCRuleBlock* UNPCRuleFuncLib::GetRuleBlock(UObject* WorldContext, const TSubclassOf<UNPCRuleBlock>& RuleBlockClass)
{
	UMPHorsoGameInstance* gameInst = UStaticFuncLib::RetrieveGameInstance(WorldContext);

	if (nullptr != gameInst)
		return gameInst->GetNPCRuleBlock(RuleBlockClass);
	else
		UStaticFuncLib::Print("UNPCRuleFuncLib::GetRuleBlock: Couldn't retrieve the game instance!", true);

	return nullptr;
}

bool UNPCRuleFuncLib::GetWorldState(UObject* WorldContext, FNPCRelevantStateHandle& WorldStateHandle)
{
	UMPHorsoGameInstance* gameInst = UStaticFuncLib::RetrieveGameInstance(WorldContext);

	if (nullptr != gameInst)
	{
		UWorldSaveBase* WorldSave = gameInst->GetWorldSave();
		if (nullptr != WorldSave)
		{
			WorldStateHandle = { &WorldSave->WorldState };
			return true;
		}
		else
			UStaticFuncLib::Print("UNPCRuleFuncLib::GetNPCWorldState: Couldn't retrieve the current world save file!", true);
	}
	else
		UStaticFuncLib::Print("UNPCRuleFuncLib::GetNPCWorldState: Couldn't retrieve the game instance!", true);

	return false;
}

bool UNPCRuleFuncLib::GetNPCMemories(UObject* WorldContext, FName NPCName, FNPCRelevantStateHandle& RetrievedMemories)
{
	UMPHorsoGameInstance* gameInst = UStaticFuncLib::RetrieveGameInstance(WorldContext);

	if (nullptr != gameInst)
	{
		UWorldSaveBase* WorldSave = gameInst->GetWorldSave();
		if (nullptr != WorldSave)
		{
			FNPC_SaveData* FoundMemories = WorldSave->NPCData.Find(NPCName);
			if (nullptr != FoundMemories)
			{
				RetrievedMemories = { &FoundMemories->Memories };
				return true;
			}
			else
				UStaticFuncLib::Print("UNPCRuleFuncLib::GetNPCMemories: Couldn't find NPC \'" + NPCName.ToString() + "\' \'s memories!", true);
		}
		else
			UStaticFuncLib::Print("UNPCRuleFuncLib::GetNPCMemories: Couldn't retrieve the current world save file!", true);
	}
	else
		UStaticFuncLib::Print("UNPCRuleFuncLib::GetNPCMemories: Couldn't retrieve the game instance!", true);

	return false;
}

bool UNPCRuleFuncLib::GetNPCPersonality(UObject* WorldContext, FName NPCName, FNPCRelevantStateHandle& RetrievedPersonality)
{
	UMPHorsoGameInstance* gameInst = UStaticFuncLib::RetrieveGameInstance(WorldContext);

	if (nullptr != gameInst)
	{
		UWorldSaveBase* WorldSave = gameInst->GetWorldSave();
		if (nullptr != WorldSave)
		{
			FNPC_SaveData* FoundMemories = WorldSave->NPCData.Find(NPCName);
			if (nullptr != FoundMemories)
			{
				RetrievedPersonality = { &FoundMemories->SavedPersonality };
				return true;
			}
			else
				UStaticFuncLib::Print("UNPCRuleFuncLib::GetNPCPersonality: Couldn't find NPC \'" + NPCName.ToString() + "\' \'s personality!", true);
		}
		else
			UStaticFuncLib::Print("UNPCRuleFuncLib::GetNPCPersonality: Couldn't retrieve the current world save file!", true);
	}
	else
		UStaticFuncLib::Print("UNPCRuleFuncLib::GetNPCPersonality: Couldn't retrieve the game instance!", true);

	return false;
}
