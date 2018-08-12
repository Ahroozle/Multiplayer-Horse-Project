// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "UObject/NoExportTypes.h"
#include "MPHorsoItemTypes.generated.h"

/*
	TODO

	Concept for amalgam gear:
	Gear slots		Amalgam gear
	O				O
	O				|
	O				O
	O				O
	O				|

	tl;dr you have to "drag and drop" gear into the slots already,
	so what if we show it as taking up all the slots of the contained
	gear already, and only let it be fitted into its proper slottage?

	Also DEFINITELY decrease inv size, considering this lets the player
	carry an absolute fuckton more items than normal. Gonna have to edit
	how gear is saved, too.
*/


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FItemConsumedNotify, class UMPHorsoItemBase*, ConsumedItem);

UCLASS(Blueprintable)
class MPHORSO_API UMPHorsoItemBase : public UObject
{
	GENERATED_BODY()

public:

	UPROPERTY(EditDefaultsOnly)
		FName ItemName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		UTexture* ItemImage;

	UPROPERTY(BlueprintReadWrite)
		TArray<FName> Prefixes;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (ClampMin = "1.0"))
		int MaxStack = 1;

	/*
		The number of times you can use this item!
		-1 refers to indefinite use while 0 is considered
		drained and the item is disposed of.
	*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta = (ClampMin = "-1.0"))
		int NumUses = -1;


	UPROPERTY(BlueprintReadWrite)
		int CurrStack = 1;


	UPROPERTY(BlueprintAssignable)
		FItemConsumedNotify ItemConsumedDelegate;

	// Allows you to change the name of the item depending on things like prefixes and such.
	UFUNCTION(BlueprintNativeEvent, BlueprintPure)
		FString GetFriendlyName();
	FString GetFriendlyName_Implementation();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
		void ApplyPrefixes();
	virtual void ApplyPrefixes_Implementation() {}

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
		void Use(class AActor* Caller);
	void Use_Implementation(class AActor* Caller) { if (NumUses > 0 && !(--NumUses)) { ItemConsumedDelegate.Broadcast(this); } }

};

UCLASS(Blueprintable)
class MPHORSO_API UMPHorsoWeaponItem : public UMPHorsoItemBase
{
	GENERATED_BODY()

public:

	// TODO STUFF

	// (also prefixes for weapons n shit)

};

UCLASS(Blueprintable)
class MPHORSO_API UMPHorsoArmorItem : public UMPHorsoItemBase
{
	GENERATED_BODY()

public:

	// TODO STUFF

	// also prefixes for armors n shit
};

UCLASS(Blueprintable)
class MPHORSO_API UMPHorsoArmorComboItem : public UMPHorsoArmorItem
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TArray<UMPHorsoArmorItem*> PiecesInSet;

	// TODO STUFF

};

UCLASS()
class MPHORSO_API UMPHorsoItemHelperLibrary : public UObject
{
	GENERATED_BODY()



public:

	UFUNCTION(BlueprintPure)
		static void GetDefaultItemImage(TSubclassOf<UMPHorsoItemBase> ItemClass, UTexture*& ReturnedImage) { if(ItemClass) ReturnedImage = ItemClass.GetDefaultObject()->ItemImage; }
	

	// TODO SPLITTING/COMBINING ITEMS FOR ARMOR AND STUFF - HOW DO???
	// I know that I want players to be able to split apart and recombine armor at will but I'm not sure how
	// I should go about it. Maybe some kind of special conglomerate type of item? idk?
	
	//UFUNCTION(BlueprintPure)
	//	static void CombineItems()

};


/**
 * 
 */
//UCLASS()
//class MPHORSO_API UMPHorsoItemTypes : public UObject
//{
//	GENERATED_BODY()
//	
//	
//	
//	
//};
