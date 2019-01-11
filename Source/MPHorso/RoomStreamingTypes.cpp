// Fill out your copyright notice in the Description page of Project Settings.

#include "MPHorso.h"
#include "RoomStreamingTypes.h"

//#include "Kismet/GameplayStatics.h"

//#include "Engine/LevelStreaming.h"
#include "Engine/LevelStreamingKismet.h"

#include "StaticFuncLib.h"

#include "MPHorsoPlayerController.h"

#include "PlayerRespawnPointBase.h"

#include "MPHorsoGameInstance.h"


void FStreamedRoom::ClearRoom()
{
	if (nullptr != RoomRef)
	{
		RoomRef->bIsRequestingUnloadAndRemoval = true;
		RoomRef->bShouldBeLoaded = false;

		RoomRef = nullptr;
	}
}


void FStreamedRoomLayer::TryUnloadRooms(FName RootRoomName)
{
	FStreamedRoom* RootRoom = RoomsInLayer.Find(RootRoomName);

	if (nullptr != RootRoom)
	{
		// Unload this room, its neighbors, and any rooms visible from this one, unless they contain a player or are adjacent to one.

		if (CanUnloadRoom(RootRoomName))
			RootRoom->UnloadRoom();

		for (FStreamedRoomAddress &currNeigh : RootRoom->NeighboringRooms)
		{
			if (currNeigh.AddressType == EStreamedRoomAddressType::NormalAddress && CanUnloadRoom(currNeigh.RoomName))
				RoomsInLayer.Find(currNeigh.RoomName)->UnloadRoom();
		}

		for (FStreamedRoomAddress &currVis : RootRoom->VisibleRooms)
		{
			if (currVis.AddressType == EStreamedRoomAddressType::NormalAddress && CanUnloadRoom(currVis.RoomName))
				RoomsInLayer.Find(currVis.RoomName)->UnloadRoom();
		}
	}
}

void FStreamedRoomLayer::TraverseFrom(FString PlayerID, FName RoomName)
{
	FStreamedRoom* RootRoom = RoomsInLayer.Find(RoomName);

	if (nullptr != RootRoom)
		RootRoom->PlayersPresent.Remove(PlayerID);

	PlayersPresent.Remove(PlayerID);
}

void FStreamedRoomLayer::TryLoadRooms(FName RootRoomName)
{
	FStreamedRoom* RootRoom = RoomsInLayer.Find(RootRoomName);

	if (nullptr != RootRoom)
	{
		// Load this room, its neighbors, and any rooms that are visible from this one.

		RootRoom->LoadRoom();

		TArray<FStreamedRoom*> Neighbors;
		GetRoomNeighbors(RootRoomName, Neighbors);

		for (FStreamedRoom* currNeigh : Neighbors)
			currNeigh->LoadRoom();

		for (FStreamedRoomAddress &currVisAddr : RootRoom->VisibleRooms)
		{
			FStreamedRoom* currVis = RoomsInLayer.Find(currVisAddr.RoomName);

			if (nullptr != currVis)
				currVis->LoadRoom();
		}
	}
}

void FStreamedRoomLayer::TraverseTo(FString PlayerID, FName RoomName)
{
	FStreamedRoom* RootRoom = RoomsInLayer.Find(RoomName);

	if (nullptr != RootRoom)
	{
		RootRoom->PlayersPresent.Add(PlayerID);

		TryLoadRooms(RoomName);
	}

	PlayersPresent.Add(PlayerID);
}

void FStreamedRoomLayer::GetRoomNeighbors(FName RoomName, TArray<FStreamedRoom*>& NeighboringRooms)
{
	FStreamedRoom* RootRoom = RoomsInLayer.Find(RoomName);

	if (nullptr != RootRoom)
	{
		for (FStreamedRoomAddress &currAddr : RootRoom->NeighboringRooms)
		{
			if (currAddr.AddressType == EStreamedRoomAddressType::NormalAddress)
			{
				FStreamedRoom* currNeigh = RoomsInLayer.Find(currAddr.RoomName);

				if (nullptr != currNeigh)
					NeighboringRooms.Add(currNeigh);
			}
		}
	}
}

bool FStreamedRoomLayer::CanUnloadRoom(FName RoomName)
{
	FStreamedRoom* RootRoom = RoomsInLayer.Find(RoomName);

	if (nullptr != RootRoom)
	{
		TArray<FStreamedRoom*> Neighbors;
		GetRoomNeighbors(RoomName, Neighbors);
		bool AnyPlayersNearby = Neighbors.ContainsByPredicate([](FStreamedRoom* a) {return a->PlayersPresent.Num() > 0; });

		return !AnyPlayersNearby && RootRoom->PlayersPresent.Num() < 1;
	}

	return false;
}

void FStreamedRoomLayer::Clear()
{
	for (auto &currPair : RoomsInLayer)
		currPair.Value.ClearRoom();

	RoomsInLayer.Empty();
}

void FStreamedRoomLayer::CloneLayer(FStreamedRoomLayer& Clone, FName NewLayerName, float NewLayerOffset)
{
	for (auto &currPair : RoomsInLayer)
	{
		if (nullptr != currPair.Value.RoomRef)
		{
			FStreamedRoom& RoomClone = Clone.RoomsInLayer.Add(currPair.Key, currPair.Value);
			RoomClone.PlayersPresent.Empty();

			FString NewRoomName = NewLayerName.ToString() + "_Room_" + currPair.Key.ToString();
			RoomClone.RoomRef = currPair.Value.RoomRef->CreateInstance(NewRoomName);

			// Hack to circumvent UObject::IsNameStableForNetworking()
			RoomClone.RoomRef->SetFlags(RF_WasLoaded);

			RoomClone.RoomRef->LevelTransform.AddToTranslation((FVector::UpVector * NewLayerOffset));
		}
		else
			UStaticFuncLib::Print("FStreamedRoomLayer::CloneLayer: Couldn't find a reference to room \'" +
								  currPair.Key.ToString() + "\' to clone!", true);
	}
}


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

	if (UKismetSystemLibrary::IsServer(this))
	{
		ARoomStreamingManager* StreamManager = URoomStreamingFuncLib::GetRoomStreamingManager(this);

		if (nullptr != StreamManager && DestinationRoom.AddressType == EStreamedRoomAddressType::InstanceEntrance)
		{
			DestPrefabLayer = DestinationRoom.LayerName;
			StreamManager->RequestNewLayer(this);
		}

		TSet<AActor*> InitialOverlappers;
		GetOverlappingActors(InitialOverlappers);
		for (AActor* curr : InitialOverlappers)
			NotifyActorBeginOverlap(curr);
	}

}

