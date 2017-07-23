// Fill out your copyright notice in the Description page of Project Settings.

#include "MPHorso.h"
#include "MPHorsoSaveGameTypes.h"

#include "Paths.h"
#include "PlatformFile.h"
#include "PlatformFilemanager.h"

#include "StaticFuncLib.h"

#include "Kismet/GameplayStatics.h"


FString UCharacterSaveBase::GetGeneratedFileName() const
{
	TArray<TCHAR> NameAsArr = CharacterName.GetCharArray();

	//for (auto &currChar : NameAsArr)
	//{
	//	if (!TChar<TCHAR>::IsAlnum(currChar))
	//		currChar = FString("_")[0];
	//}
	NameAsArr = NameAsArr.FilterByPredicate([](const TCHAR& ch) { return TChar<TCHAR>::IsAlnum(ch); });

	FString CleanedCharName(NameAsArr.Num(), NameAsArr.GetData());
	return "CSave_" + FString::FromInt(UniqueID) + "_" + CleanedCharName + "_" + VersionString;
}

FString UWorldSaveBase::GetGeneratedFileName() const
{
	TArray<TCHAR> NameAsArr = WorldName.GetCharArray();

	//for (auto &currChar : NameAsArr)
	//{
	//	if (!TChar<TCHAR>::IsAlnum(currChar))
	//		currChar = FString("_")[0];
	//}
	NameAsArr = NameAsArr.FilterByPredicate([](const TCHAR& ch) { return TChar<TCHAR>::IsAlnum(ch); });

	FString CleanedWorldName(NameAsArr.Num(), NameAsArr.GetData());
	return "WSave_" + FString::FromInt(UniqueID)  + "_" + CleanedWorldName + "_" + VersionString;
}


void USaveGameHelperLibrary::GetSaveNames(TArray<FString>& OutCharacterFileNames, TArray<FString>& OutWorldFileNames)
{
	// took this solution from https://answers.unrealengine.com/questions/145598/is-there-a-way-to-get-all-savegames-in-bp.html , third down.

	class FSavesVisitor : public IPlatformFile::FDirectoryVisitor
	{
	public:
		FSavesVisitor() {}

		TArray<FString> CSavesFound; // Character Saves Found
		TArray<FString> WSavesFound; // World Saves Found

		virtual bool Visit(const TCHAR* FilenameOrDirectory, bool bIsDirectory)
		{
			if (!bIsDirectory)
			{
				FString FullPath(FilenameOrDirectory);
				if (FPaths::GetExtension(FullPath) == "sav")
				{
					FString CleanPath = FPaths::GetBaseFilename(FullPath);
					CleanPath = CleanPath.Replace(TEXT(".sav"), TEXT(""));

					if (CleanPath.StartsWith("CSave_"))
					{
						CSavesFound.Add(CleanPath);
					}
					else if (CleanPath.StartsWith("WSave_"))
					{
						WSavesFound.Add(CleanPath);
					}
				}
			}
			return true;
		}
	};

	const FString SaveFolder = FPaths::GameSavedDir() + "SaveGames";

	if (!SaveFolder.IsEmpty())
	{
		FSavesVisitor Visi;
		FPlatformFileManager::Get().GetPlatformFile().IterateDirectory(*SaveFolder, Visi);
		OutCharacterFileNames = Visi.CSavesFound;
		OutWorldFileNames = Visi.WSavesFound;
	}
}

void USaveGameHelperLibrary::GetSaves(UObject* WorldContext, TArray<UCharacterSaveBase*>& OutCharacterSaves, TArray<UWorldSaveBase*>& OutWorldSaves)
{
	TArray<FString> CharSaves;
	TArray<FString> WorldSaves;

	GetSaveNames(CharSaves, WorldSaves);

	USaveGame* CurrLoaded;

	TArray<UCharacterSaveBase*> CharactersToUID;
	TArray<UWorldSaveBase*> WorldsToUID;

	for (auto &currCharSlot : CharSaves)
	{
		CurrLoaded = UGameplayStatics::LoadGameFromSlot(currCharSlot, 0);

		if (nullptr != CurrLoaded)
		{
			UCharacterSaveBase* CurrAsCharacterSave = Cast<UCharacterSaveBase>(CurrLoaded);

			if (nullptr != CurrAsCharacterSave)
			{
				if (CharacterSaveIsLatestVersion(WorldContext, CurrAsCharacterSave))
					OutCharacterSaves.Add(CurrAsCharacterSave);
				else
				{
					CurrAsCharacterSave = UpdateOutdatedCharacterSave(WorldContext, CurrAsCharacterSave);

					CharactersToUID.Add(CurrAsCharacterSave);

					if(nullptr != CurrAsCharacterSave)
						OutCharacterSaves.Add(CurrAsCharacterSave);

				}
			}
		}
	}

	for (auto *currCSave : CharactersToUID)
		GenUIDForCSave(currCSave, OutCharacterSaves);

	for (auto &currWorldSlot : WorldSaves)
	{
		CurrLoaded = UGameplayStatics::LoadGameFromSlot(currWorldSlot, 0);

		if (nullptr != CurrLoaded)
		{
			UWorldSaveBase* CurrAsWorldSave = Cast<UWorldSaveBase>(CurrLoaded);

			if (nullptr != CurrAsWorldSave)
			{
				if (WorldSaveIsLatestVersion(WorldContext, CurrAsWorldSave))
					OutWorldSaves.Add(CurrAsWorldSave);
				else
				{
					CurrAsWorldSave = UpdateOutdatedWorldSave(WorldContext, CurrAsWorldSave);

					WorldsToUID.Add(CurrAsWorldSave);

					if (nullptr != CurrAsWorldSave)
						OutWorldSaves.Add(CurrAsWorldSave);
				}
			}
		}
	}

	for (auto *currWSave : WorldsToUID)
		GenUIDForWSave(currWSave, OutWorldSaves);

}

