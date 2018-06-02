// Fill out your copyright notice in the Description page of Project Settings.

#include "MPHorso.h"
#include "RoomStreamingTypes.h"

//#include "Kismet/GameplayStatics.h"

//#include "Engine/LevelStreaming.h"
#include "Engine/LevelStreamingKismet.h"

#include "StaticFuncLib.h"

#include "MPHorsoPlayerController.h"


/*
	TODO room offsets might still be an issue. Fix them if they pop up again.
*/

// Sets default values
ALoadingZone::ALoadingZone(const FObjectInitializer& _init) : Super(_init)
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	RootComponent = _init.CreateDefaultSubobject<USceneComponent>(this, TEXT("RootComponent"));
}

// Called when the game starts or when spawned
void ALoadingZone::BeginPlay()
{
	Super::BeginPlay();

	ARoomStreamingManager* StreamManager = URoomStreamingFuncLib::GetRoomStreamingManager(this);

	FName LayerName;
	FRoomGraph* LayerRef = nullptr;

	if (nullptr != StreamManager)
	{
		LayerName = GetRoomLayer(StreamManager);

		LayerRef = (LayerName.IsNone() ? &StreamManager->OverworldLayer : StreamManager->InstancedLayers.Find(LayerName));

		if (nullptr != LayerRef)
		{
			FRoomEdge* FoundEdge = LayerRef->GetEdge(GetLevel()->GetOuter()->GetFName(), LZName);

			if (nullptr != FoundEdge)
				LoadingZoneEdge.InstancePrefabName = FoundEdge->InstancePrefabName;
		}
	}

	if (!InitedInstanced)
	{
		if (!LoadingZoneEdge.InstancePrefabName.IsNone())
		{
			if (LoadingZoneEdge.InstancePrefabName != ARoomStreamingManager::InstExitLayer)
			{
				if (LoadingZoneEdge.ToNode.LayerName.IsNone())
				{
					// We don't have a layer, create a new one
					if (!GetWorld()->HasBegunPlay() || UKismetSystemLibrary::IsServer(this))
						CreateNextInstancedLayer();
				}
				else
				{
					if (nullptr != StreamManager)
					{
						for (auto &currPlayer : StreamManager->PlayerVisitors)
						{
							if (currPlayer.Value.Current.LayerName == LoadingZoneEdge.ToNode.LayerName)
							{
								// There's a player in our layer, we should create a new one.
								CreateNextInstancedLayer();
								break;
							}
						}
					}
				}
			}
			else if (nullptr != LayerRef)
				LoadingZoneEdge.ToNode.LayerName = LayerRef->Parent;
		}
		else
			LoadingZoneEdge.ToNode.LayerName = LayerName;


		InitedInstanced = true;
	}
}

FName ALoadingZone::GetRoomLayer(ARoomStreamingManager* StreamManager)
{
	if (!LayerFound)
	{
		ULevel* MyLevel = GetLevel();

		for (auto &curr : StreamManager->RoomsToNodes)
		{
			if (curr.Key->IsLevelLoaded() && curr.Key->GetLoadedLevel() == MyLevel)
			{
				RetrievedLayer = curr.Value.LayerName;
				LayerFound = true;
				break;
			}
		}
	}

	return RetrievedLayer;
}

void ALoadingZone::CreateNextInstancedLayer()
{
	ARoomStreamingManager* StreamManager = URoomStreamingFuncLib::GetRoomStreamingManager(this);

	if (nullptr != StreamManager)
	{
		LoadingZoneEdge.ToNode.LayerName =
			StreamManager->AddInstancedLayer(LoadingZoneEdge.InstancePrefabName, GetRoomLayer(StreamManager));
	}
	else
		UStaticFuncLib::Print("ALoadingZone::CreateNextInstancedLayer: Couldn't retrieve the Room Streaming Manager!", true);
}

// Called every frame
void ALoadingZone::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ALoadingZone::NotifyActorBeginOverlap(AActor* OtherActor)
{
	APawn* AsPawn = Cast<APawn>(OtherActor);

	if (nullptr != AsPawn)
	{
		ARoomStreamingManager* StreamManager = URoomStreamingFuncLib::GetRoomStreamingManager(this);

		if (nullptr != StreamManager)
		{
			FPlayerRoomVisitor* RelevantVisitor = StreamManager->PlayerVisitors.Find(AsPawn);

			if (nullptr != RelevantVisitor)
			{
				if (RelevantVisitor->Previous == LoadingZoneEdge.ToNode ||
					(!LoadingZoneEdge.InstancePrefabName.IsNone() && RelevantVisitor->Previous.RoomName == LoadingZoneEdge.ToNode.RoomName))
					HandleArriving(AsPawn);
				else
					HandleDeparting(AsPawn);
			}
		}
		else
			UStaticFuncLib::Print("ALoadingZone::NotifyActorBeginOverlap: Couldn't retrieve the Room Streaming Manager!", true);
	}
}