// Called every frame
void ALoadingZone::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ALoadingZone::NotifyActorBeginOverlap(AActor* OtherActor)
{
	if (UKismetSystemLibrary::IsServer(this))
	{
		APawn* AsPawn = Cast<APawn>(OtherActor);

		if (nullptr != AsPawn)
		{
			ARoomStreamingManager* StreamManager = URoomStreamingFuncLib::GetRoomStreamingManager(this);

			if (nullptr != StreamManager)
			{
				// containers used to guarantee that ops only happen once per player, per traversal.

				if (ArrivingPlayers.Contains(AsPawn))
					HandleArriving(AsPawn);
				else if (!DepartingPlayers.Contains(AsPawn))
					HandleDeparting({ AsPawn });
			}
			else
				UStaticFuncLib::Print("ALoadingZone::NotifyActorBeginOverlap: Couldn't retrieve the Room Streaming Manager!", true);
		}
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

void ALoadingZone::HandleArriving(APawn* Target)
{
	OnRoomEntered.Broadcast(this, Target);
	BroadcastArrivingAnimation(Target);
}

void ALoadingZone::HandleDeparting(const TArray<APawn*>& Targets)
{
	ARoomStreamingManager* StreamManager = URoomStreamingFuncLib::GetRoomStreamingManager(this);

	if (nullptr != StreamManager)
	{
		DepartingPlayers.Append(Targets);

		if (StreamManager->RequestTraversal(Targets, this))
			DoDepartingAnimation(Targets);
		else
			UStaticFuncLib::Print("ALoadingZone::HandleDeparting_Implementation: Traversal has failed. Nothing will happen.", true);
	}
	else
		UStaticFuncLib::Print("ALoadingZone::HandleDeparting_Implementation: Couldn't retrieve the Room Streaming Manager!", true);
}

void ALoadingZone::FinishDepartingAnimation()
{
	if(UKismetSystemLibrary::IsServer(this))
	{
		ARoomStreamingManager* StreamManager = URoomStreamingFuncLib::GetRoomStreamingManager(this);

		if (nullptr != StreamManager)
		{
			FStreamedRoomTreeNode* RetrievedLayer = StreamManager->LayerNodes.Find(CurrentTargetAddress.LayerName);

			if (nullptr != RetrievedLayer)
			{
				FStreamedRoom* RetrievedRoom = RetrievedLayer->Layer.RoomsInLayer.Find(CurrentTargetAddress.RoomName);

				if (nullptr != RetrievedRoom)
				{
					CurrentRoomRef = RetrievedRoom->RoomRef;

					if (nullptr != CurrentRoomRef)
					{
						if (CurrentRoomRef->IsLevelLoaded())
							OnFinallyDepart();
						else
							CurrentRoomRef->OnLevelLoaded.AddDynamic(this, &ALoadingZone::OnFinallyDepart);
					}
					else
						UStaticFuncLib::Print("ALoadingZone::FinishDepartingAnimation: Successfully retrieved room at address {\'" +
											  CurrentTargetAddress.LayerName.ToString() + "\', \'" + CurrentTargetAddress.RoomName.ToString() +
											  "\'}, but the room reference stored within was invalid!", true);
				}
				else
					UStaticFuncLib::Print("ALoadingZone::FinishDepartingAnimation: Couldn't retrieve room \'" +
										  CurrentTargetAddress.RoomName.ToString() + "\' in layer \'" +
										  CurrentTargetAddress.LayerName.ToString() + "\'!", true);
			}
			else
				UStaticFuncLib::Print("ALoadingZone::FinishDepartingAnimation: Couldn't retrieve layer \'" +
									  CurrentTargetAddress.LayerName.ToString() + "\'!", true);
		}
		else
			UStaticFuncLib::Print("ALoadingZone::FinishDepartingAnimation: Couldn't retrieve the Room Streaming Manager!", true);
	}
}

void ALoadingZone::OnFinallyDepart()
{
	if (CurrentRoomRef->OnLevelLoaded.IsAlreadyBound(this, &ALoadingZone::OnFinallyDepart))
		CurrentRoomRef->OnLevelLoaded.RemoveDynamic(this, &ALoadingZone::OnFinallyDepart);

	ARoomActor* BackupRA = nullptr;
	for (AActor *curr : CurrentRoomRef->GetLoadedLevel()->Actors)
	{
		ALoadingZone* AsLZ = Cast<ALoadingZone>(curr);
		if (nullptr != AsLZ && AsLZ->LZName == DestinationLZ)
		{
			AsLZ->ArrivingPlayers.Append(DepartingPlayers);

			FVector PlaceLoc;
			
			// Stupid hack because unreal won't just let me check SceneComponent::bWorldToComponentUpdated
			if (AsLZ->GetActorTransform().Equals(FTransform::Identity))
			{
				bool PlaceSet = false;
				ARoomStreamingManager* StreamManager = URoomStreamingFuncLib::GetRoomStreamingManager(this);

				if (nullptr != StreamManager)
				{
					FStreamedRoomAddress Addr;
					StreamManager->GetRoomAddress(AsLZ, Addr);

					FStreamedRoomTreeNode* FoundLayer = StreamManager->LayerNodes.Find(Addr.LayerName);
					if (nullptr != FoundLayer)
					{
						FStreamedRoom* FoundRoom = FoundLayer->Layer.RoomsInLayer.Find(Addr.RoomName);

						if (nullptr != FoundRoom && nullptr != FoundRoom->RoomRef)
						{
							PlaceLoc = FoundRoom->RoomRef->LevelTransform.TransformPosition(AsLZ->RootComponent->RelativeLocation);
							PlaceSet = true;
						}
					}
				}

				if (!PlaceSet)
					PlaceLoc = AsLZ->RootComponent->RelativeLocation;
			}
			else
				PlaceLoc = AsLZ->GetActorLocation();

			for (APawn* currDeparting : DepartingPlayers)
				currDeparting->SetActorLocation(PlaceLoc + GetPlayerOffset(currDeparting), false, nullptr, ETeleportType::TeleportPhysics);

			BackupRA = nullptr;
			break;
		}
		else if (nullptr == BackupRA)
			BackupRA = Cast<ARoomActor>(curr);
	}

	TArray<APawn*> DepartingArr = DepartingPlayers.Array();

	if (BackupRA != nullptr)
	{
		UStaticFuncLib::Print("ALoadingZone::OnFinallyDepart: No Loading Zone named \'" +
							  DestinationLZ.ToString() + "\' was found within room {\'" +
							  CurrentTargetAddress.LayerName.ToString() + "\', \'" + CurrentTargetAddress.RoomName.ToString() +
							  "\'}! Spawning players at the room's RoomActor instead.", true);

		for (APawn* currDeparting : DepartingPlayers)
			currDeparting->SetActorLocation(BackupRA->GetActorLocation() + GetPlayerOffset(currDeparting), false, nullptr, ETeleportType::TeleportPhysics);

		BroadcastFixingArriveAnimation(DepartingArr);
	}

	OnRoomExited.Broadcast(this, DepartingArr);

	DepartingPlayers.Empty();

	ARoomStreamingManager* StreamManager = URoomStreamingFuncLib::GetRoomStreamingManager(this);

	if (nullptr != StreamManager)
		StreamManager->PostTraverse(this);
	else
		UStaticFuncLib::Print("ALoadingZone::OnFinallyDepart: Couldn't retrieve the Room Streaming Manager! "
							  "Post-Traversal behavior will not occur.", true);
}



// Sets default values
ARoomCullVolume::ARoomCullVolume(const FObjectInitializer& _init) : Super(_init)
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	//RootComponent = _init.CreateDefaultSubobject<USceneComponent>(this, TEXT("RootComponent"));
}

void ARoomCullVolume::OnConstruction(const FTransform& Transform)
{
#if WITH_EDITOR

	TInlineComponentArray<UShapeComponent*> Shapes(this);
	for (UShapeComponent* currShape : Shapes)
		currShape->ShapeColor = FColor::Red;

#endif
}

// Called when the game starts or when spawned
void ARoomCullVolume::BeginPlay()
{
	Super::BeginPlay();

}

// Called every frame
void ARoomCullVolume::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

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

	if (UpdateRoomName)
	{
		RoomName = *UGameplayStatics::GetCurrentLevelName(this);

		UpdateRoomName = false;
	}

#endif

}

