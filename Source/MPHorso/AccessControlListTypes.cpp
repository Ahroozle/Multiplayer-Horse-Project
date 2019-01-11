// Fill out your copyright notice in the Description page of Project Settings.

#include "MPHorso.h"
#include "AccessControlListTypes.h"
#include "MPHorsoGameInstance.h"


int UAccessControlListFuncLib::CompareModTiers(const FAccessControlList& ListRef, FName ModTierA, FName ModTierB)
{
	int TierA = ListRef.ModTiers.IndexOfByPredicate([&ModTierA](const FModTierPermissions& a) { return a.ModTierName == ModTierA; });
	int TierB = ListRef.ModTiers.IndexOfByPredicate([&ModTierB](const FModTierPermissions& b) { return b.ModTierName == ModTierB; });

	return (TierA > TierB ? 1 : (TierA == TierB ? 0 : -1));
}

bool UAccessControlListFuncLib::NeedToRequestAccess(const FAccessControlList& ListRef, FString UserID)
{
	return ListRef.WhitelistMode && !ListRef.PlayerStandingList.Contains(UserID);
}

bool UAccessControlListFuncLib::IsBanned(const FAccessControlList& ListRef, FString UserID)
{
	const FPlayerStandingData* UserData = ListRef.PlayerStandingList.Find(UserID);

	return nullptr != UserData && UserData->PlayerStanding == EPlayerStanding::Banned;
}

bool UAccessControlListFuncLib::IsKicked(const FAccessControlList& ListRef, FString UserID)
{
	const FPlayerStandingData* UserData = ListRef.PlayerStandingList.Find(UserID);

	return nullptr != UserData && FDateTime::UtcNow() < UserData->KickedUntil;
}

bool UAccessControlListFuncLib::IsModerator(UMPHorsoGameInstance* GameInst, const FAccessControlList& ListRef, FString UserID)
{
	const FPlayerStandingData* UserData = ListRef.PlayerStandingList.Find(UserID);

	return (nullptr != GameInst && GameInst->UserIsHost(UserID)) ||
		(nullptr != UserData && UserData->PlayerStanding == EPlayerStanding::Moderator);
}

bool UAccessControlListFuncLib::IsHigherPrivilegeThan(UMPHorsoGameInstance* GameInst, const FAccessControlList& ListRef, FString RequesterID, FString TargetID)
{
	if (nullptr != GameInst)
	{
		if (GameInst->UserIsHost(RequesterID))
			return true;
		else if (GameInst->UserIsHost(TargetID))
			return false;
	}

	const FPlayerStandingData* RequesterData = ListRef.PlayerStandingList.Find(RequesterID);
	const FPlayerStandingData* TargetData = ListRef.PlayerStandingList.Find(TargetID);

	if (nullptr != RequesterData && RequesterData->PlayerStanding == EPlayerStanding::Moderator)
	{
		return nullptr == TargetData ||
			RequesterData->PlayerStanding > TargetData->PlayerStanding ||
			CompareModTiers(ListRef, RequesterData->ModTierName, TargetData->ModTierName) == 1;
	}

	return false;
}

FString UAccessControlListFuncLib::RegisterPlayer(UMPHorsoGameInstance* GameInst, UPARAM(Ref) FAccessControlList& ListRef, FString RequesterID, FString RegisterID)
{
	if (IsModerator(GameInst, ListRef, RequesterID))
	{
		if (!ListRef.PlayerStandingList.Contains(RegisterID))
		{
			ListRef.PlayerStandingList.Add(RegisterID, { EPlayerStanding::Player, FDateTime(0), FName() });

			return "";
		}
		else
			return "ID has already been registered.";
	}
	else
		return "You are not a moderator.";
}

FString UAccessControlListFuncLib::KickPlayer(UMPHorsoGameInstance* GameInst, UPARAM(Ref) FAccessControlList& ListRef, FString RequesterID, FString KickID, int DurationSeconds)
{
	if (IsHigherPrivilegeThan(GameInst, ListRef, RequesterID, KickID))
	{
		FPlayerStandingData* TargetData = ListRef.PlayerStandingList.Find(KickID);

		if (nullptr != TargetData)
		{
			TargetData->KickedUntil = FDateTime::UtcNow() + FTimespan::FromSeconds(DurationSeconds);

			return "";
		}
		else
			return "Target ID is not registered.";
	}
	else
		return "You cannot kick someone with higher or equal privilege to you.";
}

FString UAccessControlListFuncLib::UnkickPlayer(UMPHorsoGameInstance* GameInst, UPARAM(Ref) FAccessControlList& ListRef, FString RequesterID, FString KickedID)
{
	if (IsHigherPrivilegeThan(GameInst, ListRef, RequesterID, KickedID))
	{
		FPlayerStandingData* TargetData = ListRef.PlayerStandingList.Find(KickedID);

		if (nullptr != TargetData)
		{
			TargetData->KickedUntil = FDateTime(0);

			return "";
		}
		else
			return "Target ID is not registered.";
	}
	else
		return "You cannot unkick someone with higher or equal privilege to you.";
}

