// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"

#include "CullZoneVolume.h"

#include "RoomStreamingTypes.generated.h"


// TODO: Cancel spells when transitioning between rooms! Ofc this is to avoid dragging things from rooms into other rooms.

/*
	TODO:

	server/player ID hookup process:

	On first hosting of the world, ServerID genned and assigned to world.

	When client attempts to connect:
		1) Server gives Client the ServerID, requests PlayerID from Client
		2) Client looks for PlayerID in save, gives Server:
			-bool HadID
			-the Player's name (if !HadID)
			-the PlayerID itself (if HadID)
		3) Upon getting them, Server either:
			A) Generates PlayerID for Client with Player's name and Entry Number, and gives it to the Client (if !HadID)
			B) Checks if PlayerID exists within Server. If a copy doesn't exist then the Client is rejected, otherwise Client is fine.
		4) If Client is determined to be fine, Server sends message to Client to begin connecting.
		5) Client connects to Server, process is finished and play can begin.
*/

UENUM(BlueprintType)
enum class EStreamedRoomAddressType : uint8
{
	NormalAddress		UMETA(DisplayName = "Goes To Same Layer"),
	InstanceEntrance	UMETA(DisplayName = "Goes To Child/Instanced Layer"),
	InstanceExit		UMETA(DisplayName = "Goes To Parent Layer")
};

USTRUCT(BlueprintType)
struct FStreamedRoomAddress
{
	GENERATED_USTRUCT_BODY();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FName LayerName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FName RoomName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		EStreamedRoomAddressType AddressType;

	bool operator==(const FStreamedRoomAddress& o) { return LayerName == o.LayerName && RoomName == o.RoomName; }
};

USTRUCT(BlueprintType)
struct FStreamedRoom
{
	GENERATED_USTRUCT_BODY();

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		ULevelStreaming* RoomRef;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		TArray<FStreamedRoomAddress> NeighboringRooms;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		TArray<FStreamedRoomAddress> VisibleRooms;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		TSet<FString> PlayersPresent;

	FORCEINLINE void LoadRoom()
	{
		if (nullptr != RoomRef)
			RoomRef->bShouldBeLoaded = RoomRef->bShouldBeVisible = true;
	}

	FORCEINLINE void UnloadRoom()
	{
		if (nullptr != RoomRef)
			RoomRef->bShouldBeLoaded = RoomRef->bShouldBeVisible = false;
	}

	void ClearRoom();
};

USTRUCT(BlueprintType)
struct FStreamedRoomLayer
{
	GENERATED_USTRUCT_BODY();

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		FName InstantiatingLZName;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		TMap<FName, FStreamedRoom> RoomsInLayer;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		TSet<FString> PlayersPresent;

	UPROPERTY(VisibleAnywhere)
		float Offset = 0.0f;

	void TryUnloadRooms(FName RootRoomName);
	void TraverseFrom(FString PlayerID, FName RoomName);
	void TryLoadRooms(FName RootRoomName);
	void TraverseTo(FString PlayerID, FName RoomName);

	void GetRoomNeighbors(FName RoomName, TArray<FStreamedRoom*>& NeighboringRooms);

	bool CanUnloadRoom(FName RoomName);

	void Clear();

	void CloneLayer(FStreamedRoomLayer& Clone, FName NewLayerName, float NewLayerOffset);
};

USTRUCT()
struct FStreamedRoomTreeNode
{
	GENERATED_USTRUCT_BODY();

	UPROPERTY()
		FStreamedRoomLayer Layer;

	UPROPERTY()
		FName Parent;

	UPROPERTY()
		TArray<FName> Children;
};


/*
	This system relies on a "baked" graph system
	in order to create and load rooms in bulk as
	properly as needed.

	Dummy versions of instanced areas exist for
	purposes of instancing them, as well as to
	bake their graph structure in for quick and
	easy copying at runtime.
*/

/*
	(TODO Change NPC AI to piggyback off this system;
	i.e. latch to a layer and append data as appropriate.
	Maybe it'll make it possible to have NPCs exclusive
	to instanced layers, idk)
*/

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FRoomEnteredNotify, AActor*, EnteredLZ, APawn*, EnteringPlayer);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FRoomExitedNotify, AActor*, ExitedLZ, const TArray<APawn*>&, ExitingPlayers);