// Called when the game starts or when spawned
void ARoomActor::BeginPlay()
{
	Super::BeginPlay();

	ULevel* RetrievedLevel = GetLevel();

	for (AActor* curr : RetrievedLevel->Actors)
	{
		ALoadingZone* AsLZ = Cast<ALoadingZone>(curr);

		if (nullptr != AsLZ)
		{
			AsLZ->OnRoomEntered.AddDynamic(this, &ARoomActor::HandleRoomEntry);
			AsLZ->OnRoomExited.AddDynamic(this, &ARoomActor::HandleRoomExiting);
		}
		else
		{
			ARoomCullVolume* AsRCV = Cast<ARoomCullVolume>(curr);

			if (nullptr != AsRCV)
				RoomCullVolumes.Add(AsRCV);
		}
	}

	ARoomStreamingManager* StreamManager = URoomStreamingFuncLib::GetRoomStreamingManager(this);

	if (nullptr != StreamManager)
	{
		auto Pred = [&RetrievedLevel](ULevelStreaming* a) {return a->GetLoadedLevel() == RetrievedLevel; };
		ULevelStreaming* RetrievedLS = *GetWorld()->StreamingLevels.FindByPredicate(Pred);
		FStreamedRoomAddress* FoundAddr = StreamManager->RoomAddresses.Find(RetrievedLS);

		if (nullptr != FoundAddr)
		{
			bool ShouldCull = true;
			for (auto &currPair : StreamManager->PlayerIDs)
			{
				if (StreamManager->PlayerAddresses.FindRef(currPair.Value) == *FoundAddr)
				{
					APawn* AsPawn = Cast<APawn>(currPair.Key);
					APawn* CastedOwner = Cast<APawn>(currPair.Key->GetOwner());

					AController* FocusController =
						(nullptr != CastedOwner ? CastedOwner->GetController() : (nullptr != AsPawn ? AsPawn->GetController() : nullptr));

					if (FocusController == UGameplayStatics::GetPlayerController(this, 0))
					{
						ShouldCull = false;
						break;
					}
				}
			}

			SetRoomIsCulling(ShouldCull);
		}
	}
}

// Called every frame
void ARoomActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ARoomActor::HandleRoomEntry(AActor* EnteredFromLZ, APawn* EnteringPlayer)
{
	APawn* CastedOwner = Cast<APawn>(EnteringPlayer->GetOwner());

	AController* FocusController = (nullptr != CastedOwner ? CastedOwner->GetController() : EnteringPlayer->GetController());

	SetRoomIsCulling(!(FocusController == UGameplayStatics::GetPlayerController(this, 0)));
}

