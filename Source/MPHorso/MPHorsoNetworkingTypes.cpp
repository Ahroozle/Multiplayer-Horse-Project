// Fill out your copyright notice in the Description page of Project Settings.

#include "MPHorso.h"
#include "MPHorsoNetworkingTypes.h"

#include "StaticFuncLib.h"

#include "Kismet/GameplayStatics.h"


void AMPHorsoUDPSender::InitializeSender(const FString& SocketName, const FString& IP_or_Domain, const int Port)
{
	ChosenSocketName = SocketName;
	ChosenPort = (Port <= 0 ? UMPHorsoNetLibrary::DefaultPort : Port);

	TArray<uint8> IPParts;
	if (!UMPHorsoNetLibrary::FormatIPv4(IP_or_Domain, IPParts, ErrorStr))
	{
		ResolveInfo = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->GetHostByName(TCHAR_TO_ANSI(*IP_or_Domain));

		GetWorldTimerManager().SetTimer(DomainResolutionTimerHandle, this, &AMPHorsoUDPSender::WaitForDomainResolution, 0.01f, true);
	}
	else
	{
		RemoteAddress = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();
		bool validDummy;
		RemoteAddress->SetIp(*IP_or_Domain, validDummy);
		RemoteAddress->SetPort(Port);

		CreateSocket();
	}
}

void AMPHorsoUDPSender::WaitForDomainResolution()
{
	if (ResolveInfo->IsComplete())
	{
		GetWorldTimerManager().ClearTimer(DomainResolutionTimerHandle);

		int ErrorCode = ResolveInfo->GetErrorCode();

		if (ErrorCode == 0)
		{
			const FInternetAddr& ResolvedAddr = ResolveInfo->GetResolvedAddress();

			uint32 RetrievedIP;
			ResolvedAddr.GetIp(RetrievedIP);

			RemoteAddress = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();
			RemoteAddress->SetIp(RetrievedIP);
			RemoteAddress->SetPort(ChosenPort);

			CreateSocket();
		}
		else
		{
			// error code should already be in socketerror format iirc
			ErrorStr = FString("Error Connecting To Server: ") +
					   ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->GetSocketError((ESocketErrors)ErrorCode);

			SocketInitFailedDelegate.Broadcast(ErrorStr);
		}
	}
}

void AMPHorsoUDPSender::CreateSocket()
{
	Socket = FUdpSocketBuilder(*ChosenSocketName).AsReusable();// .WithBroadcast();

	if (nullptr != Socket)
	{
		int sendsize = 2048 * 1024;

		Socket->SetSendBufferSize(sendsize, sendsize);
		Socket->SetReceiveBufferSize(sendsize, sendsize);

		SocketInitSucceededDelegate.Broadcast();
	}
	else
		SocketInitFailedDelegate.Broadcast("Failed to create the socket!");
}

bool AMPHorsoUDPSender::SendData(UPARAM(Ref) FHorseNetData& Data)
{
	if (nullptr != Socket)
	{
		FArrayWriter Writer;
		Writer << Data;

		int SentBytes = 0;
		Socket->SendTo(Writer.GetData(), Writer.Num(), SentBytes, *RemoteAddress);

		if (0 == SentBytes)
			UStaticFuncLib::Print("Socket is valid, but no bytes were sent; It may not be listening properly.", true);
		else
			return true;
	}
	else
		UStaticFuncLib::Print("Socket is null! Please call InitializeSender() first.", true);

	return false;
}

bool AMPHorsoUDPSender::SendDataTo(UPARAM(Ref) FHorseNetData& Data, const FString& ToIP)
{
	if (nullptr != Socket)
	{
		FArrayWriter Writer;
		Writer << Data;

		TSharedPtr<FInternetAddr> ToAddress = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();
		bool validDummy;
		ToAddress->SetIp(*ToIP, validDummy);
		ToAddress->SetPort(UMPHorsoNetLibrary::DefaultPort);

		int SentBytes = 0;
		Socket->SendTo(Writer.GetData(), Writer.Num(), SentBytes, *ToAddress);

		if (0 == SentBytes)
			UStaticFuncLib::Print("Socket is valid, but no bytes were sent; It may not be listening properly.", true);
		else
			return true;
	}
	else
		UStaticFuncLib::Print("Socket is null! Please call InitializeSender() first.", true);

	return false;
}

