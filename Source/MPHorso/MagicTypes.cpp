// Fill out your copyright notice in the Description page of Project Settings.

#include "MPHorso.h"
#include "MagicTypes.h"

#include "StaticFuncLib.h"


TArray<TSubclassOf<USpellArchetype>> UMagicActionsLibrary::SpellArchetypes;

void UMagicActionsLibrary::PopulateSpellArchetypes()
{
	for (TObjectIterator<UClass> iter; iter; ++iter)
	{
		if (iter->IsChildOf(USpellArchetype::StaticClass()) &&
			*iter != USpellArchetype::StaticClass() &&
			!iter->GetName().StartsWith("Skel_"))
		{
			//UStaticFuncLib::Print(iter->GetName(), true);
			SpellArchetypes.Add(*iter);
		}
	}
}

FLinearColor UMagicActionsLibrary::GetSpellColor(const FName& SpellName)
{
	if (SpellArchetypes.Num() < 1)
		PopulateSpellArchetypes();

	for (auto &curr : SpellArchetypes)
	{
		if (curr.GetDefaultObject()->SpellName == SpellName)
			return curr.GetDefaultObject()->SpellColor;
	}

	return FLinearColor::Black;
}

UTexture* UMagicActionsLibrary::GetSpellImage(const FName& SpellName)
{
	if (SpellArchetypes.Num() < 1)
		PopulateSpellArchetypes();

	for (auto &curr : SpellArchetypes)
	{
		if (curr.GetDefaultObject()->SpellName == SpellName)
			return curr.GetDefaultObject()->SpellImage;
	}

	return nullptr;
}

bool UMagicActionsLibrary::GetSpell(const FName& SpellName, TSubclassOf<USpellArchetype>& RetrievedSpell, TSubclassOf<USpellUI>& RetrievedUI)
{
	if (SpellArchetypes.Num() < 1)
		PopulateSpellArchetypes();

	for (auto &curr : SpellArchetypes)
	{
		if (curr.GetDefaultObject()->SpellName == SpellName)
		{
			RetrievedSpell = curr;
			RetrievedUI = curr.GetDefaultObject()->UI;
			return true;
		}
	}

	UStaticFuncLib::Print("UMagicActionsLibrary::GetSpell: Couldn't find Spell \'" + SpellName.ToString() + "\'!\n(Note: Did you make sure to add it to the appropriate MPHorsoGameInstanceBP array?)", true);

	return false;
}

void UMagicActionsLibrary::ExecuteSpell(AActor* Caster, TSubclassOf<USpellArchetype> Spell, USpellUI* Args)
{
	Spell.GetDefaultObject()->Use(Caster, Args);
}
