// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Engine/GameInstance.h"

#include "MPHorsoSaveGameTypes.h"

#include "MPHorsoGameInstance.generated.h"


class UCameraComponent;

class UNPCRuleBlock;

class AMPHorsoMusicManager;

class ANPCNavManager;

UENUM(BlueprintType)
enum class EAccessControlType : uint8
{
	AC_None			UMETA(DisplayName="None"),
	AC_Blacklist	UMETA(DisplayName="Blacklist"),
	AC_Whitelist	UMETA(DisplayName="Whitelist")
};

/**
 * 
 */
UCLASS()
class MPHORSO_API UMPHorsoGameInstance : public UGameInstance
{
	GENERATED_BODY()

	UCameraComponent* relevantCamera;

	UPROPERTY()
		UCharacterSaveBase* CharacterSave;

	UPROPERTY()
		UWorldSaveBase* WorldSave;

	UPROPERTY()
		TMap<TSubclassOf<UNPCRuleBlock>, UNPCRuleBlock*> InstantiatedNPCRuleBlocks;

	UPROPERTY()
		AMPHorsoMusicManager* SpawnedMusicManager = nullptr;

	UPROPERTY()
		ANPCNavManager* RetrievedNavManager = nullptr;

#if WITH_EDITOR
	TMap<FName, TSharedRef<class IPlugin>> RetrievedMods;
#endif

public:

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		TMap<FName, TAssetSubclassOf<AMagicUI>> AllSpells;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		TSubclassOf<UCharacterSaveBase> CurrentCharacterSaveType;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		TSubclassOf<UWorldSaveBase> CurrentWorldSaveType;
	UPROPERTY(EditDefaultsOnly)
		TArray<TSubclassOf<UCharacterSaveBase>> PreviousCharacterSaveTypes;
	UPROPERTY(EditDefaultsOnly)
		TArray<TSubclassOf<UWorldSaveBase>> PreviousWorldSaveTypes;

	UPROPERTY(EditDefaultsOnly)
		TSubclassOf<AMPHorsoMusicManager> MusicManagerClass;

	UPROPERTY(BlueprintReadOnly)
		TArray<FString> DetectedMods;

	/*
		Names of World Types, paired with
		the string name of the world to load.
	*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		TMap<FName, TSubclassOf<class UMPHorsoWorldType>> WorldTypes;//TMap<FName, FString> WorldTypes;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TArray<FString> RuntimeErrorList;


	// server protection stuff

	UPROPERTY(BlueprintReadWrite)
		bool IsHosting = false;
	UPROPERTY(BlueprintReadWrite)
		FString Password;

	UPROPERTY(BlueprintReadWrite)
		EAccessControlType AccessControlType = EAccessControlType::AC_None;
	UPROPERTY(BlueprintReadWrite)
		FString AccessControlListPath;
	UPROPERTY(BlueprintReadWrite)
		int ServerPort = 0;
	UPROPERTY(BlueprintReadWrite)
		FString ModListPath;


	// Volume Settings

	UPROPERTY(BlueprintReadWrite)
		float MasterVolume = 1.0f;
	UPROPERTY(BlueprintReadWrite, meta = (DisplayName = "BGM Volume"))
		float BGMVolume = 1.0f;
	UPROPERTY(BlueprintReadWrite, meta = (DisplayName = "Ambience Volume"))
		float AmbiVolume = 1.0f;
	UPROPERTY(BlueprintReadWrite, meta = (DisplayName = "SFX Volume"))
		float SFXVolume = 1.0f;


	// General Settings

	UPROPERTY(BlueprintReadWrite)
		bool Autopause = false;
	UPROPERTY(BlueprintReadWrite)
		bool PasswordsVisible = false;
	UPROPERTY(BlueprintReadWrite)
		float ServerTimeout = 10.0f;
	UPROPERTY(BlueprintReadWrite)
		bool AdvancedNPCInteraction = false;


	// Visual Settings

	/*
		in XxY[f] format for use in r.SetRes
		As a result covers both resolution and
		fullscreen
	*/
	UPROPERTY(BlueprintReadWrite)
		FString Resolution;


	virtual void Init() override;

	void OnNetFail(UWorld *World, UNetDriver *NetDriver, ENetworkFailure::Type FailureType, const FString& ErrorString);


	UFUNCTION(BlueprintCallable)
		void SetRelevantCamera(UCameraComponent* newRelevant) { relevantCamera = newRelevant; }

	UFUNCTION(BlueprintPure)
		UCameraComponent* GetRelevantCamera() { return relevantCamera; }


	UFUNCTION(BlueprintPure)
		ERaceType GetRace();

	UFUNCTION(BlueprintPure)
		void GetColorSchemeAsArrays(TArray<FName>& OutParts, TArray<FCharColorSchemePart>& OutData);

	UFUNCTION(BlueprintPure)
		FName GetLocalPlayerName();

	UFUNCTION(BlueprintCallable)
		void SetCharacterSave(UCharacterSaveBase* NewSave) { CharacterSave = NewSave; }

	UFUNCTION(BlueprintPure)
		UCharacterSaveBase* GetCharacterSave() { return CharacterSave; }

	UFUNCTION(BlueprintCallable)
		void SetWorldSave(UWorldSaveBase* NewSave) { WorldSave = NewSave; }

	UFUNCTION(BlueprintPure)
		UWorldSaveBase* GetWorldSave() { return WorldSave; }

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
		void ReadyForPlay();
	void ReadyForPlay_Implementation() {}


	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
		void KickIP(const FString& IP);
	void KickIP_Implementation(const FString& IP) {}
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
		void BanIP(const FString& IP);
	void BanIP_Implementation(const FString& IP) {}

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
		void UnbanIP(const FString& IP);
	void UnbanIP_Implementation(const FString& IP) {}

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
		void OpIP(const FString& IP);
	void OpIP_Implementation(const FString& IP) {}

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
		void UnopIP(const FString& IP);
	void UnopIP_Implementation(const FString& IP) {}


	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
		void KickPlayer(APlayerController* Player);
	void KickPlayer_Implementation(APlayerController* Player) {}

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
		void BanPlayer(APlayerController* Player);
	void BanPlayer_Implementation(APlayerController* Player) {}

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
		void OpPlayer(APlayerController* Player);
	void OpPlayer_Implementation(APlayerController* Player) {}

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
		void UnopPlayer(APlayerController* Player);
	void UnopPlayer_Implementation(APlayerController* Player) {}

	UFUNCTION()
		UNPCRuleBlock* GetNPCRuleBlock(TSubclassOf<UNPCRuleBlock> RuleBlockClass);

	UFUNCTION(BlueprintPure)
		AMPHorsoMusicManager* GetMusicManager();

	UFUNCTION(BlueprintPure)
		ANPCNavManager* GetNavManager();

	void FindAndRegisterMods(class IAssetRegistry& AssetRegistry);

	void GetBlueprintAssets(class IAssetRegistry& AssetRegistry, TArray<class FAssetData>& OutBlueprintAssets);

	void FindContentOfClass(class IAssetRegistry& AssetRegistry,
							const TArray<class FAssetData>& InFiltered,
							UClass* ContentClass,
							TArray<UClass*>& OutResults);
};
