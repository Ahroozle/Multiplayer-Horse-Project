// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"

#include "NPCSchedule.h"
#include "NPCNavigation.h"
#include "NPCRuleset.h"

#include "NPCManagement.generated.h"


// TODO:
//		- have NPCs give the AI manager schedule events, and then get pings back or something
//		- flag dead AIs accordingly and either do not spawn them at all or remove them after being spawned
//
//		- Broadcast next room an NPC/Player will be in on entering an LZ.
//		  If client detects that NPC entering their room, spawn the according NPC/Player visual and **sync it**
//		  In terms of players, the only things that exist at all times are the player controllers and the client's pawn.
//		  (Maybe always route input but use null checks when pawn isn't there to gate them?)

UCLASS(Blueprintable)
class MPHORSO_API UNPCSoul : public UObject
{
	GENERATED_BODY()

public:

	UPROPERTY(EditDefaultsOnly)
		FName NPCName;

	UPROPERTY(EditDefaultsOnly)
		TSubclassOf<UNPCSchedule> ScheduleClass;

	UPROPERTY(EditDefaultsOnly)
		TSubclassOf<UNPCRuleset> RulesetClass;

	UPROPERTY(EditDefaultsOnly)
		TSubclassOf<UNPCNavVisitor> VisitorClass = UNPCNavVisitor::StaticClass();

	// The visual representation / "body" of the NPC, for instantiation when needed.
	UPROPERTY(EditDefaultsOnly)
		TSubclassOf<APawn> Visual;

	UPROPERTY(EditDefaultsOnly)
		FNPCRelevantStateEditor PersonalityState;

	// World types this NPC is allowed to be spawned in
	UPROPERTY(EditDefaultsOnly)
		TSet<FName> ValidWorlds;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite)
		UNPCRuleset* RulesetInst;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite)
		UNPCNavVisitor* VisitorInst;

	UFUNCTION()
		void Prep();
};

// Remember to call InitChains() in the construction script of the containing actor!
UCLASS(ClassGroup = (Custom))//, meta = (BlueprintSpawnableComponent))
class MPHORSO_API UNPCSoulComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UNPCSoulComponent();

protected:

	// Called when the game starts
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		UNPCSoul* SoulInst;

	UFUNCTION()
		void Prep(TSubclassOf<UNPCSoul> SoulClass);

	UFUNCTION()
		void OnTraversalFinished(TArray<FName> Path);

};

UCLASS()
class MPHORSO_API ANPCAIManager : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ANPCAIManager();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;


	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly)
		TMap<FName, UNPCSoulComponent*> SoulInsts;
	
};

//UCLASS()
//class MPHORSO_API ANPCManagement : public AActor
//{
//	GENERATED_BODY()
//	
//public:	
//	// Sets default values for this actor's properties
//	ANPCManagement();
//
//protected:
//	// Called when the game starts or when spawned
//	virtual void BeginPlay() override;
//
//public:	
//	// Called every frame
//	virtual void Tick(float DeltaTime) override;
//
//	
//	
//};
