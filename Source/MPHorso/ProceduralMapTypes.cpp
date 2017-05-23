// Fill out your copyright notice in the Description page of Project Settings.

#include "MPHorso.h"
#include "ProceduralMapTypes.h"

#include "Components/ChildActorComponent.h"

#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"

#include "StaticFuncLib.h"


ACameraGuide::ACameraGuide(const FObjectInitializer& _init) : Super(_init)
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	//PrimaryActorTick.bCanEverTick = true;

	bReplicates = true;
	bReplicateMovement = true;
	
	RootComponent = _init.CreateDefaultSubobject<USceneComponent>(this, TEXT("RootComponent"));

}

void ACameraGuide::BeginPlay()
{
	Super::BeginPlay();
	
}

void ACameraGuide::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}


AMobSpawner::AMobSpawner(const FObjectInitializer& _init) : Super(_init)
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	//PrimaryActorTick.bCanEverTick = true;

	bReplicates = true;
	bReplicateMovement = true;

	RootComponent = _init.CreateDefaultSubobject<USceneComponent>(this, TEXT("RootComponent"));

}

void AMobSpawner::BeginPlay()
{
	Super::BeginPlay();

}

void AMobSpawner::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}


AProceduralSplineConnector::AProceduralSplineConnector(const FObjectInitializer& _init) : Super(_init)
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	//PrimaryActorTick.bCanEverTick = true;

	bReplicates = true;
	bReplicateMovement = true;

	RootComponent = RootSpline = _init.CreateDefaultSubobject<USplineComponent>(this, TEXT("RootSpline"));
}

void AProceduralSplineConnector::OnConstruction(const FTransform& Transform)
{
	// TODO : IMPL
	//		  along the lines of https://www.youtube.com/watch?v=7YUxM0NDWRY
	//		  but of course in C++

	MeshData.Sort([](const FSplineConnectorMeshData& a, const FSplineConnectorMeshData& b) { return a.Time < b.Time; });


}


AProceduralRoom::AProceduralRoom(const FObjectInitializer& _init) : Super(_init)
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	//PrimaryActorTick.bCanEverTick = true;

	bReplicates = true;
	bReplicateMovement = true;

	RootComponent = _init.CreateDefaultSubobject<USceneComponent>(this, TEXT("RootComponent"));

}

void AProceduralRoom::BeginPlay()
{
	Super::BeginPlay();

}

void AProceduralRoom::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AProceduralRoom::GetUnboundConnectors(TArray<UProceduralConnector*>& OutUnboundConnectors, bool ClearArray)
{
	if (ClearArray)
		OutUnboundConnectors.Empty();

	for (auto *curr : Connectors)
	{
		if (nullptr == curr->BoundTo)
			OutUnboundConnectors.Add(curr);
	}
}

void AProceduralRoom::Initialize()
{
	PrepopulateInternalArrays();

	TArray<USceneComponent*> RetrievedChildren;
	RootComponent->GetChildrenComponents(true, RetrievedChildren);


	// remove anything the user already dropped in from the checking list

	TArray<USceneComponent*> PredefinedComponents;
	TArray<AActor*> PredefinedActors;

	PredefinedComponents.Append(Geometry);						// TArray<USceneComponent*> Geometry;
	PredefinedComponents.Append(Connectors);					// TArray<UProceduralConnector*> Connectors;
	PredefinedComponents.Append(CameraGuideComponents);			// TArray<UChildActorComponent*> CameraGuideComponents;
	PredefinedComponents.Append(MobSpawnerComponents);			// TArray<UChildActorComponent*> MobSpawnerComponents;
	PredefinedComponents.Append(OtherObjects);					// TArray<USceneComponent*> OtherObjects;

	PredefinedActors.Append(CameraGuides);						// TArray<ACameraGuide*> CameraGuides;
	PredefinedActors.Append(MobSpawners);						// TArray<AMobSpawner*> MobSpawners;

	for (auto *currPreComp : PredefinedComponents)
		RetrievedChildren.Remove(currPreComp);

	for (auto *currPreAct : PredefinedActors)
		RetrievedChildren.Remove(currPreAct->GetRootComponent());


	// Search through the remaining pieces for things that fit the bills.

	UProceduralConnector* CurrAsConnector;
	UChildActorComponent* CurrAsCAC;
	UPrimitiveComponent* CurrAsPrimComp;
	AActor* CACChild;
	ACameraGuide* CACChildAsCG;
	AMobSpawner* CACChildAsMS;
	for (auto* curr : RetrievedChildren)
	{
		if (this == curr->GetOwner()) // tl;dr to weed out child actor rootcomponents
		{
			if (nullptr != (CurrAsConnector = Cast<UProceduralConnector>(curr)))
			{
				Connectors.Add(CurrAsConnector);
			}
			else if (nullptr != (CurrAsCAC = Cast<UChildActorComponent>(curr)))
			{
				CACChild = CurrAsCAC->GetChildActor();

				if (nullptr != (CACChildAsCG = Cast<ACameraGuide>(CACChild)))
				{
					CameraGuideComponents.Add(CurrAsCAC);
					CameraGuides.Add(CACChildAsCG);
				}
				else if (nullptr != (CACChildAsMS = Cast<AMobSpawner>(CACChild)))
				{
					MobSpawnerComponents.Add(CurrAsCAC);
					MobSpawners.Add(CACChildAsMS);
				}
				else
				{
					// lol what the fuck even *is* this :_
					// just put it in otherobjects because yo man what the fuck

					OtherObjects.Add(CurrAsCAC);
				}
			}
			else if (nullptr != (CurrAsPrimComp = Cast<UPrimitiveComponent>(curr)))
			{
				// if it's a primitive component then it's probably collision or
				// other geometry (usually UStaticMeshComponents or UShapeComponents)

				Geometry.Add(CurrAsPrimComp);
			}
			else
			{
				// just throw it in OtherObjects b/c the fuck if I can tell what it is

				OtherObjects.Add(curr);
			}
		}
	}
}