void ARoomActor::HandleRoomExiting(AActor* ExitedFromLZ, const TArray<APawn*>& ExitingPlayers)
{
	auto Pred = [](APawn* a)
	{
		APawn* CastedOwner = Cast<APawn>(a->GetOwner());
		AController* FocusController = (nullptr != CastedOwner ? CastedOwner->GetController() : a->GetController());

		return (FocusController == UGameplayStatics::GetPlayerController(a, 0));
	};

	SetRoomIsCulling(!ExitingPlayers.ContainsByPredicate(Pred));
}


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
		UpdateOverworldData = false;
		// TODO? maybe draw the structure of the world with debug lines and stuff?

		LayerNodes.Empty();
		RoomAddresses.Empty();

		FString WarnString;
		for (ULevelStreaming* currStreamVol: GetWorld()->StreamingLevels)
		{
			ULevel* currLevel = currStreamVol->GetLoadedLevel();
			if (nullptr != currLevel)
			{
				TArray<ALoadingZone*> FoundLZs;
				ARoomActor* FoundRA = nullptr;
				for (AActor* currActor : currLevel->Actors)
				{
					ALoadingZone* AsLZ = Cast<ALoadingZone>(currActor);

					if (nullptr != AsLZ)
						FoundLZs.Add(AsLZ);
					else if (nullptr == FoundRA)
						FoundRA = Cast<ARoomActor>(currActor);
				}

				if (nullptr != FoundRA)
				{
					FStreamedRoomTreeNode& FocusLayer = LayerNodes.FindOrAdd(FoundRA->LayerName);
					if (!FocusLayer.Layer.RoomsInLayer.Contains(FoundRA->RoomName))
					{
						FStreamedRoom NewStreamedRoom;

						NewStreamedRoom.RoomRef = currStreamVol;

						for (ALoadingZone* currFoundLZ : FoundLZs)
						{
							if (currFoundLZ->DestinationRoom.AddressType == EStreamedRoomAddressType::NormalAddress)
								NewStreamedRoom.NeighboringRooms.Add(currFoundLZ->DestinationRoom);
						}

						for (FName &currVisibleFrom : FoundRA->VisibleFroms)
							NewStreamedRoom.VisibleRooms.Add({ FName(), currVisibleFrom, EStreamedRoomAddressType::NormalAddress });

						FocusLayer.Layer.RoomsInLayer.Add(FoundRA->RoomName, NewStreamedRoom);

						RoomAddresses.Add(currStreamVol, { FoundRA->LayerName, FoundRA->RoomName, EStreamedRoomAddressType::NormalAddress });
					}
					else
						WarnString += "\n\t Level \'" + currLevel->GetName() +
									  "\' in Volume \'" + currStreamVol->GetName() + "\' (room name already taken)";
				}
				else
					WarnString += "\n\t Level \'" + currLevel->GetName() +
								  "\' in Volume \'" + currStreamVol->GetName() + "\' (no roomactor found)";
			}
			else
				WarnString += "\n\t Volume \'" + currStreamVol->GetName() + "\' (level is not loaded)";

			if (!WarnString.IsEmpty())
			{
				UStaticFuncLib::Print("ARoomStreamingManager::OnConstruction: Some rooms have failed to be added:" + WarnString, true);

				// TODO? maybe draw debug stuff over these rooms to make them easier to locate?
			}
		}
	}

#endif

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

void ARoomStreamingManager::HandleEnteringPlayer(APawn* EnteringPlayer, const FString& PlayerName, const FString& PlayerID)
{
	// TODO
	//	Get the player's according saved data based on their info
	//		If there is no data that means they're new, do the newbie sequence thingy.
	//		Otherwise just set them up, load proper rooms, and place them in the right place.
	//			(This might actually go unfinished for a while since nurses aren't a thing yet).

	FString FocusPlayerID = PlayerID;
	FStreamedRoomAddress SpawnAddr;

	UMPHorsoGameInstance* gameInst = UStaticFuncLib::RetrieveGameInstance(this);

	if (nullptr != gameInst)
	{
		UWorldSaveBase* WorldSave = gameInst->GetWorldSave();

		if (nullptr != WorldSave)
		{
			FWorldboundPlayerData* RetrievedData = WorldSave->PlayerIDsAndData.Find(PlayerID);

			if (nullptr != RetrievedData)
				SpawnAddr = { FName(), RetrievedData->RespawnRoomName, EStreamedRoomAddressType::NormalAddress };
			else
				SpawnAddr = { FName(), DebugSpawnRoom, EStreamedRoomAddressType::NormalAddress };
		}
	}
	else
		UStaticFuncLib::Print("ARoomStreamingManager::HandleEnteringPlayer: Couldn't retrieve the game instance!", true);


	FStreamedRoomTreeNode* FoundLayer = LayerNodes.Find(SpawnAddr.LayerName);

	if (nullptr != FoundLayer)
	{
		FStreamedRoom* FoundRoom = FoundLayer->Layer.RoomsInLayer.Find(SpawnAddr.RoomName);

		if (nullptr != FoundRoom)
		{
			if (nullptr != FoundRoom->RoomRef)
			{
				BeginTraverseTo(FocusPlayerID, SpawnAddr);
				PlayerIDs.Add(EnteringPlayer, FocusPlayerID);

				if (FoundRoom->RoomRef->IsLevelLoaded())
					PlaceEnteringPlayer(EnteringPlayer, FoundRoom->RoomRef);
				else
				{
					WaitingDuringEntry.Add(FoundRoom->RoomRef, EnteringPlayer);
					FoundRoom->RoomRef->OnLevelLoaded.AddDynamic(this, &ARoomStreamingManager::FinishEnterPlayer);
				}
			}
			else
				UStaticFuncLib::Print("ARoomStreamingManager::HandleEnteringPlayer: Found room \'" +
					SpawnAddr.RoomName.ToString() + "\' in the overworld, but its internal reference was "
					"invalid!", true);
		}
		else
			UStaticFuncLib::Print("ARoomStreamingManager::HandleEnteringPlayer: Couldn't find a room named \'" +
				SpawnAddr.RoomName.ToString() + "\' in the overworld!", true);
	}
	else
		UStaticFuncLib::Print("ARoomStreamingManager::HandleEnteringPlayer: Was unable to retrieve the overworld layer!", true);
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

			PlaceEnteringPlayer(currPair.Value, currLevel);

			Finished.Add(currLevel);
		}
	}
	
	for (ULevelStreaming* currDone : Finished)
		WaitingDuringEntry.Remove(currDone);
}

void ARoomStreamingManager::PlopEnteringPlayer(APawn* EnteringPlayer, FStreamedRoomAddress Addr)
{
	FStreamedRoomTreeNode* FoundLayer = LayerNodes.Find(Addr.LayerName);

	if (nullptr != FoundLayer)
	{
		FStreamedRoom* FoundRoom = FoundLayer->Layer.RoomsInLayer.Find(Addr.RoomName);

		if (nullptr != FoundRoom)
		{
			if (nullptr != FoundRoom->RoomRef)
			{
				FString& NewPlayerID =
					PlayerIDs.Add(EnteringPlayer, EnteringPlayer->GetName());

				BeginTraverseTo(NewPlayerID, Addr);

				if (FoundRoom->RoomRef->IsLevelLoaded())
					PlaceEnteringPlayer(EnteringPlayer, FoundRoom->RoomRef);
				else
				{
					WaitingDuringEntry.Add(FoundRoom->RoomRef, EnteringPlayer);
					FoundRoom->RoomRef->OnLevelLoaded.AddDynamic(this, &ARoomStreamingManager::FinishEnterPlayer);
				}
			}
			else
				UStaticFuncLib::Print("ARoomStreamingManager::PlopEnteringPlayer: Found room \'" +
					Addr.RoomName.ToString() + "\' in the overworld, but its internal reference was "
					"invalid!", true);
		}
		else
			UStaticFuncLib::Print("ARoomStreamingManager::PlopEnteringPlayer: Couldn't find a room named \'" +
				Addr.RoomName.ToString() + "\' in the overworld!", true);
	}
	else
		UStaticFuncLib::Print("ARoomStreamingManager::PlopEnteringPlayer: Was unable to retrieve the overworld layer!", true);
}

