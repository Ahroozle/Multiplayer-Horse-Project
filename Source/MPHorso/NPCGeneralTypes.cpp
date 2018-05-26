// Fill out your copyright notice in the Description page of Project Settings.

#include "MPHorso.h"
#include "NPCGeneralTypes.h"

#include "StaticFuncLib.h"


void FNPCRelevantStateEditor::PrepForUse(bool EmptyFacts)
{
	for (auto &currPair : Facts)
	{
		FNPCFact ResultData;
		ResultData.FactType = currPair.Value.FactType;
		switch (currPair.Value.FactType)
		{
		case ENPCFactType::Bool:
			ResultData.Data = (currPair.Value.FactValue == "true");
			break;
		case ENPCFactType::Float:
			if (currPair.Value.FactValue.IsNumeric())
				ResultData.Data = FCString::Atof(*currPair.Value.FactValue);
			else
				UStaticFuncLib::Print("FNPCRelevantStateEditor::PrepForUse: Fact \'" + currPair.Key.ToString() +
									  "\' has type \'Float\' but its data was not numeric! It will be ignored.", true);
			break;
		case ENPCFactType::Int:
			if (currPair.Value.FactValue.IsNumeric())
				ResultData.Data = FCString::Atoi(*currPair.Value.FactValue);
			else
				UStaticFuncLib::Print("FNPCRelevantStateEditor::PrepForUse: Fact \'" + currPair.Key.ToString() +
									  "\' has type \'Int\' but its data was not numeric! It will be ignored.", true);
			break;
		case ENPCFactType::Name:
			ResultData.Data = FName(*currPair.Value.FactValue);
			break;
		case ENPCFactType::String:
			ResultData.Data = currPair.Value.FactValue;
			break;
		case ENPCFactType::Vector:
		{
			FVector Parsed;
			if (Parsed.InitFromString(currPair.Value.FactValue))
				ResultData.Data = Parsed;
			else
				UStaticFuncLib::Print("FNPCRelevantStateEditor::PrepForUse: Fact \'" + currPair.Key.ToString() +
									  "\' has type \'Vector\' but its data was not parsable into vector format! It will be ignored.", true);
		}
		break;
		default:
			UStaticFuncLib::Print("FNPCRelevantStateEditor::PrepForUse: Fact \'" + currPair.Key.ToString() +
								  "\' has an invalid type! It will be ignored.", true);
			break;
		}

		if (!ResultData.Data.IsEmpty())
			InternalState.Facts.Add(currPair.Key, ResultData);
	}

	// Memory-saving measure during play.
	if (EmptyFacts)
		Facts.Empty();
}

void UNPCGeneralFuncLib::CheckFactForSet(UPARAM(Ref) FNPCRelevantState& State, const FName& FactName, const ENPCFactType& ExpectedFactType, FString CallingFuncName)
{
	FNPCFact* Found = State.Facts.Find(FactName);
	if (Found != nullptr)
	{
		if (Found->FactType != ExpectedFactType)
		{
			UEnum* GottenEnum = FindObject<UEnum>(ANY_PACKAGE, TEXT("ENPCFactType"), true);

			if(nullptr!=GottenEnum)
			{
				UStaticFuncLib::Print(CallingFuncName +
									  ": Expected fact \'" + FactName.ToString() + "\' to be of type \'" +
									  GottenEnum->GetNameByValue((int64)ExpectedFactType).ToString() +
									  "\', but it was actually of type \'" +
									  GottenEnum->GetNameByValue((int64)Found->FactType).ToString() +
									  "\'! Are you sure you aren't overwriting a fact you didn't intend to overwrite?");
			}
			else
				UStaticFuncLib::Print("UNPCGeneralFuncLib::CheckFactForSet: Something has gone terribly wrong. Couldn't find ENPCFactType UEnum when called from "
									  "function \'" + CallingFuncName + "\'.");
		}
	}
}

bool UNPCGeneralFuncLib::GetFact(const FNPCRelevantState& State, const FName& FactName, const ENPCFactType& ExpectedFactType, FNPCFact& FoundFact, bool& WasExpectedType)
{
	const FNPCFact* Found;
	if (nullptr == (Found = State.Facts.Find(FactName)))
		return false;

	FoundFact = *Found;
	WasExpectedType = Found->FactType == ExpectedFactType;

	return true;
}

