// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/SaveGame.h"
#include "MPHorsoGameInstance.h"
#include "RuleTypes.h"
#include "MPHorsoSaveGameTypes.generated.h"


/*
	The base class for saves in MPHorso!
*/
UCLASS(abstract)
class MPHORSO_API UMPHorsoSaveBase : public USaveGame
{
	GENERATED_BODY()

public:

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MP Horso SaveGame Class Base")
		FString VersionString;

	UPROPERTY()
		uint32 UniqueID;


	UFUNCTION(BlueprintCallable, Category = "MP Horso SaveGame Class Base")
		virtual FString GetGeneratedFileName() const { return ""; }

	UFUNCTION(BlueprintNativeEvent, Category = "MP Horso SaveGame Class Base")
		UMPHorsoSaveBase* GenerateNextVersion() const;
	virtual UMPHorsoSaveBase* GenerateNextVersion_Implementation() const { return nullptr; }

};

/*
	SaveGame class for characters.

	Files of this type are intended to be in the format: CSave_<UniqueID>_<CharacterName>_<VersionString>
	e.g.
	"CSave_1290437_Twi_1"
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


	virtual FString GetGeneratedFileName() const override;
	
};


//UENUM(BlueprintType)
//enum class ENPC_Type : uint8
//{
//	/*
//		Default NPC!
//
//		These ones populate towns and stay in them. They can provide
//		helpful information (and maybe even quests if I can impl them)!
//
//		Most of their knowledge is probably going to be about the town
//		they live in, but some of them may be retired from a life outside
//		their current home, or may even have old tales and rumors to share!
//	*/
//	NPC_Default			UMETA(DisplayName = "Townsfolk (Default)"),
//
//	/*
//		Healer NPC!
//
//		These ones heal you for a price when you ask, and also
//		take you back to their home (always a hospital or some house
//		that can function as one) to revive you when you die.
//
//		The one that revives you is always the one in the last town
//		you visited, so be careful where you die.
//	*/
//	NPC_Healer			UMETA(DisplayName = "Healer/Reviver"),
//
//	/*
//		Merchant NPC!
//
//		These man the shops in towns in order to sell you stuff, but
//		otherwise function as default NPCs when they're off their work
//		hours.
//	*/
//	NPC_Merchant		UMETA(DisplayName = "Town Merchant/Shopkeep"),
//
//	/*
//		Travelling NPC!
//
//		These may run around the entire map whenever they feel like it,
//		but are otherwise default NPCs. They have a broader knowledge of
//		the world throughout their travels, so you're more likely to
//		hear about juicy rumors or sneaky shortcuts across the land from
//		them, or even by just by watching them wander!
//	*/
//	NPC_Traveller		UMETA(DisplayName = "Travelling NPC"),
//
//	/*
//		Travelling Merchant NPC!
//
//		These function like travelling NPCs but will also sell you
//		stuff! Things you may find in these NPCs' inventories are
//		a more varied bag than your average shop, so be sure to hit
//		one of them up when you see them just in case they might have
//		something really neat!
//	*/
//	NPC_TravMerch		UMETA(DisplayName = "Travelling Merchant"),
//
//
//
//
//	NPC_TYPE_MAX	UMETA(Hidden)
//};

USTRUCT(BlueprintType)
struct FNPC_SaveData
{
	GENERATED_USTRUCT_BODY();


	//UPROPERTY(BlueprintReadWrite)
	//	ENPC_Type NPCType;

	UPROPERTY(BlueprintReadWrite)
		FVector Location;

	// TODO: some way to assign homes by names or some other way?
	UPROPERTY(BlueprintReadWrite)
		FName HomeName;


	// personality parts to reconstruct the NPC's behavior.

	UPROPERTY(BlueprintReadWrite)
		FName NPC_Name;

	UPROPERTY(BlueprintReadWrite)
		TArray<TSubclassOf<URuleBlock>> SavedRules;

	UPROPERTY(BlueprintReadWrite)
		FRuleQuery Memories;

};

USTRUCT(BlueprintType)
struct FWeatherSaveData
{
	GENERATED_USTRUCT_BODY();

	UPROPERTY(BlueprintReadWrite)
		TSubclassOf<class AWeatherActor> WeatherClass;

	UPROPERTY(BlueprintReadWrite)
		FVector CurrentPosition;

	UPROPERTY(BlueprintReadWrite)
		FVector Direction;

};

/*
	SaveGame class for worlds

	Files of this type are intended to be in the format: WSave_<UniqueID>_<WorldName>_<VersionString>
	e.g.
	"WSave_129492_Equus_1"

	(TODO : This is WIP and won't be used until world generation takes a front seat again)
*/
UCLASS(abstract, Blueprintable)
class MPHORSO_API UWorldSaveBase : public UMPHorsoSaveBase
{
	GENERATED_BODY()
	
public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Save")
		FString WorldName;

	/*
		The state of the world in query form!

		This gets filled out upon the world being
		created / generated / whatever and is of
		course modified when certain events happen.
	*/
	UPROPERTY(BlueprintReadWrite, Category = "World Save")
		FRuleQuery WorldState;

	/*
		Data pertaining to every created NPC. This is
		what should take up the bulk of the memory used.
	*/
	UPROPERTY(BlueprintReadWrite, Category = "World Save")
		TArray<FNPC_SaveData> NPCData;

	/*
		Data saving the current weather.
	*/
	UPROPERTY(BlueprintReadWrite, Category = "World Save")
		TArray<FWeatherSaveData> WeatherData;
	

	virtual FString GetGeneratedFileName() const override;

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

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Generate Unique ID For Character Save"))
		static void GenUIDForCSave(UCharacterSaveBase* NewSave, const TArray<UCharacterSaveBase*>& OtherExistingSaves);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Generate Unique ID For World Save"))
		static void GenUIDForWSave(UWorldSaveBase* NewSave, const TArray<UWorldSaveBase*>& OtherExistingSaves);

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