UCLASS(Blueprintable)
class MPHORSO_API ALoadingZone : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ALoadingZone(const FObjectInitializer& _init);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	virtual void NotifyActorBeginOverlap(AActor* OtherActor);

	UPROPERTY()
		bool LayerFound = false;

	UPROPERTY(EditInstanceOnly)
		FName LZName;

	UPROPERTY()
		FName DestPrefabLayer;

	UPROPERTY(EditInstanceOnly)
		FStreamedRoomAddress DestinationRoom;

	UPROPERTY(EditInstanceOnly)
		FName DestinationLZ;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		TSet<APawn*> DepartingPlayers;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		TSet<APawn*> ArrivingPlayers;

	// This is used during traversal to keep track of exactly where the current travellers are going.
	UPROPERTY()
		FStreamedRoomAddress CurrentTargetAddress;

	// This is used during traversal as a minor optimization.
	UPROPERTY()
		ULevelStreaming* CurrentRoomRef;

	UPROPERTY(BlueprintAssignable)
		FRoomEnteredNotify OnRoomEntered;

	UPROPERTY(BlueprintAssignable)
		FRoomExitedNotify OnRoomExited;

	
	UFUNCTION(BlueprintNativeEvent, BlueprintPure)
		FVector GetPlayerOffset(APawn* Player);
	FVector GetPlayerOffset_Implementation(APawn* Player);

	UFUNCTION(BlueprintNativeEvent, BlueprintPure)
		FVector GetPlayerDirection(APawn* Player, bool Leaving);
	FVector GetPlayerDirection_Implementation(APawn* Player, bool Leaving);


	UFUNCTION(BlueprintCallable)
		void HandleArriving(APawn* Target);

	/*
		Call this at the very end of your override of DoArrivingAnimation()
		to properly finish up the transition!
	*/
	UFUNCTION(BlueprintCallable)
		void FinishArrivingAnimation(APawn* FinishedArriving) { ArrivingPlayers.Remove(FinishedArriving); }

	UFUNCTION(NetMulticast, Reliable)
		void BroadcastArrivingAnimation(APawn* Target);
	void BroadcastArrivingAnimation_Implementation(APawn* Target) { DoArrivingAnimation(Target); }

	UFUNCTION(BlueprintNativeEvent)
		void DoArrivingAnimation(APawn* Target);
	void DoArrivingAnimation_Implementation(APawn* Target) { FinishArrivingAnimation(Target); }

	UFUNCTION(NetMulticast, Reliable)
		void BroadcastFixingArriveAnimation(const TArray<APawn*>& Targets);
	void BroadcastFixingArriveAnimation_Implementation(const TArray<APawn*>& Targets) { DoFixingArriveAnimation(Targets); }

	/*
		This gets called whenever arriving at a new room ends buggily.
		Disable any effects here to make sure that even if the system fucks up,
		they can still see and play the game with minimal interruption!
	*/
	UFUNCTION(BlueprintNativeEvent)
		void DoFixingArriveAnimation(const TArray<APawn*>& Targets);
	void DoFixingArriveAnimation_Implementation(const TArray<APawn*>& Targets) { for (APawn* curr : Targets) FinishArrivingAnimation(curr); }


	UFUNCTION(BlueprintCallable)
		void HandleDeparting(const TArray<APawn*>& Targets);

	/*
		Call this at the very end of your override of DoDepartingAnimation()
		to properly finish up the transition to the next room!
	*/
	UFUNCTION(BlueprintCallable)
		void FinishDepartingAnimation();

	UFUNCTION()
		void OnFinallyDepart();

	UFUNCTION(NetMulticast, Reliable)
		void BroadcastDepartingAnimation(const TArray<APawn*>& Targets);
	void BroadcastDepartingAnimation_Implementation(const TArray<APawn*>& Targets) { DoDepartingAnimation(Targets); }

	UFUNCTION(BlueprintNativeEvent)
		void DoDepartingAnimation(const TArray<APawn*>& Targets);
	void DoDepartingAnimation_Implementation(const TArray<APawn*>& Targets) { FinishDepartingAnimation(); }

};


/*
	This actor type is intended to be used for room-wide culling operations!
	When the CullVolume is put into culling mode, everything within it has its
	max draw distance set to 1 UU, essentially rendering it invisible without causing
	it to stop ticking or do weird stuff.
	
	This actor pretty much only exists to bypass how level streaming volumes work;
	if you invis a level streaming volume, everything within it stops ticking.
*/
UCLASS(Blueprintable)
class MPHORSO_API ARoomCullVolume : public ACullZoneVolume
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ARoomCullVolume(const FObjectInitializer& _init);

	virtual void OnConstruction(const FTransform& Transform) override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};


/*
	TODO Maybe have the room actor act as a pivot point of sorts?
		 i.e. on the position changed it offsets all of the things in
		 the level by that amount, on rotate it rotates all the things
		 in the level around based on it as a pivot, etc.

		 Basically a LevelTransform for runtime use.
*/

