// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "RoomStreamingTypes.generated.h"


/*
	This system relies on a "baked" graph system
	in order to create and load rooms in bulk as
	properly as needed.

	Dummy versions of instanced areas exist for
	purposes of instancing them, as well as to
	bake their graph structure in for quick and
	easy copying at runtime.

	(TODO Change NPC AI to piggyback off this system;
	i.e. latch to a layer and append data as appropriate.
	Maybe it'll make it possible to have NPCs exclusive
	to instanced layers, idk)
*/

USTRUCT()
struct FRoomAddress
{
	GENERATED_USTRUCT_BODY();

	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite)
		FName LayerName;

	UPROPERTY(EditInstanceOnly, BlueprintReadWrite)
		FName RoomName;

	bool operator==(const FRoomAddress& o) { return LayerName == o.LayerName && RoomName == o.RoomName; }
};

USTRUCT()
struct FRoomEdge
{
	GENERATED_USTRUCT_BODY();

	UPROPERTY(EditInstanceOnly)
		FRoomAddress ToNode;

	UPROPERTY(EditInstanceOnly)
		FName ToLZName;

	/*
		If not None, denotes that this
		edge is an instanced edge, and
		this name will be used when
		creating new instances of areas.
	*/
	UPROPERTY(EditInstanceOnly)
		FName InstancePrefabName;
};

USTRUCT()
struct FRoomNode
{
	GENERATED_USTRUCT_BODY();

	// Reference to the room this node represents.
	UPROPERTY(VisibleInstanceOnly)
		ULevelStreaming* RoomRef;

	// Map of LZ Names to Neighboring Rooms
	UPROPERTY(VisibleInstanceOnly)
		TMap<FName, FRoomEdge> Edges;

	/*
		Rooms visible from but not necessarily
		connected to this room. Only rooms from
		the same layer are valid candidates to
		be visible from a room.
	*/
	UPROPERTY(VisibleInstanceOnly)
		TArray<FName> VisibleFrom;
};

USTRUCT()
struct FRoomGraph
{
	GENERATED_USTRUCT_BODY();

	// Name of the parent of this graph/layer.
	UPROPERTY(VisibleInstanceOnly)
		FName Parent;

	// Map of Room Names to Rooms
	UPROPERTY(VisibleInstanceOnly)
		TMap<FName, FRoomNode> Nodes;

	UPROPERTY()
		float Offset = 0;

	FRoomEdge* GetEdge(FName Room, FName LZ)
	{
		FRoomNode* NodeRef = Nodes.Find(Room);
		if (nullptr != NodeRef)
			return NodeRef->Edges.Find(LZ);
		return nullptr;
	}
};

USTRUCT()
struct FPlayerRoomVisitor
{
	GENERATED_USTRUCT_BODY();

	// The address of the room the player previously occupied
	UPROPERTY()
		FRoomAddress Previous;

	// The address of the room the player currently occupies
	UPROPERTY()
		FRoomAddress Current;
};


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

	UPROPERTY()
		FName RetrievedLayer;

	UPROPERTY(EditInstanceOnly)
		FName LZName;

	UPROPERTY(EditInstanceOnly)
		FRoomEdge LoadingZoneEdge;

	UPROPERTY()
		bool InitedInstanced;


	UFUNCTION()
		FName GetRoomLayer(ARoomStreamingManager* StreamManager);

	UFUNCTION(BlueprintCallable)
		void CreateNextInstancedLayer();
	
	UFUNCTION(BlueprintNativeEvent, BlueprintPure)
		FVector GetPlayerOffset(APawn* Player);
	FVector GetPlayerOffset_Implementation(APawn* Player);

	UFUNCTION(BlueprintNativeEvent, BlueprintPure)
		FVector GetPlayerDirection(APawn* Player, bool Leaving);
	FVector GetPlayerDirection_Implementation(APawn* Player, bool Leaving);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
		void HandleArriving(APawn* Target);
	void HandleArriving_Implementation(APawn* Target);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
		void HandleDeparting(APawn* Target);
	void HandleDeparting_Implementation(APawn* Target);

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
		bool UpdateRoomData;

	UPROPERTY(VisibleInstanceOnly)
		FName RoomName;

	// Names of rooms visible from, but not necessarily neighbors of, this room.
	UPROPERTY(EditInstanceOnly)
		TArray<FName> VisibleFroms;

};


USTRUCT()
struct FTraversalPacket
{
	GENERATED_USTRUCT_BODY();

	UPROPERTY()
		APawn* Player;