AProceduralArea::AProceduralArea(const FObjectInitializer& _init) : Super(_init)
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	//PrimaryActorTick.bCanEverTick = true;

	bReplicates = true;
	bReplicateMovement = true;

	RootComponent = _init.CreateDefaultSubobject<USceneComponent>(this, TEXT("RootComponent"));

}

void AProceduralArea::BeginPlay()
{
	Super::BeginPlay();

}

void AProceduralArea::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AProceduralArea::Build(UPARAM(Ref) FRandomStream& RandomStream)
{
	RandStream = RandomStream;

	SpecialRoomArchetypes.RemoveAll([](const FProceduralAreaRoomTypeData& curr) { return curr.NumInstances < 1; });
	RoomArchetypes.RemoveAll([](const FProceduralAreaRoomTypeData& curr) { return curr.NumInstances < 1; });

	SpawnSpecialRooms();

	GetWorld()->GetTimerManager().SetTimer(IterationTimer, this, &AProceduralArea::Iterate, 0.01f, true);
}

void AProceduralArea::SpawnSpecialRooms()
{
	while (SpecialRoomArchetypes.Num() > 0)
	{
		int ArchInd = RandStream.RandRange(0, SpecialRoomArchetypes.Num() - 1);
		FProceduralAreaRoomTypeData& CurrArchetype = SpecialRoomArchetypes[ArchInd];
		AProceduralRoom* NewestSpawned;

		if (!SpawnRoom(CurrArchetype, NewestSpawned))
			UStaticFuncLib::Print(GetClass()->GetName() + "::SpawnSpecialRooms: Error! Couldn't create room \'" + CurrArchetype.RoomType.GetDefaultObject()->GetName() + "\'!", true);
		else
			UStaticFuncLib::Print(GetClass()->GetName() + "::SpawnSpecialRooms: Successfully placed room \'" + CurrArchetype.RoomType.GetDefaultObject()->GetName() + "\'!");

		SpawnedRooms.Add(NewestSpawned);

		--CurrArchetype.NumInstances;

		if (CurrArchetype.NumInstances < 1)
			SpecialRoomArchetypes.RemoveAt(ArchInd);
	}
}

void AProceduralArea::Iterate()
{
	int ArchInd = RandStream.RandRange(0, RoomArchetypes.Num() - 1);
	FProceduralAreaRoomTypeData& CurrArchetype = RoomArchetypes[ArchInd];
	AProceduralRoom* NewestSpawned;

	if (!SpawnRoom(CurrArchetype, NewestSpawned))
		UStaticFuncLib::Print(GetClass()->GetName() + "::Iterate: Error! Couldn't create room \'" + CurrArchetype.RoomType.GetDefaultObject()->GetName() + "\'!", true);
	else
		UStaticFuncLib::Print(GetClass()->GetName() + "::Iterate: Successfully placed room \'" + CurrArchetype.RoomType.GetDefaultObject()->GetName() + "\'!");

	SpawnedRooms.Add(NewestSpawned);

	--CurrArchetype.NumInstances;

	if (CurrArchetype.NumInstances < 1)
		RoomArchetypes.RemoveAt(ArchInd);

	if (RoomArchetypes.Num() < 1)
	{
		GetWorld()->GetTimerManager().ClearTimer(IterationTimer);
		Fasten();
		FinishedBuildingDelegate.Broadcast(this);
	}
}

