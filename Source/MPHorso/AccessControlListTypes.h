// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "UObject/NoExportTypes.h"
#include "AccessControlListTypes.generated.h"


UENUM(BlueprintType)
enum class EPlayerStanding : uint8
{
	Banned		UMETA(Tooltip = "Player is banned from the server ('formally banned' in whitelist mode)."),
	Player		UMETA(Tooltip = "Player is in normal standing with the server; They just play the game"),
	Moderator	UMETA(Tooltip = "Player is an operator/moderator/admin/etc.; They maintain order in the server.")
};

USTRUCT(BlueprintType)
struct FModTierPermissions
{
	GENERATED_USTRUCT_BODY();

	// Name of this moderator tier
	UPROPERTY(BlueprintReadOnly)
		FName ModTierName;

	// Perm strings that are checked for within their respective functions before execution.
	UPROPERTY(BlueprintReadOnly)
		TSet<FString> ModPerms;
};

USTRUCT(BlueprintType)
struct FPlayerStandingData
{
	GENERATED_USTRUCT_BODY();

	UPROPERTY(BlueprintReadOnly)
		EPlayerStanding PlayerStanding;

	// If the user is kicked, they are unable to reenter the server until this date.
	UPROPERTY(BlueprintReadOnly)
		FDateTime KickedUntil;

	// If the player is a mod, what tier are they?
	UPROPERTY(BlueprintReadOnly)
		FName ModTierName;
};

USTRUCT(BlueprintType)
struct FAccessControlList
{
	GENERATED_USTRUCT_BODY();

	UPROPERTY(BlueprintReadOnly)
		bool WhitelistMode;

	/*
		List of mod tiers for this server, in descending order of power.
		Only the server owner is able to edit the full list; mods themselves
		are only able to edit tiers below theirs, and only able to add or take
		away permissions that their tier has access to.
	*/
	UPROPERTY(BlueprintReadOnly)
		TArray<FModTierPermissions> ModTiers;

	/*
		The heart of the list. Keeps track of all players' standing in
		terms of the server, be they moderators, or normal players.
	*/
	UPROPERTY(BlueprintReadOnly)
		TMap<FString, FPlayerStandingData> PlayerStandingList;

	/*
		List of all actions by moderators, in chronological order.
	*/
	UPROPERTY(BlueprintReadOnly)
		TArray<FString> AuditLog;

	bool RegisterNewPlayer(const FString& NewID)
	{
		if (!PlayerStandingList.Contains(NewID))
		{
			PlayerStandingList.Add(NewID, { EPlayerStanding::Player, FDateTime(), FName() });
			return true;
		}
		return false;
	}
};


class UMPHorsoGameInstance;

UCLASS()
class MPHORSO_API UAccessControlListFuncLib : public UObject
{
	GENERATED_BODY()

public:

	// Takes in the given player's IP and returns their ID.
	UFUNCTION(BlueprintPure, Category = "Access Control")
		static FString ConstructPlayerID(FString IPString) { return FString::Printf(TEXT("%u"), GetTypeHash(IPString)); }

	UFUNCTION(BlueprintPure, Category = "Access Control")
		static FString ConstructCharacterID(FString IPString, FString CharString)
		{
			return FString::Printf(TEXT("%u"), GetTypeHash(IPString)) + "_" + FString::Printf(TEXT("%u"), GetTypeHash(CharString));
		}


	/*
		1	A > B
		0	A == B
		-1	A < B
	*/
	UFUNCTION(BlueprintPure, Category = "Access Control")
		static int CompareModTiers(const FAccessControlList& ListRef, FName ModTierA, FName ModTierB);

	UFUNCTION(BlueprintPure, Category = "Access Control")
		static bool NeedToRequestAccess(const FAccessControlList& ListRef, FString UserID);

	UFUNCTION(BlueprintPure, Category = "Access Control")
		static bool IsBanned(const FAccessControlList& ListRef, FString UserID);

	UFUNCTION(BlueprintPure, Category = "Access Control")
		static bool IsKicked(const FAccessControlList& ListRef, FString UserID);

	UFUNCTION(BlueprintPure, Category = "Access Control")
		static bool IsModerator(UMPHorsoGameInstance* GameInst, const FAccessControlList& ListRef, FString UserID);