void USaveGameHelperLibrary::GenUIDForCSave(UCharacterSaveBase* NewSave, const TArray<UCharacterSaveBase*>& OtherExistingSaves)
{
	if (OtherExistingSaves.Num() <= 0)
		NewSave->UniqueID = FMath::RandRange(0, 1000000);
	else
	{
		uint32 MaxID = 0;
		for (auto *currOther : OtherExistingSaves)
			MaxID = FMath::Max(MaxID, currOther->UniqueID);

		NewSave->UniqueID = ++MaxID;
	}
}

void USaveGameHelperLibrary::GenUIDForWSave(UWorldSaveBase* NewSave, const TArray<UWorldSaveBase*>& OtherExistingSaves)
{
	if (OtherExistingSaves.Num() <= 0)
		NewSave->UniqueID = FMath::RandRange(0, 0xFFFFFFFF);
	else
	{
		uint32 MaxID = 0;
		for (auto *currOther : OtherExistingSaves)
			MaxID = FMath::Max(MaxID, currOther->UniqueID);

		NewSave->UniqueID = ++MaxID;
	}
}


bool USaveGameHelperLibrary::CharacterSaveIsLatestVersion(UObject* WorldContext, const UCharacterSaveBase* CSave)
{
	UMPHorsoGameInstance* GameInst = UStaticFuncLib::RetrieveGameInstance(WorldContext);

	if (nullptr == GameInst)
		return false;

	return CSave->VersionString == GameInst->CurrentCharacterSaveType.GetDefaultObject()->VersionString;
}

bool USaveGameHelperLibrary::WorldSaveIsLatestVersion(UObject* WorldContext, const UWorldSaveBase* WSave)
{
	UMPHorsoGameInstance* GameInst = UStaticFuncLib::RetrieveGameInstance(WorldContext);

	if (nullptr == GameInst)
		return false;

	return WSave->VersionString == GameInst->CurrentWorldSaveType.GetDefaultObject()->VersionString;
}

UCharacterSaveBase* USaveGameHelperLibrary::UpdateOutdatedCharacterSave(UObject* WorldContext, const UCharacterSaveBase* OutdatedCSave)
{
	UMPHorsoGameInstance* GameInst = UStaticFuncLib::RetrieveGameInstance(WorldContext);

	if (nullptr == GameInst)
		return nullptr;

	UMPHorsoSaveBase* CurrNewSave = OutdatedCSave->GenerateNextVersion();

	return (UCharacterSaveBase*)SaveUpdateHelper(GameInst, CurrNewSave);
}

UWorldSaveBase* USaveGameHelperLibrary::UpdateOutdatedWorldSave(UObject* WorldContext, const UWorldSaveBase* OutdatedWSave)
{
	UMPHorsoGameInstance* GameInst = UStaticFuncLib::RetrieveGameInstance(WorldContext);

	if (nullptr == GameInst)
		return nullptr;

	UMPHorsoSaveBase* CurrNewSave = OutdatedWSave->GenerateNextVersion();

	return (UWorldSaveBase*)SaveUpdateHelper(GameInst, CurrNewSave);
}

UMPHorsoSaveBase* USaveGameHelperLibrary::SaveUpdateHelper(UMPHorsoGameInstance* GameInst, UMPHorsoSaveBase* Start)
{
	UMPHorsoSaveBase* CurrNewSave = Start;

	TArray<TSubclassOf<UMPHorsoSaveBase>> PreviousClasses;
	TArray<FString> PreviousVersions;
	while (nullptr != CurrNewSave && CurrNewSave->VersionString != GameInst->CurrentCharacterSaveType.GetDefaultObject()->VersionString)
	{
		PreviousClasses.Add(CurrNewSave->GetClass());
		PreviousVersions.Add(CurrNewSave->VersionString);

		CurrNewSave = CurrNewSave->GenerateNextVersion();

		if (CurrNewSave != nullptr)
		{
			int FoundInd;
			if (PreviousClasses.Contains(CurrNewSave->GetClass()))
			{
				UStaticFuncLib::Print("USaveGameHelperLibrary::UpdateHelper: Infinite loop detected! class \'" +
					PreviousClasses.Last()->GetName() + "\' created a previously-created class \'" +
					CurrNewSave->GetClass()->GetName() + "\' instead of continuing up the expected chain.", true);
				CurrNewSave = nullptr;
			}
			else if (PreviousVersions.Find(CurrNewSave->VersionString, FoundInd))
			{
				UStaticFuncLib::Print("USaveGameHelperLibrary::UpdateHelper: Infinite loop detected! class \'" +
					CurrNewSave->GetClass()->GetName() + "\' has Version String \'" + CurrNewSave->VersionString +
					"\', which is already taken by previously-created class \'" + PreviousClasses[FoundInd]->GetName() +
					"\', which has caused a null instead of continuing up the expected chain.", true);
				CurrNewSave = nullptr;
			}
		}
	}

	return CurrNewSave;
}