void UNPCGeneralFuncLib::AddOrSetBoolFact(UPARAM(Ref) FNPCRelevantState& State, FName FactName, bool Value)
{
	CheckFactForSet(State, FactName, ENPCFactType::Bool, "UNPCGeneralFuncLib::AddOrSetBoolFact");

	State.Facts.Add(FactName, { ENPCFactType::Bool, FVariant(Value) });
}

void UNPCGeneralFuncLib::AddOrSetIntFact(UPARAM(Ref) FNPCRelevantState& State, FName FactName, int Value)
{
	CheckFactForSet(State, FactName, ENPCFactType::Int, "UNPCGeneralFuncLib::AddOrSetIntFact");

	State.Facts.Add(FactName, { ENPCFactType::Int, FVariant(Value) });
}

void UNPCGeneralFuncLib::AddOrSetFloatFact(UPARAM(Ref) FNPCRelevantState& State, FName FactName, float Value)
{
	CheckFactForSet(State, FactName, ENPCFactType::Float, "UNPCGeneralFuncLib::AddOrSetFloatFact");

	State.Facts.Add(FactName, { ENPCFactType::Float, FVariant(Value) });
}

void UNPCGeneralFuncLib::AddOrSetNameFact(UPARAM(Ref) FNPCRelevantState& State, FName FactName, FName Value)
{
	CheckFactForSet(State, FactName, ENPCFactType::Name, "UNPCGeneralFuncLib::AddOrSetNameFact");

	State.Facts.Add(FactName, { ENPCFactType::Name, FVariant(Value) });
}

void UNPCGeneralFuncLib::AddOrSetStringFact(UPARAM(Ref) FNPCRelevantState& State, FName FactName, FString Value)
{
	CheckFactForSet(State, FactName, ENPCFactType::String, "UNPCGeneralFuncLib::AddOrSetStringFact");

	State.Facts.Add(FactName, { ENPCFactType::String, FVariant(Value) });
}

void UNPCGeneralFuncLib::AddOrSetVectorFact(UPARAM(Ref) FNPCRelevantState& State, FName FactName, FVector Value)
{
	CheckFactForSet(State, FactName, ENPCFactType::Vector, "UNPCGeneralFuncLib::AddOrSetVectorFact");

	State.Facts.Add(FactName, { ENPCFactType::Vector, FVariant(Value) });
}

void UNPCGeneralFuncLib::AddOrSetArrayFact(UPARAM(Ref) FNPCRelevantState& State, FName FactName, TArray<FString> Value)
{
	CheckFactForSet(State, FactName, ENPCFactType::String, "UNPCGeneralFuncLib::AddOrSetArrayFact");

	FString AggregateVal;
	for (FString &curr : Value)
		AggregateVal += curr + ",";

	State.Facts.Add(FactName, { ENPCFactType::String, FVariant(AggregateVal) });
}

bool UNPCGeneralFuncLib::AddEntryToArrayFact(UPARAM(Ref) FNPCRelevantState& State, FName FactName, FString NewEntry, bool AllowRepeats)
{
	FNPCFact* found;
	if (nullptr != (found = State.Facts.Find(FactName)))
	{
		if (found->FactType != ENPCFactType::String)
		{
			UStaticFuncLib::Print("UNPCGeneralFuncLib::AddEntryToArrayFact: Type Mismatch! Fact was not of type String. Doing nothing, "
								  "returning false.", true);
			return false;
		}
		else
		{
			FString RetrievedData = found->Data.GetValue<FString>();
			if (!RetrievedData.Contains(","))
			{
				UStaticFuncLib::Print("UNPCGeneralFuncLib::AddEntryToArrayFact: Type Mismatch! Fact was of type string, but was not "
									  "an array! Doing nothing, returning false.", true);
				return false;
			}
			else
			{
				if (AllowRepeats)
					RetrievedData += NewEntry + ",";
				else
				{
					TArray<FString> Chopped;
					RetrievedData.ParseIntoArray(Chopped, TEXT(","));
					Chopped.AddUnique(NewEntry);

					RetrievedData.Empty();

					for (FString &curr : Chopped)
						RetrievedData += curr + ",";
				}

				State.Facts.Add(FactName, { ENPCFactType::String, FVariant(RetrievedData) });
			}
		}
	}
	else
		State.Facts.Add(FactName, { ENPCFactType::String, FVariant(NewEntry + ",") });

	return true;
}