bool AProceduralArea::SpawnRoom(FProceduralAreaRoomTypeData ArchetypeToSpawn, AProceduralRoom*& OutSpawned)
{
	/*
		TODO : Switch method over to one that doesn't take a
			   billion failed placement attempts to finish the map

			   might switch it over to always spawn and then just
			   move the room around until it doesn't collide
			   anymore (should work with GetOverlappingActors())
			   just keep the fail state after N iterations and
			   if it's reached just delete the room to get
			   basically the same effect.

			   at the very least this should stop flooding the
			   output with failed spawn attempts and make it other
			   debug easier to find.
	*/

	FActorSpawnParameters params;
	params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::DontSpawnIfColliding;

	FVector RandLoc;
	FRotator RandRot;

	GetAreaRandomPoint(ArchetypeToSpawn.RoomType, RandLoc, RandRot);

	OutSpawned = GetWorld()->SpawnActor<AProceduralRoom>(ArchetypeToSpawn.RoomType, RandLoc, RandRot, params);
	int iterationcap = 0;
	while (nullptr == OutSpawned && iterationcap < MaxRoomPlacementAttempts)
	{
		GetAreaRandomPoint(ArchetypeToSpawn.RoomType, RandLoc, RandRot);
		OutSpawned = GetWorld()->SpawnActor<AProceduralRoom>(ArchetypeToSpawn.RoomType, RandLoc, RandRot, params);
		++iterationcap;
	}

	return nullptr != OutSpawned || iterationcap < MaxRoomPlacementAttempts;
}

void AProceduralArea::Fasten()
{
	FVector AveragePos = FVector::ZeroVector;
	for (auto *CurrRoom : SpawnedRooms)
		AveragePos += CurrRoom->GetActorLocation();
	AveragePos /= SpawnedRooms.Num();

	SetActorLocation(AveragePos);

	for (auto *CurrRoom : SpawnedRooms)
		CurrRoom->AttachToActor(this, FAttachmentTransformRules::KeepWorldTransform/*::KeepRelativeTransform*/);
}

void AProceduralArea::Finalize()
{
	// TODO : IMPL

	/*
		TODO : Also find some way to add in the procedural terrain stuff.
			   it's fucking 4 am and I've not got the brainpower right now
			   to figure it out and write it down but it can't be all that
			   hard if you use a proceduralmeshcomponent
			   (link https://garvinized.com/posts/2016/voxel-terrain-in-unreal-engine-4-part-3/ )
			   if anything it really just seems a bit more verbose than normal
			   and can probably with luck the nasty stuff can be abstracted out
			   into a function or two
	*/
}


// Sets default values
AProceduralMap::AProceduralMap(const FObjectInitializer& _init) : Super(_init)
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	//PrimaryActorTick.bCanEverTick = true;

	bReplicates = true;
	bReplicateMovement = true;

	RootComponent = _init.CreateDefaultSubobject<USceneComponent>(this, TEXT("RootComponent"));

}

// Called when the game starts or when spawned
void AProceduralMap::BeginPlay()
{
	Super::BeginPlay();

}

// Called every frame
void AProceduralMap::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AProceduralMap::Build(int InitialRandStreamSeed)
{
	RandStream.Initialize(InitialRandStreamSeed);

	FActorSpawnParameters params;
	params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;


	FVector CurrOffset(50000, 0, 0);
	FVector OffsDelta(50000, 0, 0);//(0, 0, -10000);
	AProceduralArea* CurrSpawnedArea;
	for (auto &curr : Areas)
	{
		CurrSpawnedArea = GetWorld()->SpawnActor<AProceduralArea>(curr, CurrOffset, FRotator::ZeroRotator, params);
		CurrSpawnedArea->FinishedBuildingDelegate.AddDynamic(this, &AProceduralMap::OnAreaFinishedBuilding);
		CurrSpawnedArea->Build(RandStream);
		IteratingAreas.Add(CurrSpawnedArea);

		CurrOffset += OffsDelta;
	}
}

void AProceduralMap::OnAreaFinishedBuilding(AProceduralArea* FinishedArea)
{
	FinishedAreas.Add(FinishedArea);
	IteratingAreas.Remove(FinishedArea);

	if (IteratingAreas.Num() < 1)
	{
		PreFinalize();

		for (auto *currArea : FinishedAreas)
			currArea->Finalize();
	}
}

void AProceduralMap::PreFinalize()
{
	// TODO : IMPL
}