FString UAccessControlListFuncLib::BanPlayer(UMPHorsoGameInstance* GameInst, UPARAM(Ref) FAccessControlList& ListRef, FString RequesterID, FString BanID)
{
	if (IsHigherPrivilegeThan(GameInst, ListRef, RequesterID, BanID))
	{
		FPlayerStandingData* TargetData = ListRef.PlayerStandingList.Find(BanID);

		if (nullptr != TargetData)
		{
			TargetData->PlayerStanding = EPlayerStanding::Banned;
			TargetData->ModTierName = FName();

			return "";
		}
		else
			return "Target ID is not registered.";
	}
	else
		return "You cannot ban someone with higher or equal privilege to you.";
}

FString UAccessControlListFuncLib::UnbanPlayer(UMPHorsoGameInstance* GameInst, UPARAM(Ref) FAccessControlList& ListRef, FString RequesterID, FString BannedID)
{
	if (IsHigherPrivilegeThan(GameInst, ListRef, RequesterID, BannedID))
	{
		FPlayerStandingData* TargetData = ListRef.PlayerStandingList.Find(BannedID);

		if (nullptr != TargetData)
		{
			TargetData->PlayerStanding = EPlayerStanding::Player;

			return "";
		}
		else
			return "Target ID is not registered.";
	}
	else
		return "You cannot unban someone with higher or equal privilege to you.";
}

FString UAccessControlListFuncLib::PromotePlayer(UMPHorsoGameInstance* GameInst, UPARAM(Ref) FAccessControlList& ListRef, FString RequesterID, FString PromoteID, FName ModTierName)
{
	if (IsHigherPrivilegeThan(GameInst, ListRef, RequesterID, PromoteID))
	{
		FPlayerStandingData* TargetData = ListRef.PlayerStandingList.Find(PromoteID);

		if (nullptr != TargetData)
		{
			FPlayerStandingData* RequesterData = ListRef.PlayerStandingList.Find(RequesterID);
			if (ModTierName.IsNone())
			{
				if (ListRef.ModTiers.Num() < 1)
				{
					if (nullptr != GameInst && GameInst->UserIsHost(RequesterID))
					{
						TargetData->PlayerStanding = EPlayerStanding::Moderator;
						return "";
					}
					else
						return "You are not the server owner.";
				}
				else
					return "Tier \'None\' is invalid. Please choose a tier from the current list of tiers.";
			}
			else
			{
				int Ind =
					ListRef.ModTiers.IndexOfByPredicate([&ModTierName](const FModTierPermissions& a) { return a.ModTierName == ModTierName; });

				if (Ind != INDEX_NONE)
				{
					if ((nullptr != GameInst && GameInst->UserIsHost(RequesterID)) || CompareModTiers(ListRef, RequesterData->ModTierName, ModTierName) == 1)
					{
						TargetData->PlayerStanding = EPlayerStanding::Moderator;
						TargetData->ModTierName = ModTierName;

						return "";
					}
					else
						return "You cannot assign someone to a higher or equal tier to you.";
				}
				else
					return "Tier \'" + ModTierName.ToString() + "\' was not found.";
			}
		}
		else
			return "Target ID is not registered.";
	}
	else
		return "You cannot promote someone with higher or equal privilege to you.";
}

FString UAccessControlListFuncLib::DemotePlayer(UMPHorsoGameInstance* GameInst, UPARAM(Ref) FAccessControlList& ListRef, FString RequesterID, FString DemoteID, FName ModTierName)
{
	if (IsHigherPrivilegeThan(GameInst, ListRef, RequesterID, DemoteID))
	{
		FPlayerStandingData* TargetData = ListRef.PlayerStandingList.Find(DemoteID);

		if (nullptr != TargetData)
		{
			if (TargetData->PlayerStanding == EPlayerStanding::Moderator)
			{
				if (ModTierName.IsNone())
				{
					TargetData->PlayerStanding = EPlayerStanding::Player;
					TargetData->ModTierName = FName();

					return "";
				}
				else
				{
					FPlayerStandingData* RequesterData = ListRef.PlayerStandingList.Find(RequesterID);

					int Ind =
						ListRef.ModTiers.IndexOfByPredicate([&ModTierName](const FModTierPermissions& a) { return a.ModTierName == ModTierName; });

					if (Ind != INDEX_NONE)
					{
						if ((nullptr != GameInst && GameInst->UserIsHost(RequesterID)) || CompareModTiers(ListRef, RequesterData->ModTierName, ModTierName) == 1)
						{
							TargetData->PlayerStanding = EPlayerStanding::Moderator;
							TargetData->ModTierName = ModTierName;

							return "";
						}
						else
							return "You cannot demote someone to a higher or equal tier to you.";
					}
					else
						return "Tier \'" + ModTierName.ToString() + "\' was not found.";
				}
			}
			else
				return "You cannot demote someone who is not a moderator.";
		}
		else
			return "Target ID is not registered.";
	}
	else
		return "You cannot demote someone with higher or equal privilege to you.";
}