void AMPHorsoUDPSender::Reset()
{
	if (nullptr != Socket)
	{
		Socket->Close();
		ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(Socket);
		Socket = nullptr;
	}
}

void AMPHorsoUDPSender::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	Reset();
}


void AMPHorsoUDPReceiver::InitializeReceiver(const FString& SocketName, const FString& IP_or_Domain, const int Port)
{
	ChosenSocketName = SocketName;
	ChosenPort = (Port <= 0 ? UMPHorsoNetLibrary::DefaultPort : Port);

	TArray<uint8> IPParts;
	if (!UMPHorsoNetLibrary::FormatIPv4(IP_or_Domain, IPParts, ErrorStr))
	{
		ResolveInfo = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->GetHostByName(TCHAR_TO_ANSI(*IP_or_Domain));

		GetWorldTimerManager().SetTimer(DomainResolutionTimerHandle, this, &AMPHorsoUDPReceiver::WaitForDomainResolution, 0.01f, true);
	}
	else
	{
		RemoteAddress = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();
		bool validDummy;
		RemoteAddress->SetIp(*IP_or_Domain, validDummy);
		RemoteAddress->SetPort(Port);

		CreateSocket();
	}
}

void AMPHorsoUDPReceiver::WaitForDomainResolution()
{
	if (ResolveInfo->IsComplete())
	{
		GetWorldTimerManager().ClearTimer(DomainResolutionTimerHandle);

		int ErrorCode = ResolveInfo->GetErrorCode();

		if (ErrorCode == 0)
		{
			const FInternetAddr& ResolvedAddr = ResolveInfo->GetResolvedAddress();

			uint32 RetrievedIP;
			ResolvedAddr.GetIp(RetrievedIP);

			RemoteAddress = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();
			RemoteAddress->SetIp(RetrievedIP);
			RemoteAddress->SetPort(ChosenPort);

			CreateSocket();
		}
		else
		{
			// error code should already be in socketerror format iirc
			ErrorStr = FString("Error Connecting To Server: ") +
				ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->GetSocketError((ESocketErrors)ErrorCode);

			SocketInitFailedDelegate.Broadcast(ErrorStr);
		}
	}
}

void AMPHorsoUDPReceiver::CreateSocket()
{
	FIPv4Endpoint EndPt(RemoteAddress);

	int buffsize = 2048 * 1024;
	Socket = FUdpSocketBuilder(*ChosenSocketName).AsNonBlocking().AsReusable().BoundToEndpoint(EndPt).WithReceiveBufferSize(buffsize);

	if (nullptr != Socket)
	{
		GetWorldTimerManager().SetTimer(ListeningTimerHandle, this, &AMPHorsoUDPReceiver::ListenForData, 0.01f, true);

		SocketInitSucceededDelegate.Broadcast();
	}
	else
		SocketInitFailedDelegate.Broadcast("Failed to create the socket!");
}

void AMPHorsoUDPReceiver::ListenForData()
{
	uint32 DataSize;
	if (Socket->HasPendingData(DataSize))
	{
		FArrayReader Buff;
		Buff.SetNumZeroed(FMath::Min(DataSize, 2048U * 1024U));

		TSharedRef<FInternetAddr> Sender = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();

		int ReadBytes = 0;
		Socket->RecvFrom(Buff.GetData(), DataSize, ReadBytes, *Sender);

		if (ReadBytes > 0)
		{
			FHorseNetData Data;

			Buff << Data;

			FString FromIP = FIPv4Endpoint(Sender).Address.ToString();

			DataReceivedDelegate.Broadcast(Data, FromIP);
		}
	}
}

