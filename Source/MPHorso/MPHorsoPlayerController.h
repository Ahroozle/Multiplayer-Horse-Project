// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/PlayerController.h"
#include "ChatTypes.h"
#include "MPHorsoPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class MPHORSO_API AMPHorsoPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated)
		FName MyName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated)
		TArray<FName> EquippedSpells;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated)
		TSet<FName> OpenChannels;

	UFUNCTION(BlueprintNativeEvent, Category = "MP Horso Player Controller")
		void PassMessage(const FChatMessage& Msg);
	virtual void PassMessage_Implementation(const FChatMessage& Msg);
	
	UFUNCTION(BlueprintNativeEvent, Category = "MP Horso Player Controller")
		void PassToPersonalBubble(const FChatMessage& Msg);
	void PassToPersonalBubble_Implementation(const FChatMessage& Msg);
};