FVector ALoadingZone::GetPlayerOffset_Implementation(APawn* Player)
{
	return GetActorRightVector() * FVector::DotProduct(Player->GetActorLocation() - GetActorLocation(), GetActorRightVector());
}

FVector ALoadingZone::GetPlayerDirection_Implementation(APawn* Player, bool Leaving)
{
	return GetActorForwardVector() * (Leaving ? 1 : -1);
}

void ALoadingZone::HandleArriving_Implementation(APawn* Target)
{

	ARoomStreamingManager* StreamManager = URoomStreamingFuncLib::GetRoomStreamingManager(this);

	if (nullptr != StreamManager)
	{
		FPlayerRoomVisitor* RelevantVisitor = StreamManager->PlayerVisitors.Find(Target);
		
		// NOTE: This is kind of an ad-hoc to avoid issues like entering a place and then going out the way you came
		if (nullptr != RelevantVisitor)
			RelevantVisitor->Previous = RelevantVisitor->Current;
		else
			UStaticFuncLib::Print("ALoadingZone::HandleArriving_Implementation: Couldn't find a player visitor for pawn \'" +
								  Target->GetName() + "\'! It will be ignored.", true);
	}
	else
		UStaticFuncLib::Print("ALoadingZone::HandleArriving_Implementation: Couldn't retrieve the Room Streaming Manager!", true);
}

void ALoadingZone::HandleDeparting_Implementation(APawn* Target)
{
	ARoomStreamingManager* StreamManager = URoomStreamingFuncLib::GetRoomStreamingManager(this);

	if (nullptr != StreamManager)
	{
		if (!LoadingZoneEdge.InstancePrefabName.IsNone() && LoadingZoneEdge.InstancePrefabName != ARoomStreamingManager::InstExitLayer
			&& UKismetSystemLibrary::IsServer(this))
			CreateNextInstancedLayer();

		StreamManager->RequestTraversal(Target, LoadingZoneEdge, GetPlayerOffset(Target));
	}
	else
		UStaticFuncLib::Print("ALoadingZone::HandleDeparting_Implementation: Couldn't retrieve the Room Streaming Manager!", true);
}


// Sets default values
ARoomActor::ARoomActor(const FObjectInitializer& _init) : Super(_init)
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	RootComponent = _init.CreateDefaultSubobject<USceneComponent>(this, TEXT("RootComponent"));
}

void ARoomActor::OnConstruction(const FTransform& Transform)
{
#if WITH_EDITOR

	if (UpdateRoomData)
	{
		RoomName = *UGameplayStatics::GetCurrentLevelName(this);

		UpdateRoomData = false;
	}

#endif

}

// Called when the game starts or when spawned
void ARoomActor::BeginPlay()
{
	Super::BeginPlay();

}

// Called every frame
void ARoomActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}


FName ARoomStreamingManager::InstExitLayer("__PARENTLAYER__");

// Sets default values
ARoomStreamingManager::ARoomStreamingManager(const FObjectInitializer& _init) : Super(_init)
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	bReplicates = true;

	RootComponent = _init.CreateDefaultSubobject<USceneComponent>(this, TEXT("RootComponent"));
}

void ARoomStreamingManager::OnConstruction(const FTransform& Transform)
{

#if WITH_EDITOR

	if (UpdateOverworldData)
	{
		OverworldLayer.Nodes.Empty();
		InstancedLayerPrefabs.Empty();
		RoomsToNodes.Empty();

		for (ULevelStreaming* currRoom : GetWorld()->StreamingLevels)
		{
			if (currRoom->IsLevelLoaded())
				AddToConstructGraph(currRoom);
			else
			{
				LoadingDuringConstruct.Add(currRoom);
				currRoom->bShouldBeLoaded = true;
				currRoom->OnLevelLoaded.AddDynamic(this, &ARoomStreamingManager::OnLoadedDuringGraphConstruct);
			}
		}

		if (LoadingDuringConstruct.Num() <= 0)
			FinishGraphConstruct();

		UpdateOverworldData = false;
	}

#endif

}

