// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "Networking.h"
#include "MPHorsoNetworkingTypes.generated.h"

UENUM(BlueprintType)
enum class EHorseDataFlags : uint8
{
	// Just plain data with no specific purpose
	HorseData_RawData		UMETA(DisplayName = "Raw Data"),

	HorseData_JoinRequest	UMETA(DisplayName = "Join Request")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSocketInitFailedNotify, FString, ErrorString);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FSocketReadyNotify);

UCLASS(BlueprintType)
class MPHORSO_API AMPHorsoUDPSender : public AActor
{
	GENERATED_BODY()

public:

	FString ChosenSocketName;
	int ChosenPort;

	TSharedPtr<FInternetAddr> RemoteAddress;
	FSocket* Socket = nullptr;

	FResolveInfo* ResolveInfo = nullptr;
	FTimerHandle DomainResolutionTimerHandle;

	FString ErrorStr;

	UPROPERTY(BlueprintAssignable, Category = "UDP Sender")
		FSocketInitFailedNotify SocketInitFailedDelegate;

	UPROPERTY(BlueprintAssignable, Category = "UDP Sender")
		FSocketReadyNotify SocketInitSucceededDelegate;


	UFUNCTION(BlueprintCallable, Category = "UDP Sender")
		void InitializeSender(const FString& SocketName, const FString& IP_or_Domain, const int Port);

	UFUNCTION()
		void WaitForDomainResolution();

	UFUNCTION()
		void CreateSocket();

	UFUNCTION(BlueprintCallable, Category = "UDP Sender")
		bool SendData(EHorseDataFlags DataFlag, const TArray<uint8>& DataBytes);

	UFUNCTION(BlueprintCallable, Category = "UDP Sender")
		void Reset();

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
};


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDataReceivedNotify, const TArray<uint8>&, ReceivedData);

UCLASS(BlueprintType)
class MPHORSO_API AMPHorsoUDPReceiver : public AActor
{
	GENERATED_BODY()

public:

	FString ChosenSocketName;
	int ChosenPort;

	TSharedPtr<FInternetAddr> RemoteAddress;
	FSocket* Socket;
	FUdpSocketReceiver* UDPReceiver = nullptr;

	FResolveInfo* ResolveInfo = nullptr;
	FTimerHandle DomainResolutionTimerHandle;

	FString ErrorStr;

	UPROPERTY(BlueprintAssignable, Category = "UDP Sender")
		FSocketInitFailedNotify SocketInitFailedDelegate;

	UPROPERTY(BlueprintAssignable, Category = "UDP Sender")
		FSocketReadyNotify SocketInitSucceededDelegate;

	UPROPERTY(BlueprintAssignable, Category = "UDP Sender")
		FDataReceivedNotify DataReceivedDelegate;


	UFUNCTION(BlueprintCallable, Category = "UDP Receiver")
		void InitializeReceiver(const FString& SocketName, const FString& IP_or_Domain, const int Port);

	UFUNCTION()
		void WaitForDomainResolution();

	UFUNCTION()
		void CreateSocket();

	void OnDataReceived(const FArrayReaderPtr& ArrayReaderPtr, const FIPv4Endpoint& EndPt);

	UFUNCTION(BlueprintCallable, Category = "UDP Receiver")
		void Reset();

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
};


UCLASS()
class MPHORSO_API UMPHorsoNetLibrary : public UObject
{
	GENERATED_BODY()

public:

	static uint16 DefaultPort;

	UFUNCTION(BlueprintPure)
		static int32 GetDefaultPort() { return DefaultPort; }

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Format IPv4 String Into Numbers"))
		static bool FormatIPv4(const FString& IPString, TArray<uint8>& OutParts, FString& OutErrorString);

	UFUNCTION(BlueprintPure)
		static void StringToBytes(FString InString, TArray<uint8>& OutBytes);


	UFUNCTION(BlueprintPure)
		static void StringFromBytes(const TArray<uint8>& InBytes, FString& OutString);

private:
};
