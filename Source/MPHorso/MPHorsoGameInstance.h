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

	//TMap<TSubclassOf<USkinAnimation>, USkinAnimation*> SkinInsts;
	//TMap<TSubclassOf<UOffsetAnimation>, UOffsetAnimation*> OffsInsts;

	UCameraComponent* relevantCamera;

	ERaceType Race = ERaceType::Race_EarthP;

	FName PlayerName;

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

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TArray<FString> RuntimeErrorList;


	// TODO SWITCH TO PLAYER PROFILES VIA SAVEGAME STUFF
	UPROPERTY(BlueprintReadWrite)
		TMap<FName, FLinearColor> ColorScheme;


	virtual void Init() override;

	void OnNetFail(UWorld *World, UNetDriver *NetDriver, ENetworkFailure::Type FailureType, const FString& ErrorString);
	
	//USkinAnimation* GetSkinAnim(TSubclassOf<USkinAnimation> InstClass);
	//UOffsetAnimation* GetOffsAnim(TSubclassOf<UOffsetAnimation> InstClass);

	UFUNCTION(BlueprintCallable)
		void SetRelevantCamera(UCameraComponent* newRelevant) { relevantCamera = newRelevant; }

	UFUNCTION(BlueprintPure)
		UCameraComponent* GetRelevantCamera() { return relevantCamera; }

	UFUNCTION(BlueprintCallable)
		void SetRace(ERaceType NewRace) { Race = NewRace; }
	UFUNCTION(BlueprintPure)
		ERaceType GetRace() { return Race; }

	UFUNCTION(BlueprintCallable)
		void SetLocalPlayerName(FName NewName) { PlayerName = NewName; }
	UFUNCTION(BlueprintPure)
		FName GetLocalPlayerName() { return PlayerName; }
};