void ARoomStreamingManager::AddToConstructGraph(ULevelStreaming* Room)
{
	FRoomNode NewNode;
	NewNode.RoomRef = Room;

	FName RoomName;

	for (AActor* currActor : Room->GetLoadedLevel()->Actors)
	{
		ALoadingZone* AsLZ = Cast<ALoadingZone>(currActor);

		if (nullptr != AsLZ)
			NewNode.Edges.Add(AsLZ->LZName, AsLZ->LoadingZoneEdge);
		else
		{
			ARoomActor* AsRA = Cast<ARoomActor>(currActor);

			if (nullptr != AsRA)
			{
				RoomName = AsRA->RoomName;
				NewNode.VisibleFrom = AsRA->VisibleFroms;
			}
		}
	}

	if (NewNode.Edges.Num() <= 0)
		UStaticFuncLib::Print("ARoomStreamingManager::AddToConstructGraph: Couldn't find any Loading Zones in room \'" +
							  Room->GetName() + "\'! Node will be created but is not guaranteed to work properly.", true);
	if (RoomName.IsNone())
		UStaticFuncLib::Print("ARoomStreamingManager::AddToConstructGraph: Couldn't find any Room Actors in room \'" +
							  Room->GetName() + "\'! Node will be created but may easily be overwritten by other nodes "
							  "which are missing Room Actors.", true);

	OverworldLayer.Nodes.Add(RoomName, NewNode);
}

void ARoomStreamingManager::OnLoadedDuringGraphConstruct()
{
	for (int i = LoadingDuringConstruct.Num() - 1; i >= 0; --i)
	{
		ULevelStreaming* currRoom = LoadingDuringConstruct[i];

		if (currRoom->IsLevelLoaded())
		{
			if (currRoom->OnLevelLoaded.IsAlreadyBound(this, &ARoomStreamingManager::OnLoadedDuringGraphConstruct))
				currRoom->OnLevelLoaded.RemoveDynamic(this, &ARoomStreamingManager::OnLoadedDuringGraphConstruct);

			AddToConstructGraph(currRoom);

			LoadingDuringConstruct.RemoveAt(i);
		}

		currRoom->bShouldBeLoaded = false;
	}

	if (LoadingDuringConstruct.Num() <= 0)
		FinishGraphConstruct();
}

void ARoomStreamingManager::FloodFillConstructGraph(FName StartNodeName, FName StartLZName,
	TArray<FName>& OutRooms, TMap<FName, int>& InstancedExits)
{
	OutRooms.Empty();
	InstancedExits.Empty();

	int Index = 0;

	FRoomEdge* FoundEdge = OverworldLayer.GetEdge(StartNodeName, StartLZName);

	if (nullptr == FoundEdge)
	{
		UStaticFuncLib::Print("ARoomStreamingManager::FloodFillConstructGraph: Couldn't get an edge defined by LZ \'" +
							  StartLZName.ToString() + "\' and Node \'" + StartNodeName.ToString() + "\'! Flood fill has failed.", true);
		return;
	}

	FRoomAddress& CurrAdd = FoundEdge->ToNode;
	OutRooms.Add(CurrAdd.RoomName);

	do
	{
		FRoomNode* CurrNode = OverworldLayer.Nodes.Find(OutRooms[Index]);

		if (nullptr != CurrNode)
		{
			for (auto &currEdgePair : CurrNode->Edges)
			{
				FRoomEdge& currEdge = currEdgePair.Value;
				if (currEdge.InstancePrefabName.IsNone())
				{
					FRoomEdge* OtherEdge = OverworldLayer.GetEdge(currEdge.ToNode.RoomName, currEdge.ToLZName);
					if (nullptr != OtherEdge && !OtherEdge->InstancePrefabName.IsNone())
					{
						// The fact that an edge that leads to this one is instanced means that this one is an instance exit
						InstancedExits.Add(currEdgePair.Key, Index);
					}
					else
						OutRooms.AddUnique(currEdge.ToNode.RoomName);
				}
			}
		}
		else
			UStaticFuncLib::Print("ARoomStreamingManager::FloodFillConstructGraph: Couldn't find a node named \'" +
								  OutRooms[Index].ToString() + "\'! Node has been skipped during flood fill.", true);

	} while (++Index < OutRooms.Num());
}

