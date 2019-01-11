// Fill out your copyright notice in the Description page of Project Settings.

#include "MPHorso.h"
#include "PreConnectionBeaconClient.h"
#include "StaticFuncLib.h"
#include "MPHorsoGameInstance.h"
#include "MPHorsoSettingsSave.h"

APreConnectionBeaconClient::APreConnectionBeaconClient(const FObjectInitializer& ObjectInitializer) :
	Super(ObjectInitializer)
{
}

void APreConnectionBeaconClient::OnFailure()
{
	Super::OnFailure();

	OnConnectionFailed.Broadcast();
}

bool APreConnectionBeaconClient::Start(FString address, int32 port)
{
	FURL url(nullptr, *address, ETravelType::TRAVEL_Absolute);
	url.Port = port;

	return InitClient(url);
}

void APreConnectionBeaconClient::Ready_Implementation() { bConnectionReady = true; OnConnectionSucceeded.Broadcast(); }

void APreConnectionBeaconClient::Disconnect() { DestroyBeacon(); }

void APreConnectionBeaconClient::RequestServerData_Implementation(const FString& CharacterNameID)
{
	CharacterNameString = CharacterNameID;

	FServerInfoPacket NewPacket;

	NewPacket.CurrNumPlayers = GetWorld()->GetAuthGameMode()->GetNumPlayers();

	UMPHorsoGameInstance* gameInst = UStaticFuncLib::RetrieveGameInstance(this);

	if (nullptr != gameInst)
	{
		UWorldSaveBase* WorldSave = gameInst->GetWorldSave();

		if (nullptr != WorldSave)
		{
			NewPacket.WorldName = WorldSave->WorldName;
			NewPacket.WorldSettings = WorldSave->WorldSettings;

			NewPacket.RelevantACLData.Whitelisted = WorldSave->ServerData.ACL.WhitelistMode;
			NewPacket.MaxPlayers = WorldSave->ServerData.MaxPlayers;

			TMap<FString, FPlayerStandingData> Aggregate = gameInst->GetSettingsSave()->UniversalACL.PlayerStandingList;
			Aggregate.Append(WorldSave->ServerData.ACL.PlayerStandingList);

			FString PlayerID, CharacterID;

			{
				FString Addr = GetNetConnection()->LowLevelGetRemoteAddress(false);

				if (!Addr.IsEmpty())
				{
					PlayerID = UAccessControlListFuncLib::ConstructPlayerID(Addr);
					CharacterID = UAccessControlListFuncLib::ConstructCharacterID(Addr, CharacterNameString);
				}
			}

			FPlayerStandingData* RetrievedCharStanding = Aggregate.Find(CharacterID);
			FPlayerStandingData* RetrievedPlayerStanding = Aggregate.Find(PlayerID);

			NewPacket.RelevantACLData.RequesteeIsRegistered = (nullptr != RetrievedCharStanding || nullptr != RetrievedPlayerStanding);

			if (NewPacket.RelevantACLData.RequesteeIsRegistered)
			{
				{
					float CharTimeLeft = 0, PlayerTimeLeft = 0;

					if (nullptr != RetrievedCharStanding)
						CharTimeLeft = (RetrievedCharStanding->KickedUntil - FDateTime::UtcNow()).GetTotalSeconds();

					if (nullptr != RetrievedPlayerStanding)
						PlayerTimeLeft = (RetrievedPlayerStanding->KickedUntil - FDateTime::UtcNow()).GetTotalSeconds();

					NewPacket.RelevantACLData.KickTimeLeft = FMath::Max(CharTimeLeft, PlayerTimeLeft);
				}

				NewPacket.RelevantACLData.RequesteeIsBanned =
					((nullptr != RetrievedCharStanding && RetrievedCharStanding->PlayerStanding == EPlayerStanding::Banned) ||
						(nullptr != RetrievedPlayerStanding && RetrievedPlayerStanding->PlayerStanding == EPlayerStanding::Banned));
			}
		}

		NewPacket.RequiresPassword = !gameInst->ServerPassword.IsEmpty();
	}

	ReceiveServerData(NewPacket);
}

bool APreConnectionBeaconClient::RequestServerData_Validate(const FString& CharacterNameID) { return true; }

void APreConnectionBeaconClient::ReceiveServerData_Implementation(const FServerInfoPacket& ReceivedData)
{
	OnReceivedServerData.Broadcast(ReceivedData);
}

