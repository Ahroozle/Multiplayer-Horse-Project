// Fill out your copyright notice in the Description page of Project Settings.

#include "MPHorso.h"
#include "PreConnectionBeaconHostObject.h"
#include "PreConnectionBeaconClient.h"

APreConnectionBeaconHostObject::APreConnectionBeaconHostObject(const FObjectInitializer& ObjectInitializer) :
	Super(ObjectInitializer)
{
	ClientBeaconActorClass = APreConnectionBeaconClient::StaticClass();
	BeaconTypeName = ClientBeaconActorClass->GetName();

	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bAllowTickOnDedicatedServer = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
}

void APreConnectionBeaconHostObject::OnClientConnected(AOnlineBeaconClient* NewClientActor, UNetConnection* ClientConnection)
{
	Super::OnClientConnected(NewClientActor, ClientConnection);

	APreConnectionBeaconClient* BeaconClient = Cast<APreConnectionBeaconClient>(NewClientActor);
	if (nullptr != BeaconClient)
		BeaconClient->Ready();
}

AOnlineBeaconClient* APreConnectionBeaconHostObject::SpawnBeaconActor(UNetConnection* ClientConnection)
{
	return Super::SpawnBeaconActor(ClientConnection);
}

bool APreConnectionBeaconHostObject::Init()
{
	return true;
}
