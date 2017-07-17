// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/SaveGame.h"
#include "MPHorsoGameInstance.h"
#include "MPHorsoSaveGameTypes.generated.h"


/*
	The base class for saves in MPHorso!
*/
UCLASS(abstract)
class MPHORSO_API UMPHorsoSaveBase : public USaveGame
{
	GENERATED_BODY()

public:

	UPROPERTY(EditDefaultsOnly, Category = "MP Horso SaveGame Class Base")
		FString VersionString;


	UFUNCTION(BlueprintCallable, Category = "MP Horso SaveGame Class Base")
		virtual FString GetGeneratedFileName() const { return ""; }

	UFUNCTION(BlueprintNativeEvent, Category = "MP Horso SaveGame Class Base")
		UMPHorsoSaveBase* GenerateNextVersion() const;
	virtual UMPHorsoSaveBase* GenerateNextVersion_Implementation() const { return nullptr; }

};

/*
	SaveGame class for characters.

	Files of this type are intended to be in the format: CSave_<CharacterName>_<VersionString>
	e.g.
	"CSave_Twi_1"
*/
UCLASS(abstract, Blueprintable)
class MPHORSO_API UCharacterSaveBase : public UMPHorsoSaveBase
{
	GENERATED_BODY()
	
public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Save")
		FString CharacterName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Save")
		ERaceType Race;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Save")
		TMap<FName, FLinearColor> ColorScheme;


	virtual FString GetGeneratedFileName() const override { return "CSave_" + CharacterName + "_" + VersionString; }
	
};

/*
	SaveGame class for worlds

	Files of this type are intended to be in the format: WSave_<WorldName>_<VersionString>
	e.g.
	"WSave_Equus_1"

	(TODO : This is WIP and won't be used until world generation takes a front seat again)
*/
UCLASS(abstract, Blueprintable)
class MPHORSO_API UWorldSaveBase : public UMPHorsoSaveBase
{
	GENERATED_BODY()
	
public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Save")
		FString WorldName;
	

	virtual FString GetGeneratedFileName() const override { return "WSave_" + WorldName + "_" + VersionString; }

};


UCLASS()
class MPHORSO_API USaveGameHelperLibrary : public UObject
{
	GENERATED_BODY()
	
public:
	
	
	UFUNCTION(BlueprintCallable)
		static void GetSaveNames(TArray<FString>& OutCharacterFileNames, TArray<FString>& OutWorldFileNames);

	UFUNCTION(BlueprintCallable)
		static void GetSaves(UObject* WorldContext, TArray<UCharacterSaveBase*>& OutCharacterSaves, TArray<UWorldSaveBase*>& OutWorldSaves);

private:

	UFUNCTION()
		static bool CharacterSaveIsLatestVersion(UObject* WorldContext, const UCharacterSaveBase* CSave);

	UFUNCTION()
		static bool WorldSaveIsLatestVersion(UObject* WorldContext, const UWorldSaveBase* WSave);

	UFUNCTION()
		static UCharacterSaveBase* UpdateOutdatedCharacterSave(UObject* WorldContext, const UCharacterSaveBase* OutdatedCSave);

	UFUNCTION()
		static UWorldSaveBase* UpdateOutdatedWorldSave(UObject* WorldContext, const UWorldSaveBase* OutdatedWSave);

	UFUNCTION()
		static UMPHorsoSaveBase* SaveUpdateHelper(UMPHorsoGameInstance* GameInst, UMPHorsoSaveBase* Start);

};