void ARoomStreamingManager::FinishGraphConstruct()
{
	TMap<FName, FName> LZsToRooms;
	TMap<FName, FName> LayersToLZs;

	for (auto &currNodePair : OverworldLayer.Nodes)
	{
		for (auto &currEdgePair : currNodePair.Value.Edges)
		{
			if (!currEdgePair.Value.InstancePrefabName.IsNone() && !LayersToLZs.Contains(currEdgePair.Value.InstancePrefabName))
			{
				LZsToRooms.Add(currEdgePair.Key, currNodePair.Key);
				LayersToLZs.Add(currEdgePair.Value.InstancePrefabName, currEdgePair.Key);
			}
		}
	}

	TArray<FName> RoomsInLayer;
	TMap<FName, int> InstExitLZs; // Map InstancedExit edge names to Indices into RoomsInLayer

	TArray<FName> OWToRemove;

	for (auto &currLayerPair : LayersToLZs)
	{
		FName RoomName = LZsToRooms.FindRef(currLayerPair.Value);
		FloodFillConstructGraph(RoomName, currLayerPair.Value, RoomsInLayer, InstExitLZs);

		FRoomGraph& NewLayer = InstancedLayerPrefabs.Add(currLayerPair.Key);
		for (FName& currRoom : RoomsInLayer)
		{
			FRoomNode& ShiftedNode = NewLayer.Nodes.Add(currRoom, OverworldLayer.Nodes.FindRef(currRoom));
			OWToRemove.Add(currRoom);

			RoomsToNodes.Add(ShiftedNode.RoomRef, { currLayerPair.Key, currRoom });
		}

		for (auto &currInstExitPair : InstExitLZs)
		{
			FRoomNode& currRelevantNode = *NewLayer.Nodes.Find(RoomsInLayer[currInstExitPair.Value]);
			FRoomEdge& currRelevantEdge = *currRelevantNode.Edges.Find(currInstExitPair.Key);

			currRelevantEdge.InstancePrefabName = InstExitLayer;
		}
	}

	for(FName &currToRem : OWToRemove)
		OverworldLayer.Nodes.Remove(currToRem);

	for (auto &currOWRoom : OverworldLayer.Nodes)
		RoomsToNodes.Add(currOWRoom.Value.RoomRef, { FName(), currOWRoom.Key });
}

// Called when the game starts or when spawned
void ARoomStreamingManager::BeginPlay()
{
	Super::BeginPlay();

}

// Called every frame
void ARoomStreamingManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ARoomStreamingManager::HandleEnteringPlayer(APawn* EnteringPlayer, FRoomAddress SpawnRoom)
{
	// TODO
	//	Get the player's according saved data based on their info
	//		If there is no data that means they're new, do the newbie sequence thingy.
	//		Otherwise just set them up, load proper rooms, and place them in the right place.
	//			(This might actually go unfinished for a while since nurses aren't a thing yet).

	FPlayerRoomVisitor& NewVisitor = PlayerVisitors.Add(EnteringPlayer);

	NewVisitor.Current = NewVisitor.Previous = SpawnRoom;

	FRoomGraph* FoundLayer = (SpawnRoom.LayerName.IsNone() ? &OverworldLayer : InstancedLayers.Find(SpawnRoom.LayerName));
	FRoomNode* FoundNode = nullptr;

	if (nullptr != FoundLayer)
	{
		FoundNode = FoundLayer->Nodes.Find(SpawnRoom.RoomName);

		if (nullptr != FoundNode)
		{
			for (auto &currEdgePair : FoundNode->Edges)
			{
				if (currEdgePair.Value.InstancePrefabName.IsNone() || currEdgePair.Value.InstancePrefabName == InstExitLayer)
					RequestLoadInvis(currEdgePair.Value.ToNode);
			}

			for (FName &currVisible : FoundNode->VisibleFrom)
				RequestLoad({ SpawnRoom.LayerName, currVisible });

			ULevelStreaming* GrabbedRoom = FoundNode->RoomRef;

			if (nullptr != GrabbedRoom)
			{
				RequestLoad(SpawnRoom);

				WaitingDuringEntry.Add(GrabbedRoom, EnteringPlayer);

				if (GrabbedRoom->IsLevelLoaded())
					FinishEnterPlayer();
				else
					GrabbedRoom->OnLevelLoaded.AddDynamic(this, &ARoomStreamingManager::FinishEnterPlayer);
			}
			else
				UStaticFuncLib::Print("ARoomStreamingManager::HandleEnteringPlayer: Node at specified address (Layer \'" +
									  SpawnRoom.LayerName.ToString() + "\', Room \'" + SpawnRoom.RoomName.ToString() + "\') was found, "
									  "but did not have a reference to its streaming level counterpart! Player entry has failed.", true);
		}
		else
			UStaticFuncLib::Print("ARoomStreamingManager::HandleEnteringPlayer: Couldn't find a room named \'" +
								  SpawnRoom.RoomName.ToString() + "\' inside layer \'" + SpawnRoom.LayerName.ToString() +
								  "\'! Player entry has failed.", true);
	}
	else
		UStaticFuncLib::Print("ARoomStreamingManager::HandleEnteringPlayer: Couldn't find a layer named \'" +
							  SpawnRoom.LayerName.ToString() + "\'! Player entry has failed.", true);
}

