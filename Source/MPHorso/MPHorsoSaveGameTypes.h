// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/SaveGame.h"
#include "MPHorsoGameInstance.h"
#include "NPCGeneralTypes.h"
#include "MPHorsoItemTypes.h"
#include "MPHorsoSaveGameTypes.generated.h"


/*
	TODO
		Somehow, store ID information.
		i.e. Have clients keep a lost of their ID on each server, and have servers keep a list of client IDs
		to organize various information with (i.e. log-off positions and whatnot)

		Probably also store overworld room data within worldsaves in the form of NPC-relevant states
*/

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


USTRUCT(BlueprintType)
struct FInvSaveData
{
	GENERATED_USTRUCT_BODY();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TArray<FName> Prefixes;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TSubclassOf<class UMPHorsoItemBase> ItemType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		int Stack;
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

	/*
		The name of the difficulty the player is at!

		list of them is:
			- Default (White Player Name)
					Player drops half their cash on death.
			- Midcore (Cyan Player Name)
					Player drops all their items on death.
			- Semi-Hardcore (Purple Player Name)
					Player drops all their items on death.
					Permadeath if not within (X) rooms of
					a hospital.
			- Hardcore (Red Player Name)
					Player drops all their items on death.
					Permadeath regardless of proximity to
					hospitals.

		Items/Cash dropped on death can be navigated back to
		and picked up.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Save")
		FName Difficulty = "Default";

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Save")
		float NativeStamina = 1.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Save")
		float TemporaryStamina = 0.0f;

	// Hotbar, then Main Inv, then Saddlebags.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Save")
		TArray<FInvSaveData> Inventory;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Save", meta = (ClampMin = "0.0", ClampMax = "2.0"))
		int NumSaddleBags;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Save")
		TArray<FName> AllSpells;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Save")
		TArray<FName> EquippedSpells;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Save")
		FString Notes;

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


	// personality parts to reconstruct the NPC's behavior.

	UPROPERTY(BlueprintReadWrite)
		FNPCRelevantState Memories;//FRuleQuery Memories;

	/*
		This personality contains any edits that have happened, and overwrites the default if it isn't empty.
	*/
	UPROPERTY(BlueprintReadWrite)
		FNPCRelevantState SavedPersonality;

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

USTRUCT(BlueprintType)
struct FWorldSettingsData
{
	GENERATED_USTRUCT_BODY();

	/*
		The type of world! This basically just governs which
		static world to load into.

		NOTE: "Combined" is explicitly saved for the combined
		world mode. Don't use it to reference static worlds!
	*/
	UPROPERTY(BlueprintReadWrite, Category = "World Settings")
		FName WorldType;

	/*
		The range of intensities weather can be in this world.
	*/
	UPROPERTY(BlueprintReadWrite, Category = "World Settings")
		FIntPoint WeatherIntensity = { 1, 3 };

	/*
		How often does weather happen?
		Directly correlates to how many storms there are on
		the map at any given time.
	*/
	UPROPERTY(BlueprintReadWrite, Category = "World Settings")
		int WeatherFrequency = 1;

	/*
		Enemy Density Multiplier. This determines how many enemies
		there are at any given point in the map and may even cause
		certain spawners to start or stop being used to more evenly
		distribute the lack or surplus of foes.

		NOTE: Setting this to zero also gets rid of the bosses!
			  This is intended to allow it to serve as a peaceful-
			  type mode for those who aren't into combat. The focus
			  becomes more exploration-oriented as a result.
	*/
	UPROPERTY(BlueprintReadWrite, Category = "World Settings")
		float EnemyDensity = 1;

	/*
		Enemy Strength Multiplier. This determines how strong your
		foes are.
	*/
	UPROPERTY(BlueprintReadWrite, Category = "World Settings")
		float EnemyStrength = 1;

	/*
		Enemy Viciousness Multiplier. This determines spawn rates
		for higher-powered enemies and whether or not enemies use
		more vicious actions (like debuff-causing attacks or looting
		your corpse).
	*/
	UPROPERTY(BlueprintReadWrite, Category = "World Settings")
		float EnemyViciousness = 1;

