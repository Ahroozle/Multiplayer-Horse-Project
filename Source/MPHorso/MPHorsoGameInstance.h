// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Engine/GameInstance.h"
#include "MPHorsoGameInstance.generated.h"


class USkinAnimation;
class UOffsetAnimation;

class UChatCommandBlock;
class UChatTagBlock;
class UChatWord;

class USpellArchetype;

class UCameraComponent;

class UCharacterSaveBase;
class UWorldSaveBase;

UENUM(BlueprintType)
enum class ERaceType : uint8
{
	Race_EarthP		UMETA(DisplayName="Earth Pony"),
	Race_Pega		UMETA(DisplayName="Pegasus"),
	Race_Uni		UMETA(DisplayName="Unicorn"),
	RACE_MAX		UMETA(Hidden)
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

public:

	UPROPERTY(EditDefaultsOnly)
		TArray<TSubclassOf<USkinAnimation>> AllSkinAnims;
	UPROPERTY(EditDefaultsOnly)
		TArray<TSubclassOf<UOffsetAnimation>> AllOffsetAnims;
	UPROPERTY(EditDefaultsOnly)
		TSubclassOf<UChatCommandBlock> ChatCommandBlock;
	UPROPERTY(EditDefaultsOnly)
		TSubclassOf<UChatTagBlock> ChatTagBlock;
	UPROPERTY(EditDefaultsOnly)
		TSubclassOf<UChatWord> DefaultChatWordType;
	UPROPERTY(EditDefaultsOnly)
		TArray<TSubclassOf<USpellArchetype>> AllSpells;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		TSubclassOf<UCharacterSaveBase> CurrentCharacterSaveType;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		TSubclassOf<UWorldSaveBase> CurrentWorldSaveType;
	UPROPERTY(EditDefaultsOnly)
		TArray<TSubclassOf<UCharacterSaveBase>> PreviousCharacterSaveTypes;
	UPROPERTY(EditDefaultsOnly)
		TArray<TSubclassOf<UWorldSaveBase>> PreviousWorldSaveTypes;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TArray<FString> RuntimeErrorList;


	virtual void Init() override;

	void OnNetFail(UWorld *World, UNetDriver *NetDriver, ENetworkFailure::Type FailureType, const FString& ErrorString);


	UFUNCTION(BlueprintCallable)
		void SetRelevantCamera(UCameraComponent* newRelevant) { relevantCamera = newRelevant; }

	UFUNCTION(BlueprintPure)
		UCameraComponent* GetRelevantCamera() { return relevantCamera; }


	UFUNCTION(BlueprintPure)
		ERaceType GetRace();

	UFUNCTION(BlueprintPure)
		void GetColorSchemeAsArrays(TArray<FName>& OutParts, TArray<FLinearColor>& OutColors);

	UFUNCTION(BlueprintPure)
		FName GetLocalPlayerName();

	UFUNCTION(BlueprintCallable)
		void SetSave(UCharacterSaveBase* NewSave) { CharacterSave = NewSave; }

	UFUNCTION(BlueprintPure)
		UCharacterSaveBase* GetSave() { return CharacterSave; }
};