bool UNPCGeneralFuncLib::GetBoolFact(const FNPCRelevantState& State, FName FactName, bool& FoundEntry, bool& OutFact)
{
	FNPCFact Grabbed;
	bool WasExp;
	FoundEntry = GetFact(State, FactName, ENPCFactType::Bool, Grabbed, WasExp);
	if (FoundEntry)
	{
		if (WasExp)
		{
			OutFact = Grabbed.Data.GetValue<bool>();
			return true;
		}
		else
			UStaticFuncLib::Print("UNPCGeneralFuncLib::GetBoolFact: Found Fact \'" + FactName.ToString() +
								  "\', But it wasn't a bool!");
	}
	else
		UStaticFuncLib::Print("UNPCGeneralFuncLib::GetBoolFact: Couldn't find fact \'" + FactName.ToString() + "\'!");

	return false;
}

bool UNPCGeneralFuncLib::GetIntFact(const FNPCRelevantState& State, FName FactName, bool& FoundEntry, int& OutFact)
{
	FNPCFact Grabbed;
	bool WasExp;
	FoundEntry = GetFact(State, FactName, ENPCFactType::Int, Grabbed, WasExp);
	if (FoundEntry)
	{
		if (WasExp)
		{
			OutFact = Grabbed.Data.GetValue<int>();
			return true;
		}
		else
			UStaticFuncLib::Print("UNPCGeneralFuncLib::GetIntFact: Found Fact \'" + FactName.ToString() +
				"\', But it wasn't an int!");
	}
	else
		UStaticFuncLib::Print("UNPCGeneralFuncLib::GetIntFact: Couldn't find fact \'" + FactName.ToString() + "\'!");

	return false;
}

bool UNPCGeneralFuncLib::GetFloatFact(const FNPCRelevantState& State, FName FactName, bool& FoundEntry, float& OutFact)
{
	FNPCFact Grabbed;
	bool WasExp;
	FoundEntry = GetFact(State, FactName, ENPCFactType::Float, Grabbed, WasExp);
	if (FoundEntry)
	{
		if (WasExp)
		{
			OutFact = Grabbed.Data.GetValue<float>();
			return true;
		}
		else
			UStaticFuncLib::Print("UNPCGeneralFuncLib::GetFloatFact: Found Fact \'" + FactName.ToString() +
				"\', But it wasn't a float!");
	}
	else
		UStaticFuncLib::Print("UNPCGeneralFuncLib::GetFloatFact: Couldn't find fact \'" + FactName.ToString() + "\'!");

	return false;
}

bool UNPCGeneralFuncLib::GetNameFact(const FNPCRelevantState& State, FName FactName, bool& FoundEntry, FName& OutFact)
{
	FNPCFact Grabbed;
	bool WasExp;
	FoundEntry = GetFact(State, FactName, ENPCFactType::Name, Grabbed, WasExp);
	if (FoundEntry)
	{
		if (WasExp)
		{
			OutFact = Grabbed.Data.GetValue<FName>();
			return true;
		}
		else
			UStaticFuncLib::Print("UNPCGeneralFuncLib::GetNameFact: Found Fact \'" + FactName.ToString() +
				"\', But it wasn't a name!");
	}
	else
		UStaticFuncLib::Print("UNPCGeneralFuncLib::GetNameFact: Couldn't find fact \'" + FactName.ToString() + "\'!");

	return false;
}

bool UNPCGeneralFuncLib::GetStringFact(const FNPCRelevantState& State, FName FactName, bool& FoundEntry, FString& OutFact)
{
	FNPCFact Grabbed;
	bool WasExp;
	FoundEntry = GetFact(State, FactName, ENPCFactType::String, Grabbed, WasExp);
	if (FoundEntry)
	{
		if (WasExp)
		{
			OutFact = Grabbed.Data.GetValue<FString>();
			return true;
		}
		else
			UStaticFuncLib::Print("UNPCGeneralFuncLib::GetStringFact: Found Fact \'" + FactName.ToString() +
				"\', But it wasn't a string!");
	}
	else
		UStaticFuncLib::Print("UNPCGeneralFuncLib::GetStringFact: Couldn't find fact \'" + FactName.ToString() + "\'!");

	return false;
}