UCLASS()
class MPHORSO_API ARoomActor : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ARoomActor(const FObjectInitializer& _init);

	virtual void OnConstruction(const FTransform& Transform) override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;


	UPROPERTY(EditInstanceOnly)
		bool UpdateRoomName;

	/*
		What world layer does this level take place on?

		If this is set to anything other than None, this room will
		be put on a world layer that can be instanced.
	*/
	UPROPERTY(EditInstanceOnly)
		FName LayerName;

	UPROPERTY(VisibleInstanceOnly)
		FName RoomName;

	// Names of rooms visible from, but not necessarily neighbors of, this room.
	UPROPERTY(EditInstanceOnly)
		TArray<FName> VisibleFroms;

	// The culling volumes associated with this room, retrieved in BeginPlay at runtime.
	UPROPERTY(VisibleInstanceOnly)
		TArray<ARoomCullVolume*> RoomCullVolumes;


	UFUNCTION()
		void HandleRoomEntry(AActor* EnteredFromLZ, APawn* EnteringPlayer);

	UFUNCTION()
		void HandleRoomExiting(AActor* ExitedFromLZ, const TArray<APawn*>& ExitingPlayers);

	UFUNCTION(BlueprintCallable)
		void SetRoomIsCulling(bool NewIsCulling) { for (ARoomCullVolume* curr : RoomCullVolumes) curr->SetIsCulling(NewIsCulling); }

	// TODO room actors should also probably be used to change things like the directional light source and skybox. maybe.

};


USTRUCT()
struct FLSKReplaceData
{
	GENERATED_USTRUCT_BODY();

	UPROPERTY()
		FName RoomName;

	UPROPERTY()
		FName LSKReplacementName;
};

USTRUCT()
struct FLayerReconstructionData
{
	GENERATED_USTRUCT_BODY();

	UPROPERTY()
		FName LayerName;

	UPROPERTY()
		FName PrefabLayerName;

	UPROPERTY()
		FName ParentLayerName;

	// This data is used to rename the resulting LevelStreamingKismets so that they match the serverside versions.
	UPROPERTY()
		TArray<FLSKReplaceData> NameFixDatas;
};

USTRUCT()
struct FRoomStateReconstructPacket
{
	GENERATED_USTRUCT_BODY();

	UPROPERTY()
		TArray<FLayerReconstructionData> LayerDatas;

	// Used to modify the unique name generator number to properly sync LevelStreamingKismet names.
	UPROPERTY()
		int UniqueIDNumber;

	UPROPERTY()
		TArray<FString> PlayerIDs;

	UPROPERTY()
		TArray<FStreamedRoomAddress> PlayerAddrs;

};

/*
	This class exists to maintain all of the data the system needs to function.
*/
UCLASS()
class MPHORSO_API ARoomStreamingManager : public AActor
{
	GENERATED_BODY()

	UPROPERTY()
		TMap<ULevelStreaming*, APawn*> WaitingDuringEntry;

	UPROPERTY()
		TMap<FName, ALoadingZone*> WaitingDuringLayerCreation;

	// This data is used to reconstruct the current existing layers on client machines.
	UPROPERTY()
		FRoomStateReconstructPacket StateReconstructionData;

public:
	// Sets default values for this actor's properties
	ARoomStreamingManager(const FObjectInitializer& _init);