void AMPHorsoUDPReceiver::Reset()
{

	if (nullptr != Socket)
	{
		Socket->Close();
		ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(Socket);
		Socket = nullptr;
	}
}

void AMPHorsoUDPReceiver::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	GetWorldTimerManager().ClearTimer(ListeningTimerHandle);

	Reset();
}


uint16 UMPHorsoNetLibrary::DefaultPort = 7878;

bool UMPHorsoNetLibrary::FormatIPv4(const FString& IPString, TArray<uint8>& OutParts, FString& OutErrorString)
{
	FString ModifiableIP = IPString;

	TArray<FString> IPChunks;
	ModifiableIP.Replace(TEXT(" "), TEXT("")).ParseIntoArray(IPChunks, TEXT("."), false);

	if (IPChunks.Num() != 4)
	{
		OutErrorString = "IP presented contains incorrect number of fields! Must be four 0-255 numbers separated by .\'s";
		return false;
	}

	for (int currInd = 0; currInd < IPChunks.Num(); ++currInd)
	{
		FString& currPart = IPChunks[currInd];
		if (!currPart.IsNumeric())
		{
			OutErrorString = "IP presented is non-numeric at section " + FString::FromInt(currInd + 1) + "!";
			return false;
		}
		
		int PartAsNumber = FCString::Atoi(*currPart);

		if (PartAsNumber < 0 || PartAsNumber > 255)
		{
			OutErrorString = "Section " + FString::FromInt(currInd + 1) + " of presented IP is numeric, but out of bounds! "
							 "Please change it to a number in the range of 0 to 255.";
			return false;
		}

		OutParts.Add(PartAsNumber);
	}

	return true;
}

void UMPHorsoNetLibrary::StringToBytes(FString InString, TArray<uint8>& OutBytes)
{
	FArrayWriter Writer;
	Writer << InString;

	OutBytes = Writer;
}

void UMPHorsoNetLibrary::StringFromBytes(const TArray<uint8>& InBytes, FString& OutString)
{
	FArrayReader Reader;
	Reader.Append(InBytes);

	Reader << OutString;
}

APlayerController* UMPHorsoNetLibrary::GetPlayerByUID(UObject* WorldContext, int UID)
{
	if (nullptr != WorldContext)
	{
		TArray<AActor*> RetrievedPlayers;
		UGameplayStatics::GetAllActorsOfClass(WorldContext, APlayerController::StaticClass(), RetrievedPlayers);

		APlayerController* currCasted;
		for (auto *curr : RetrievedPlayers)
		{
			currCasted = Cast<APlayerController>(curr);

			if (currCasted->PlayerState->PlayerId == UID)
				return currCasted;
		}
	}
	else
		UStaticFuncLib::Print("UMPHorsoNetLibrary::GetPlayerByUID: World Context was null!", true);

	return nullptr;
}

APlayerController* UMPHorsoNetLibrary::GetPlayerByIP(UObject* WorldContext, FString IP)
{
	if (nullptr != WorldContext)
	{
		TArray<AActor*> RetrievedPlayers;
		UGameplayStatics::GetAllActorsOfClass(WorldContext, APlayerController::StaticClass(), RetrievedPlayers);

		APlayerController* currCasted;
		for (auto *curr : RetrievedPlayers)
		{
			currCasted = Cast<APlayerController>(curr);

			if (currCasted->GetNetConnection()->LowLevelGetRemoteAddress() == IP)
				return currCasted;
		}
	}
	else
		UStaticFuncLib::Print("UMPHorsoNetLibrary::GetPlayerByIP: World Context was null!", true);

	return nullptr;
}

FString UMPHorsoNetLibrary::GetPlayerIP(class APlayerController* Player)
{
	if(nullptr != Player && nullptr != Player->GetNetConnection())
		return Player->GetNetConnection()->LowLevelGetRemoteAddress();

	return "";
}