FString UAccessControlListFuncLib::MakeModTier(UMPHorsoGameInstance* GameInst, UPARAM(Ref) FAccessControlList& ListRef, FString RequesterID, FName TierName, const TArray<FString>& ModPerms, int InsertIndex)
{
	if (IsModerator(GameInst, ListRef, RequesterID))
	{
		FPlayerStandingData* RequesterData = ListRef.PlayerStandingList.Find(RequesterID);

		if (ListRef.ModTiers.Num() < 1)
		{
			if (nullptr != GameInst && GameInst->UserIsHost(RequesterID))
			{
				ListRef.ModTiers.Add({ TierName, TSet<FString>{ ModPerms } });

				return "";
			}
			else
				return "You are not the server owner.";
		}
		else
		{
			if (nullptr != GameInst && GameInst->UserIsHost(RequesterID))
			{
				if (InsertIndex >= ListRef.ModTiers.Num())
					ListRef.ModTiers.Add({ TierName, TSet<FString>{ ModPerms } });
				else if (InsertIndex > -1)
					ListRef.ModTiers.Insert({ TierName, TSet<FString>{ ModPerms } }, InsertIndex);
				else
					return "Mod tier insert index is invalid.";

				return "";
			}
			else
			{
				int RequesteeTierInd =
					ListRef.ModTiers.IndexOfByPredicate([&RequesterData](const FModTierPermissions& a) { return a.ModTierName == RequesterData->ModTierName; });

				if (InsertIndex > RequesteeTierInd)
				{
					if (InsertIndex >= ListRef.ModTiers.Num())
						ListRef.ModTiers.Add({ TierName, TSet<FString>{ ModPerms } });
					else if (InsertIndex > -1)
						ListRef.ModTiers.Insert({ TierName, TSet<FString>{ ModPerms } }, InsertIndex);
					else
						return "Mod tier insert index is invalid.";

					return "";
				}
				else
					return "You cannot create a tier higher or equal to your own.";
			}
		}
	}
	else
		return "You are not a moderator.";
}

FString UAccessControlListFuncLib::RemoveModTier(UMPHorsoGameInstance* GameInst, UPARAM(Ref) FAccessControlList& ListRef, FString RequesterID, FName TierName, FName MoveTierName)
{
	if (IsModerator(GameInst, ListRef, RequesterID))
	{
		if (ListRef.ModTiers.Num() > 0)
		{
			FPlayerStandingData* RequesterData = ListRef.PlayerStandingList.Find(RequesterID);

			if (ListRef.ModTiers.ContainsByPredicate([&TierName](const FModTierPermissions& a) {return a.ModTierName == TierName; }))
			{
				if (MoveTierName.IsNone() || ListRef.ModTiers.ContainsByPredicate([&MoveTierName](const FModTierPermissions& a) {return a.ModTierName == MoveTierName; }))
				{
					if (nullptr != GameInst && GameInst->UserIsHost(RequesterID))
					{
						ListRef.ModTiers.RemoveAll([&TierName](const FModTierPermissions& a) {return a.ModTierName == TierName; });

						for (auto &currPair : ListRef.PlayerStandingList)
						{
							if (currPair.Value.PlayerStanding == EPlayerStanding::Moderator && currPair.Value.ModTierName == TierName)
							{
								if (MoveTierName.IsNone())
								{
									currPair.Value.PlayerStanding = EPlayerStanding::Player;
									currPair.Value.ModTierName = FName();
								}
								else
									currPair.Value.ModTierName = MoveTierName;
							}
						}

						return "";
					}
					else if (CompareModTiers(ListRef, RequesterData->ModTierName, TierName) == 1)
					{
						if (CompareModTiers(ListRef, RequesterData->ModTierName, MoveTierName) == 1)
						{
							ListRef.ModTiers.RemoveAll([&TierName](const FModTierPermissions& a) {return a.ModTierName == TierName; });

							for (auto &currPair : ListRef.PlayerStandingList)
							{
								if (currPair.Value.PlayerStanding == EPlayerStanding::Moderator && currPair.Value.ModTierName == TierName)
								{
									if (MoveTierName.IsNone())
									{
										currPair.Value.PlayerStanding = EPlayerStanding::Player;
										currPair.Value.ModTierName = FName();
									}
									else
										currPair.Value.ModTierName = MoveTierName;
								}
							}

							return "";
						}
						else
							return "You cannot move members of a removed tier to a tier higher or equal to yours.";
					}
					else
						return "You cannot remove a tier higher or equal to yours.";
				}
				else
					return "Tier \'" + MoveTierName.ToString() + "\' was not found.";
			}
			else
				return "Tier \'" + TierName.ToString() + "\' was not found.";
		}
		else
			return "The tier list is empty; i.e. there are no tiers to remove.";
	}
	else
		return "You are not a moderator.";
}

FString UAccessControlListFuncLib::ChangeListMode(UMPHorsoGameInstance* GameInst, UPARAM(Ref) FAccessControlList& ListRef, FString RequesterID, bool Whitelist)
{
	if (nullptr != GameInst && GameInst->UserIsHost(RequesterID))
	{
		ListRef.WhitelistMode = Whitelist;

		return "";
	}
	else
		return "You are not the owner.";
}
