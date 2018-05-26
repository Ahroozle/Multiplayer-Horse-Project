// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "UObject/NoExportTypes.h"
#include "NPCDateTime.h"
#include "NPCSchedule.generated.h"


USTRUCT(BlueprintType, meta = (DisplayName = "NPC Routine"))
struct FNPCRoutine
{
	GENERATED_USTRUCT_BODY();
	
	/*
		What weekdays does this routine happen on (if applicable)?
		Enabling any of these will cause the routine to be used
		every time that day happens, unless a more specific routine
		overwrites it.
	*/
	UPROPERTY(EditAnywhere, meta = (Bitmask, BitmaskEnum = ENPCWeekday))
		int RegularDays;

	/*
		What specific days does this routine happen on (if applicable)?
		Adding any dates to this will cause this routine to happen on
		those days, taking precedence over any regular routines.
	*/
	UPROPERTY(EditAnywhere)
		TArray<FNPCDateTime> SpecificDays;
};

/**
 * 
 */
UCLASS(Blueprintable)
class MPHORSO_API UNPCSchedule : public UObject
{
	GENERATED_BODY()
	
public:

	UPROPERTY(EditAnywhere)
		TArray<FNPCRoutine> Routines;
	
};