void ARoomStreamingManager::FinishEnterPlayer()
{
	TArray<ULevelStreaming*> Finished;
	for (auto &currPair : WaitingDuringEntry)
	{
		if (currPair.Key->IsLevelLoaded())
		{
			ULevelStreaming* currLevel = currPair.Key;

			if (currLevel->OnLevelLoaded.IsAlreadyBound(this, &ARoomStreamingManager::FinishEnterPlayer))
				currLevel->OnLevelLoaded.RemoveDynamic(this, &ARoomStreamingManager::FinishEnterPlayer);

			APawn* currPlayer = currPair.Value;

			// TODO Edit this stuff when player placement is finally figured out.
			ARoomActor* FoundRA = nullptr;
			for (AActor* currActor : currLevel->GetLoadedLevel()->Actors)
			{
				APlayerStart* AsPS = Cast<APlayerStart>(currActor);

				if (nullptr != AsPS)
				{
					currPlayer->SetActorLocation(AsPS->GetActorLocation() + (FVector::UpVector * 1000));
					FoundRA = nullptr;
					break;
				}
				else if (nullptr == FoundRA)
					FoundRA = Cast<ARoomActor>(currActor);
			}

			if(nullptr != FoundRA)
				currPlayer->SetActorLocation(FoundRA->GetActorLocation() + (FVector::UpVector * 1000));

		}
	}

	for (ULevelStreaming* currDone : Finished)
		WaitingDuringEntry.Remove(currDone);
}

void ARoomStreamingManager::HandleExitingPlayer(APawn* LeavingPlayer)
{
	// TODO
	//		Might not get done for some time but tl;dr
	//		Save out the player's next spawning loc to file
	//		Make horse collapse on ground, have nearest nurse get them
	//
	//		(Maybe have a timer where you're collapsed but still considered
	//		'in', and if you reenter in time you don't get carried away by
	//		a nurse and you just snap back up in the place you were?)

	/*
		TODO also might be a good idea to check for layer removals here as well.
	*/

	PlayerVisitors.Remove(LeavingPlayer);
}

void ARoomStreamingManager::InstLoadOp_Implementation(ULevelStreaming* Seed, const FString& NewRoomName, float Offset)
{
	if (nullptr != Seed)
	{
		LastInstanceCreated = Seed->CreateInstance(NewRoomName);

		LastInstanceCreated->SetFlags(RF_WasLoaded);

		LastInstanceCreated->LevelTransform.SetLocation(FVector::UpVector * Offset);
	}
	else
		UStaticFuncLib::Print("ARoomStreamingManager::InstLoadOp: Seed null!", true);
}

void ARoomStreamingManager::InstUnloadOp_Implementation(ULevelStreaming* ToRemove)
{
	if (nullptr != ToRemove)
	{
		ToRemove->bIsRequestingUnloadAndRemoval = true;
		ToRemove->bShouldBeLoaded = false;
	}
	else
		UStaticFuncLib::Print("ARoomStreamingManager::InstLoadOp: ToRemove was null!", true);
}

FName ARoomStreamingManager::AddInstancedLayer(FName PrefabName, FName ParentLayerName)
{
	FRoomGraph* PrefabLayer = InstancedLayerPrefabs.Find(PrefabName);

	FName NewLayerName;
	if (nullptr != PrefabLayer)
	{
		NewLayerName = URoomStreamingFuncLib::MakeInstancedLayerName(this, PrefabName);

		FRoomGraph& NewLayer = InstancedLayers.Add(NewLayerName);

		NewLayer.Parent = ParentLayerName;
		NewLayer.Nodes = PrefabLayer->Nodes;

		if (UnusedOffsets.Num() > 0)
		{
			NewLayer.Offset = UnusedOffsets[0];
			UnusedOffsets.RemoveAt(0);
		}
		else
			NewLayer.Offset = InstancedLayers.Num() * -10000;

		for (auto &currNodePair : NewLayer.Nodes)
		{
			FString NewRoomName = NewLayerName.ToString() + "_Room_" + currNodePair.Key.ToString();

			InstLoadOp(currNodePair.Value.RoomRef, NewRoomName, NewLayer.Offset);

			currNodePair.Value.RoomRef = LastInstanceCreated;

			RoomsToNodes.Add(LastInstanceCreated, { NewLayerName, currNodePair.Key });

			for (auto &currEdgePair : currNodePair.Value.Edges)
			{
				if (currEdgePair.Value.InstancePrefabName == InstExitLayer)
					currEdgePair.Value.ToNode.LayerName = ParentLayerName;
				else if (currEdgePair.Value.InstancePrefabName.IsNone())
					currEdgePair.Value.ToNode.LayerName = NewLayerName;
			}
		}
	}
	else
		UStaticFuncLib::Print("ARoomStreamingManager::AddInstancedLayer: Couldn't find a prefab layer named \'" +
							  PrefabName.ToString() + "\'! Instanced layer creation has failed.", true);

	return NewLayerName;
}