void APreConnectionBeaconClient::SendWorldSettingsUpdate()
{
	if (UKismetSystemLibrary::IsServer(this))
	{
		UMPHorsoGameInstance* gameInst = UStaticFuncLib::RetrieveGameInstance(this);

		if (nullptr != gameInst)
		{
			UWorldSaveBase* WorldSave = gameInst->GetWorldSave();

			if (nullptr != WorldSave)
				ReceiveWSUpdate(WorldSave->WorldSettings);
		}
	}
}

void APreConnectionBeaconClient::ReceiveWSUpdate_Implementation(const FWorldSettingsData& NewWorldSettings)
{
	OnNewWorldSettings.Broadcast(NewWorldSettings);
}

void APreConnectionBeaconClient::SendAccessControlUpdate()
{
	if (UKismetSystemLibrary::IsServer(this))
	{
		UMPHorsoGameInstance* gameInst = UStaticFuncLib::RetrieveGameInstance(this);

		if (nullptr != gameInst)
		{
			UWorldSaveBase* WorldSave = gameInst->GetWorldSave();

			if (nullptr != WorldSave)
			{
				TMap<FString, FPlayerStandingData> Aggregate = gameInst->GetSettingsSave()->UniversalACL.PlayerStandingList;
				Aggregate.Append(WorldSave->ServerData.ACL.PlayerStandingList);

				FString PlayerID, CharacterID;

				{
					FString Addr = GetNetConnection()->LowLevelGetRemoteAddress(false);

					if (!Addr.IsEmpty())
					{
						PlayerID = UAccessControlListFuncLib::ConstructPlayerID(Addr);
						CharacterID = UAccessControlListFuncLib::ConstructCharacterID(Addr, CharacterNameString);
					}
				}

				FPlayerStandingData* RetrievedCharStanding = Aggregate.Find(CharacterID);
				FPlayerStandingData* RetrievedPlayerStanding = Aggregate.Find(PlayerID);

				FACLUpdateData NewACLUpdate;
				NewACLUpdate.Whitelisted = WorldSave->ServerData.ACL.WhitelistMode;

				NewACLUpdate.RequesteeIsRegistered = (nullptr != RetrievedCharStanding || nullptr != RetrievedPlayerStanding);

				if (NewACLUpdate.RequesteeIsRegistered)
				{
					{
						float CharTimeLeft = 0, PlayerTimeLeft = 0;

						if (nullptr != RetrievedCharStanding)
							CharTimeLeft = (RetrievedCharStanding->KickedUntil - FDateTime::UtcNow()).GetTotalSeconds();

						if (nullptr != RetrievedPlayerStanding)
							PlayerTimeLeft = (RetrievedPlayerStanding->KickedUntil - FDateTime::UtcNow()).GetTotalSeconds();

						NewACLUpdate.KickTimeLeft = FMath::Max(CharTimeLeft, PlayerTimeLeft);
					}

					NewACLUpdate.RequesteeIsBanned =
						((nullptr != RetrievedCharStanding && RetrievedCharStanding->PlayerStanding == EPlayerStanding::Banned) ||
						(nullptr != RetrievedPlayerStanding && RetrievedPlayerStanding->PlayerStanding == EPlayerStanding::Banned));
				}

				ReceiveACLUpdate(NewACLUpdate);
			}
		}
	}
}

void APreConnectionBeaconClient::ReceiveACLUpdate_Implementation(const FACLUpdateData& NewACL)
{
	OnNewACL.Broadcast(NewACL);
}

void APreConnectionBeaconClient::SendPlayerCountsUpdate()
{
	if (UKismetSystemLibrary::IsServer(this))
	{
		UMPHorsoGameInstance* gameInst = UStaticFuncLib::RetrieveGameInstance(this);

		if (nullptr != gameInst)
		{
			UWorldSaveBase* WorldSave = gameInst->GetWorldSave();

			if (nullptr != WorldSave)
			{
				int CurrNumPlayers = GetWorld()->GetAuthGameMode()->GetNumPlayers();
				int MaxNumPlayers = WorldSave->ServerData.MaxPlayers;

				ReceivePlayerCountsUpdate(CurrNumPlayers, MaxNumPlayers);
			}
		}
	}
}

void APreConnectionBeaconClient::ReceivePlayerCountsUpdate_Implementation(int NewCurrPlayerCount, int NewMaxPlayerCount)
{
	OnNewPlayerCounts.Broadcast(NewCurrPlayerCount, NewMaxPlayerCount);
}