	UFUNCTION(BlueprintPure, Category = "Access Control")
		static bool IsHigherPrivilegeThan(UMPHorsoGameInstance* GameInst, const FAccessControlList& ListRef, FString RequesterID, FString TargetID);


	/*
		Registers a player. Done immediately on login when in blacklist mode, or when given the OK by a moderator in whitelist mode.
	*/
	UFUNCTION(BlueprintCallable, Category = "Access Control")
		static FString RegisterPlayer(UMPHorsoGameInstance* GameInst, UPARAM(Ref) FAccessControlList& ListRef, FString RequesterID, FString RegisterID);
	
	/*
		Kicks a player or moderator.
	*/
	UFUNCTION(BlueprintCallable, Category = "Access Control")
		static FString KickPlayer(UMPHorsoGameInstance* GameInst, UPARAM(Ref) FAccessControlList& ListRef, FString RequesterID, FString KickID, int DurationSeconds);

	/*
		Unkicks a player or moderator.
	*/
	UFUNCTION(BlueprintCallable, Category = "Access Control")
		static FString UnkickPlayer(UMPHorsoGameInstance* GameInst, UPARAM(Ref) FAccessControlList& ListRef, FString RequesterID, FString KickedID);

	/*
		Bans a player or moderator. If done to a player, bans them. If done to a moderator, automatically revokes their status.
	*/
	UFUNCTION(BlueprintCallable, Category = "Access Control")
		static FString BanPlayer(UMPHorsoGameInstance* GameInst, UPARAM(Ref) FAccessControlList& ListRef, FString RequesterID, FString BanID);

	/*
		Unbans a player.
	*/
	UFUNCTION(BlueprintCallable, Category = "Access Control")
		static FString UnbanPlayer(UMPHorsoGameInstance* GameInst, UPARAM(Ref) FAccessControlList& ListRef, FString RequesterID, FString BannedID);

	/*
		Promotes a player or moderator to a mod tier explicitly below the invoking moderator's tier. 'None' is an invalid input when
		any mod tiers have been specified.
	*/
	UFUNCTION(BlueprintCallable, Category = "Access Control")
		static FString PromotePlayer(UMPHorsoGameInstance* GameInst, UPARAM(Ref) FAccessControlList& ListRef, FString RequesterID, FString PromoteID, FName ModTierName);

	/*
		Demotes a moderator to a tier below them. Tier 'None' revokes all perms.
	*/
	UFUNCTION(BlueprintCallable, Category = "Access Control")
		static FString DemotePlayer(UMPHorsoGameInstance* GameInst, UPARAM(Ref) FAccessControlList& ListRef, FString RequesterID, FString DemoteID, FName ModTierName);

	/*
		Creates a mod tier. Gives back an error and does nothing if the requestee is attempting to make a tier higher than their own, or
		with permissions they themselves do not have.
	*/
	UFUNCTION(BlueprintCallable, Category = "Access Control")
		static FString MakeModTier(UMPHorsoGameInstance* GameInst, UPARAM(Ref) FAccessControlList& ListRef, FString RequesterID, FName TierName, const TArray<FString>& ModPerms, int InsertIndex);

	/*
		Removes a mod tier from the list. Gives back an error and does nothing if the requestee is attempting to remove a tier higher than
		or equal to their own. All moderators of the tier being removed will be switched over to the tier specified by 'MoveTierName.' If
		MoveTierName is 'None', they are completely demoted. (NOTE: Popup should come up with confirmation in case they do this, to confirm
		this is what they want.)
	*/
	UFUNCTION(BlueprintCallable, Category = "Access Control")
		static FString RemoveModTier(UMPHorsoGameInstance* GameInst, UPARAM(Ref) FAccessControlList& ListRef, FString RequesterID, FName TierName, FName MoveTierName);

	/*
		Changes the Access Control List's mode of operation. ONLY the server owner can use this function.
	*/
	UFUNCTION(BlueprintCallable, Category = "Access Control")
		static FString ChangeListMode(UMPHorsoGameInstance* GameInst, UPARAM(Ref) FAccessControlList& ListRef, FString RequesterID, bool Whitelist);
	
};

/**
 * 
 */
//UCLASS()
//class MPHORSO_API UAccessControlListTypes : public UObject
//{
//	GENERATED_BODY()
//	
//	
//	
//	
//};