void ARoomStreamingManager::RemoveInstancedLayer(FName LayerName)
{
	FRoomGraph* FoundLayer = InstancedLayers.Find(LayerName);

	if (nullptr != FoundLayer)
	{
		for (auto &currNodePair : FoundLayer->Nodes)
		{
			ULevelStreaming* RoomToRemove = currNodePair.Value.RoomRef;

			RoomsToNodes.Remove(RoomToRemove);

			InstUnloadOp(RoomToRemove);
		}

		UnusedOffsets.Add(FoundLayer->Offset);

		InstancedLayers.Remove(LayerName);
	}
	else
		UStaticFuncLib::Print("ARoomStreamingManager::RemoveInstancedLayer: Couldn't find an instance layer named \'" +
							  LayerName.ToString() + "\'! Removal request will be ignored.", true);
}

void ARoomStreamingManager::LoadOp_Implementation(ULevelStreaming* Room, bool Loaded, bool Visible)
{
	if (nullptr != Room)
	{
		Room->bShouldBeLoaded = Loaded;
		Room->bShouldBeVisible = Visible;
	}
}

void ARoomStreamingManager::RequestUnload(FRoomAddress RoomAddr)
{
	FRoomGraph* FoundLayer = (RoomAddr.LayerName.IsNone() ? &OverworldLayer : InstancedLayers.Find(RoomAddr.LayerName));
	FRoomNode* FoundNode = nullptr;
	if (nullptr != FoundLayer)
		FoundNode = FoundLayer->Nodes.Find(RoomAddr.RoomName);

	for (auto &currVisitorPair : PlayerVisitors)
	{
		FPlayerRoomVisitor& currVisitor = currVisitorPair.Value;

		// If a player is in this room we're not allowed to unload it.
		if (currVisitor.Current == RoomAddr)
			return;
		else
		{
			if (nullptr != FoundNode)
			{
				for (auto &currEdgePair : FoundNode->Edges)
				{
					// If a player neighbors this room we're not allowed to unload it.
					if (currEdgePair.Value.ToNode == RoomAddr)
						return;
				}
			}
		}
	}

	// otherwise we're allowed to unload it
	if (nullptr != FoundNode)
	{
		ULevelStreaming* GrabbedRoom = FoundNode->RoomRef;

		LoadOp(GrabbedRoom, false, false);
	}
}

void ARoomStreamingManager::RequestLoadInvis(FRoomAddress RoomAddr)
{
	for (auto &currVisitorPair : PlayerVisitors)
	{
		FPlayerRoomVisitor& currVisitor = currVisitorPair.Value;

		// If a player is in this room we're not allowed to invis it.
		if (currVisitor.Current == RoomAddr)
			return;
	}

	// otherwise we're allowed to load and/or invis it.

	FRoomGraph* FoundLayer = (RoomAddr.LayerName.IsNone() ? &OverworldLayer : InstancedLayers.Find(RoomAddr.LayerName));
	FRoomNode* FoundNode = nullptr;
	if (nullptr != FoundLayer)
		FoundNode = FoundLayer->Nodes.Find(RoomAddr.RoomName);

	if (nullptr != FoundNode)
	{
		ULevelStreaming* GrabbedRoom = FoundNode->RoomRef;

		LoadOp(GrabbedRoom, true, false);
	}
}

void ARoomStreamingManager::RequestLoad(FRoomAddress RoomAddr)
{
	// this always passes.

	FRoomGraph* FoundLayer = (RoomAddr.LayerName.IsNone() ? &OverworldLayer : InstancedLayers.Find(RoomAddr.LayerName));
	FRoomNode* FoundNode = nullptr;
	if (nullptr != FoundLayer)
		FoundNode = FoundLayer->Nodes.Find(RoomAddr.RoomName);

	if (nullptr != FoundNode)
	{
		ULevelStreaming* GrabbedRoom = FoundNode->RoomRef;

		LoadOp(GrabbedRoom, true, true);
	}
}

