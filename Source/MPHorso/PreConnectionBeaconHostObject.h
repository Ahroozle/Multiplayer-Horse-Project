// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "OnlineBeaconHostObject.h"
#include "PreConnectionBeaconHostObject.generated.h"

/**
 * 
 */
UCLASS(BlueprintType)
class MPHORSO_API APreConnectionBeaconHostObject : public AOnlineBeaconHostObject
{
	GENERATED_BODY()
	
public:

	APreConnectionBeaconHostObject(const FObjectInitializer& ObjectInitializer);
	
	//~ Begin AOnlineBeaconHost Interface
	virtual AOnlineBeaconClient* SpawnBeaconActor(class UNetConnection* ClientConnection) override;
	virtual void OnClientConnected(class AOnlineBeaconClient* NewClientActor, class UNetConnection* ClientConnection) override;
	//~ End AOnlineBeaconHost Interface
	
	virtual bool Init();

};
