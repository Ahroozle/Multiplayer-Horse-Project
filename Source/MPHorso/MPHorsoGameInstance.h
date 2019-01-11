// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Engine/GameInstance.h"

#include "MPHorsoSaveGameTypes.h"

#include "MPHorsoGameInstance.generated.h"


class UCameraComponent;

class UNPCRuleBlock;

class AMPHorsoMusicManager;

class ANPCNavManager;

class UMPHorsoSettingsSave;

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
		UMPHorsoSettingsSave* SettingsSave;

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
		TMap<FName, TAssetSubclassOf<class AMagicUI>> AllSpells;

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

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TArray<FString> RuntimeErrorList;


	// server protection stuff

	UPROPERTY(BlueprintReadWrite)
		bool IsHosting = false;

	UPROPERTY(BlueprintReadWrite)
		FString HostID;

	/*
		Note: Only used when in dedicated server mode.

		When the server is in listen mode, the host can simply
		be determined as the first playercontroller present, but this
		isn't the case for a dedicated server. Instead, the host must define
		this password so that, when needed, they can request that the server
		give them ownership at the cost of knowing this password.

		Note for when actually doing this: I should probably encrypt this lmao
	*/
	UPROPERTY(BlueprintReadWrite)
		FString HostPassword;


	UPROPERTY(BlueprintReadWrite)
		FString ServerPassword;


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

	UFUNCTION(BlueprintPure)
		UMPHorsoSettingsSave* GetSettingsSave();

	UFUNCTION(BlueprintPure)
		bool GetWorldAccessControlList(FAccessControlList& WorldACL);

	UFUNCTION(BlueprintPure)
		void GetUniversalAccessControlList(FAccessControlList& UniversalACL);

	UFUNCTION(BlueprintCallable)
		void SaveSettings();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
		void ReadyForPlay();
	void ReadyForPlay_Implementation() {}


	UFUNCTION(BlueprintPure)
		bool GetPlayerIP(APlayerController* Player, FString& OutIP);

	UFUNCTION(BlueprintPure)
		bool UserIsHost(const FString& ID) { return ID == HostID; }

	UFUNCTION(BlueprintCallable)
		bool Kick(const FString& InvokerIP, const FString& TargetID, float DurationSeconds, bool Universal, FString& ErrorString);

	UFUNCTION(BlueprintCallable)
		bool Unkick(const FString& InvokerIP, const FString& TargetID, bool Universal, FString& ErrorString);

	UFUNCTION(BlueprintCallable)
		bool Ban(const FString& InvokerIP, const FString& TargetID, bool Universal, FString& ErrorString);

	UFUNCTION(BlueprintCallable)
		bool Unban(const FString& InvokerIP, const FString& TargetID, bool Universal, FString& ErrorString);

	UFUNCTION(BlueprintCallable)
		bool Promote(const FString& InvokerIP, const FString& TargetID, const FString& ModTier, bool Universal, FString& ErrorString);

	UFUNCTION(BlueprintCallable)
		bool Demote(const FString& InvokerIP, const FString& TargetID, const FString& ModTier, bool Universal, FString& ErrorString);

	UFUNCTION(BlueprintCallable)
		bool MakeModTier(const FString& InvokerIP, const FString& TierName, const TArray<FString>& ModPerms, int InsertIndex, bool Universal, FString& ErrorString);

	UFUNCTION(BlueprintCallable)
		bool RemoveModTier(const FString& InvokerIP, const FString& TierName, const FString& MoveTierName, bool Universal, FString& ErrorString);

	UFUNCTION(BlueprintCallable)
		bool ChangeListMode(const FString& InvokerIP, bool Whitelist, bool Universal, FString& ErrorString);


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