void ARoomStreamingManager::PlaceEnteringPlayer(APawn* EnteringPlayer, ULevelStreaming* Room)
{
	if (Room->IsLevelLoaded())
	{
		ARoomActor* BackupRA = nullptr;
		for (AActor *curr : Room->GetLoadedLevel()->Actors)
		{
			APlayerRespawnPointBase* AsResp = Cast<APlayerRespawnPointBase>(curr);
			if (nullptr != AsResp)
			{
				AsResp->HandleRespawn(EnteringPlayer);

				BackupRA = nullptr;
				break;
			}
			else if (nullptr == BackupRA)
				BackupRA = Cast<ARoomActor>(curr);
		}

		if (BackupRA != nullptr)
		{
			UStaticFuncLib::Print("ARoomStreamingManager::PlaceEnteringPlayer: No valid respawn point actor was found within room \'" +
				BackupRA->RoomName.ToString() + "\' within the overworld! Spawning player at the room's RoomActor instead.", true);

			EnteringPlayer->SetActorLocation(BackupRA->GetActorLocation(), false, nullptr, ETeleportType::TeleportPhysics);
		}
	}
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


	FString* LeavingPlayerID = PlayerIDs.Find(LeavingPlayer);

	if (nullptr != LeavingPlayerID)
	{
		FStreamedRoomAddress* PlayerAddr = PlayerAddresses.Find(*LeavingPlayerID);

		if (nullptr != PlayerAddr)
		{
			FStreamedRoomTreeNode* FoundLayer = LayerNodes.Find(PlayerAddr->LayerName);

			if (nullptr != FoundLayer)
			{
				if (FoundLayer->Layer.RoomsInLayer.Contains(PlayerAddr->RoomName))
				{
					BeginTraverseFrom(*LeavingPlayerID, *PlayerAddr, true);
					PostTraverseDirect(*PlayerAddr);
				}
			}
		}

		PlayerIDs.Remove(LeavingPlayer);
	}
}

bool ARoomStreamingManager::GetRoomAddress(AActor* FocusActor, FStreamedRoomAddress& FoundAddr)
{
	ULevel* FocusLevel = FocusActor->GetLevel();

	for (auto &currPair : RoomAddresses)
	{
		if (currPair.Key->GetLoadedLevel() == FocusLevel)
		{
			FoundAddr = currPair.Value;
			return true;
		}
	}

	UStaticFuncLib::Print("ARoomStreamingManager::GetRoomAddress: Couldn't find an address for level \'" +
						  FocusLevel->GetName() + "\'!", true);

	return false;
}

void ARoomStreamingManager::RequestNewLayer(ALoadingZone* Requester)
{
	FStreamedRoomAddress RequesteeAddr;
	if (GetRoomAddress(Requester, RequesteeAddr))
	{
		FStreamedRoomTreeNode* RequesteeLayer = LayerNodes.Find(RequesteeAddr.LayerName);

		if (nullptr != RequesteeLayer)
		{
			for (int i = RequesteeLayer->Children.Num() - 1; i >= 0; --i)
			{
				FName& currCandidate = RequesteeLayer->Children[i];

				FStreamedRoomTreeNode* candidateLayer = LayerNodes.Find(currCandidate);

				if (nullptr != candidateLayer)
				{
					if (candidateLayer->Layer.InstantiatingLZName == Requester->LZName && candidateLayer->Layer.PlayersPresent.Num() < 1)
						return;
				}
			}

			/*
				If we got this far that means there definitely isn't a layer waiting for this guy so we'll have to create one
				for them
			*/
			BeginConstructLayer(Requester, RequesteeAddr.LayerName);
		}
		else
			UStaticFuncLib::Print("ARoomStreamingManager::RequestNewLayer: Couldn't find a layer named \'" +
				RequesteeAddr.LayerName.ToString() + "\'!", true);
	}
	else
		UStaticFuncLib::Print("ARoomStreamingManager::RequestNewLayer: GetRoomAddress call has failed.", true);
}

void ARoomStreamingManager::BeginConstructLayer(ALoadingZone* Creator, const FName& ParentLayerName)
{
	if (UKismetSystemLibrary::IsServer(this))
	{
		if (LayerNodes.Contains(ParentLayerName))
		{
			if (LayerNodes.Contains(Creator->DestPrefabLayer))
			{
				FName NewLayerName = *(Creator->DestPrefabLayer.ToString() + "_Inst_" + FString::FromInt(UniqueInstanceLayerID++));

				WaitingDuringLayerCreation.Add(NewLayerName, Creator);

				ConstructLayer(NewLayerName, Creator->DestPrefabLayer, ParentLayerName);
			}
			else
				UStaticFuncLib::Print("ARoomStreamingManager::BeginConstructLayer: Couldn't find a prefab layer named \'" +
					Creator->DestPrefabLayer.ToString() + "\'!", true);
		}
		else
			UStaticFuncLib::Print("ARoomStreamingManager::BeginConstructLayer: Couldn't find a layer named \'" +
				ParentLayerName.ToString() + "\'!", true);
	}
}

