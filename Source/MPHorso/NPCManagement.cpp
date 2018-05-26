// Fill out your copyright notice in the Description page of Project Settings.

#include "MPHorso.h"
#include "NPCManagement.h"

#include "AssetRegistryModule.h"

#include "StaticFuncLib.h"

#include "MPHorsoGameInstance.h"

#include "MPHorsoSaveGameTypes.h"


void UNPCSoul::Prep()
{
	PersonalityState.PrepForUse();
	FNPCRelevantStateHandle ToPersonalityState = { &PersonalityState.InternalState };
	FNPCRelevantStateHandle ToMemoryState = { nullptr };
	UMPHorsoGameInstance* gameInst = UStaticFuncLib::RetrieveGameInstance(this);
	if (nullptr != gameInst)
	{
		UWorldSaveBase* CurrWorldSave = gameInst->GetWorldSave();
		if (nullptr != CurrWorldSave)
		{
			FNPC_SaveData* RelevantSaveData;
			if (nullptr != (RelevantSaveData = CurrWorldSave->NPCData.Find(NPCName)))
			{
				ToMemoryState = { &RelevantSaveData->Memories };

				if (RelevantSaveData->SavedPersonality.Facts.Num() > 0)
				{
					PersonalityState.InternalState.Facts = RelevantSaveData->SavedPersonality.Facts;
					ToPersonalityState = { &PersonalityState.InternalState };
				}
				else
					RelevantSaveData->SavedPersonality = PersonalityState.InternalState;
			}
		}
	}

	RulesetInst = NewObject<UNPCRuleset>(this, RulesetClass);
	if (nullptr != RulesetInst)
	{
		RulesetInst->Prep();
		
		RulesetInst->MemoryStateHandle = ToMemoryState;
		RulesetInst->PersonalityStateHandle = ToPersonalityState;
	}

	VisitorInst = NewObject<UNPCNavVisitor>(this, VisitorClass);
	if (nullptr != VisitorInst)
	{
		VisitorInst->MemoryStateHandle = ToMemoryState;
		VisitorInst->PersonalityStateHandle = ToPersonalityState;
	}
}


// Sets default values for this component's properties
UNPCSoulComponent::UNPCSoulComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}

// Called when the game starts
void UNPCSoulComponent::BeginPlay()
{
	Super::BeginPlay();

	// TODO?
}

// Called every frame
void UNPCSoulComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// TODO?
}

void UNPCSoulComponent::Prep(TSubclassOf<UNPCSoul> SoulClass)
{
	SoulInst = NewObject<UNPCSoul>(this, SoulClass);
	if (nullptr != SoulInst)
	{
		SoulInst->Prep();

		if (nullptr != SoulInst->VisitorInst)
			SoulInst->VisitorInst->OnFinishedTraversal.AddDynamic(this, &UNPCSoulComponent::OnTraversalFinished);
	}
}

void UNPCSoulComponent::OnTraversalFinished(TArray<FName> Path)
{
	// TODO
}


// Sets default values
ANPCAIManager::ANPCAIManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ANPCAIManager::BeginPlay()
{
	Super::BeginPlay();

	UMPHorsoGameInstance* gameInst = UStaticFuncLib::RetrieveGameInstance(this);

	if (nullptr != gameInst)
	{
		FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(FName("AssetRegistry"));
		IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

		TArray<FAssetData> BPAssets;
		gameInst->GetBlueprintAssets(AssetRegistry, BPAssets);

		TArray<UClass*> FoundSouls;
		gameInst->FindContentOfClass(AssetRegistry, BPAssets, UNPCSoul::StaticClass(), FoundSouls);

		FName WorldType = gameInst->GetWorldSave()->WorldSettings.WorldType;

		for (UClass *curr : FoundSouls)
		{
			TSubclassOf<UNPCSoul> castedCurr = curr;

			if (nullptr != castedCurr)
			{
				if (castedCurr.GetDefaultObject()->ValidWorlds.Contains(WorldType))
				{
					FString CompName = "Soul_" + castedCurr.GetDefaultObject()->NPCName.ToString();
					UNPCSoulComponent* NewComp = NewObject<UNPCSoulComponent>(this, *CompName);

					if (nullptr != NewComp)
					{
						NewComp->RegisterComponent();
						NewComp->Prep(castedCurr);
					}
				}
			}
		}
	}
	else
		UStaticFuncLib::Print("ANPCAIManager::BeginPlay: Couldn't retrieve the game instance!", true);
}

// Called every frame
void ANPCAIManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// TODO?
}
