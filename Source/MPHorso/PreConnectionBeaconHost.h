// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "OnlineBeaconHost.h"
#include "PreConnectionBeaconHost.generated.h"

/**
 * 
 */
UCLASS(BlueprintType)
class MPHORSO_API APreConnectionBeaconHost : public AOnlineBeaconHost
{
	GENERATED_BODY()
	
public:

	APreConnectionBeaconHost(const FObjectInitializer& ObjectInitializer);
	
	UFUNCTION(BlueprintCallable)
		bool Start();

	virtual bool InitHost() override;
	
	// in case of emergency break comment glass
	//UFUNCTION(BlueprintCallable)
	//	static void AddBeaconDef();
};