	/*
		Loot Stinginess Divider. This determines how hard literally
		getting anything is. Applies to both enemies and chests.
	*/
	UPROPERTY(BlueprintReadWrite, Category = "World Settings")
		float LootStinginess = 1;

	/*
		Spawn in the default location or a random one?
		If NoSavior is enabled you can literally spawn anywhere
		on the map that is considered out in the open. Otherwise
		you will only spawn near towns so that the respective
		nurse can save you.
	*/
	UPROPERTY(BlueprintReadWrite, Category = "World Settings")
		bool RandomSpawnLocation = false;

	/*
		Does a nurse come to save you when you first land?
		This dramatically alters how the game starts because
		you spawn in with no health left into a potentially
		hostile area of the map.
	*/
	UPROPERTY(BlueprintReadWrite, Category = "World Settings")
		bool NoSavior = false;

	/*
		Don't spawn any NPCs? This literally means there are no
		ponies to help you. Hospitals do not work because there
		are no nurses. Merchants don't exist.
		Obviously means NoSavior is on.
	*/
	UPROPERTY(BlueprintReadWrite, Category = "World Settings")
		bool Alone = false;

	/*
		Total hostility for easy reference
	*/
	UPROPERTY(BlueprintReadWrite, Category = "World Settings")
		float TotalHostility;

};

/*
	SaveGame class for worlds

	Files of this type are intended to be in the format: WSave_<UniqueID>_<WorldName>_<VersionString>
	e.g.
	"WSave_129492_Equus_1"


*/
UCLASS(abstract, Blueprintable)
class MPHORSO_API UWorldSaveBase : public UMPHorsoSaveBase
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Save")
		FString WorldName;

	UPROPERTY(BlueprintReadWrite, Category = "World Save")
		FWorldSettingsData WorldSettings;

	/*
		The state of the world in query form!

		This gets filled out upon the world being
		created / generated / whatever and is of
		course modified when certain events happen.
	*/
	UPROPERTY(BlueprintReadWrite, Category = "World Save")
		FNPCRelevantState WorldState;//FRuleQuery WorldState;

	/*
		Data pertaining to every created NPC. This is
		what should take up the bulk of the memory used.

		Organized as NPC names to NPC Data
	*/
	UPROPERTY(BlueprintReadWrite, Category = "World Save")
		TMap<FName, FNPC_SaveData> NPCData;

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
	
	static FWorldSettingsData MinSettings;
	static bool MinSettingsSet;

	static FWorldSettingsData MaxSettings;
	static bool MaxSettingsSet;

public:
	
	
	UFUNCTION(BlueprintCallable)
		static void GetSaveNames(TArray<FString>& OutCharacterFileNames, TArray<FString>& OutWorldFileNames);

	UFUNCTION(BlueprintCallable)
		static void GetSaves(UObject* WorldContext, TArray<UCharacterSaveBase*>& OutCharacterSaves, TArray<UWorldSaveBase*>& OutWorldSaves);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Generate Unique ID For Character Save"))
		static void GenUIDForCSave(UCharacterSaveBase* NewSave, const TArray<UCharacterSaveBase*>& OtherExistingSaves);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Generate Unique ID For World Save"))
		static void GenUIDForWSave(UWorldSaveBase* NewSave, const TArray<UWorldSaveBase*>& OtherExistingSaves);

	UFUNCTION(BlueprintPure)
		static FWorldSettingsData& GetWorldSettingsMinimums();
	UFUNCTION(BlueprintPure)
		static FWorldSettingsData& GetWorldSettingsMaximums();

	UFUNCTION(BlueprintPure)
		static float CalculateHostilityPercentage(const FWorldSettingsData& WorldData, float& EnemyHostility, float& WeatherHostility, float& FateHostility);
	
	UFUNCTION(BlueprintPure)
		static float GetHostilityLevel(const FWorldSettingsData& WorldData);

	UFUNCTION(BlueprintCallable)
		static void CalculateAndApplyHostility(UPARAM(Ref) FWorldSettingsData& WorldData);

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