	virtual void OnConstruction(const FTransform& Transform) override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditInstanceOnly)
		bool UpdateOverworldData;

	// Map of Layer Names to Layer Tree Nodes
	UPROPERTY(VisibleInstanceOnly)
		TMap<FName, FStreamedRoomTreeNode> LayerNodes;

	/*
		Map of existing rooms (loaded or otherwise) to their respective
		{Layer, RoomName} address, for accessing the above LayerNodes map.
		'None' layer corresponds to Overworld Layer.
	*/
	UPROPERTY(VisibleInstanceOnly)
		TMap<ULevelStreaming*, FStreamedRoomAddress> RoomAddresses;

	// The name of the room in the overworld layer where players spawn when logging in under strange circumstances.
	UPROPERTY(EditInstanceOnly)
		FName DebugSpawnRoom;

	/*
		Map of player actors to their respective IDs.
	*/
	UPROPERTY()
		TMap<AActor*, FString> PlayerIDs;

	/*
		Keeps track of players' current room addresses for various uses. Indexed by Player ID.
	*/
	UPROPERTY(VisibleInstanceOnly)
		TMap<FString, FStreamedRoomAddress> PlayerAddresses;

	UPROPERTY()
		TArray<float> UnusedOffsets;

	UPROPERTY()
		TSet<float> UsedOffsets;

	UPROPERTY()
		int UniqueInstanceLayerID = 0;


	UFUNCTION(BlueprintCallable)
		void HandleEnteringPlayer(APawn* EnteringPlayer, const FString& PlayerName, const FString& PlayerID);

	UFUNCTION()
		void FinishEnterPlayer();

	// in case of testing emergency break comment glass
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "(Debug) Plop Entering Player"))
		void PlopEnteringPlayer(APawn* EnteringPlayer, FStreamedRoomAddress Addr);

	UFUNCTION()
		void PlaceEnteringPlayer(APawn* EnteringPlayer, ULevelStreaming* Room);

	UFUNCTION(BlueprintCallable)
		void HandleExitingPlayer(APawn* LeavingPlayer);


	UFUNCTION(BlueprintCallable)
		bool GetPlayerID(AActor* FocusActor, FString& FoundID)
	{
		if (PlayerIDs.Contains(FocusActor))
		{
			FoundID = PlayerIDs.FindRef(FocusActor);

			return true;
		}

		return false;
	}

	UFUNCTION(BlueprintCallable)
		bool GetRoomAddress(AActor* FocusActor, FStreamedRoomAddress& FoundAddr);

	UFUNCTION()
		void RequestNewLayer(ALoadingZone* Requester);

	UFUNCTION()
		void BeginConstructLayer(ALoadingZone* Creator, const FName& ParentLayerName);

	UFUNCTION(NetMulticast, Reliable)
		void ConstructLayer(const FName& LayerName, const FName& PrefabLayerName, const FName& ParentLayerName);
	void ConstructLayer_Implementation(const FName& LayerName, const FName& PrefabLayerName, const FName& ParentLayerName);
	

	UFUNCTION(BlueprintCallable)
		void BroadcastPreexistingState();

	UFUNCTION(NetMulticast, Reliable)
		void ConstructPreexistingState(const FRoomStateReconstructPacket& StatePacket);
	void ConstructPreexistingState_Implementation(const FRoomStateReconstructPacket& StatePacket);


	//UFUNCTION(NetMulticast, Reliable)
	//	void BeginTryLoad(const FStreamedRoomAddress& Address);
	//void BeginTryLoad_Implementation(const FStreamedRoomAddress& Address);

	UFUNCTION(NetMulticast, Reliable)
		void BeginTraverseFrom(const FString& TraverserID, const FStreamedRoomAddress& Addr, bool Leaving = false);
	void BeginTraverseFrom_Implementation(const FString& TraverserID, const FStreamedRoomAddress& Addr, bool Leaving = false);

	UFUNCTION(NetMulticast, Reliable)
		void BeginTraverseTo(const FString& TraverserID, const FStreamedRoomAddress& Addr);
	void BeginTraverseTo_Implementation(const FString& TraverserID, const FStreamedRoomAddress& Addr);

	UFUNCTION()
		void TraverseBetween(AActor* TraversingActor, FName FromLayer, FName FromRoom, FName ToLayer, FName ToRoom);

	UFUNCTION(BlueprintCallable)
		bool RequestTraversal(const TArray<APawn*>& Players, ALoadingZone* FromLZ);

	bool AnyPlayersInLayer(FStreamedRoomTreeNode* LayerRef);
	void RemoveLayer(FStreamedRoomTreeNode* LayerRef, FName LayerName);

	UFUNCTION(NetMulticast, Reliable)
		void BeginTryUnload(const FStreamedRoomAddress& Address, bool IsInstancedLayer);
	void BeginTryUnload_Implementation(const FStreamedRoomAddress& Address, bool IsInstancedLayer);

	UFUNCTION(BlueprintCallable)
		void PostTraverse(ALoadingZone* FromLZ);


	/*
		This is meant to be called by non-loading zone actions that require room traversal
		(e.g. deaths and other non LZ-based traversals).

		Returns true if the traversal has gone through, false otherwise.
		DestRoom is also valid when returning true, and is meant to allow you to wait for the room to load.

		Remember to call PostTraverseDirect() at the end of your traversal action.
	*/
	UFUNCTION(BlueprintCallable)
		bool RequestTraversalDirect(TArray<APawn*>& Players, FStreamedRoomAddress FromAddress, FStreamedRoomAddress ToAddress, ULevelStreaming*& DestRoom);

	UFUNCTION(BlueprintCallable)
		void PostTraverseDirect(FStreamedRoomAddress FromAddress);
};


UCLASS()
class MPHORSO_API URoomStreamingFuncLib : public UObject
{
	GENERATED_BODY()
	
public:
	
	UFUNCTION(BlueprintPure, meta = (WorldContext = "WorldContext"))
		static ARoomStreamingManager* GetRoomStreamingManager(UObject* WorldContext);

};