void ARoomStreamingManager::ConstructLayer_Implementation(const FName& LayerName, const FName& PrefabLayerName, const FName& ParentLayerName)
{
	FLayerReconstructionData NewReconstructData;
	NewReconstructData.LayerName = LayerName;
	NewReconstructData.PrefabLayerName = PrefabLayerName;
	NewReconstructData.ParentLayerName = ParentLayerName;

	FStreamedRoomTreeNode* PrefabLayer = LayerNodes.Find(PrefabLayerName);

	FStreamedRoomTreeNode& NewLayerNode = LayerNodes.Add(LayerName);

	float NewLayerOffset = 0.0f;
	if (UnusedOffsets.Num() > 0)
	{
		NewLayerOffset = UnusedOffsets[0];
		UnusedOffsets.RemoveAt(0);
	}
	else
	{
		if (UsedOffsets.Num() > 0)
		{
			TArray<float> UsedOffsArray = UsedOffsets.Array();
			UsedOffsArray.Sort();
			NewLayerOffset = UsedOffsArray[0];
		}

		NewLayerOffset -= 500;//100;//10000;
	}

	//UStaticFuncLib::NetPrint(this,
	//	"Layer Offset Assigned to layer \'" + LayerName.ToString() + "\': " + FString::SanitizeFloat(NewLayerOffset), true);

	UsedOffsets.Add(NewLayerOffset);

	PrefabLayer->Layer.CloneLayer(NewLayerNode.Layer, LayerName, NewLayerOffset);

	//{
	//	FString LSVStr = "{ ";
	//	for (ULevelStreaming* currLSV : GetWorld()->StreamingLevels)
	//	{
	//		FString currStr = currLSV->GetName() + ", ";
	//		currStr.RemoveFromStart("LevelStreamingKismet");
	//		LSVStr += currStr;
	//	}

	//	LSVStr.RemoveFromEnd(", ");

	//	UStaticFuncLib::NetPrint(this, "Constructed Layer \'" + LayerName.ToString() + "\'" + "\n\tCurrent Volumes: " + LSVStr + " }", true);
	//}

	NewLayerNode.Parent = ParentLayerName;
	NewLayerNode.Layer.Offset = NewLayerOffset;

	FStreamedRoomTreeNode* ParentLayer = LayerNodes.Find(ParentLayerName);
	if (nullptr != ParentLayer)
		ParentLayer->Children.Add(LayerName);
	else
		UStaticFuncLib::NetPrint(this,"ConstructLayer_Implementation: Couldn't find parent layer \'" + ParentLayerName.ToString() +
									  "\' for new layer \'" + LayerName.ToString() + "\'!", true);

	for (auto &currRoomPair : NewLayerNode.Layer.RoomsInLayer)
	{
		RoomAddresses.Add(currRoomPair.Value.RoomRef, { LayerName,currRoomPair.Key,EStreamedRoomAddressType::NormalAddress });

		NewReconstructData.NameFixDatas.Add({ currRoomPair.Key, currRoomPair.Value.RoomRef->GetFName() });
	}

	if (UKismetSystemLibrary::IsServer(this))
	{
		ALoadingZone* Creator = WaitingDuringLayerCreation.FindRef(LayerName);
		if (nullptr != Creator)
		{
			NewLayerNode.Layer.InstantiatingLZName = Creator->LZName;
			Creator->DestinationRoom.LayerName = LayerName;
		}
		WaitingDuringLayerCreation.Remove(LayerName);
	}

	StateReconstructionData.LayerDatas.Add(NewReconstructData);
}

void ARoomStreamingManager::BroadcastPreexistingState()
{
	if (UKismetSystemLibrary::IsServer(this))
	{
		StateReconstructionData.UniqueIDNumber = ULevelStreamingKismet::StaticClass()->ClassUnique;

		for (auto &curr : PlayerAddresses)
		{
			StateReconstructionData.PlayerIDs.Add(curr.Key);
			StateReconstructionData.PlayerAddrs.Add(curr.Value);
		}

		//UStaticFuncLib::Print(FString::FromInt(StateReconstructionData.StaticStruct()->GetStructureSize()), true);

		ConstructPreexistingState(StateReconstructionData);
	}
}

void ARoomStreamingManager::ConstructPreexistingState_Implementation(const FRoomStateReconstructPacket& StatePacket)
{
	if (!UKismetSystemLibrary::IsServer(this))
	{
		for (const FLayerReconstructionData &currLayerData : StatePacket.LayerDatas)
		{
			if (!LayerNodes.Contains(currLayerData.LayerName))
			{
				ConstructLayer(currLayerData.LayerName, currLayerData.PrefabLayerName, currLayerData.ParentLayerName);

				FStreamedRoomTreeNode* NewlyConstructedLayer = LayerNodes.Find(currLayerData.LayerName);

				if (nullptr != NewlyConstructedLayer)
				{
					for (const FLSKReplaceData& currNameFixData : currLayerData.NameFixDatas)
					{
						FStreamedRoom* RetrievedRoom = NewlyConstructedLayer->Layer.RoomsInLayer.Find(currNameFixData.RoomName);

						if (nullptr != RetrievedRoom && nullptr != RetrievedRoom->RoomRef)
						{
							FString NameStr = currNameFixData.LSKReplacementName.ToString();
							RetrievedRoom->RoomRef->Rename(*NameStr, RetrievedRoom->RoomRef->GetOuter());
						}
					}
				}
			}
		}

		ULevelStreamingKismet::StaticClass()->ClassUnique = StatePacket.UniqueIDNumber;

		for (int i = 0; i < StatePacket.PlayerIDs.Num(); ++i)
		{
			const FString& currPlayerID = StatePacket.PlayerIDs[i];
			const FStreamedRoomAddress& currAddr = StatePacket.PlayerAddrs[i];
			PlayerAddresses.Add(currPlayerID, currAddr);

			FStreamedRoomTreeNode* RetrievedLayer = LayerNodes.Find(currAddr.LayerName);
			if (nullptr != RetrievedLayer)
				RetrievedLayer->Layer.TraverseTo(currPlayerID, currAddr.RoomName);
		}

		StateReconstructionData = StatePacket;
	}
}

void ARoomStreamingManager::BeginTraverseFrom_Implementation(const FString& TraverserID, const FStreamedRoomAddress& Addr, bool Leaving)
{
	//UStaticFuncLib::NetPrint(this, "Traversing from address {" + Addr.LayerName.ToString() + ", " + Addr.RoomName.ToString() + "}", true);

	FStreamedRoomTreeNode* RetrievedLayer = LayerNodes.Find(Addr.LayerName);

	if (nullptr != RetrievedLayer)
	{
		RetrievedLayer->Layer.TraverseFrom(TraverserID, Addr.RoomName);

		if (Leaving)
			PlayerAddresses.Remove(TraverserID);
	}
}

void ARoomStreamingManager::BeginTraverseTo_Implementation(const FString& TraverserID, const FStreamedRoomAddress& Addr)
{
	//UStaticFuncLib::NetPrint(this, "Traversing to address {" + Addr.LayerName.ToString() + ", " + Addr.RoomName.ToString() + "}", true);

	FStreamedRoomTreeNode* RetrievedLayer = LayerNodes.Find(Addr.LayerName);

	if (nullptr != RetrievedLayer)
	{
		RetrievedLayer->Layer.TraverseTo(TraverserID, Addr.RoomName);
		PlayerAddresses.Add(TraverserID, Addr);
	}
}