void APreConnectionBeaconClient::SendPasswordRequirementUpdate()
{
	if (UKismetSystemLibrary::IsServer(this))
	{
		UMPHorsoGameInstance* gameInst = UStaticFuncLib::RetrieveGameInstance(this);

		if (nullptr != gameInst)
		{
			ReceivePasswordRequirementUpdate(!gameInst->ServerPassword.IsEmpty());
		}
	}
}

void APreConnectionBeaconClient::ReceivePasswordRequirementUpdate_Implementation(bool NewPassRequirement)
{
	OnNewPasswordRequirement.Broadcast(NewPassRequirement);
}

void APreConnectionBeaconClient::RequestAccess_Implementation(const FString& PasswordGiven)
{
	UMPHorsoGameInstance* gameInst = UStaticFuncLib::RetrieveGameInstance(this);

	if (nullptr != gameInst)
	{
		FString PlayerID, CharacterID;

		{
			FString Addr = GetNetConnection()->LowLevelGetRemoteAddress(false);

			if (!Addr.IsEmpty())
			{
				PlayerID = UAccessControlListFuncLib::ConstructPlayerID(Addr);
				CharacterID = UAccessControlListFuncLib::ConstructCharacterID(Addr, CharacterNameString);
			}
		}

		TMap<FString, FPlayerStandingData> Aggregate = gameInst->GetSettingsSave()->UniversalACL.PlayerStandingList;
		
		UWorldSaveBase* WorldSave = gameInst->GetWorldSave();
		if (nullptr != WorldSave)
		{
			Aggregate.Append(WorldSave->ServerData.ACL.PlayerStandingList);

			FPlayerStandingData* RequesteeCharData = Aggregate.Find(CharacterID);
			FPlayerStandingData* RequesteePlayerData = Aggregate.Find(PlayerID);

			int NumCurrPlayers = GetWorld()->GetAuthGameMode()->GetNumPlayers();

			FString DeniedString;

			{
				auto IDIsBanned = [](FPlayerStandingData* data) { return (nullptr != data && data->PlayerStanding == EPlayerStanding::Banned); };
				auto IDIsKicked = [](FPlayerStandingData* data) { return (nullptr != data && FDateTime::UtcNow() < data->KickedUntil); };
				bool EitherDataValid = (nullptr != RequesteeCharData || nullptr != RequesteePlayerData);

				if (!EitherDataValid && WorldSave->ServerData.ACL.WhitelistMode)
					DeniedString = "The server is in whitelist mode, and you aren't in the player registry.";
				else if (IDIsBanned(RequesteeCharData) || IDIsBanned(RequesteePlayerData))
					DeniedString = "You are currently banned from this server.";
				else if (IDIsKicked(RequesteeCharData) || IDIsKicked(RequesteePlayerData))
					DeniedString = "You are currently kicked from this server.";
				else if (NumCurrPlayers >= WorldSave->ServerData.MaxPlayers)
					DeniedString = "The server is currently full.";
				else if (!gameInst->ServerPassword.IsEmpty() && gameInst->ServerPassword != PasswordGiven)
					DeniedString = "Wrong password.";
			}

			if (!DeniedString.IsEmpty())
				SendAccessDenied(DeniedString);
			else
			{
				// Register player
				WorldSave->ServerData.ACL.RegisterNewPlayer(CharacterID);
				//WorldSave->ServerData.ACL.RegisterNewPlayer(PlayerID); // TODO: Only generate PlayerID when necessary, i.e. banning, demoting, etc.
				WorldSave->RegisterNewPlayerInWorld(CharacterID);

				SendAccessGranted();
			}
		}
	}
}

bool APreConnectionBeaconClient::RequestAccess_Validate(const FString& PasswordGiven) { return true; }

void APreConnectionBeaconClient::SendAccessDenied_Implementation(const FString& DeniedMessage) { OnDeniedAccess.Broadcast(DeniedMessage); }

void APreConnectionBeaconClient::SendAccessGranted_Implementation() { OnGrantedAccess.Broadcast(); }

void APreConnectionBeaconClient::RequestWhitelistEntry_Implementation()
{
	// TODO
}

bool APreConnectionBeaconClient::RequestWhitelistEntry_Validate() { return true; }

void APreConnectionBeaconClient::TravelToConnectedServer()
{
	UGameplayStatics::OpenLevel(this, *GetNetConnection()->URL.Host);
}
