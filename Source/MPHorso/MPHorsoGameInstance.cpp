// Fill out your copyright notice in the Description page of Project Settings.

#include "MPHorso.h"
#include "MPHorsoGameInstance.h"

#include "SkeletalSpriteAnimation.h"

#include "StaticFuncLib.h"

#include "MPHorsoSaveGameTypes.h"
#include "NPCRuleset.h"

#include "MPHorsoMusicManager.h"

#include "NPCNavigation.h"

#include "IPluginManager.h"
#include "AssetRegistryModule.h"
#include "ARFilter.h"

#include "Runtime/PakFile/Public/IPlatformFilePak.h"


void UMPHorsoGameInstance::Init()
{
	GEngine->OnNetworkFailure().AddUObject(this, &UMPHorsoGameInstance::OnNetFail);

	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(FName("AssetRegistry"));
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

	FindAndRegisterMods(AssetRegistry);

	TArray<FAssetData> FoundBlueprintAssets;
	GetBlueprintAssets(AssetRegistry, FoundBlueprintAssets);
	
	TArray<UClass*> CurrResults;
	FindContentOfClass(AssetRegistry, FoundBlueprintAssets, UMPHorsoWorldType::StaticClass(), CurrResults);
	WorldTypes.Empty();
	for (UClass* curr : CurrResults)
	{
		TSubclassOf<UMPHorsoWorldType> currCasted(curr);
		if (nullptr != currCasted)
			WorldTypes.Add(currCasted.GetDefaultObject()->WorldTypeName, currCasted);
	}


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

UNPCRuleBlock* UMPHorsoGameInstance::GetNPCRuleBlock(TSubclassOf<UNPCRuleBlock> RuleBlockClass)
{
	UNPCRuleBlock* found = InstantiatedNPCRuleBlocks.FindRef(RuleBlockClass);

	if (nullptr == found)
	{
		found = NewObject<UNPCRuleBlock>(this, RuleBlockClass);
		found->Prep();
		InstantiatedNPCRuleBlocks.Add(RuleBlockClass, found);
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

ANPCNavManager* UMPHorsoGameInstance::GetNavManager()
{
	if (!IsValid(RetrievedNavManager))
	{
		TArray<AActor*> FoundNavMans;
		UGameplayStatics::GetAllActorsOfClass(this, ANPCNavManager::StaticClass(), FoundNavMans);

		if (FoundNavMans.Num() > 0)
			RetrievedNavManager = Cast<ANPCNavManager>(FoundNavMans[0]);
		else
			UStaticFuncLib::Print("UMPHorsoGameInstance::GetNavManager: No nav manager exists in the world! Remember to add one in and "
								  "bake its World Graph in order to properly support NPC Navigation. Returning null.", true);
	}

	return RetrievedNavManager;
}

void UMPHorsoGameInstance::FindAndRegisterMods(IAssetRegistry& AssetRegistry)
{
	// IN CASE OF STUPIDITY BREAK COMMENT GLASS
	// IFileManager::Get().FindFilesRecursive(MapFiles, FPaths::GameContentDir(), TEXT(".umap"), true, false);

#if WITH_EDITOR
	TArray<TSharedRef<IPlugin>> FoundPlugs = IPluginManager::Get().GetDiscoveredPlugins();

	for (TSharedRef<IPlugin> &currPlug : FoundPlugs)
	{
		//UStaticFuncLib::Print(currPlug->GetName());
		const FPluginDescriptor& CurrDesc = currPlug->GetDescriptor();
		if (CurrDesc.Category == "User Mod" && currPlug->IsEnabled())
		{
			UStaticFuncLib::Print(currPlug->GetName(), true);
			RetrievedMods.Add(*currPlug->GetName(), currPlug);
		}
	}
#else

	// NOTE: Remember, FPakPlatformFile had to be edited for this to happen in the first place!
	// just add 'public:' right after the opening bracket.

	// TO MAKE A MOD:
	//
	// 0) Make a new plugin, or just cannibalize the existing testing mod. I recommend setting its category
	//    to 'User Mod' so that it is detected in-editor for easier testing. Remember to disable the mod
	//    in the plugins menu before building!
	//
	//
	// 1) Add a launch profile for your mod
	//    (window > project launcher, click the '+' at the bottom of the window, NOT the dropdown next to it)
	//
	// 2) Copy http://www.tomlooman.com/add-mod-support-to-your-unreal-engine-4-game/ 's mod profile up until
	//    the packaging step, which you instead set to 'do not package'.
	//
	// 3) Launch the profile. This should go without a hitch, unless you accidentally had it package the game,
	//    in which case it errors at the packaging stage.
	//   
	// 4) Go to <ProjectName>/Plugins/<ModName>/Saved/Cooked/<BuildName>/<ProjectName>/ to find the generated
	//    AssetRegistry.bin.
	//
	// 5) Go to <ProjectName>/Plugins/<ModName>/Saved/StagedBuilds/<BuildName>/<ProjectName>/Content/Paks/ to
	//    find the generated .pak file
	//
	// 6) Rename both your assetregistry.bin and generated .pak to have the same name. This is the name that
	//    the game will detect the mod as, so I would suggest you use the name of your mod.
	//
	// 7) Your mod is now ready for deployment! Keep in mind that both the asset registry .bin and the mod's
	//    .pak will have to be put into the game's Content/Paks folder in order to be properly detected.

	IPlatformFile& InnerPlatform = FPlatformFileManager::Get().GetPlatformFile();
	FPakPlatformFile* PakPlatform = new FPakPlatformFile();
	PakPlatform->Initialize(&InnerPlatform, TEXT(""));
	FPlatformFileManager::Get().SetPlatformFile(*PakPlatform);

	TArray<FPakPlatformFile::FPakListEntry> FoundPaks;
	PakPlatform->GetMountedPaks(FoundPaks);

	for (FPakPlatformFile::FPakListEntry& CurrPak : FoundPaks)
	{
		if (CurrPak.PakFile->GetMountPoint() != "../../../")
		{
			FString stdfilename = CurrPak.PakFile->GetFilename();
			FPaths::MakeStandardFilename(stdfilename);

			FString BaseFilename = FPaths::GetBaseFilename(stdfilename);

			if (CurrPak.PakFile->IsValid())
			{
				IFileManager& FileManager = IFileManager::Get();

				FString ExpectedBin = FPaths::GameContentDir() + "Paks/" + BaseFilename + ".bin";
				FPaths::MakeStandardFilename(ExpectedBin);

				if (FileManager.FileExists(*ExpectedBin))
				{
					FPackageName::RegisterMountPoint("/" + BaseFilename + "/", FPaths::GetPath(stdfilename));

					FArrayReader binReader;
					if (FFileHelper::LoadFileToArray(binReader, *ExpectedBin))
					{
						AssetRegistry.Serialize(binReader);

						DetectedMods.Add(BaseFilename);
						UStaticFuncLib::Print("Detected Mod \'" + BaseFilename + "\'");
					}
					else
						UStaticFuncLib::Print("Mod \'" + BaseFilename + "\' was loaded, but its asset registry failed to load.");
				}
				else
					UStaticFuncLib::Print("Failed to load mod \'" + BaseFilename + "\'; Couldn't find \'" + BaseFilename + ".bin\'.");
			}
			else
				UStaticFuncLib::Print("Failed to load mod \'" + BaseFilename + "\'; FPakFile::IsValid() returned false!");
		}
	}
#endif

}

void UMPHorsoGameInstance::GetBlueprintAssets(IAssetRegistry& AssetRegistry, TArray<FAssetData>& OutBlueprintAssets)
{
	OutBlueprintAssets.Empty();

	TArray< FString > ContentPaths;
	ContentPaths.Add(TEXT("/Game"));

#if WITH_EDITOR
	for (auto curr : RetrievedMods)
	{
		if (curr.Value->CanContainContent())
			ContentPaths.Add(curr.Value->GetMountedAssetPath());
	}
#else
	for (FString& currMod : DetectedMods)
		ContentPaths.Add("/" + currMod);
#endif

	AssetRegistry.ScanPathsSynchronous(ContentPaths);

	FARFilter Filter;
	Filter.ClassNames.Add(UBlueprint::StaticClass()->GetFName());
	Filter.bRecursiveClasses = true;

	for (FString curr : ContentPaths)
		Filter.PackagePaths.Add(*curr);

	Filter.bRecursivePaths = true;

	AssetRegistry.GetAssets(Filter, OutBlueprintAssets);
}


void UMPHorsoGameInstance::FindContentOfClass(IAssetRegistry& AssetRegistry,
											  const TArray<FAssetData>& InFiltered,
											  UClass* ContentClass,
											  TArray<UClass*>& OutResults)
{
	OutResults.Empty();

	// from http://kantandev.com/articles/finding-all-classes-blueprints-with-a-given-base

	TSet< FName > DerivedNames;
	{
		TArray< FName > BaseNames;
		BaseNames.Add(ContentClass->GetFName());

		TSet< FName > Excluded;
		AssetRegistry.GetDerivedClassNames(BaseNames, Excluded, DerivedNames);

		for (auto curr : DerivedNames)
			UStaticFuncLib::Print(curr.ToString());

		DerivedNames.Remove(ContentClass->GetFName());
	}

	for (const FAssetData& Asset : InFiltered)
	{
		if (const FString* GeneratedClassPathPtr = Asset.TagsAndValues.Find(TEXT("GeneratedClass")))
		{
			const FString ClassObjectPath = FPackageName::ExportTextPathToObjectPath(*GeneratedClassPathPtr);
			const FString ClassName = FPackageName::ObjectPathToObjectName(ClassObjectPath);

			if (!DerivedNames.Contains(*ClassName) || ClassName.StartsWith("SKEL_"))
				continue;

			TAssetSubclassOf<UObject> AssetObj = TAssetSubclassOf<UObject>(FStringAssetReference(ClassObjectPath));
			OutResults.Add(AssetObj.LoadSynchronous());
		}
	}
}
