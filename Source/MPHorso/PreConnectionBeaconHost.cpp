// Fill out your copyright notice in the Description page of Project Settings.

#include "MPHorso.h"
#include "PreConnectionBeaconHost.h"
#include "PreConnectionBeaconHostObject.h"
#include "MPHorsoGameInstance.h"
#include "StaticFuncLib.h"

APreConnectionBeaconHost::APreConnectionBeaconHost(const FObjectInitializer& ObjectInitializer) :
	Super(ObjectInitializer)
{
	//Set the beacon host state to allow requests
	BeaconState = EBeaconState::AllowRequests;
}

bool APreConnectionBeaconHost::Start()
{
	if (InitHost())
	{
		APreConnectionBeaconHostObject* HostObj = GetWorld()->SpawnActor<APreConnectionBeaconHostObject>();

		if (nullptr != HostObj)
		{
			RegisterHost(HostObj);

			return true;
		}
	}

	return false;
}

bool APreConnectionBeaconHost::InitHost()
{
	UMPHorsoGameInstance* gameInst = UStaticFuncLib::RetrieveGameInstance(this);

	if (nullptr != gameInst)
	{
		UWorldSaveBase* WorldSave = gameInst->GetWorldSave();

		if (nullptr != WorldSave)
			ListenPort = WorldSave->ServerData.Port;
	}

	return Super::InitHost();
}

//void APreConnectionBeaconHost::AddBeaconDef()
//{
//	for (auto& Definition : GEngine->NetDriverDefinitions)
//	{
//		if (Definition.DefName == NAME_BeaconNetDriver)
//			return;
//	}
//
//	FNetDriverDefinition BeaconDriverDefinition;
//	BeaconDriverDefinition.DefName = NAME_BeaconNetDriver;
//	BeaconDriverDefinition.DriverClassName = FName("/Script/OnlineSubsystemUtils.IpNetDriver");
//	BeaconDriverDefinition.DriverClassNameFallback = FName("/Script/OnlineSubsystemUtils.IpNetDriver");
//
//	GEngine->NetDriverDefinitions.Add(BeaconDriverDefinition);
//}
