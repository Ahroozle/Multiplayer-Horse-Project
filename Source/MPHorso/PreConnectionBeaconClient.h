// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "OnlineBeaconClient.h"
#include "MPHorsoSaveGameTypes.h"
#include "PreConnectionBeaconClient.generated.h"


USTRUCT(BlueprintType)
struct FACLUpdateData
{
	GENERATED_USTRUCT_BODY();

	UPROPERTY(BlueprintReadWrite)
		bool Whitelisted;

	UPROPERTY(BlueprintReadWrite)
		bool RequesteeIsRegistered;

	UPROPERTY(BlueprintReadWrite)
		bool RequesteeIsBanned = false;

	UPROPERTY(BlueprintReadWrite)
		float KickTimeLeft = 0.0f;
};

USTRUCT(BlueprintType)
struct FServerInfoPacket
{
	GENERATED_USTRUCT_BODY();

	UPROPERTY(BlueprintReadWrite)
		FString WorldName;

	UPROPERTY(BlueprintReadWrite)
		FWorldSettingsData WorldSettings;

	UPROPERTY(BlueprintReadWrite)
		FACLUpdateData RelevantACLData;

	UPROPERTY(BlueprintReadWrite)
		int CurrNumPlayers;

	UPROPERTY(BlueprintReadWrite)
		int MaxPlayers;

	UPROPERTY(BlueprintReadWrite)
		bool RequiresPassword;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnBeaconFailure);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnBeaconReady);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnServerDataReceived, const FServerInfoPacket&, ReceivedData);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWorldSettingsChanged, const FWorldSettingsData&, NewWorldSettings);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnACLChanged, const FACLUpdateData&, NewACL);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPlayerCountsChanged, int, NewCurrPlayerCount, int, NewMaxPlayerCount);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPassRequirementChanged, bool, NewPasswordRequirement);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPasswordRequested);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAccessDenied, const FString&, DeniedMessage);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAccessGranted);


/**
 * 
 */
UCLASS(BlueprintType)
class MPHORSO_API APreConnectionBeaconClient : public AOnlineBeaconClient
{
	GENERATED_BODY()
	
public:

	APreConnectionBeaconClient(const FObjectInitializer& ObjectInitializer);


	UPROPERTY(BlueprintReadOnly)
		bool bConnectionReady = false;

	UPROPERTY(BlueprintAssignable, BlueprintCallable)
		FOnBeaconFailure OnConnectionFailed;

	UPROPERTY(BlueprintAssignable, BlueprintCallable)
		FOnBeaconReady OnConnectionSucceeded;

	UPROPERTY(BlueprintAssignable, BlueprintCallable)
		FOnServerDataReceived OnReceivedServerData;


	UPROPERTY(BlueprintAssignable, BlueprintCallable)
		FOnWorldSettingsChanged OnNewWorldSettings;

	UPROPERTY(BlueprintAssignable, BlueprintCallable)
		FOnACLChanged OnNewACL;

	UPROPERTY(BlueprintAssignable, BlueprintCallable)
		FOnPlayerCountsChanged OnNewPlayerCounts;

	UPROPERTY(BlueprintAssignable, BlueprintCallable)
		FOnPassRequirementChanged OnNewPasswordRequirement;


	UPROPERTY(BlueprintAssignable, BlueprintCallable)
		FOnPasswordRequested OnNeedPassword;

	UPROPERTY(BlueprintAssignable, BlueprintCallable)
		FOnAccessDenied OnDeniedAccess;

	UPROPERTY(BlueprintAssignable, BlueprintCallable)
		FOnAccessGranted OnGrantedAccess;

	// This is initialized serverside for checking ops
	UPROPERTY()
		FString CharacterNameString;


	//~ Begin AOnlineBeaconClient Interface
	virtual void OnFailure() override;
	//~ End AOnlineBeaconClient Interface

	UFUNCTION(BlueprintCallable)
		bool Start(FString address, int32 port);
	
	UFUNCTION(Client, Reliable)
		void Ready();

	UFUNCTION(BlueprintCallable)
		void Disconnect();


	UFUNCTION(BlueprintCallable, Server, Reliable, WithValidation)
		void RequestServerData(const FString& CharacterNameID);

	UFUNCTION(Client, Reliable)
		void ReceiveServerData(const FServerInfoPacket& ReceivedData);


	UFUNCTION(BlueprintCallable)
		void SendWorldSettingsUpdate();

	UFUNCTION(Client, Reliable)
		void ReceiveWSUpdate(const FWorldSettingsData& NewWorldSettings);

	UFUNCTION(BlueprintCallable)
		void SendAccessControlUpdate();

	UFUNCTION(Client, Reliable)
		void ReceiveACLUpdate(const FACLUpdateData& NewACL);

	UFUNCTION(BlueprintCallable)
		void SendPlayerCountsUpdate();

	UFUNCTION(Client, Reliable)
		void ReceivePlayerCountsUpdate(int NewCurrPlayerCount, int NewMaxPlayerCount);

	UFUNCTION(BlueprintCallable)
		void SendPasswordRequirementUpdate();

	UFUNCTION(Client, Reliable)
		void ReceivePasswordRequirementUpdate(bool NewPassRequirement);


	UFUNCTION(BlueprintCallable, Server, Reliable, WithValidation)
		void RequestAccess(const FString& PasswordGiven);


	UFUNCTION(Client, Reliable)
		void SendAccessDenied(const FString& DeniedMessage);

	UFUNCTION(Client, Reliable)
		void SendAccessGranted();


	UFUNCTION(BlueprintCallable, Server, Reliable, WithValidation)
		void RequestWhitelistEntry();

	UFUNCTION(BlueprintCallable)
		void TravelToConnectedServer();
};