void ARoomStreamingManager::TraverseBetween(AActor* TraversingActor, FName FromLayer, FName FromRoom, FName ToLayer, FName ToRoom)
{
	FString* TraverserID = PlayerIDs.Find(TraversingActor);

	if (nullptr != TraverserID)
	{
		if (LayerNodes.Contains(FromLayer) && LayerNodes.Contains(ToLayer))
		{
			BeginTraverseFrom(*TraverserID, { FromLayer,FromRoom });
			BeginTraverseTo(*TraverserID, { ToLayer,ToRoom });
		}
	}
}

bool ARoomStreamingManager::RequestTraversal(const TArray<APawn*>& Players, ALoadingZone* FromLZ)
{
	bool Result = false;

	FStreamedRoomAddress CurrentRoomAddr;

	if (GetRoomAddress(FromLZ, CurrentRoomAddr))
	{
		FName& CurrentLayerName = CurrentRoomAddr.LayerName;
		FName& CurrentRoomName = CurrentRoomAddr.RoomName;
		FStreamedRoomTreeNode* CurrentLayer = LayerNodes.Find(CurrentLayerName);

		if (nullptr != CurrentLayer)
		{
			if (CurrentLayer->Layer.RoomsInLayer.Contains(CurrentRoomName))
			{
				FStreamedRoomTreeNode* NextLayer = nullptr;

				switch (FromLZ->DestinationRoom.AddressType)
				{
				case EStreamedRoomAddressType::NormalAddress:
					// Just use the same layer
					NextLayer = CurrentLayer;

					if (NextLayer->Layer.RoomsInLayer.Contains(FromLZ->DestinationRoom.RoomName))
					{
						for (APawn* currPlayer : Players)
						{
							FString currPlayerID = PlayerIDs.FindRef(currPlayer);

							if (currPlayerID.Len() > 0)
								TraverseBetween(currPlayer, CurrentLayerName, CurrentRoomName, CurrentLayerName, FromLZ->DestinationRoom.RoomName);
						}

						FromLZ->CurrentTargetAddress =
						{ CurrentLayerName, FromLZ->DestinationRoom.RoomName, EStreamedRoomAddressType::NormalAddress };

						Result = true;
					}
					else
						UStaticFuncLib::Print("ARoomStreamingManager::RequestTraversal: Couldn't find a room named \'" +
											  CurrentLayerName.ToString() + "\' within layer \'" + FromLZ->DestinationRoom.RoomName.ToString() +
											  "\'! Traversal has failed.", true);

					break;

				case EStreamedRoomAddressType::InstanceEntrance:
				{
					// Get the latest child instanced layer whose parent is this LZ and is empty.
					FName ChosenChildLayerName;
					for (int i = CurrentLayer->Children.Num() - 1; i >= 0; --i)
					{
						ChosenChildLayerName = CurrentLayer->Children[i];

						NextLayer = LayerNodes.Find(ChosenChildLayerName);

						if (nullptr != NextLayer)
						{
							if (NextLayer->Layer.InstantiatingLZName == FromLZ->LZName && NextLayer->Layer.PlayersPresent.Num() < 1)
								break;
							else
								NextLayer = nullptr;
						}
					}

					if (nullptr != NextLayer)
					{
						if (NextLayer->Layer.RoomsInLayer.Contains(FromLZ->DestinationRoom.RoomName))
						{
							for (APawn* currPlayer : Players)
							{
								FString currPlayerID = PlayerIDs.FindRef(currPlayer);

								if (currPlayerID.Len() > 0)
								{
									TraverseBetween(currPlayer,
													CurrentLayerName, CurrentRoomName,
													ChosenChildLayerName, FromLZ->DestinationRoom.RoomName);
								}
							}

							BeginConstructLayer(FromLZ, CurrentLayerName);

							FromLZ->CurrentTargetAddress =
							{ ChosenChildLayerName, FromLZ->DestinationRoom.RoomName, EStreamedRoomAddressType::NormalAddress };

							Result = true;
						}
						else
							UStaticFuncLib::Print("ARoomStreamingManager::RequestTraversal: Couldn't find a room named \'" +
												  FromLZ->DestinationRoom.RoomName.ToString() + "\' within layer \'" +
												  ChosenChildLayerName.ToString() + "\'! Traversal has failed.", true);
					}
					else
						UStaticFuncLib::Print("ARoomStreamingManager::RequestTraversal: Failed to retrieve child layer \'"
											  + ChosenChildLayerName.ToString() + "\' from layer \'"
											  + CurrentLayerName.ToString() +
											  "\'!  Traversal has failed.", true);
				}
				break;

				case EStreamedRoomAddressType::InstanceExit:
					// Get the parent layer.
					NextLayer = LayerNodes.Find(CurrentLayer->Parent);

					if (nullptr != NextLayer)
					{
						if (NextLayer->Layer.RoomsInLayer.Contains(FromLZ->DestinationRoom.RoomName))
						{
							for (APawn* currPlayer : Players)
							{
								FString currPlayerID = PlayerIDs.FindRef(currPlayer);

								if (currPlayerID.Len() > 0)
									TraverseBetween(currPlayer, CurrentLayerName, CurrentRoomName, CurrentLayer->Parent, FromLZ->DestinationRoom.RoomName);
							}

							FromLZ->CurrentTargetAddress =
							{ CurrentLayer->Parent, FromLZ->DestinationRoom.RoomName, EStreamedRoomAddressType::NormalAddress };

							Result = true;
						}
						else
							UStaticFuncLib::Print("ARoomStreamingManager::RequestTraversal: Couldn't find a room named \'" +
												  FromLZ->DestinationRoom.RoomName.ToString() + "\' within layer \'" +
												  CurrentLayer->Parent.ToString() + "\'! Traversal has failed.", true);
					}
					else
						UStaticFuncLib::Print("ARoomStreamingManager::RequestTraversal: Failed to retrieve parent layer \'"
											  + CurrentLayer->Parent.ToString() + "\' from layer \'"
											  + CurrentLayerName.ToString() +
											  "\'!  Traversal has failed.", true);
					break;

				default:
					UStaticFuncLib::Print("ARoomStreamingManager::RequestTraversal: LZ \'" + FromLZ->GetName() +
										  "\' contained an invalid room address type!  Traversal has failed.", true);
					break;
				}
			}
			else
				UStaticFuncLib::Print("ARoomStreamingManager::RequestTraversal: Couldn't find a room named \'" +
									  CurrentRoomName.ToString() + "\' within layer \'" + CurrentLayerName.ToString() +
									  "\'! Traversal has failed.", true);
		}
		else
			UStaticFuncLib::Print("ARoomStreamingManager::RequestTraversal: Couldn't find a layer named \'" +
								  CurrentLayerName.ToString() + "\'! Traversal has failed.", true);
	}
	else
		UStaticFuncLib::Print("ARoomStreamingManager::RequestTraversal: GetRoomAddress returned false. Traversal has failed.", true);

	return Result;
}

