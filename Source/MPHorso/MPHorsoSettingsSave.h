// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/SaveGame.h"
#include "AccessControlListtypes.h"
#include "MPHorsoSettingsSave.generated.h"

USTRUCT(BlueprintType)
struct FServerTabData
{
	GENERATED_USTRUCT_BODY();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FString WorldName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FString Address;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		int Port;
};

/**
 * 
 */
UCLASS()
class MPHORSO_API UMPHorsoSettingsSave : public USaveGame
{
	GENERATED_BODY()
	
public:

	// Volume Settings

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float MasterVolume = 1;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float BGM_Volume = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float AmbienceVolume = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float SFX_Volume = 1;


	// General Settings
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		bool Autopause;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		bool PasswordsVisible;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float ServerTimeout = 10;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		bool Advanced_NPC_Interaction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		bool FadeInv = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		bool FlipHotbarScroll;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		bool LockHotbar;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		bool JoystickMovement;


	// Visual Settings

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FString Resolution;


	/*
		This ban list is a special one that only the
		server owner can add or remove from. It functions
		as a way to ban, mod, etc. a user on all servers
		you host, rather than just on a world-by-world
		basis.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FAccessControlList UniversalACL;

	// Servers that the player has added to their join list.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		TArray<FServerTabData> ServerTabs;

	// Returns false if already exists, and outputs the index that was found into OutFoundInd
	UFUNCTION(BlueprintCallable)
		bool AddServerTab(const FString& Address, int Port, int& OutFoundInd)
		{
			FServerTabData NewTab = { "",Address,Port };

			{
				auto Pred = [&NewTab](const FServerTabData& a) {return a.Address == NewTab.Address && a.Port == NewTab.Port; };
				OutFoundInd = ServerTabs.IndexOfByPredicate(Pred);
			}

			if (OutFoundInd == INDEX_NONE)
			{
				ServerTabs.Insert(NewTab, 0);
				return true;
			}

			return false;
		}

	UFUNCTION(BlueprintCallable)
		void SetServerTabWorldName(const FString& NewWorldName, int Index)
		{
			if (Index > -1 && Index < ServerTabs.Num())
				ServerTabs[Index].WorldName = NewWorldName;
		}

	UFUNCTION(BlueprintCallable)
		void SetMostRecentServer(int Index)
		{
			if (Index > -1 && Index < ServerTabs.Num())
				ServerTabs.Swap(0, Index);
		}

	UFUNCTION(BlueprintCallable)
		void RemoveServerTab(int Index)
		{
			if (Index > -1 && Index < ServerTabs.Num())
				ServerTabs.RemoveAt(Index);
		}
};