void ARoomStreamingManager::RequestTraversal(APawn* Player, FRoomEdge Edge, FVector PlayerOffset)
{
	FPlayerRoomVisitor* RelevantVisitor = PlayerVisitors.Find(Player);

	FRoomAddress OldPrevious;

	if (nullptr != RelevantVisitor)
	{
		FRoomAddress& CurrRoomAddr = RelevantVisitor->Current;
		FRoomAddress& NextRoomAddr = Edge.ToNode;

		RelevantVisitor->Previous = OldPrevious = RelevantVisitor->Current;
		RelevantVisitor->Current = Edge.ToNode;

		FRoomGraph* FromLayer = (CurrRoomAddr.LayerName.IsNone() ? &OverworldLayer : InstancedLayers.Find(CurrRoomAddr.LayerName));
		FRoomGraph* ToLayer = (NextRoomAddr.LayerName.IsNone() ? &OverworldLayer : InstancedLayers.Find(NextRoomAddr.LayerName));

		FRoomNode *FromNode = nullptr, *ToNode = nullptr;

		if (nullptr != FromLayer)
			FromNode = FromLayer->Nodes.Find(CurrRoomAddr.RoomName);

		if (nullptr != ToLayer)
			ToNode = ToLayer->Nodes.Find(NextRoomAddr.RoomName);

		if (nullptr != FromNode && nullptr != ToNode)
		{
			TArray<FRoomAddress> ToUnload;
			TArray<FRoomAddress> ToLoadInvis;
			TArray<FRoomAddress> ToLoad;

			for (auto &currOldNeighbor : FromNode->Edges)
				ToUnload.Add(currOldNeighbor.Value.ToNode);

			for (FName &currFromVisible : FromNode->VisibleFrom)
				ToUnload.Add({ CurrRoomAddr.LayerName, currFromVisible });

			ToUnload.Remove(NextRoomAddr);
			ToLoad.Add(NextRoomAddr);

			for (auto &currNewNeighbor : ToNode->Edges)
			{
				ToUnload.Remove(currNewNeighbor.Value.ToNode);
				ToLoadInvis.Add(currNewNeighbor.Value.ToNode);
			}

			for (FName &currToVisible : ToNode->VisibleFrom)
			{
				ToUnload.Remove({ CurrRoomAddr.LayerName, currToVisible });
				ToLoad.Add({ NextRoomAddr.LayerName, currToVisible });
			}

			for (FRoomAddress &currToUnload : ToUnload)
				RequestUnload(currToUnload);

			for (FRoomAddress &currToLoadInvis : ToLoadInvis)
				RequestLoadInvis(currToLoadInvis);

			for (FRoomAddress &currToLoad : ToLoad)
				RequestLoad(currToLoad);


			ULevelStreaming* GrabbedRoom = ToNode->RoomRef;

			if (nullptr != GrabbedRoom)
			{
				WaitingDuringTraverse.Add(GrabbedRoom, { Player, Edge.ToLZName, PlayerOffset });

				if (GrabbedRoom->IsLevelVisible())
					FinishTraversal();
				else
					GrabbedRoom->OnLevelShown.AddDynamic(this, &ARoomStreamingManager::FinishTraversal);
			}
		}

		if (UKismetSystemLibrary::IsStandalone(this))
			OldPrevious = RelevantVisitor->Previous;

		if (Edge.InstancePrefabName == InstExitLayer)
		{

			for (auto &currVisPair : PlayerVisitors)
			{
				FPlayerRoomVisitor& currVisitor = currVisPair.Value;

				if (RelevantVisitor != &currVisitor && currVisitor.Current.LayerName == OldPrevious.LayerName)
					return;
			}

			RemoveInstancedLayer(OldPrevious.LayerName);
		}
	}
	else
		UStaticFuncLib::Print("ARoomStreamingManager::RequestTraversal: Couldn't find a preexisting visitor for player \'" +
							  Player->GetName() + "\'! Traversal has failed.", true);
}

void ARoomStreamingManager::FinishTraversal()
{
	TArray<ULevelStreaming*> Finished;
	for (auto &currPair : WaitingDuringTraverse)
	{
		if (currPair.Key->IsLevelVisible())
		{
			ULevelStreaming* currLevel = currPair.Key;

			if (currLevel->OnLevelShown.IsAlreadyBound(this, &ARoomStreamingManager::FinishTraversal))
				currLevel->OnLevelShown.RemoveDynamic(this, &ARoomStreamingManager::FinishTraversal);

			APawn* currPlayer = currPair.Value.Player;

			for (AActor* currActor : currLevel->GetLoadedLevel()->Actors)
			{
				ALoadingZone* AsLZ = Cast<ALoadingZone>(currActor);

				if (nullptr != AsLZ && AsLZ->LZName == currPair.Value.LZName)
					currPlayer->SetActorLocation(AsLZ->GetActorLocation() + currPair.Value.Offset);
			}

			Finished.Add(currLevel);
		}
	}

	for (ULevelStreaming* currDone : Finished)
		WaitingDuringTraverse.Remove(currDone);
}