	UPROPERTY()
		FName LZName;

	UPROPERTY()
		FVector Offset;
};

/*
	This class exists to maintain all of the data the system needs to function.
*/
UCLASS()
class MPHORSO_API ARoomStreamingManager : public AActor
{
	GENERATED_BODY()

	UPROPERTY()
		TArray<ULevelStreaming*> LoadingDuringConstruct;

	UPROPERTY()
		TMap<ULevelStreaming*, FTraversalPacket> WaitingDuringTraverse;

	UPROPERTY()
		TMap<ULevelStreaming*, APawn*> WaitingDuringEntry;

	UPROPERTY()
		ULevelStreaming* LastInstanceCreated;

public:
	// Sets default values for this actor's properties
	ARoomStreamingManager(const FObjectInitializer& _init);

	virtual void OnConstruction(const FTransform& Transform) override;

	UFUNCTION()
		void AddToConstructGraph(ULevelStreaming* Room);
	
	UFUNCTION()
		void OnLoadedDuringGraphConstruct();

	void FloodFillConstructGraph(FName StartNodeName, FName StartLZName, TArray<FName>& OutRooms, TMap<FName, int>& InstancedExits);

	UFUNCTION()
		void FinishGraphConstruct();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// This is the constant layer name used to identify instanced layer exits in prefab layers
	static FName InstExitLayer;

	UPROPERTY(EditInstanceOnly)
		bool UpdateOverworldData;

	UPROPERTY()
		TMap<APawn*, FPlayerRoomVisitor> PlayerVisitors;

	/*
		Map of existing rooms (loaded or otherwise) to layer and node names.
		'None' layer corresponds to Overworld Layer.
	*/
	UPROPERTY(VisibleInstanceOnly)
		TMap<ULevelStreaming*, FRoomAddress> RoomsToNodes;

	UPROPERTY(VisibleInstanceOnly)
		FRoomGraph OverworldLayer;

	UPROPERTY(VisibleInstanceOnly)
		TMap<FName, FRoomGraph> InstancedLayerPrefabs;

	// Map of instanced layer names to layer data.
	UPROPERTY(VisibleInstanceOnly)
		TMap<FName, FRoomGraph> InstancedLayers;

	UPROPERTY()
		TArray<float> UnusedOffsets;

	UPROPERTY()
		int UniqueInstanceLayerID = 0;


	UFUNCTION(BlueprintCallable)
		void HandleEnteringPlayer(APawn* EnteringPlayer, FRoomAddress SpawnRoom);

	UFUNCTION()
		void FinishEnterPlayer();

	UFUNCTION(BlueprintCallable)
		void HandleExitingPlayer(APawn* LeavingPlayer);
	
	UFUNCTION(NetMulticast, Reliable)
		void InstLoadOp(ULevelStreaming* Seed, const FString& NewRoomName, float Offset);
	void InstLoadOp_Implementation(ULevelStreaming* Seed, const FString& NewRoomName, float Offset);

	UFUNCTION(NetMulticast, Reliable)
		void InstUnloadOp(ULevelStreaming* ToRemove);
	void InstUnloadOp_Implementation(ULevelStreaming* ToRemove);

	UFUNCTION()
		FName AddInstancedLayer(FName PrefabName, FName ParentLayerName);

	UFUNCTION()
		void RemoveInstancedLayer(FName LayerName);

	UFUNCTION(NetMulticast, Reliable)
		void LoadOp(ULevelStreaming* Room, bool Loaded, bool Visible);
	void LoadOp_Implementation(ULevelStreaming* Room, bool Loaded, bool Visible);

	UFUNCTION()
		void RequestUnload(FRoomAddress RoomAddr);

	UFUNCTION()
		void RequestLoadInvis(FRoomAddress RoomAddr);

	UFUNCTION()
		void RequestLoad(FRoomAddress RoomAddr);

	UFUNCTION(BlueprintCallable)
		void RequestTraversal(APawn* Player, FRoomEdge Edge, FVector PlayerOffset);

	UFUNCTION()
		void FinishTraversal();

};


UCLASS()
class MPHORSO_API URoomStreamingFuncLib : public UObject
{
	GENERATED_BODY()
	
public:
	
	UFUNCTION(BlueprintPure, meta = (WorldContext = "WorldContext"))
		static ARoomStreamingManager* GetRoomStreamingManager(UObject* WorldContext);

	UFUNCTION(meta = (WorldContext = "WorldContext"))
		static FName MakeInstancedLayerName(UObject* WorldContext, FName PrefixName);

};
