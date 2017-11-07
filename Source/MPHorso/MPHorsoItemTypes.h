// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "UObject/NoExportTypes.h"
#include "MPHorsoItemTypes.generated.h"


UCLASS(Blueprintable)
class MPHORSO_API UMPHorsoItemPrefixBlock : public UObject
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintNativeEvent, BlueprintPure)
		void GetRandomPrefix(FName& RandomPrefix);
	void GetRandomPrefix_Implementation(FName& RandomPrefix) {}

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
		void ApplyPrefixes(class UMPHorsoItemBase* Focus);
	void ApplyPrefixes_Implementation(class UMPHorsoItemBase* Focus) {}
};

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

	// The prefix block that applies to this item or type of item.
	UPROPERTY(EditDefaultsOnly)
		TSubclassOf<UMPHorsoItemPrefixBlock> PrefixBlock;

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
		void Use(class AActor* Caller);
	void Use_Implementation(class AActor* Caller) { if (NumUses > 0 && !(--NumUses)) { ItemConsumedDelegate.Broadcast(this); } }

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