void ARoomStreamingManager::RebindNode(ULevelStreaming* NodeRoom,
	const TArray<FName>& NewEdgeNames, const TArray<FRoomEdge>& NewEdges, const TArray<FName>& NewVisibleFroms)
{
	FRoomAddress* FoundAddr = RoomsToNodes.Find(NodeRoom);

	if (nullptr != FoundAddr)
	{
		FRoomGraph* FoundGraph = (FoundAddr->LayerName.IsNone() ? &OverworldLayer : InstancedLayers.Find(FoundAddr->LayerName));

		if (nullptr != FoundGraph)
		{
			FRoomNode* FoundNode = FoundGraph->Nodes.Find(FoundAddr->RoomName);

			if (nullptr != FoundNode)
			{
				TMap<FName, FRoomEdge> NewEdgesCombined;
				for (int i = 0; i < NewEdgeNames.Num(); ++i)
					NewEdgesCombined.Add(NewEdgeNames[i], NewEdges[i]);

				TArray<FRoomAddress> ToUnload;
				TArray<FRoomAddress> ToLoadInvis;
				TArray<FRoomAddress> ToLoad;

				for (auto &currOldNeighbor : FoundNode->Edges)
					ToUnload.Add(currOldNeighbor.Value.ToNode);

				for (FName &currFromVisible : FoundNode->VisibleFrom)
					ToUnload.Add({ FoundAddr->LayerName, currFromVisible });

				FoundNode->Edges.Append(NewEdgesCombined);

				for (auto &currNewNeighbor : FoundNode->Edges)
				{
					ToUnload.Remove(currNewNeighbor.Value.ToNode);
					ToLoadInvis.Add(currNewNeighbor.Value.ToNode);
				}

				for (FName &currToVisible : FoundNode->VisibleFrom)
				{
					ToUnload.Remove({ FoundAddr->LayerName, currToVisible });
					ToLoad.Add({ FoundAddr->LayerName, currToVisible });
				}

				for (FRoomAddress &currToUnload : ToUnload)
					RequestUnload(currToUnload);

				for (FRoomAddress &currToLoadInvis : ToLoadInvis)
					RequestLoadInvis(currToLoadInvis);

				for (FRoomAddress &currToLoad : ToLoad)
					RequestLoad(currToLoad);
			}
			else
				UStaticFuncLib::Print("ARoomStreamingManager::RebindNode: Couldn't find a room named \'" +
					FoundAddr->RoomName.ToString() + "\' in layer \'" + FoundAddr->LayerName.ToString() + "\'!", true);
		}
		else
			UStaticFuncLib::Print("ARoomStreamingManager::RebindNode: Couldn't find a layer named \'" +
				FoundAddr->LayerName.ToString() + "\'!", true);
	}
	else
		UStaticFuncLib::Print("ARoomStreamingManager::RebindNode: Couldn't find a Room Node associated with Streamed Level \'" +
			NodeRoom->GetName() + "\'!", true);
}


ARoomStreamingManager* URoomStreamingFuncLib::GetRoomStreamingManager(UObject* WorldContext)
{
	for (TActorIterator<ARoomStreamingManager> Iter(WorldContext->GetWorld()); Iter; ++Iter)
	{
		if (!(*Iter)->IsPendingKill())
			return *Iter;
	}

	return nullptr;
}

FName URoomStreamingFuncLib::MakeInstancedLayerName(UObject* WorldContext, FName PrefixName)
{
	ARoomStreamingManager* StreamManager = URoomStreamingFuncLib::GetRoomStreamingManager(WorldContext);

	if (nullptr != StreamManager)
		return *(PrefixName.ToString() + "_Inst_" + FString::FromInt(StreamManager->UniqueInstanceLayerID++));
	else
		UStaticFuncLib::Print("URoomStreamingFuncLib::MakeInstancedLayerName: Couldn't retrieve the Room Streaming Manager!", true);

	return "INST_LAYER_ERROR";
}

void URoomStreamingFuncLib::RebindNode(UObject* WorldContext,
	ULevelStreaming* NodeRoom, const TArray<FName>& NewEdgeNames, const TArray<FRoomEdge>& NewEdges, const TArray<FName>& NewVisibleFroms)
{
	ARoomStreamingManager* StreamManager = URoomStreamingFuncLib::GetRoomStreamingManager(WorldContext);

	if (nullptr != StreamManager)
		StreamManager->RebindNode(NodeRoom, NewEdgeNames, NewEdges, NewVisibleFroms);
	else
		UStaticFuncLib::Print("URoomStreamingFuncLib::RebindNode: Couldn't retrieve the Room Streaming Manager!", true);
}