bool UNPCGeneralFuncLib::GetVectorFact(const FNPCRelevantState& State, FName FactName, bool& FoundEntry, FVector& OutFact)
{
	FNPCFact Grabbed;
	bool WasExp;
	FoundEntry = GetFact(State, FactName, ENPCFactType::Vector, Grabbed, WasExp);
	if (FoundEntry)
	{
		if (WasExp)
		{
			OutFact = Grabbed.Data.GetValue<FVector>();
			return true;
		}
		else
			UStaticFuncLib::Print("UNPCGeneralFuncLib::GetVectorFact: Found Fact \'" + FactName.ToString() +
				"\', But it wasn't a vector!");
	}
	else
		UStaticFuncLib::Print("UNPCGeneralFuncLib::GetVectorFact: Couldn't find fact \'" + FactName.ToString() + "\'!");

	return false;
}

bool UNPCGeneralFuncLib::GetArrayFact(const FNPCRelevantState& State, FName FactName, bool& FoundEntry, TArray<FString>& OutFact)
{
	FNPCFact Grabbed;
	bool WasExp;
	FoundEntry = GetFact(State, FactName, ENPCFactType::String, Grabbed, WasExp);
	if (FoundEntry)
	{
		FString GrabbedData = Grabbed.Data.GetValue<FString>();
		if (WasExp && GrabbedData.Contains(","))
		{
			GrabbedData.ParseIntoArray(OutFact, TEXT(","));
			return true;
		}
		else
			UStaticFuncLib::Print("UNPCGeneralFuncLib::GetArrayFact: Found Fact \'" + FactName.ToString() +
				"\', But it wasn't an array!");
	}
	else
		UStaticFuncLib::Print("UNPCGeneralFuncLib::GetArrayFact: Couldn't find fact \'" + FactName.ToString() + "\'!");

	return false;
}

void UNPCGeneralFuncLib::RemoveFact(UPARAM(Ref) FNPCRelevantState& State, FName FactName)
{
	State.Facts.Remove(FactName);
}

bool UNPCGeneralFuncLib::RemoveArrayFactEntry(UPARAM(Ref) FNPCRelevantState& State, FName FactName, FString Entry)
{
	bool FoundFact;
	TArray<FString> Entries;
	if (GetArrayFact(State, FactName, FoundFact, Entries))
	{
		Entries.Remove(Entry);
		AddOrSetArrayFact(State, FactName, Entries);
		return true;
	}
	else
		UStaticFuncLib::Print("UNPCGeneralFuncLib::RemoveArrayFactEntry: Failed to get fact \'" +
							  FactName.ToString() + "\'.",true);

	return false;
}

FNPCRelevantState UNPCGeneralFuncLib::JoinStates(const FNPCRelevantState& Base, const FNPCRelevantState& Append)
{
	FNPCRelevantState Aggregate = Base;

	Aggregate.Facts.Append(Append.Facts);

	return Aggregate;
}

void UNPCGeneralFuncLib::AddOrSetBoolFactFromHandle(UPARAM(Ref) FNPCRelevantStateHandle& Handle, FName FactName, bool Value)
{
	AddOrSetBoolFact(*Handle.ToState, FactName, Value);
}

void UNPCGeneralFuncLib::AddOrSetIntFactFromHandle(UPARAM(Ref) FNPCRelevantStateHandle& Handle, FName FactName, int Value)
{
	AddOrSetIntFact(*Handle.ToState, FactName, Value);
}

void UNPCGeneralFuncLib::AddOrSetFloatFactFromHandle(UPARAM(Ref) FNPCRelevantStateHandle& Handle, FName FactName, float Value)
{
	AddOrSetFloatFact(*Handle.ToState, FactName, Value);
}

void UNPCGeneralFuncLib::AddOrSetNameFactFromHandle(UPARAM(Ref) FNPCRelevantStateHandle& Handle, FName FactName, FName Value)
{
	AddOrSetNameFact(*Handle.ToState, FactName, Value);
}

void UNPCGeneralFuncLib::AddOrSetStringFactFromHandle(UPARAM(Ref) FNPCRelevantStateHandle& Handle, FName FactName, FString Value)
{
	AddOrSetStringFact(*Handle.ToState, FactName, Value);
}

void UNPCGeneralFuncLib::AddOrSetVectorFactFromHandle(UPARAM(Ref) FNPCRelevantStateHandle& Handle, FName FactName, FVector Value)
{
	AddOrSetVectorFact(*Handle.ToState, FactName, Value);
}

void UNPCGeneralFuncLib::GetStateFromEditorFriendly(UPARAM(Ref) FNPCRelevantStateEditor& In, FNPCRelevantState& Out)
{
	if (In.InternalState.Facts.Num() <= 0)
		In.PrepForUse();

	Out = In.InternalState;
}
