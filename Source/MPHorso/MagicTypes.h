// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Object.h"
#include "MagicTypes.generated.h"


USTRUCT(BlueprintType)
struct FNetworkSendableSpellArgs
{
	GENERATED_USTRUCT_BODY();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spell UI Arg Package")
		TSubclassOf<class USpellUI> SpellUIClass;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spell UI Arg Package")
		TSubclassOf<class USpellArchetype> SpellArchetypeClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spell UI Arg Package")
		TArray<FName> ActorArgNames;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spell UI Arg Package")
		TArray<class AActor*> ActorArgs;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spell UI Arg Package")
		TArray<FName> VectorArgNames;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spell UI Arg Package")
		TArray<FVector> VectorArgs;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spell UI Arg Package")
		TArray<FName> RotatorArgNames;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spell UI Arg Package")
		TArray<FRotator> RotatorArgs;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spell UI Arg Package")
		TArray<FName> FloatArgNames;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spell UI Arg Package")
		TArray<float> FloatArgs;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FSpellUICancelled);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FSpellUIArgRejected);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSpellUIArgConfirmed, int, ArgNum);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FSpellUIFinished);

UCLASS(Blueprintable)
class MPHORSO_API USpellUI : public UObject
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category = "Spell UI")
		FSpellUICancelled CancelledDelegate;
	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category = "Spell UI")
		FSpellUIArgRejected ArgRejectedDelegate;
	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category = "Spell UI")
		FSpellUIArgConfirmed ArgConfirmedDelegate;
	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category = "Spell UI")
		FSpellUIFinished FinishedDelegate;

	// This version of Use is intended for player use; feedback is desired to be built into the event ladder
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Spell UI")
		void Use(class AController* Caster);
	void Use_Implementation(class AController* Caster) {}

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Spell UI")
		void Kill();
	void Kill_Implementation() {}

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Spell UI")
		void UseWithArgPackage(const FNetworkSendableSpellArgs& ArgPackage);
	void UseWithArgPackage_Implementation(const FNetworkSendableSpellArgs& ArgPackage) {}

	UFUNCTION(BlueprintNativeEvent, BlueprintPure, Category = "Spell UI")
		void GetArgPackage(FNetworkSendableSpellArgs& GeneratedPackage);
	void GetArgPackage_Implementation(FNetworkSendableSpellArgs& GeneratedPackage) {}

	UFUNCTION(BlueprintNativeEvent, BlueprintPure, Category = "Spell UI")
		float GetCurrentStaminaCost();
	float GetCurrentStaminaCost_Implementation() { return 0; }

	// SpecialUse should be defined on a per-UI basis as a way for non-player entities to use spells
	// in other ways without having to interact with player feedback classes they don't need.
};

UCLASS(Blueprintable)
class MPHORSO_API USpellArchetype : public UObject
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spell Archetype")
		FName SpellName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spell Archetype")
		FLinearColor SpellColor;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spell Archetype")
		UTexture* SpellImage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spell Archetype")
		TSubclassOf<USpellUI> UI;

	UFUNCTION(BlueprintNativeEvent, Category = "Spell Archetype")
		void Use(class AActor* Caster, USpellUI* Args);
	void Use_Implementation(class AActor* Caster, USpellUI* Args) {}
};

UCLASS()
class MPHORSO_API UMagicActionsLibrary : public UObject
{
	GENERATED_BODY()

	static TArray<TSubclassOf<USpellArchetype>> SpellArchetypes;

	static void PopulateSpellArchetypes();

public:

	UFUNCTION(BlueprintPure, Category = "Magic Actions Library")
		static FLinearColor GetSpellColor(const FName& SpellName);
	UFUNCTION(BlueprintPure, Category = "Magic Actions Library")
		static UTexture* GetSpellImage(const FName& SpellName);

	UFUNCTION(BlueprintCallable, Category = "Magic Actions Library")
		static bool GetSpell(const FName& SpellName, TSubclassOf<USpellArchetype>& RetrievedSpell, TSubclassOf<USpellUI>& RetrievedUI);

	UFUNCTION(BlueprintCallable, Category = "Magic Actions Library")
		static void ExecuteSpell(AActor* Caster, TSubclassOf<USpellArchetype> Spell, USpellUI* Args);
};