bool ARoomStreamingManager::AnyPlayersInLayer(FStreamedRoomTreeNode* LayerRef)
{
	if (LayerRef->Layer.PlayersPresent.Num() < 1)
	{
		for (FName currChild : LayerRef->Children)
		{
			FStreamedRoomTreeNode* RetrievedChild = LayerNodes.Find(currChild);

			if (nullptr != RetrievedChild && AnyPlayersInLayer(RetrievedChild))
				return true;
		}

		return false;
	}

	return true;
}

void ARoomStreamingManager::RemoveLayer(FStreamedRoomTreeNode* LayerRef, FName LayerName)
{
	for (FName &currChild : LayerRef->Children)
	{
		FStreamedRoomTreeNode* RetrievedChild = LayerNodes.Find(currChild);

		if (nullptr != RetrievedChild)
			RemoveLayer(RetrievedChild, currChild);
	}

	for (auto &currRoomPair : LayerRef->Layer.RoomsInLayer)
		RoomAddresses.Remove(currRoomPair.Value.RoomRef);

	UsedOffsets.Remove(LayerRef->Layer.Offset);
	UnusedOffsets.Add(LayerRef->Layer.Offset);

	LayerRef->Layer.Clear();

	LayerNodes.Remove(LayerName);

	StateReconstructionData.LayerDatas.RemoveAll([&LayerName](FLayerReconstructionData& a) { return a.LayerName == LayerName; });
}

void ARoomStreamingManager::BeginTryUnload_Implementation(const FStreamedRoomAddress& Address, bool IsInstancedLayer)
{
	FStreamedRoomTreeNode* RetrievedLayer = LayerNodes.Find(Address.LayerName);

	if (nullptr != RetrievedLayer)
	{
		if (IsInstancedLayer && !AnyPlayersInLayer(RetrievedLayer))
			RemoveLayer(RetrievedLayer, Address.LayerName);
		else
			RetrievedLayer->Layer.TryUnloadRooms(Address.RoomName);
	}
}

void ARoomStreamingManager::PostTraverse(ALoadingZone* FromLZ)
{
	FStreamedRoomAddress ToCheckForRemove;
	if (GetRoomAddress(FromLZ, ToCheckForRemove))
	{
		FStreamedRoomTreeNode* RetrievedLayer = LayerNodes.Find(ToCheckForRemove.LayerName);

		if (nullptr != RetrievedLayer)
			BeginTryUnload(ToCheckForRemove, !RetrievedLayer->Layer.InstantiatingLZName.IsNone());
		else
			UStaticFuncLib::Print("ARoomStreamingManager::PostTraverse: Couldn't find a layer \'" +
								  ToCheckForRemove.LayerName.ToString() + "\' to evaluate for removal. "
								  "No operations will happen.", true);
	}
	else
		UStaticFuncLib::Print("ARoomStreamingManager::PostTraverse: GetRoomAddress returned false. No operations will happen.", true);
}

bool ARoomStreamingManager::RequestTraversalDirect(TArray<APawn*>& Players, FStreamedRoomAddress FromAddress, FStreamedRoomAddress ToAddress, ULevelStreaming*& DestRoom)
{
	FStreamedRoomTreeNode* FromLayer = LayerNodes.Find(FromAddress.LayerName);
	if (nullptr != FromLayer)
	{
		if (FromLayer->Layer.RoomsInLayer.Contains(FromAddress.RoomName))
		{
			FStreamedRoomTreeNode* ToLayer = LayerNodes.Find(ToAddress.LayerName);
			if (nullptr != ToLayer)
			{
				if (ToLayer->Layer.RoomsInLayer.Contains(ToAddress.RoomName))
				{
					for (APawn* currPlayer : Players)
					{
						FString currPlayerID = PlayerIDs.FindRef(currPlayer);

						if (currPlayerID.Len() > 0)
							TraverseBetween(currPlayer, FromAddress.LayerName, FromAddress.RoomName, ToAddress.LayerName, ToAddress.RoomName);
					}

					return true;
				}
				else
					UStaticFuncLib::Print("ARoomStreamingManager::RequestTraversalDirect: Couldn't find a room named \'" +
										  ToAddress.RoomName.ToString() + "\' in layer \'" + ToAddress.LayerName.ToString() +
										  "\'! Traversal has failed.", true);
			}
			else
				UStaticFuncLib::Print("ARoomStreamingManager::RequestTraversalDirect: Couldn't find a layer named \'" +
									  ToAddress.LayerName.ToString() + "\'! Traversal has failed.", true);
		}
		else
			UStaticFuncLib::Print("ARoomStreamingManager::RequestTraversalDirect: Couldn't find a room named \'" +
								  FromAddress.RoomName.ToString() + "\' in layer \'" + FromAddress.LayerName.ToString() +
								  "\'! Traversal has failed.", true);
	}
	else
		UStaticFuncLib::Print("ARoomStreamingManager::RequestTraversalDirect: Couldn't find a layer named \'" +
							  FromAddress.LayerName.ToString() + "\'! Traversal has failed.", true);

	return false;
}

void ARoomStreamingManager::PostTraverseDirect(FStreamedRoomAddress FromAddress)
{
	FStreamedRoomTreeNode* FocusLayer = LayerNodes.Find(FromAddress.LayerName);

	if (nullptr != FocusLayer)
		BeginTryUnload(FromAddress, !FocusLayer->Layer.InstantiatingLZName.IsNone());
	else
		UStaticFuncLib::Print("ARoomStreamingManager::PostTraverseDirect: Couldn't find a layer \'" +
							  FromAddress.LayerName.ToString() + "\' to evaluate for removal. "
							  "No operations will happen.", true);
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
