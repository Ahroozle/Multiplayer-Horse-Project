// Fill out your copyright notice in the Description page of Project Settings.

#include "MPHorso.h"
#include "MPHorsoGameInstance.h"

#include "SkeletalSpriteAnimation.h"

#include "StaticFuncLib.h"

#include "MPHorsoSaveGameTypes.h"

#include "MPHorsoMusicManager.h"

#include "IPluginManager.h"


void UMPHorsoGameInstance::Init()
{
	GEngine->OnNetworkFailure().AddUObject(this, &UMPHorsoGameInstance::OnNetFail);
	LoadModContent();
}

void UMPHorsoGameInstance::OnNetFail(UWorld *World, UNetDriver *NetDriver, ENetworkFailure::Type FailureType, const FString& ErrorString)
{
	RuntimeErrorList.Add(ErrorString);
}

ERaceType UMPHorsoGameInstance::GetRace()
{
	if (nullptr != CharacterSave)
		return CharacterSave->Race;

	else return ERaceType::Race_EarthP;
}

void UMPHorsoGameInstance::GetColorSchemeAsArrays(TArray<FName>& OutParts, TArray<FLinearColor>& OutColors)
{
	if (nullptr != CharacterSave)
	{
		CharacterSave->ColorScheme.GenerateKeyArray(OutParts);
		CharacterSave->ColorScheme.GenerateValueArray(OutColors);
	}
}

FName UMPHorsoGameInstance::GetLocalPlayerName()
{
	if (nullptr != CharacterSave)
		return *CharacterSave->CharacterName;

	else return FName(EName::NAME_None);
}

URuleBlock* UMPHorsoGameInstance::GetRuleBlock(TSubclassOf<URuleBlock> RuleBlockClass)
{
	URuleBlock* found = InstantiatedRuleBlocks.FindRef(RuleBlockClass);

	if (nullptr == found)
	{
		found = NewObject<URuleBlock>(this, RuleBlockClass);
		found->Prep();
		InstantiatedRuleBlocks.Add(RuleBlockClass, found);
	}

	return found;
}

AMPHorsoMusicManager* UMPHorsoGameInstance::GetMusicManager()
{
	if (!IsValid(SpawnedMusicManager))
	{
		SpawnedMusicManager = GetWorld()->SpawnActor<AMPHorsoMusicManager>(MusicManagerClass, FTransform(FVector::ZeroVector));

		if (!IsValid(SpawnedMusicManager))
			UStaticFuncLib::Print("UMPHorsoGameInstance::GetMusicManager: Failed to spawn the music manager!", true);
	}

	return SpawnedMusicManager;
}

void UMPHorsoGameInstance::LoadModContent()
{
	// IN CASE OF STUPIDITY BREAK COMMENT GLASS
	// IFileManager::Get().FindFilesRecursive(MapFiles, FPaths::GameContentDir(), TEXT(".umap"), true, false);

	TArray<TSharedRef<IPlugin>> FoundPlugs = IPluginManager::Get().GetDiscoveredPlugins();

	for (TSharedRef<IPlugin> &currPlug : FoundPlugs)
	{
		//UStaticFuncLib::Print(currPlug->GetName());
		const FPluginDescriptor& CurrDesc = currPlug->GetDescriptor();
		if (CurrDesc.Category == "User Mod")
		{
			RetrievedMods.Add(*currPlug->GetName(), currPlug);

			if (currPlug->CanContainContent())
			{
				UObjectLibrary* ObjLib = UObjectLibrary::CreateLibrary(UMPHorsoWorldType::StaticClass(), true, true);
				ObjLib->LoadAssetDataFromPath(currPlug->GetMountedAssetPath());

				TArray<FAssetData> RetrievedAssetDatas;
				ObjLib->GetAssetDataList(RetrievedAssetDatas);

				for (FAssetData& currAsset : RetrievedAssetDatas)
				{
					TSubclassOf<UMPHorsoWorldType> AsSubOf(currAsset.GetClass());
					if (nullptr != *AsSubOf)
						WorldTypes.Add(AsSubOf.GetDefaultObject()->WorldTypeName, AsSubOf);
				}
			}
		}
	}
}
