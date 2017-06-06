// Fill out your copyright notice in the Description page of Project Settings.

#include "MPHorso.h"
#include "ProceduralMapTypes.h"

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

void AProceduralSplineConnector::GetSmallestMeshTypeSectionIndices(TArray<int>& OutSmallest)
{
	struct DataWithInd
	{
		FSplineConnectorMeshData Data;
		int Ind;
	};

	TArray<DataWithInd> DataInds;

	for (int currInd = 0; currInd < MeshData.Num(); ++currInd)
		DataInds.Add({ MeshData[currInd], currInd });

	DataInds.Sort([](const DataWithInd& a, const DataWithInd& b) { return a.Data.Indices.Num() < b.Data.Indices.Num(); });
	int LowestIndices = DataInds[0].Data.Indices.Num();
	DataInds.RemoveAll([&LowestIndices](const DataWithInd& a) { return a.Data.Indices.Num() > LowestIndices; });

	for (auto &currDWI : DataInds)
		OutSmallest.Add(currDWI.Ind);

}

void AProceduralSplineConnector::GetLargestMeshTypeSectionIndices(TArray<int>& OutLargest)
{
	struct DataWithInd
	{
		FSplineConnectorMeshData Data;
		int Ind;
	};

	TArray<DataWithInd> DataInds;

	for (int currInd = 0; currInd < MeshData.Num(); ++currInd)
		DataInds.Add({ MeshData[currInd], currInd });

	DataInds.Sort([](const DataWithInd& a, const DataWithInd& b) { return a.Data.Indices.Num() > b.Data.Indices.Num(); });
	int HighestIndices = DataInds[0].Data.Indices.Num();
	DataInds.RemoveAll([&HighestIndices](const DataWithInd& a) { return a.Data.Indices.Num() , HighestIndices; });

	for (auto &currDWI : DataInds)
		OutLargest.Add(currDWI.Ind);
}


AProceduralRoomDecor::AProceduralRoomDecor(const FObjectInitializer& _init) : Super(_init)
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	//PrimaryActorTick.bCanEverTick = true;

	bReplicates = true;
	bReplicateMovement = true;

	RootComponent = _init.CreateDefaultSubobject<USceneComponent>(this, TEXT("RootComponent"));

}

void AProceduralRoomDecor::BeginPlay()
{
	Super::BeginPlay();

}

void AProceduralRoomDecor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}


void UProceduralRoomDecorComponent::SpawnDecor(TSubclassOf<AProceduralRoomDecor> DecorType)
{
	SetChildActorClass(DecorType);

	AProceduralRoom* OwnerAsRoom = Cast<AProceduralRoom>(GetOwner());
	if (nullptr != OwnerAsRoom)
	{
		AProceduralRoomDecor* ChildAsDecor = Cast<AProceduralRoomDecor>(GetChildActor());
		if (nullptr != ChildAsDecor)
			ChildAsDecor->Initialize(OwnerAsRoom->RandStream);
	}
}

void UProceduralRoomDecorComponent::SpawnRandomDecor()
{
	AProceduralRoom* OwnerAsRoom = Cast<AProceduralRoom>(GetOwner());

	int RandInd;

	if(nullptr != OwnerAsRoom)
		RandInd = OwnerAsRoom->RandStream.RandRange(0, DecorTypes.Num() - 1);
	else
		RandInd = FMath::RandRange(0, DecorTypes.Num() - 1);

	SpawnDecor(DecorTypes[RandInd]);

	AProceduralRoomDecor* ChildAsDecor = Cast<AProceduralRoomDecor>(GetChildActor());
	if (nullptr != ChildAsDecor)
		ChildAsDecor->Initialize(OwnerAsRoom->RandStream);
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

void AProceduralRoom::Initialize(UPARAM(Ref) FRandomStream& RandomStream)
{
	RandStream = RandomStream;

	PrepopulateInternalArrays();

	TArray<USceneComponent*> RetrievedChildren;
	RootComponent->GetChildrenComponents(true, RetrievedChildren);


	// remove anything the user already dropped in from the checking list

	TArray<USceneComponent*> PredefinedComponents;
	TArray<AActor*> PredefinedActors;

	PredefinedComponents.Append(Geometry);						// TArray<USceneComponent*> Geometry;
	PredefinedComponents.Append(Connectors);					// TArray<UProceduralConnector*> Connectors;
	PredefinedComponents.Append(DecorComponents);				// TArray<UProceduralRoomDecorComponent*> DecorComponents;
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
	UProceduralRoomDecorComponent* CurrAsDecor;
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
			else if (nullptr != (CurrAsDecor = Cast<UProceduralRoomDecorComponent>(curr)))
			{
				DecorComponents.Add(CurrAsDecor);
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

	PrepareDecor();
}

void AProceduralRoom::PrepareDecor()
{
	TArray<UProceduralRoomDecorComponent*> UnoccupiedDecorComps = DecorComponents;
	UnoccupiedDecorComps.RemoveAll([](const UProceduralRoomDecorComponent* a) {return nullptr != a->GetChildActor(); });

	int RandInd;
	while (RequiredDecor.Num() > 0 && UnoccupiedDecorComps.Num() > 0)
	{
		RandInd = RandStream.RandRange(0, UnoccupiedDecorComps.Num() - 1);
		UnoccupiedDecorComps[RandInd]->SpawnDecor(RequiredDecor[0]);

		RequiredDecor.RemoveAt(0);
		UnoccupiedDecorComps.RemoveAt(RandInd);
	}

	for (auto *currDComp : UnoccupiedDecorComps)
		currDComp->SpawnRandomDecor();
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
		//else
		//	UStaticFuncLib::Print(GetClass()->GetName() + "::SpawnSpecialRooms: Successfully placed room \'" + CurrArchetype.RoomType.GetDefaultObject()->GetName() + "\'!");

		NewestSpawned->Initialize(RandStream);
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
	{
		NewestSpawned->Initialize(RandStream);
		SpawnedRooms.Add(NewestSpawned);

		//UStaticFuncLib::Print(GetClass()->GetName() + "::Iterate: Successfully placed room \'" + CurrArchetype.RoomType.GetDefaultObject()->GetName() + "\'!");
	}

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
	FActorSpawnParameters params;
	params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;// DontSpawnIfColliding;

	FVector RandLoc;
	FRotator RandRot;

	GetAreaRandomPoint(ArchetypeToSpawn.RoomType, RandLoc, RandRot);

	OutSpawned = GetWorld()->SpawnActor<AProceduralRoom>(ArchetypeToSpawn.RoomType, RandLoc, RandRot, params);
	int iterationcap = 0;
	while (GetWorld()->EncroachingBlockingGeometry(OutSpawned, RandLoc, RandRot) && iterationcap < MaxRoomPlacementAttempts)
	{
		GetAreaRandomPoint(ArchetypeToSpawn.RoomType, RandLoc, RandRot);
		++iterationcap;
	}

	if (iterationcap >= MaxRoomPlacementAttempts)
	{
		OutSpawned->Destroy();
		OutSpawned = nullptr;
	}
	else
		OutSpawned->SetActorLocationAndRotation(RandLoc, RandRot);

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
	// TODO : REFINE AND ADJUST UNTIL ACCEPTABLE.

	TArray<UProceduralConnector*> AllUnbounds;

	for (auto *currRoom : SpawnedRooms)
		currRoom->GetUnboundConnectors(AllUnbounds, false);

	struct PairedConnectors
	{
		UProceduralConnector *A;
		UProceduralConnector *B;
		float Dist;

		PairedConnectors(UProceduralConnector* a, UProceduralConnector* b)
		{
			A = a;
			B = b;
			Dist = FVector::Dist(a->GetComponentLocation(), b->GetComponentLocation());
		}

		inline bool operator==(const PairedConnectors& o) const { return (A == o.A&&B == o.B) || (A == o.B&&B == o.A); }
	};

	TArray<PairedConnectors> AlreadyDone;
	float avgDist = 0;
	for (auto *currA : AllUnbounds)
	{
		for (auto *currB : AllUnbounds)
		{
			FVector dirTo = currA->GetComponentLocation() - currB->GetComponentLocation();
			dirTo.Normalize();

			if (currA != currB &&
				currA->GetOwner() != currB->GetOwner() &&
				FVector::DotProduct(currA->GetForwardVector(), currB->GetForwardVector()) <= 0 &&
				FVector::DotProduct(dirTo, currB->GetForwardVector()) > /*0.25*/0.5f &&
				FVector::DotProduct(-dirTo, currA->GetForwardVector()) > /*0.25*/0.5f)
			{
				PairedConnectors ThisPair(currA, currB);

				if (!AlreadyDone.Contains(ThisPair))
				{
					avgDist += ThisPair.Dist;
					AlreadyDone.Add(MoveTemp(ThisPair));
				}
			}
		}
	}
	avgDist /= AlreadyDone.Num();

	AlreadyDone.RemoveAll([&avgDist](const PairedConnectors& a) { return a.Dist > avgDist; });

	for (auto *currUnb : AllUnbounds)
	{
		if (nullptr == currUnb->BoundTo)
		{

			TArray<PairedConnectors> MyChoices = AlreadyDone.FilterByPredicate([&currUnb](const PairedConnectors& a) { return a.A == currUnb; });

			if (MyChoices.Num() > 0)
			{
				MyChoices.Sort([](const PairedConnectors& a, const PairedConnectors& b) { return a.Dist < b.Dist; });

				while (MyChoices.Num() > 0 && nullptr != MyChoices[0].B->BoundTo)
					MyChoices.RemoveAt(0);

				if (MyChoices.Num() > 0)
				{
					AProceduralSplineConnector* Corrid = GetWorld()->SpawnActor<AProceduralSplineConnector>(CorridorTypes[0]);
					
					if (Corrid)
					{
						TArray<FVector> points;
						points.Add(currUnb->GetComponentLocation());
						points.Add(MyChoices[0].B->GetComponentLocation());

						Corrid->RootSpline->SetSplinePoints(points, ESplineCoordinateSpace::World, true);
						Corrid->RootSpline->SetTangentAtSplinePoint(0, currUnb->GetForwardVector(), ESplineCoordinateSpace::World, true);
						Corrid->RootSpline->SetTangentAtSplinePoint(1, -MyChoices[0].B->GetForwardVector(), ESplineCoordinateSpace::World, true);

						FVector MidP = ((MyChoices[0].B->GetComponentLocation() - currUnb->GetComponentLocation()) / 2);
						MidP += (currUnb->GetForwardVector() + MyChoices[0].B->GetForwardVector()) *(MidP.Size() / 2);

						Corrid->RootSpline->AddSplinePointAtIndex(MidP+ currUnb->GetComponentLocation(), 1, ESplineCoordinateSpace::World, true);

						Corrid->ConstructMeshes(RandStream);

						if (!GetWorld()->EncroachingBlockingGeometry(Corrid, Corrid->GetActorLocation(), Corrid->GetActorRotation()))
						{

							currUnb->BindConnectorTo(MyChoices[0].B);
							MyChoices[0].B->BindConnectorTo(currUnb);

							//UKismetSystemLibrary::DrawDebugLine(this, currUnb->GetComponentLocation(), MyChoices[0].B->GetComponentLocation(), FLinearColor::Red, 10000, 100);
						}
					}

				}

				AlreadyDone.RemoveAll([&currUnb](const PairedConnectors& a) { return a.A == currUnb; });
			}
		}
	}

	for (int i = SpawnedRooms.Num() - 1; i >= 0; --i)
	{
		AProceduralRoom* currRoom = SpawnedRooms[i];
		TArray<UProceduralConnector*> UnboundConnectors;

		currRoom->GetUnboundConnectors(UnboundConnectors);

		if (UnboundConnectors.Num() >= currRoom->Connectors.Num())
		{
			currRoom->Destroy();
			SpawnedRooms.RemoveAt(i);
		}

	}



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































#include "ProceduralMeshComponent.h"


AProceduralWholeMap::AProceduralWholeMap(const FObjectInitializer& _init) : Super(_init)
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	bReplicates = true;
	bReplicateMovement = true;

	RootComponent = _init.CreateDefaultSubobject<USceneComponent>(this, TEXT("RootComponent"));

}

void AProceduralWholeMap::BeginPlay()
{
	Super::BeginPlay();

}

void AProceduralWholeMap::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AProceduralWholeMap::Build(int InitialRandSeed)
{
	RandStream.Initialize(InitialRandSeed);

	USimplexNoiseLibrary::setNoiseSeed(RandStream.GetUnsignedInt());


	TerrainMap.AddDefaulted(MapWidth);


	TopHeightMapColorTexture = UTexture2D::CreateTransient(MapWidth, MapHeight);
	BottomHeightMapColorTexture = UTexture2D::CreateTransient(MapWidth, MapHeight);
	TopHeightMapTexture = UTexture2D::CreateTransient(MapWidth, MapHeight);
	BottomHeightMapTexture = UTexture2D::CreateTransient(MapWidth, MapHeight);
	FalloffMapTexture = UTexture2D::CreateTransient(MapWidth, MapHeight);
	OffsetMapTexture = UTexture2D::CreateTransient(MapWidth, MapHeight);
	SteepnessMapTexture = UTexture2D::CreateTransient(MapWidth, MapHeight);


	TopColorMipData = static_cast<FColor*>(TopHeightMapColorTexture->PlatformData->Mips[0].BulkData.Lock(LOCK_READ_WRITE));
	BotColorMipData = static_cast<FColor*>(BottomHeightMapColorTexture->PlatformData->Mips[0].BulkData.Lock(LOCK_READ_WRITE));
	TopMipData = static_cast<FColor*>(TopHeightMapTexture->PlatformData->Mips[0].BulkData.Lock(LOCK_READ_WRITE));
	BotMipData = static_cast<FColor*>(BottomHeightMapTexture->PlatformData->Mips[0].BulkData.Lock(LOCK_READ_WRITE));
	FallMipData = static_cast<FColor*>(FalloffMapTexture->PlatformData->Mips[0].BulkData.Lock(LOCK_READ_WRITE));
	OffsMipData = static_cast<FColor*>(OffsetMapTexture->PlatformData->Mips[0].BulkData.Lock(LOCK_READ_WRITE));
	SteepMipData = static_cast<FColor*>(SteepnessMapTexture->PlatformData->Mips[0].BulkData.Lock(LOCK_READ_WRITE));


	FullDepth = (TopMaxDepth*2) + (UnderMaxDepth*2);

	HalfBounds.X = float(MapWidth) / 2;
	HalfBounds.Y = float(MapHeight) / 2;
	HalfBounds.Z = float(FullDepth) / 2;

	for (int col = 0; col < MapWidth; ++col)
	{
		TerrainMap	[col].AddDefaulted(MapHeight);

		for (int row = 0; row < MapHeight; ++row)
			TerrainMap	[col][row].AddDefaulted(FullDepth);
	}

	GetWorld()->GetTimerManager().SetTimer(IterationTimerHandle, this, &AProceduralWholeMap::Iterate, IterationTime, true);
}

void AProceduralWholeMap::Iterate()
{
	FVector MapPoint = FVector(MapIterator) - HalfBounds;

	if (TerrainMapProgress < 100)
	{
		float Falloff = DetermineFalloff(MapPoint, HalfBounds);
		float Steepness = DetermineSteepness(MapPoint, HalfBounds);
		float TopH = DetermineTopHeight(MapPoint, HalfBounds) - Falloff;
		float UndH = DetermineUndersideHeight(MapPoint, HalfBounds);

		if (TopH < HeightMapCutoffThreshold)
			TopH = UndH = -1;
		else if (TopH < Steepness)
			TopH *= Steepness*Steepness;
		else
			TopH *= (TopH - Steepness)*MountainScale;

		float Offset = FMath::GetMappedRangeValueClamped({ -1,1 }, { MinOffs,MaxOffs }, DetermineOffset(MapPoint, HalfBounds));

		{
			// texture stuff

			float& TopHeight = TopH;
			float& BotHeight = UndH;
			float& FallAmt = Falloff;
			float& OffsetAmt = Offset;
			float& SteepAmt = Steepness;
			int index = MapWidth * MapIterator.X + MapIterator.Y;

			FLinearColor TopColor = FMath::Lerp(FLinearColor(FColor::Cyan), FLinearColor::Green, TopHeight);
			FLinearColor BotColor = FMath::Lerp(FLinearColor(FColor::FromHex("#A9A9A9")), FLinearColor(FColor::FromHex("#696969")), BotHeight);

			TopColor.A = (TopHeight > -1 ? 1 : 0);
			BotColor.A = (BotHeight > -1 ? 1 : 0);

			TopColorMipData[index] = TopColor.ToFColor(true);
			BotColorMipData[index] = BotColor.ToFColor(true);

			float TopClamp = FMath::Clamp(TopHeight, 0.0f, 1.0f);
			float BotClamp = FMath::Clamp(BotHeight, 0.0f, 1.0f);
			TopMipData[index] = FColor(uint8(255 * TopClamp), uint8(255 * TopClamp), uint8(255 * TopClamp));
			BotMipData[index] = FColor(uint8(255 * BotClamp), uint8(255 * BotClamp), uint8(255 * BotClamp));

			uint8 Fall = (uint8)FMath::GetMappedRangeValueClamped({ -1,1 }, { 0,255 }, FallAmt);
			FallMipData[index] = FColor(Fall, Fall, Fall);

			uint8 Offs = (uint8)FMath::GetMappedRangeValueClamped({ -1,1 }, { 0,255 }, OffsetAmt);
			OffsMipData[index] = FColor(Offs, Offs, Offs);

			uint8 Steep = (uint8)FMath::GetMappedRangeValueClamped({ 0,1 }, { 0,255 }, SteepAmt);
			SteepMipData[index] = FColor(Steep, Steep, Steep);
		}

		if (TopH > -1)
		{
			for (int z = 0; z < FullDepth; ++z)
			{
				float ZMapped = float(z) - (FullDepth / 2.0f);

				float OffsMapped = FMath::GetMappedRangeValueClamped({ -1.0f, 1.0f }, { -float(UnderMaxDepth),float(TopMaxDepth) }, Offset);

				float UndBoundMapped = FMath::GetMappedRangeValueClamped({ -1.0f, 0.0f }, { -float(UnderMaxDepth), /*1.0f*/-(EquatorThickness*(1-Falloff)) }, -FMath::Clamp(UndH, 0.0f, 1.0f));
				float TopBoundMapped = FMath::GetMappedRangeValueClamped({ 0.0f, 1.0f }, { /*1.0f*/ (EquatorThickness*(1-Falloff)), float(TopMaxDepth) }, FMath::Clamp(TopH, 0.0f, 1.0f));

				UndBoundMapped += OffsMapped;
				TopBoundMapped += OffsMapped;

				float BaseTerrain = float(ZMapped <= TopBoundMapped && ZMapped >= UndBoundMapped ? 1 : 0);

				FVector MapPoint3D = MapPoint;
				MapPoint3D.Z = float(z) - HalfBounds.Z;

				float Decay = DetermineDecay(MapPoint3D, HalfBounds) * MaxDecayVariance;

				Decay *= FMath::Clamp((UndBoundMapped - ZMapped) / UndBoundMapped, 0.0f, 1.0f);

				if (BaseTerrain > 0)
					TerrainMap[MapIterator.X][MapIterator.Y][z] = FMath::Clamp(FMath::RoundToInt((BaseTerrain + Decay)/2.0f), 0, 1);
			}
		}

		++MapIterator.Y;
		if (MapIterator.Y == MapHeight)
		{
			MapIterator.Y = 0;
			++MapIterator.X;

			if (MapIterator.X == MapWidth)
			{
				MapIterator = FIntVector::ZeroValue;
				TerrainMapProgress = 100;

				TopHeightMapColorTexture->PlatformData->Mips[0].BulkData.Unlock();
				BottomHeightMapColorTexture->PlatformData->Mips[0].BulkData.Unlock();
				TopHeightMapTexture->PlatformData->Mips[0].BulkData.Unlock();
				BottomHeightMapTexture->PlatformData->Mips[0].BulkData.Unlock();
				FalloffMapTexture->PlatformData->Mips[0].BulkData.Unlock();
				OffsetMapTexture->PlatformData->Mips[0].BulkData.Unlock();
				SteepnessMapTexture->PlatformData->Mips[0].BulkData.Unlock();

				TopHeightMapColorTexture->UpdateResource();
				BottomHeightMapColorTexture->UpdateResource();
				TopHeightMapTexture->UpdateResource();
				BottomHeightMapTexture->UpdateResource();
				FalloffMapTexture->UpdateResource();
				OffsetMapTexture->UpdateResource();
				SteepnessMapTexture->UpdateResource();

				// OPEN THE WORM CAN
				while (Worms.Num() < MaxNumWorms)
				{
					FIntVector currpick(RandStream.RandRange(0, MapWidth - 1), RandStream.RandRange(0, MapHeight - 1), FMath::RoundToInt(RandStream.FRandRange(0, 1)) * (FullDepth - 1));

					if (currpick.Z == 0)
					{
						// cast line up to nearest solid point on underside

						while (currpick.Z < FullDepth && 0 == TerrainMap[currpick.X][currpick.Y][currpick.Z])
							++currpick.Z;
					}
					else
					{
						// cast line down to nearest solid point on topside

						while (currpick.Z > -1 && 0 == TerrainMap[currpick.X][currpick.Y][currpick.Z])
							--currpick.Z;
					}

					if (currpick.Z < FullDepth && currpick.Z > -1)
					{
						FVector2D SizeRange(RandStream.FRandRange(MinWormSizes.X, MinWormSizes.Y), RandStream.FRandRange(MaxWormSizes.X, MaxWormSizes.Y));
						Worms[Worms.AddDefaulted()].Init(currpick, SizeRange, WormOctaveSettings, WormAngleVariance, RandStream, /*RandStream.FRandRange(0, 1) < 0.2f ? 1 :*/ 0);
					}

					if (Worms.Num() > MinNumWorms && RandStream.FRandRange(0, 1) < 0.2f)
						break;
				}


				OGNumWorms = Worms.Num();
			}
		}

		if (TerrainMapProgress != 100)
		{
			int PrevProgress = TerrainMapProgress;
			TerrainMapProgress = (float(MapIterator.X) / MapWidth) * 100;
			if (PrevProgress != TerrainMapProgress)
				UStaticFuncLib::Print("Base Terrain Map Progress At " + FString::FromInt(TerrainMapProgress) + "%", true);
		}
		else
			UStaticFuncLib::Print("Base Terrain Map Generated.", true);
	}
	else if (WormProgress < 100)
	{
		for (int currWorm = Worms.Num() - 1; currWorm >= 0; --currWorm)
		{
			//FVector PrevPos = ((FVector(Worms[currWorm].Pos) - HalfBounds)*TileSize) + GetActorLocation();
			Worms[currWorm].Step(FMath::Clamp(float(currWormIterations) / MinWormIterations, 0.0f, 1.0f), HalfBounds);
			//UKismetSystemLibrary::DrawDebugLine(this, PrevPos, ((FVector(Worms[currWorm].Pos) - HalfBounds)*TileSize) + GetActorLocation(), FLinearColor::Red, 10000, 10);

			Worms[currWorm].Tunnel(TerrainMap);

			if (currWormIterations >= MaxWormIterations || (currWormIterations >= MinWormIterations && RandStream.FRandRange(0, 1) < 0.2f))
			{
				Worms.RemoveAt(currWorm);
			}
		}

		++currWormIterations;


		if (Worms.Num() < 1)
		{
			WormProgress = 100;

			GetWorld()->GetTimerManager().ClearTimer(IterationTimerHandle);
			GenerationFinishedDelegate.Broadcast();
		}

		if (WormProgress != 100)
		{
			int PrevProgress = WormProgress;
			WormProgress = (1 - (float(Worms.Num()) / OGNumWorms)) * 100;
			if (PrevProgress != WormProgress)
				UStaticFuncLib::Print("Cave Digging Progress At " + FString::FromInt(WormProgress) + "%", true);
		}
		else
			UStaticFuncLib::Print("Caves Generated.", true);
	}
}

float AProceduralWholeMap::DetermineFalloff_Implementation(const FVector& CurrPoint, const FVector& MapExtent)
{
	return FMath::GetMappedRangeValueClamped({ 2.8f, 3.0f }, { 0.0f, 1.0f }, (CurrPoint / (MapExtent*0.45f)).Size());
}

float AProceduralWholeMap::DetermineSteepness_Implementation(const FVector& CurrPoint, const FVector& MapExtent)
{

	int octaves = SteepnessMapOctaveSettings.Octaves;						//8;
	const float& persistence = SteepnessMapOctaveSettings.Persistence;		//0.5f;
	const float& NoiseScale = SteepnessMapOctaveSettings.NoiseScale;		//0.0015f;

	FVector MapPoint = CurrPoint * NoiseScale;

	float frequency = 1, amplitude = 1, ResultVal = 0, maxval = 0;
	while (--octaves >= 0)
	{
		ResultVal += USimplexNoiseLibrary::SimplexNoise2D(MapPoint.X * frequency, MapPoint.Y * frequency/*, -1, 1*/) * amplitude;
		maxval += amplitude;
		amplitude *= persistence;
		frequency *= 2;
	}

	return FMath::GetMappedRangeValueClamped({ -3.0f, 3.0f }, { 0.0f, SteepnessThreshold }, FMath::Pow(3, ResultVal / maxval));
}

float AProceduralWholeMap::DetermineTopHeight_Implementation(const FVector& CurrPoint, const FVector& MapExtent)
{
	int octaves = TopHeightMapOctaveSettings.Octaves;					//8;
	const float& persistence = TopHeightMapOctaveSettings.Persistence;	//0.5f;
	const float& NoiseScale = TopHeightMapOctaveSettings.NoiseScale;	//0.01f;
	FVector MapPoint = CurrPoint * NoiseScale;

	float frequency = 1, amplitude = 1, ResultVal = 0, maxval = 0;
	while (--octaves >= 0)
	{
		ResultVal += USimplexNoiseLibrary::SimplexNoise2D(MapPoint.X * frequency, MapPoint.Y * frequency/*, -1, 1*/) * amplitude;
		maxval += amplitude;
		amplitude *= persistence;
		frequency *= 2;
	}

	return ResultVal /= maxval;
}

float AProceduralWholeMap::DetermineUndersideHeight_Implementation(const FVector& CurrPoint, const FVector& MapExtent)
{
	int octaves = BottomHeightMapOctaveSettings.Octaves;					//4;
	const float& persistence = BottomHeightMapOctaveSettings.Persistence;	//2;
	const float& NoiseScale = BottomHeightMapOctaveSettings.NoiseScale;		//0.0025f;
	FVector MapPoint = -CurrPoint * NoiseScale;

	float frequency = 1, amplitude = 1, ResultVal = 0, maxval = 0;
	while (--octaves >= 0)
	{
		ResultVal += USimplexNoiseLibrary::SimplexNoise2D(MapPoint.X * frequency, MapPoint.Y * frequency/*, -1, 1*/) * amplitude;
		maxval += amplitude;
		amplitude *= persistence;
		frequency *= 2;
	}

	return ResultVal /= maxval;
}

float AProceduralWholeMap::DetermineOffset_Implementation(const FVector& CurrPoint, const FVector& MapExtent)
{
	int octaves = OffsetMapOctaveSettings.Octaves;						//8;
	const float& persistence = OffsetMapOctaveSettings.Persistence;		//0.5f;
	const float& NoiseScale = OffsetMapOctaveSettings.NoiseScale;		//0.0015f;

	FVector MapPoint = CurrPoint * NoiseScale;

	float frequency = 1, amplitude = 1, ResultVal = 0, maxval = 0;
	while (--octaves >= 0)
	{
		ResultVal += USimplexNoiseLibrary::SimplexNoise2D(MapPoint.X * frequency, MapPoint.Y * frequency/*, -1, 1*/) * amplitude;
		maxval += amplitude;
		amplitude *= persistence;
		frequency *= 2;
	}

	return ResultVal / maxval;
}

float AProceduralWholeMap::DetermineDecay_Implementation(const FVector& CurrPoint, const FVector& MapExtent)
{
	int octaves = DecayMapOctaveSettings.Octaves;						//16;
	const float& persistence = DecayMapOctaveSettings.Persistence;		//0.8f;
	const float& NoiseScale = DecayMapOctaveSettings.NoiseScale;		//0.005f;

	FVector MapPoint = CurrPoint * NoiseScale;

	float frequency = 1, amplitude = 1, ResultVal = 0, maxval = 0;
	while (--octaves >= 0)
	{
		ResultVal += USimplexNoiseLibrary::SimplexNoise3D(MapPoint.X * frequency, MapPoint.Y * frequency, MapPoint.Z * frequency) * amplitude;
		maxval += amplitude;
		amplitude *= persistence;
		frequency *= 2;
	}

	return ResultVal / maxval;
}

void AProceduralWholeMap::GetTerrainIsosurface(UPARAM(Ref) class UProceduralMeshComponent*& ProcMeshComp)
{
	TArray<FVector> Verts;
	TArray<int> Indices; // triangles
	TArray<FVector> Normals;
	TArray<FVector2D> UVs;
	TArray<FColor> VertColors;
	TArray<FProcMeshTangent> Tangents;

	MapIterator = FIntVector::ZeroValue;

	FVector CubeVerts[8];
	char CubeVals[8];
	while (MapIterator.X < MapWidth - 1)
	{
		while (MapIterator.Y < MapHeight - 1)
		{
			while (MapIterator.Z < FullDepth - 1)
			{
				FIntVector PointA = MapIterator + FIntVector(0, 0, 0);
				FIntVector PointB = MapIterator + FIntVector(1, 0, 0);
				FIntVector PointC = MapIterator + FIntVector(0, 1, 0);
				FIntVector PointD = MapIterator + FIntVector(1, 1, 0);
				FIntVector PointE = MapIterator + FIntVector(0, 0, 1);
				FIntVector PointF = MapIterator + FIntVector(1, 0, 1);
				FIntVector PointG = MapIterator + FIntVector(0, 1, 1);
				FIntVector PointH = MapIterator + FIntVector(1, 1, 1);

				CubeVerts[0] = (FVector(PointA) - HalfBounds);
				CubeVerts[1] = (FVector(PointB) - HalfBounds);
				CubeVerts[2] = (FVector(PointC) - HalfBounds);
				CubeVerts[3] = (FVector(PointD) - HalfBounds);
				CubeVerts[4] = (FVector(PointE) - HalfBounds);
				CubeVerts[5] = (FVector(PointF) - HalfBounds);
				CubeVerts[6] = (FVector(PointG) - HalfBounds);
				CubeVerts[7] = (FVector(PointH) - HalfBounds);

				CubeVals[0] = TerrainMap[PointA.X][PointA.Y][PointA.Z];
				CubeVals[1] = TerrainMap[PointB.X][PointB.Y][PointB.Z];
				CubeVals[2] = TerrainMap[PointC.X][PointC.Y][PointC.Z];
				CubeVals[3] = TerrainMap[PointD.X][PointD.Y][PointD.Z];
				CubeVals[4] = TerrainMap[PointE.X][PointE.Y][PointE.Z];
				CubeVals[5] = TerrainMap[PointF.X][PointF.Y][PointF.Z];
				CubeVals[6] = TerrainMap[PointG.X][PointG.Y][PointG.Z];
				CubeVals[7] = TerrainMap[PointH.X][PointH.Y][PointH.Z];


				TArray<FVector> RetrievedVerts;
				if (March(CubeVerts, CubeVals, RetrievedVerts))
				{
					for (int currTri = 0; currTri < RetrievedVerts.Num(); currTri += 3)
					{
						FVector& p2 = RetrievedVerts[currTri];
						FVector& p1 = RetrievedVerts[currTri + 1];
						FVector& p0 = RetrievedVerts[currTri + 2];

						Indices.Add(Verts.Add(p2 * TileSize));
						Indices.Add(Verts.Add(p1 * TileSize));
						Indices.Add(Verts.Add(p0 * TileSize));

						FVector Edge1 = p1 - p0;
						FVector Edge2 = p2 - p0;

						FVector TanX = Edge1.GetSafeNormal();
						FVector TanZ = (Edge1^Edge2).GetSafeNormal();

						for (int i = 0; i < 3; ++i)
						{
							Tangents.Add(FProcMeshTangent(TanX, false));
							Normals.Add(TanZ);
						}
					}
				}

				++MapIterator.Z;
			}
			MapIterator.Z = 0;
			++MapIterator.Y;
		}
		MapIterator.Y = 0;
		++MapIterator.X;
	}

	ProcMeshComp->CreateMeshSection(0, Verts, Indices, Normals, UVs, VertColors, Tangents, true);
}

// const T(&l)[N]
bool AProceduralWholeMap::March(const FVector(&CubeVerts)[8], const char(&CubeVals)[8], TArray<FVector>& OutVerts)
{
	unsigned char CubeIndex = CubeVals[0] |
							 (CubeVals[1] << 1) |
							 (CubeVals[3] << 2) |
							 (CubeVals[2] << 3) |
							 (CubeVals[4] << 4) |
							 (CubeVals[5] << 5) |
							 (CubeVals[7] << 6) |
							 (CubeVals[6] << 7);

	int edgeTable[256] = {
		0x0  , 0x109, 0x203, 0x30a, 0x406, 0x50f, 0x605, 0x70c,
		0x80c, 0x905, 0xa0f, 0xb06, 0xc0a, 0xd03, 0xe09, 0xf00,
		0x190, 0x99 , 0x393, 0x29a, 0x596, 0x49f, 0x795, 0x69c,
		0x99c, 0x895, 0xb9f, 0xa96, 0xd9a, 0xc93, 0xf99, 0xe90,
		0x230, 0x339, 0x33 , 0x13a, 0x636, 0x73f, 0x435, 0x53c,
		0xa3c, 0xb35, 0x83f, 0x936, 0xe3a, 0xf33, 0xc39, 0xd30,
		0x3a0, 0x2a9, 0x1a3, 0xaa , 0x7a6, 0x6af, 0x5a5, 0x4ac,
		0xbac, 0xaa5, 0x9af, 0x8a6, 0xfaa, 0xea3, 0xda9, 0xca0,
		0x460, 0x569, 0x663, 0x76a, 0x66 , 0x16f, 0x265, 0x36c,
		0xc6c, 0xd65, 0xe6f, 0xf66, 0x86a, 0x963, 0xa69, 0xb60,
		0x5f0, 0x4f9, 0x7f3, 0x6fa, 0x1f6, 0xff , 0x3f5, 0x2fc,
		0xdfc, 0xcf5, 0xfff, 0xef6, 0x9fa, 0x8f3, 0xbf9, 0xaf0,
		0x650, 0x759, 0x453, 0x55a, 0x256, 0x35f, 0x55 , 0x15c,
		0xe5c, 0xf55, 0xc5f, 0xd56, 0xa5a, 0xb53, 0x859, 0x950,
		0x7c0, 0x6c9, 0x5c3, 0x4ca, 0x3c6, 0x2cf, 0x1c5, 0xcc ,
		0xfcc, 0xec5, 0xdcf, 0xcc6, 0xbca, 0xac3, 0x9c9, 0x8c0,
		0x8c0, 0x9c9, 0xac3, 0xbca, 0xcc6, 0xdcf, 0xec5, 0xfcc,
		0xcc , 0x1c5, 0x2cf, 0x3c6, 0x4ca, 0x5c3, 0x6c9, 0x7c0,
		0x950, 0x859, 0xb53, 0xa5a, 0xd56, 0xc5f, 0xf55, 0xe5c,
		0x15c, 0x55 , 0x35f, 0x256, 0x55a, 0x453, 0x759, 0x650,
		0xaf0, 0xbf9, 0x8f3, 0x9fa, 0xef6, 0xfff, 0xcf5, 0xdfc,
		0x2fc, 0x3f5, 0xff , 0x1f6, 0x6fa, 0x7f3, 0x4f9, 0x5f0,
		0xb60, 0xa69, 0x963, 0x86a, 0xf66, 0xe6f, 0xd65, 0xc6c,
		0x36c, 0x265, 0x16f, 0x66 , 0x76a, 0x663, 0x569, 0x460,
		0xca0, 0xda9, 0xea3, 0xfaa, 0x8a6, 0x9af, 0xaa5, 0xbac,
		0x4ac, 0x5a5, 0x6af, 0x7a6, 0xaa , 0x1a3, 0x2a9, 0x3a0,
		0xd30, 0xc39, 0xf33, 0xe3a, 0x936, 0x83f, 0xb35, 0xa3c,
		0x53c, 0x435, 0x73f, 0x636, 0x13a, 0x33 , 0x339, 0x230,
		0xe90, 0xf99, 0xc93, 0xd9a, 0xa96, 0xb9f, 0x895, 0x99c,
		0x69c, 0x795, 0x49f, 0x596, 0x29a, 0x393, 0x99 , 0x190,
		0xf00, 0xe09, 0xd03, 0xc0a, 0xb06, 0xa0f, 0x905, 0x80c,
		0x70c, 0x605, 0x50f, 0x406, 0x30a, 0x203, 0x109, 0x0 };
	int triTable[256][16] =
	{ { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ 0, 8, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ 0, 1, 9, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ 1, 8, 3, 9, 8, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ 1, 2, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ 0, 8, 3, 1, 2, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ 9, 2, 10, 0, 2, 9, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ 2, 8, 3, 2, 10, 8, 10, 9, 8, -1, -1, -1, -1, -1, -1, -1 },
	{ 3, 11, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ 0, 11, 2, 8, 11, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ 1, 9, 0, 2, 3, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ 1, 11, 2, 1, 9, 11, 9, 8, 11, -1, -1, -1, -1, -1, -1, -1 },
	{ 3, 10, 1, 11, 10, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ 0, 10, 1, 0, 8, 10, 8, 11, 10, -1, -1, -1, -1, -1, -1, -1 },
	{ 3, 9, 0, 3, 11, 9, 11, 10, 9, -1, -1, -1, -1, -1, -1, -1 },
	{ 9, 8, 10, 10, 8, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ 4, 7, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ 4, 3, 0, 7, 3, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ 0, 1, 9, 8, 4, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ 4, 1, 9, 4, 7, 1, 7, 3, 1, -1, -1, -1, -1, -1, -1, -1 },
	{ 1, 2, 10, 8, 4, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ 3, 4, 7, 3, 0, 4, 1, 2, 10, -1, -1, -1, -1, -1, -1, -1 },
	{ 9, 2, 10, 9, 0, 2, 8, 4, 7, -1, -1, -1, -1, -1, -1, -1 },
	{ 2, 10, 9, 2, 9, 7, 2, 7, 3, 7, 9, 4, -1, -1, -1, -1 },
	{ 8, 4, 7, 3, 11, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ 11, 4, 7, 11, 2, 4, 2, 0, 4, -1, -1, -1, -1, -1, -1, -1 },
	{ 9, 0, 1, 8, 4, 7, 2, 3, 11, -1, -1, -1, -1, -1, -1, -1 },
	{ 4, 7, 11, 9, 4, 11, 9, 11, 2, 9, 2, 1, -1, -1, -1, -1 },
	{ 3, 10, 1, 3, 11, 10, 7, 8, 4, -1, -1, -1, -1, -1, -1, -1 },
	{ 1, 11, 10, 1, 4, 11, 1, 0, 4, 7, 11, 4, -1, -1, -1, -1 },
	{ 4, 7, 8, 9, 0, 11, 9, 11, 10, 11, 0, 3, -1, -1, -1, -1 },
	{ 4, 7, 11, 4, 11, 9, 9, 11, 10, -1, -1, -1, -1, -1, -1, -1 },
	{ 9, 5, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ 9, 5, 4, 0, 8, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ 0, 5, 4, 1, 5, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ 8, 5, 4, 8, 3, 5, 3, 1, 5, -1, -1, -1, -1, -1, -1, -1 },
	{ 1, 2, 10, 9, 5, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ 3, 0, 8, 1, 2, 10, 4, 9, 5, -1, -1, -1, -1, -1, -1, -1 },
	{ 5, 2, 10, 5, 4, 2, 4, 0, 2, -1, -1, -1, -1, -1, -1, -1 },
	{ 2, 10, 5, 3, 2, 5, 3, 5, 4, 3, 4, 8, -1, -1, -1, -1 },
	{ 9, 5, 4, 2, 3, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ 0, 11, 2, 0, 8, 11, 4, 9, 5, -1, -1, -1, -1, -1, -1, -1 },
	{ 0, 5, 4, 0, 1, 5, 2, 3, 11, -1, -1, -1, -1, -1, -1, -1 },
	{ 2, 1, 5, 2, 5, 8, 2, 8, 11, 4, 8, 5, -1, -1, -1, -1 },
	{ 10, 3, 11, 10, 1, 3, 9, 5, 4, -1, -1, -1, -1, -1, -1, -1 },
	{ 4, 9, 5, 0, 8, 1, 8, 10, 1, 8, 11, 10, -1, -1, -1, -1 },
	{ 5, 4, 0, 5, 0, 11, 5, 11, 10, 11, 0, 3, -1, -1, -1, -1 },
	{ 5, 4, 8, 5, 8, 10, 10, 8, 11, -1, -1, -1, -1, -1, -1, -1 },
	{ 9, 7, 8, 5, 7, 9, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ 9, 3, 0, 9, 5, 3, 5, 7, 3, -1, -1, -1, -1, -1, -1, -1 },
	{ 0, 7, 8, 0, 1, 7, 1, 5, 7, -1, -1, -1, -1, -1, -1, -1 },
	{ 1, 5, 3, 3, 5, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ 9, 7, 8, 9, 5, 7, 10, 1, 2, -1, -1, -1, -1, -1, -1, -1 },
	{ 10, 1, 2, 9, 5, 0, 5, 3, 0, 5, 7, 3, -1, -1, -1, -1 },
	{ 8, 0, 2, 8, 2, 5, 8, 5, 7, 10, 5, 2, -1, -1, -1, -1 },
	{ 2, 10, 5, 2, 5, 3, 3, 5, 7, -1, -1, -1, -1, -1, -1, -1 },
	{ 7, 9, 5, 7, 8, 9, 3, 11, 2, -1, -1, -1, -1, -1, -1, -1 },
	{ 9, 5, 7, 9, 7, 2, 9, 2, 0, 2, 7, 11, -1, -1, -1, -1 },
	{ 2, 3, 11, 0, 1, 8, 1, 7, 8, 1, 5, 7, -1, -1, -1, -1 },
	{ 11, 2, 1, 11, 1, 7, 7, 1, 5, -1, -1, -1, -1, -1, -1, -1 },
	{ 9, 5, 8, 8, 5, 7, 10, 1, 3, 10, 3, 11, -1, -1, -1, -1 },
	{ 5, 7, 0, 5, 0, 9, 7, 11, 0, 1, 0, 10, 11, 10, 0, -1 },
	{ 11, 10, 0, 11, 0, 3, 10, 5, 0, 8, 0, 7, 5, 7, 0, -1 },
	{ 11, 10, 5, 7, 11, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ 10, 6, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ 0, 8, 3, 5, 10, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ 9, 0, 1, 5, 10, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ 1, 8, 3, 1, 9, 8, 5, 10, 6, -1, -1, -1, -1, -1, -1, -1 },
	{ 1, 6, 5, 2, 6, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ 1, 6, 5, 1, 2, 6, 3, 0, 8, -1, -1, -1, -1, -1, -1, -1 },
	{ 9, 6, 5, 9, 0, 6, 0, 2, 6, -1, -1, -1, -1, -1, -1, -1 },
	{ 5, 9, 8, 5, 8, 2, 5, 2, 6, 3, 2, 8, -1, -1, -1, -1 },
	{ 2, 3, 11, 10, 6, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ 11, 0, 8, 11, 2, 0, 10, 6, 5, -1, -1, -1, -1, -1, -1, -1 },
	{ 0, 1, 9, 2, 3, 11, 5, 10, 6, -1, -1, -1, -1, -1, -1, -1 },
	{ 5, 10, 6, 1, 9, 2, 9, 11, 2, 9, 8, 11, -1, -1, -1, -1 },
	{ 6, 3, 11, 6, 5, 3, 5, 1, 3, -1, -1, -1, -1, -1, -1, -1 },
	{ 0, 8, 11, 0, 11, 5, 0, 5, 1, 5, 11, 6, -1, -1, -1, -1 },
	{ 3, 11, 6, 0, 3, 6, 0, 6, 5, 0, 5, 9, -1, -1, -1, -1 },
	{ 6, 5, 9, 6, 9, 11, 11, 9, 8, -1, -1, -1, -1, -1, -1, -1 },
	{ 5, 10, 6, 4, 7, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ 4, 3, 0, 4, 7, 3, 6, 5, 10, -1, -1, -1, -1, -1, -1, -1 },
	{ 1, 9, 0, 5, 10, 6, 8, 4, 7, -1, -1, -1, -1, -1, -1, -1 },
	{ 10, 6, 5, 1, 9, 7, 1, 7, 3, 7, 9, 4, -1, -1, -1, -1 },
	{ 6, 1, 2, 6, 5, 1, 4, 7, 8, -1, -1, -1, -1, -1, -1, -1 },
	{ 1, 2, 5, 5, 2, 6, 3, 0, 4, 3, 4, 7, -1, -1, -1, -1 },
	{ 8, 4, 7, 9, 0, 5, 0, 6, 5, 0, 2, 6, -1, -1, -1, -1 },
	{ 7, 3, 9, 7, 9, 4, 3, 2, 9, 5, 9, 6, 2, 6, 9, -1 },
	{ 3, 11, 2, 7, 8, 4, 10, 6, 5, -1, -1, -1, -1, -1, -1, -1 },
	{ 5, 10, 6, 4, 7, 2, 4, 2, 0, 2, 7, 11, -1, -1, -1, -1 },
	{ 0, 1, 9, 4, 7, 8, 2, 3, 11, 5, 10, 6, -1, -1, -1, -1 },
	{ 9, 2, 1, 9, 11, 2, 9, 4, 11, 7, 11, 4, 5, 10, 6, -1 },
	{ 8, 4, 7, 3, 11, 5, 3, 5, 1, 5, 11, 6, -1, -1, -1, -1 },
	{ 5, 1, 11, 5, 11, 6, 1, 0, 11, 7, 11, 4, 0, 4, 11, -1 },
	{ 0, 5, 9, 0, 6, 5, 0, 3, 6, 11, 6, 3, 8, 4, 7, -1 },
	{ 6, 5, 9, 6, 9, 11, 4, 7, 9, 7, 11, 9, -1, -1, -1, -1 },
	{ 10, 4, 9, 6, 4, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ 4, 10, 6, 4, 9, 10, 0, 8, 3, -1, -1, -1, -1, -1, -1, -1 },
	{ 10, 0, 1, 10, 6, 0, 6, 4, 0, -1, -1, -1, -1, -1, -1, -1 },
	{ 8, 3, 1, 8, 1, 6, 8, 6, 4, 6, 1, 10, -1, -1, -1, -1 },
	{ 1, 4, 9, 1, 2, 4, 2, 6, 4, -1, -1, -1, -1, -1, -1, -1 },
	{ 3, 0, 8, 1, 2, 9, 2, 4, 9, 2, 6, 4, -1, -1, -1, -1 },
	{ 0, 2, 4, 4, 2, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ 8, 3, 2, 8, 2, 4, 4, 2, 6, -1, -1, -1, -1, -1, -1, -1 },
	{ 10, 4, 9, 10, 6, 4, 11, 2, 3, -1, -1, -1, -1, -1, -1, -1 },
	{ 0, 8, 2, 2, 8, 11, 4, 9, 10, 4, 10, 6, -1, -1, -1, -1 },
	{ 3, 11, 2, 0, 1, 6, 0, 6, 4, 6, 1, 10, -1, -1, -1, -1 },
	{ 6, 4, 1, 6, 1, 10, 4, 8, 1, 2, 1, 11, 8, 11, 1, -1 },
	{ 9, 6, 4, 9, 3, 6, 9, 1, 3, 11, 6, 3, -1, -1, -1, -1 },
	{ 8, 11, 1, 8, 1, 0, 11, 6, 1, 9, 1, 4, 6, 4, 1, -1 },
	{ 3, 11, 6, 3, 6, 0, 0, 6, 4, -1, -1, -1, -1, -1, -1, -1 },
	{ 6, 4, 8, 11, 6, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ 7, 10, 6, 7, 8, 10, 8, 9, 10, -1, -1, -1, -1, -1, -1, -1 },
	{ 0, 7, 3, 0, 10, 7, 0, 9, 10, 6, 7, 10, -1, -1, -1, -1 },
	{ 10, 6, 7, 1, 10, 7, 1, 7, 8, 1, 8, 0, -1, -1, -1, -1 },
	{ 10, 6, 7, 10, 7, 1, 1, 7, 3, -1, -1, -1, -1, -1, -1, -1 },
	{ 1, 2, 6, 1, 6, 8, 1, 8, 9, 8, 6, 7, -1, -1, -1, -1 },
	{ 2, 6, 9, 2, 9, 1, 6, 7, 9, 0, 9, 3, 7, 3, 9, -1 },
	{ 7, 8, 0, 7, 0, 6, 6, 0, 2, -1, -1, -1, -1, -1, -1, -1 },
	{ 7, 3, 2, 6, 7, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ 2, 3, 11, 10, 6, 8, 10, 8, 9, 8, 6, 7, -1, -1, -1, -1 },
	{ 2, 0, 7, 2, 7, 11, 0, 9, 7, 6, 7, 10, 9, 10, 7, -1 },
	{ 1, 8, 0, 1, 7, 8, 1, 10, 7, 6, 7, 10, 2, 3, 11, -1 },
	{ 11, 2, 1, 11, 1, 7, 10, 6, 1, 6, 7, 1, -1, -1, -1, -1 },
	{ 8, 9, 6, 8, 6, 7, 9, 1, 6, 11, 6, 3, 1, 3, 6, -1 },
	{ 0, 9, 1, 11, 6, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ 7, 8, 0, 7, 0, 6, 3, 11, 0, 11, 6, 0, -1, -1, -1, -1 },
	{ 7, 11, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ 7, 6, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ 3, 0, 8, 11, 7, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ 0, 1, 9, 11, 7, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ 8, 1, 9, 8, 3, 1, 11, 7, 6, -1, -1, -1, -1, -1, -1, -1 },
	{ 10, 1, 2, 6, 11, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ 1, 2, 10, 3, 0, 8, 6, 11, 7, -1, -1, -1, -1, -1, -1, -1 },
	{ 2, 9, 0, 2, 10, 9, 6, 11, 7, -1, -1, -1, -1, -1, -1, -1 },
	{ 6, 11, 7, 2, 10, 3, 10, 8, 3, 10, 9, 8, -1, -1, -1, -1 },
	{ 7, 2, 3, 6, 2, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ 7, 0, 8, 7, 6, 0, 6, 2, 0, -1, -1, -1, -1, -1, -1, -1 },
	{ 2, 7, 6, 2, 3, 7, 0, 1, 9, -1, -1, -1, -1, -1, -1, -1 },
	{ 1, 6, 2, 1, 8, 6, 1, 9, 8, 8, 7, 6, -1, -1, -1, -1 },
	{ 10, 7, 6, 10, 1, 7, 1, 3, 7, -1, -1, -1, -1, -1, -1, -1 },
	{ 10, 7, 6, 1, 7, 10, 1, 8, 7, 1, 0, 8, -1, -1, -1, -1 },
	{ 0, 3, 7, 0, 7, 10, 0, 10, 9, 6, 10, 7, -1, -1, -1, -1 },
	{ 7, 6, 10, 7, 10, 8, 8, 10, 9, -1, -1, -1, -1, -1, -1, -1 },
	{ 6, 8, 4, 11, 8, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ 3, 6, 11, 3, 0, 6, 0, 4, 6, -1, -1, -1, -1, -1, -1, -1 },
	{ 8, 6, 11, 8, 4, 6, 9, 0, 1, -1, -1, -1, -1, -1, -1, -1 },
	{ 9, 4, 6, 9, 6, 3, 9, 3, 1, 11, 3, 6, -1, -1, -1, -1 },
	{ 6, 8, 4, 6, 11, 8, 2, 10, 1, -1, -1, -1, -1, -1, -1, -1 },
	{ 1, 2, 10, 3, 0, 11, 0, 6, 11, 0, 4, 6, -1, -1, -1, -1 },
	{ 4, 11, 8, 4, 6, 11, 0, 2, 9, 2, 10, 9, -1, -1, -1, -1 },
	{ 10, 9, 3, 10, 3, 2, 9, 4, 3, 11, 3, 6, 4, 6, 3, -1 },
	{ 8, 2, 3, 8, 4, 2, 4, 6, 2, -1, -1, -1, -1, -1, -1, -1 },
	{ 0, 4, 2, 4, 6, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ 1, 9, 0, 2, 3, 4, 2, 4, 6, 4, 3, 8, -1, -1, -1, -1 },
	{ 1, 9, 4, 1, 4, 2, 2, 4, 6, -1, -1, -1, -1, -1, -1, -1 },
	{ 8, 1, 3, 8, 6, 1, 8, 4, 6, 6, 10, 1, -1, -1, -1, -1 },
	{ 10, 1, 0, 10, 0, 6, 6, 0, 4, -1, -1, -1, -1, -1, -1, -1 },
	{ 4, 6, 3, 4, 3, 8, 6, 10, 3, 0, 3, 9, 10, 9, 3, -1 },
	{ 10, 9, 4, 6, 10, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ 4, 9, 5, 7, 6, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ 0, 8, 3, 4, 9, 5, 11, 7, 6, -1, -1, -1, -1, -1, -1, -1 },
	{ 5, 0, 1, 5, 4, 0, 7, 6, 11, -1, -1, -1, -1, -1, -1, -1 },
	{ 11, 7, 6, 8, 3, 4, 3, 5, 4, 3, 1, 5, -1, -1, -1, -1 },
	{ 9, 5, 4, 10, 1, 2, 7, 6, 11, -1, -1, -1, -1, -1, -1, -1 },
	{ 6, 11, 7, 1, 2, 10, 0, 8, 3, 4, 9, 5, -1, -1, -1, -1 },
	{ 7, 6, 11, 5, 4, 10, 4, 2, 10, 4, 0, 2, -1, -1, -1, -1 },
	{ 3, 4, 8, 3, 5, 4, 3, 2, 5, 10, 5, 2, 11, 7, 6, -1 },
	{ 7, 2, 3, 7, 6, 2, 5, 4, 9, -1, -1, -1, -1, -1, -1, -1 },
	{ 9, 5, 4, 0, 8, 6, 0, 6, 2, 6, 8, 7, -1, -1, -1, -1 },
	{ 3, 6, 2, 3, 7, 6, 1, 5, 0, 5, 4, 0, -1, -1, -1, -1 },
	{ 6, 2, 8, 6, 8, 7, 2, 1, 8, 4, 8, 5, 1, 5, 8, -1 },
	{ 9, 5, 4, 10, 1, 6, 1, 7, 6, 1, 3, 7, -1, -1, -1, -1 },
	{ 1, 6, 10, 1, 7, 6, 1, 0, 7, 8, 7, 0, 9, 5, 4, -1 },
	{ 4, 0, 10, 4, 10, 5, 0, 3, 10, 6, 10, 7, 3, 7, 10, -1 },
	{ 7, 6, 10, 7, 10, 8, 5, 4, 10, 4, 8, 10, -1, -1, -1, -1 },
	{ 6, 9, 5, 6, 11, 9, 11, 8, 9, -1, -1, -1, -1, -1, -1, -1 },
	{ 3, 6, 11, 0, 6, 3, 0, 5, 6, 0, 9, 5, -1, -1, -1, -1 },
	{ 0, 11, 8, 0, 5, 11, 0, 1, 5, 5, 6, 11, -1, -1, -1, -1 },
	{ 6, 11, 3, 6, 3, 5, 5, 3, 1, -1, -1, -1, -1, -1, -1, -1 },
	{ 1, 2, 10, 9, 5, 11, 9, 11, 8, 11, 5, 6, -1, -1, -1, -1 },
	{ 0, 11, 3, 0, 6, 11, 0, 9, 6, 5, 6, 9, 1, 2, 10, -1 },
	{ 11, 8, 5, 11, 5, 6, 8, 0, 5, 10, 5, 2, 0, 2, 5, -1 },
	{ 6, 11, 3, 6, 3, 5, 2, 10, 3, 10, 5, 3, -1, -1, -1, -1 },
	{ 5, 8, 9, 5, 2, 8, 5, 6, 2, 3, 8, 2, -1, -1, -1, -1 },
	{ 9, 5, 6, 9, 6, 0, 0, 6, 2, -1, -1, -1, -1, -1, -1, -1 },
	{ 1, 5, 8, 1, 8, 0, 5, 6, 8, 3, 8, 2, 6, 2, 8, -1 },
	{ 1, 5, 6, 2, 1, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ 1, 3, 6, 1, 6, 10, 3, 8, 6, 5, 6, 9, 8, 9, 6, -1 },
	{ 10, 1, 0, 10, 0, 6, 9, 5, 0, 5, 6, 0, -1, -1, -1, -1 },
	{ 0, 3, 8, 5, 6, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ 10, 5, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ 11, 5, 10, 7, 5, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ 11, 5, 10, 11, 7, 5, 8, 3, 0, -1, -1, -1, -1, -1, -1, -1 },
	{ 5, 11, 7, 5, 10, 11, 1, 9, 0, -1, -1, -1, -1, -1, -1, -1 },
	{ 10, 7, 5, 10, 11, 7, 9, 8, 1, 8, 3, 1, -1, -1, -1, -1 },
	{ 11, 1, 2, 11, 7, 1, 7, 5, 1, -1, -1, -1, -1, -1, -1, -1 },
	{ 0, 8, 3, 1, 2, 7, 1, 7, 5, 7, 2, 11, -1, -1, -1, -1 },
	{ 9, 7, 5, 9, 2, 7, 9, 0, 2, 2, 11, 7, -1, -1, -1, -1 },
	{ 7, 5, 2, 7, 2, 11, 5, 9, 2, 3, 2, 8, 9, 8, 2, -1 },
	{ 2, 5, 10, 2, 3, 5, 3, 7, 5, -1, -1, -1, -1, -1, -1, -1 },
	{ 8, 2, 0, 8, 5, 2, 8, 7, 5, 10, 2, 5, -1, -1, -1, -1 },
	{ 9, 0, 1, 5, 10, 3, 5, 3, 7, 3, 10, 2, -1, -1, -1, -1 },
	{ 9, 8, 2, 9, 2, 1, 8, 7, 2, 10, 2, 5, 7, 5, 2, -1 },
	{ 1, 3, 5, 3, 7, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ 0, 8, 7, 0, 7, 1, 1, 7, 5, -1, -1, -1, -1, -1, -1, -1 },
	{ 9, 0, 3, 9, 3, 5, 5, 3, 7, -1, -1, -1, -1, -1, -1, -1 },
	{ 9, 8, 7, 5, 9, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ 5, 8, 4, 5, 10, 8, 10, 11, 8, -1, -1, -1, -1, -1, -1, -1 },
	{ 5, 0, 4, 5, 11, 0, 5, 10, 11, 11, 3, 0, -1, -1, -1, -1 },
	{ 0, 1, 9, 8, 4, 10, 8, 10, 11, 10, 4, 5, -1, -1, -1, -1 },
	{ 10, 11, 4, 10, 4, 5, 11, 3, 4, 9, 4, 1, 3, 1, 4, -1 },
	{ 2, 5, 1, 2, 8, 5, 2, 11, 8, 4, 5, 8, -1, -1, -1, -1 },
	{ 0, 4, 11, 0, 11, 3, 4, 5, 11, 2, 11, 1, 5, 1, 11, -1 },
	{ 0, 2, 5, 0, 5, 9, 2, 11, 5, 4, 5, 8, 11, 8, 5, -1 },
	{ 9, 4, 5, 2, 11, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ 2, 5, 10, 3, 5, 2, 3, 4, 5, 3, 8, 4, -1, -1, -1, -1 },
	{ 5, 10, 2, 5, 2, 4, 4, 2, 0, -1, -1, -1, -1, -1, -1, -1 },
	{ 3, 10, 2, 3, 5, 10, 3, 8, 5, 4, 5, 8, 0, 1, 9, -1 },
	{ 5, 10, 2, 5, 2, 4, 1, 9, 2, 9, 4, 2, -1, -1, -1, -1 },
	{ 8, 4, 5, 8, 5, 3, 3, 5, 1, -1, -1, -1, -1, -1, -1, -1 },
	{ 0, 4, 5, 1, 0, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ 8, 4, 5, 8, 5, 3, 9, 0, 5, 0, 3, 5, -1, -1, -1, -1 },
	{ 9, 4, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ 4, 11, 7, 4, 9, 11, 9, 10, 11, -1, -1, -1, -1, -1, -1, -1 },
	{ 0, 8, 3, 4, 9, 7, 9, 11, 7, 9, 10, 11, -1, -1, -1, -1 },
	{ 1, 10, 11, 1, 11, 4, 1, 4, 0, 7, 4, 11, -1, -1, -1, -1 },
	{ 3, 1, 4, 3, 4, 8, 1, 10, 4, 7, 4, 11, 10, 11, 4, -1 },
	{ 4, 11, 7, 9, 11, 4, 9, 2, 11, 9, 1, 2, -1, -1, -1, -1 },
	{ 9, 7, 4, 9, 11, 7, 9, 1, 11, 2, 11, 1, 0, 8, 3, -1 },
	{ 11, 7, 4, 11, 4, 2, 2, 4, 0, -1, -1, -1, -1, -1, -1, -1 },
	{ 11, 7, 4, 11, 4, 2, 8, 3, 4, 3, 2, 4, -1, -1, -1, -1 },
	{ 2, 9, 10, 2, 7, 9, 2, 3, 7, 7, 4, 9, -1, -1, -1, -1 },
	{ 9, 10, 7, 9, 7, 4, 10, 2, 7, 8, 7, 0, 2, 0, 7, -1 },
	{ 3, 7, 10, 3, 10, 2, 7, 4, 10, 1, 10, 0, 4, 0, 10, -1 },
	{ 1, 10, 2, 8, 7, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ 4, 9, 1, 4, 1, 7, 7, 1, 3, -1, -1, -1, -1, -1, -1, -1 },
	{ 4, 9, 1, 4, 1, 7, 0, 8, 1, 8, 7, 1, -1, -1, -1, -1 },
	{ 4, 0, 3, 7, 4, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ 4, 8, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ 9, 10, 8, 10, 11, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ 3, 0, 9, 3, 9, 11, 11, 9, 10, -1, -1, -1, -1, -1, -1, -1 },
	{ 0, 1, 10, 0, 10, 8, 8, 10, 11, -1, -1, -1, -1, -1, -1, -1 },
	{ 3, 1, 10, 11, 3, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ 1, 2, 11, 1, 11, 9, 9, 11, 8, -1, -1, -1, -1, -1, -1, -1 },
	{ 3, 0, 9, 3, 9, 11, 1, 2, 9, 2, 11, 9, -1, -1, -1, -1 },
	{ 0, 2, 11, 8, 0, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ 3, 2, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ 2, 3, 8, 2, 8, 10, 10, 8, 9, -1, -1, -1, -1, -1, -1, -1 },
	{ 9, 10, 2, 0, 9, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ 2, 3, 8, 2, 8, 10, 0, 1, 8, 1, 10, 8, -1, -1, -1, -1 },
	{ 1, 10, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ 1, 3, 8, 9, 1, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ 0, 9, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ 0, 3, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } };


	int RetrievedEdge;
	if (0 == (RetrievedEdge = edgeTable[CubeIndex]))
		return false;

	FVector VertList[12];

	if (RetrievedEdge & 1)
		VertList[0] = FMath::Lerp(CubeVerts[0], CubeVerts[1], 0.5f);
	if (RetrievedEdge & 2)
		VertList[1] = FMath::Lerp(CubeVerts[1], CubeVerts[3], 0.5f);
	if (RetrievedEdge & 4)
		VertList[2] = FMath::Lerp(CubeVerts[2], CubeVerts[3], 0.5f);
	if (RetrievedEdge & 8)
		VertList[3] = FMath::Lerp(CubeVerts[0], CubeVerts[2], 0.5f);

	if (RetrievedEdge & 16)
		VertList[4] = FMath::Lerp(CubeVerts[4], CubeVerts[5], 0.5f);
	if (RetrievedEdge & 32)
		VertList[5] = FMath::Lerp(CubeVerts[5], CubeVerts[7], 0.5f);
	if (RetrievedEdge & 64)
		VertList[6] = FMath::Lerp(CubeVerts[6], CubeVerts[7], 0.5f);
	if (RetrievedEdge & 128)
		VertList[7] = FMath::Lerp(CubeVerts[4], CubeVerts[6], 0.5f);

	if (RetrievedEdge & 256)
		VertList[8] = FMath::Lerp(CubeVerts[0], CubeVerts[4], 0.5f);
	if (RetrievedEdge & 512)
		VertList[9] = FMath::Lerp(CubeVerts[1], CubeVerts[5], 0.5f);
	if (RetrievedEdge & 1024)
		VertList[10] = FMath::Lerp(CubeVerts[3], CubeVerts[7], 0.5f);
	if (RetrievedEdge & 2048)
		VertList[11] = FMath::Lerp(CubeVerts[2], CubeVerts[6], 0.5f);


	for (int currTriplet = 0; triTable[CubeIndex][currTriplet] != -1; currTriplet += 3)
	{
		OutVerts.Add(VertList[triTable[CubeIndex][currTriplet]]);
		OutVerts.Add(VertList[triTable[CubeIndex][currTriplet + 1]]);
		OutVerts.Add(VertList[triTable[CubeIndex][currTriplet + 2]]);
	}

	return OutVerts.Num() > 0;
}





































FRandomStream UWorldGenFuncLib::RandStream;


void UWorldGenFuncLib::ConnectPoints(UObject* WorldContext, const TArray<FVector>& InPoints, TArray<FSpanEdge>& OutEdges, TArray<FRankedPoint>& OutRankedPoints)
{
	//TArray<FSpanTetra> GeneratedTets;
	//
	//Delaunay3D(WorldContext, InPoints, GeneratedTets);
	//
	//for (auto &currTet : GeneratedTets)
	//{
	//	OutEdges.AddUnique(currTet.ABC.AB);
	//	OutEdges.AddUnique(currTet.ABD.AB);
	//	OutEdges.AddUnique(currTet.BCD.AB);
	//	OutEdges.AddUnique(currTet.ACD.AB);
	//
	//	OutEdges.AddUnique(currTet.ABC.BC);
	//	OutEdges.AddUnique(currTet.ABD.BC);
	//	OutEdges.AddUnique(currTet.BCD.BC);
	//	OutEdges.AddUnique(currTet.ACD.BC);
	//
	//	OutEdges.AddUnique(currTet.ABC.CA);
	//	OutEdges.AddUnique(currTet.ABD.CA);
	//	OutEdges.AddUnique(currTet.BCD.CA);
	//	OutEdges.AddUnique(currTet.ACD.CA);
	//}

	TArray<FSpanTri> GeneratedTris;

	Delaunay2D(WorldContext, InPoints, GeneratedTris, OutEdges);

	KruskalMST(InPoints, OutEdges);

	OutRankedPoints.AddDefaulted(InPoints.Num());

	for (int currInd = 0; currInd < InPoints.Num(); ++currInd)
		OutRankedPoints[currInd].Point = InPoints[currInd];

	for (auto& currEdge : OutEdges)
	{
		++OutRankedPoints[InPoints.Find(currEdge.A)].Rank;
		++OutRankedPoints[InPoints.Find(currEdge.B)].Rank;
	}
}

void UWorldGenFuncLib::Delaunay3D(UObject* WorldContext, const TArray<FVector>& Points, TArray<FSpanTetra>& Tetras, TArray<FSpanEdge>& Edges)
{
	FVector Min = Points[0];
	FVector Max = Min;

	for (auto &currPt : Points)
	{
		if (currPt.X < Min.X) Min.X = currPt.X;
		else if (currPt.X > Max.X) Max.X = currPt.X;

		if (currPt.Y < Min.Y) Min.Y = currPt.Y;
		else if (currPt.Y > Max.Y) Max.Y = currPt.Y;

		if (currPt.Z < Min.Z) Min.Z = currPt.Z;
		else if (currPt.Z > Max.Z) Max.Z = currPt.Z;
	}

	// lazily making a tet that encapsulates everything
	// by circumscribing it around the containing sphere

	FVector FitCenter = ((Max - Min) / 2) + Min;
	float FitRadius = (Max - Min).Size() / 2;

	float TetAng = UKismetMathLibrary::DegAcos(-1.0f / 3.0f);
	FRotator TetBottomRotator = UKismetMathLibrary::RotatorFromAxisAndAngle(FVector(0, 0, 1), TetAng);

	FVector SuperTetVertA(0, 0, FitRadius * 10);
	FVector SuperTetVertB = UKismetMathLibrary::RotatorFromAxisAndAngle(FVector(0, 1, 0), TetAng).RotateVector(SuperTetVertA);
	FVector SuperTetVertC = TetBottomRotator.RotateVector(SuperTetVertB);
	FVector SuperTetVertD = TetBottomRotator.RotateVector(SuperTetVertC);

	FSpanTetra SuperTet(SuperTetVertA + FitCenter, SuperTetVertB + FitCenter, SuperTetVertC + FitCenter, SuperTetVertD + FitCenter);

	Tetras.Add(SuperTet);

	for (auto &currPoint : Points)
	{
		TArray<FSpanTri> PolyHed;

		for (int currInd = Tetras.Num() - 1; currInd >= 0; --currInd)
		{
			FSpanTetra& currTet = Tetras[currInd];

			if (currTet.CircumContains(currPoint))
			{
				PolyHed.Add(currTet.ABC);
				PolyHed.Add(currTet.ABD);
				PolyHed.Add(currTet.BCD);
				PolyHed.Add(currTet.ACD);

				Tetras.RemoveAt(currInd);
			}
		}

		TArray<FSpanTri> BadFaces;
		for (auto currA = PolyHed.CreateIterator(); currA; ++currA)
		{
			for (auto currB = PolyHed.CreateIterator(); currB; ++currB)
			{
				if (currA != currB && *currA == *currB)
				{
					BadFaces.Add(*currA);
					BadFaces.Add(*currB);
				}
			}
		}

		for (auto &currBad : BadFaces)
			PolyHed.Remove(currBad);

		for (auto& currRemaining : PolyHed)
			Tetras.Add(FSpanTetra(currRemaining.A, currRemaining.B, currRemaining.C, currPoint));

	}

	Tetras.RemoveAll([&SuperTet](FSpanTetra a) { return a.HasVert(SuperTet.A) || a.HasVert(SuperTet.B) || a.HasVert(SuperTet.C) || a.HasVert(SuperTet.D); });

	for (auto &currTet : Tetras)
	{
		Edges.AddUnique(currTet.ABC.AB);
		Edges.AddUnique(currTet.ABD.AB);
		Edges.AddUnique(currTet.BCD.AB);
		Edges.AddUnique(currTet.ACD.AB);
	
		Edges.AddUnique(currTet.ABC.BC);
		Edges.AddUnique(currTet.ABD.BC);
		Edges.AddUnique(currTet.BCD.BC);
		Edges.AddUnique(currTet.ACD.BC);
	
		Edges.AddUnique(currTet.ABC.CA);
		Edges.AddUnique(currTet.ABD.CA);
		Edges.AddUnique(currTet.BCD.CA);
		Edges.AddUnique(currTet.ACD.CA);
	}
}

void UWorldGenFuncLib::Delaunay2D(UObject* WorldContext, const TArray<FVector>& Points, TArray<FSpanTri>& Tris, TArray<FSpanEdge>& Edges)
{
	FVector Min = Points[0];
	FVector Max = Min;

	for (auto &currPt : Points)
	{
		if (currPt.X < Min.X) Min.X = currPt.X;
		else if (currPt.X > Max.X) Max.X = currPt.X;

		if (currPt.Y < Min.Y) Min.Y = currPt.Y;
		else if (currPt.Y > Max.Y) Max.Y = currPt.Y;
	}

	// lazily making a tet that encapsulates everything
	// by circumscribing it around the containing sphere

	FVector FitCenter = ((Max - Min) / 2) + Min;
	float FitRadius = (Max - Min).Size() / 2;

	float TriAng = UKismetMathLibrary::DegAcos(-1.0f / 3.0f);
	FRotator TriRotator = UKismetMathLibrary::RotatorFromAxisAndAngle(FVector(0, 0, 1), TriAng);

	FVector SuperTriVertA(FitRadius * 10, 0, 0);
	FVector SuperTriVertB = TriRotator.RotateVector(SuperTriVertA);
	FVector SuperTriVertC = TriRotator.RotateVector(SuperTriVertB);

	FSpanTri SuperTri(SuperTriVertA, SuperTriVertB, SuperTriVertC);

	Tris.Add(SuperTri);

	for (auto &currPoint : Points)
	{
		TArray<FSpanEdge> Poly;

		for (int currInd = Tris.Num() - 1; currInd >= 0; --currInd)
		{
			FSpanTri& currTri = Tris[currInd];

			if (currTri.CircumContains(currPoint))
			{
				Poly.Add(currTri.AB);
				Poly.Add(currTri.BC);
				Poly.Add(currTri.CA);

				Tris.RemoveAt(currInd);
			}
		}

		TArray<FSpanEdge> BadEdges;
		for (auto currA = Poly.CreateIterator(); currA; ++currA)
		{
			for (auto currB = Poly.CreateIterator(); currB; ++currB)
			{
				if (currA != currB && *currA == *currB)
				{
					BadEdges.Add(*currA);
					BadEdges.Add(*currB);
				}
			}
		}

		for (auto &currBad : BadEdges)
			Poly.Remove(currBad);

		for (auto& currRemaining : Poly)
			Tris.Add(FSpanTri(currRemaining.A, currRemaining.B, currPoint));

	}

	Tris.RemoveAll([&SuperTri](FSpanTri a) { return a.HasVert(SuperTri.A) || a.HasVert(SuperTri.B) || a.HasVert(SuperTri.C); });

	for (auto& currTri : Tris)
	{
		Edges.Add(currTri.AB);
		Edges.Add(currTri.BC);
		Edges.Add(currTri.CA);
	}
}

void UWorldGenFuncLib::KruskalMST(const TArray<FVector>& Points, UPARAM(Ref) TArray<FSpanEdge>& InOutEdges)
{
	// broken disjointed set implementation, woohoooooo

	struct DisjSet
	{
		TArray<int> Parents;
		TArray<int> Ranks;

		DisjSet(int Num)
		{
			Parents.AddDefaulted(Num);
			Ranks.AddZeroed(Num);

			for (int i = 0; i < Num; ++i)
				Parents[i] = i;
		}

		int FindParent(int Ind)
		{
			if (Parents[Ind] != Ind)
				Parents[Ind] = FindParent(Parents[Ind]);
			return Parents[Ind];
		}

		void Unite(int IndA, int IndB)
		{
			int ParentA = FindParent(IndA);
			int ParentB = FindParent(IndB);

			// assuming ParentA != ParentB

			if (Ranks[ParentA] < Ranks[ParentB])
				Parents[ParentA] = ParentB;
			else if(Ranks[ParentB] < Ranks[ParentA])
				Parents[ParentB] = ParentA;

			if (Ranks[ParentA] == Ranks[ParentB])
				++Ranks[ParentB];
		}
	};

	DisjSet DS(Points.Num());


	InOutEdges.Sort([](const FSpanEdge& a, const FSpanEdge& b) { return ((a.B - a.A)*FVector(1,1,100)).Size() > ((b.B - b.A)*FVector(1,1,100)).Size(); });

	for (int currInd = InOutEdges.Num() - 1; currInd >= 0; --currInd)
	{
		FSpanEdge& currEdge = InOutEdges[currInd];

		const int ParentA = DS.FindParent(Points.Find(currEdge.A));
		const int ParentB = DS.FindParent(Points.Find(currEdge.B));

		if (ParentA == ParentB) // if they're in the same tree
			InOutEdges.RemoveAt(currInd);
		else
			DS.Unite(ParentA, ParentB);
	}
}

void UWorldGenFuncLib::Delaunay2DToVoronoi2D(UObject* WorldContext, const TArray<FSpanTri>& InDelaunay, TArray<FSpanPoly>& OutPolygons, TArray<FSpanEdge>& OutEdges, float MaxDelaunayCircumradius)
{
	TArray<FVector> DelaunayCircumcenters;
	TArray<float> DelaunayCircumradii;

	FVector CurrCircumcenter;
	float CurrCircumradius;
	for (auto& currDelTri : InDelaunay)
	{
		currDelTri.GetCircumcircle(CurrCircumcenter, CurrCircumradius);
		DelaunayCircumcenters.Add(CurrCircumcenter);
		DelaunayCircumradii.Add(CurrCircumradius);
		OutPolygons.Add(FSpanPoly(CurrCircumcenter));
	}

	for (int AInd = 0; AInd < InDelaunay.Num(); ++AInd)
	{
		const FSpanTri& currA = InDelaunay[AInd];
		for (int BInd = AInd + 1; BInd < InDelaunay.Num(); ++BInd)
		{
			const FSpanTri& currB = InDelaunay[BInd];

			if (currB.HasEdge(currA.AB) || currB.HasEdge(currA.BC) || currB.HasEdge(currA.CA))
			{
				if (MaxDelaunayCircumradius <= 0 || (DelaunayCircumradii[AInd] < MaxDelaunayCircumradius && DelaunayCircumradii[BInd] < MaxDelaunayCircumradius))
				{
					FSpanEdge NewEdge = FSpanEdge(DelaunayCircumcenters[AInd], DelaunayCircumcenters[BInd]);
					OutEdges.AddUnique(NewEdge);
					OutPolygons[AInd].Sides.AddUnique(NewEdge);
					OutPolygons[BInd].Sides.AddUnique(NewEdge);
				}
			}
		}
	}
}

//void UWorldGenFuncLib::FloodFillAmong(UWorldCell* StartingCell, const TArray<UWorldCell*>& BoundsArray, TArray<class UWorldCell*>& OutFloodFill)
//{
//	OutFloodFill.Add(StartingCell);
//
//	for (int currInd = 0; currInd < OutFloodFill.Num(); ++currInd)
//	{
//		for (UWorldCell* currNeigh : OutFloodFill[currInd]->NeighborCells)
//		{
//			OutFloodFill.AddUnique(currNeigh);
//		}
//	}
//}

void UWorldGenFuncLib::FloodFillIfHasAttribute(UWorldCell* StartingCell, FName AttributeName, TArray<class UWorldCell*>& OutFloodFill)
{
	OutFloodFill.Add(StartingCell);

	for (int currInd = 0; currInd < OutFloodFill.Num(); ++currInd)
	{
		for (UWorldCell* currNeigh : OutFloodFill[currInd]->NeighborCells)
		{
			if (nullptr != currNeigh->CellData.EnvData.InternalMap.Find(AttributeName))
				OutFloodFill.AddUnique(currNeigh);
		}
	}
}


FName UBiomeMap::GetBiome(const FWorldGenEnvironmentData& InEnvironmentalData, TMap<FName, FVector2D>& OutBiomeRanges)
{
	return GetBiomeFromTable(StartingTable, InEnvironmentalData.InternalMap, OutBiomeRanges);
}

FName UBiomeMap::GetBiomeFromTable(const FBiomeTable& Table, const TMap<FName, float>& InEnvironmentalData, TMap<FName, FVector2D>& OutBiomeRanges)
{
	FBiomeBox FoundBox;

	if (Table.GetOverlappingBox(InEnvironmentalData, FoundBox))
	{
		for (int currAxInd = 0; currAxInd < Table.TableAxes.Num(); ++currAxInd)
		{
			OutBiomeRanges.Add(Table.TableAxes[currAxInd],
				currAxInd >= FoundBox.AxisRanges.Num() ? FVector2D::ZeroVector : FoundBox.AxisRanges[currAxInd]);
		}

		if (!FoundBox.BiomeName.IsNone())
			return FoundBox.BiomeName;
		else if (!FoundBox.TableName.IsNone())
		{
			FBiomeTable* FoundSubtable = BiomeSubtables.Find(FoundBox.TableName);

			if (nullptr != FoundSubtable)
				return GetBiomeFromTable(*FoundSubtable, InEnvironmentalData, OutBiomeRanges);
			else
				UStaticFuncLib::Print("UBiomeMap::GetBiomeFromTable: Couldn't find a subtable named \'" + FoundBox.TableName.ToString() + "\'!");
		}
	}
	else
	{
		FString Err = "UBiomeMap::GetBiomeFromTable: No biome boxes were overlapped with the current data!\n"
					  "Biome Table Axes:\n\t";

		for (auto &curr : Table.TableAxes)
			Err += "\'" + curr.ToString() + "\' ";

		Err += "\nData:\n";

		for (auto &curr : InEnvironmentalData)
			Err += "\t\'" + curr.Key.ToString() + "\' : " + FString::SanitizeFloat(curr.Value) + "\n";

		UStaticFuncLib::Print(Err, true);
	}

	return FName();
}


void UWorldShapeFunction::DetermineShape(UPARAM(Ref) TArray<UWorldCell*>& InOutWorldCells)
{
	/*
		Three main "base types" are used for tiles in the beginning:
			-	Ground			( 2)
			-	Water			( 1)
			-	Hanging Water	(-1)
			-	Hole			(-2)

		Shape functions determine cells are ground or holes.
		afterward the outer function (AWorldGenerator::ConstructContinentBase)
		determines which hole cells will stay as holes and which ones
		will become water.
	*/
	for (auto *curr : InOutWorldCells)
		curr->CellData.EnvData.InternalMap.Add((CellIsGround(curr) ? "IsGround" : "IsHole"), 1.0f);

}


AWorldGenerator::AWorldGenerator(const FObjectInitializer& _init) : Super(_init)
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	//PrimaryActorTick.bCanEverTick = true;

	bReplicates = true;
	//bReplicateMovement = true;

	RootComponent = _init.CreateDefaultSubobject<USceneComponent>(this, TEXT("RootComponent"));
}

void AWorldGenerator::BeginPlay()
{
	Super::BeginPlay();
}

void AWorldGenerator::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AWorldGenerator::Build(FName WorldTypeToBuild, int InitialRandSeed)
{
	/*
		currently single-threaded implementation, will switch to worker thread
		impl once the system actually works.
	*/

	FWorldGenerationSettings* ChosenWorldType = WorldTypes.Find(WorldTypeToBuild);

	if (nullptr != ChosenWorldType)
	{
		WorldMap = NewObject<UWorldMap>();

		Generate(*ChosenWorldType, InitialRandSeed);

		Rasterize(*ChosenWorldType);
	}
	else
		UStaticFuncLib::Print("AWorldGenerator::Build: WorldType \'" +
							  WorldTypeToBuild.ToString() +
							  "\' Wasn't in the WorldTypes map! Building was not completed successfully.", true);
}

void AWorldGenerator::Generate(const FWorldGenerationSettings& WorldSettings, int InitialRandSeed)
{
	UWorldGenFuncLib::InitializeWorldRandom(InitialRandSeed);

	struct IslandMapCollection
	{
		TArray<FColor> BiomeMap;
		TArray<FColor> RegionMap;
		TMap<FName, TArray<FColor>> AttributeMaps;
	};
	
	TArray<FBox> PrevIslandBoxes;
	TArray<IslandMapCollection> IslandMaps;
	int currIslInd = 0;

	for (auto &currIsl : WorldSettings.Islands)
	{
		/*
			Make a bounding box to create the island with.
		*/
		FBox CreationBoundingBox(FVector::ZeroVector, FVector(currIsl.BoxDimensions));

		/*
			Construct the island
		*/
		TArray<UWorldCell*> NonHoleCells;
		ConstructContinentBase(currIsl, CreationBoundingBox, NonHoleCells);

		/*
			Do all the attribute-populating passes
			on this island.

			NOTE: At this point in time, these
				parts of WorldMap are VALID:
					-WorldCells
					-IslandEdges
					-HangingLakeCoasts
					-NormalLakeCoasts
					-InnerIslandEdges
				
				these parts are INVALID:
					WorldRegions
					WorldContinents
		*/
		for (auto &currPass : WorldSettings.Passes)
			currPass.GetDefaultObject()->Apply(WorldMap);

		/*
			Assign Biomes
		*/
		if (nullptr != WorldSettings.BiomeMap)
		{
			for (auto *currCell : NonHoleCells)
			{
				FWorldGenVoronoiCellData& CellData = currCell->CellData;
				CellData.Biome = WorldSettings.BiomeMap.GetDefaultObject()->GetBiome(CellData.EnvData, CellData.BiomeRanges);
			}
		}
		else
			UStaticFuncLib::Print("AWorldGenerator::Generate: Island " +
								  FString::FromInt(currIslInd) +
								  " has no Biome Map! Please supply one.", true);


		/*
			Determine Regions and Add to WorldMap
		*/
		TArray<UWorldCell*> RemainingWorldCells = NonHoleCells;
		TArray<UWorldRegion*> CurrCreatedRegions;
		while (RemainingWorldCells.Num() > 0) // While there's still cells left unregioned
		{
			int RandCellInd = UWorldGenFuncLib::GetWorldRandom().RandRange(0, RemainingWorldCells.Num() - 1);
			UWorldCell* ChosenStartCell = RemainingWorldCells[RandCellInd];

			auto currPred = [&ChosenStartCell](UWorldCell* a) { return a->CellData.Biome == ChosenStartCell->CellData.Biome; };
			TArray<UWorldCell*> CurrRegionCells = FloodFillCells(ChosenStartCell, currPred);

			UWorldRegion* CurrWorldRegion = WorldMap->WorldRegions[WorldMap->WorldRegions.Add(NewObject<UWorldRegion>())];

			CurrWorldRegion->Cells = MoveTemp(CurrRegionCells);

			CurrCreatedRegions.Add(CurrWorldRegion);
		}


		/*
			Populate Continent and Add to WorldMap
		*/
		UWorldContinent* NewContinent = WorldMap->WorldContinents[WorldMap->WorldContinents.Add(NewObject<UWorldContinent>())];
		NewContinent->Regions = MoveTemp(CurrCreatedRegions);


		/*
			Then Populate and Add IslandMapCollection to IslandMaps
		*/
		IslandMapCollection NewCollection;
		int BoxDims = currIsl.BoxDimensions.X * currIsl.BoxDimensions.Y;
		NewCollection.BiomeMap.SetNum(BoxDims);
		NewCollection.RegionMap.SetNum(BoxDims);

		for (UWorldRegion *currRegion : NewContinent->Regions)
		{
			for (UWorldCell *currCell : currRegion->Cells)
			{
				FVector CurrCellMin, CurrCellMax;

				currCell->CellData.GetBounds(CurrCellMin, CurrCellMax);

				CurrCellMin = CreationBoundingBox.GetClosestPointTo(CurrCellMin);
				CurrCellMax = CreationBoundingBox.GetClosestPointTo(CurrCellMax);

				for (int currX = CurrCellMin.X; currX <= CurrCellMax.X; ++currX)
				{
					for (int currY = CurrCellMin.Y; currY <= CurrCellMax.Y; ++currY)
					{
						if (currCell->CellData.ContainsPoint(FVector(currX, currY, 0)))
						{
							int ind = currX + currY * currIsl.BoxDimensions.X;

							/*
								TODO : GET BIOME COLOR OF CELL

								Immediate thoughts: BiomeMaps also contain a
								subclass ptr to their biome "map", which
								is full of biome data structures that
								keep track of miscellaneous biome-specific
								information, among which of course is their
								color on the map.
							*/
							NewCollection.BiomeMap[ind] = FColor::White;

							/*
								TODO : DETERMINE WAY TO DO REGION COLORATION

								Immediate thoughts: Index the regions in order
								of occurrence in the array and then of course
								later look up said indices to apply names to
								them, which of course will then get saved with
								the rest of the stuff in the files.
							*/
							NewCollection.RegionMap[ind] = FColor::White;

							for (auto &currAttr : currCell->CellData.EnvData.InternalMap)
							{
								TArray<FColor>* CurrAttrMap = NewCollection.AttributeMaps.Find(currAttr.Key);

								if (nullptr == CurrAttrMap)
								{
									CurrAttrMap = &NewCollection.AttributeMaps.Add(currAttr.Key);
									CurrAttrMap->SetNum(BoxDims);
								}

								// All attrs are in the (-1)->1 range so it's just a matter of mapping
								float AttrNorm = (currAttr.Value + 1) / 2;
								(*CurrAttrMap)[ind] = FLinearColor(AttrNorm, AttrNorm, AttrNorm, AttrNorm).ToFColor(true);
							}
						}
					}
				}
			}
		}

		++currIslInd;
	}



	/*
		TODO : 
			Remember, this is in the middle of a change from the
			previous system to the new system, which:

			For Each Island:
				1) Generates it at origin
				2) Parses this into several *images* of data at resolutions based on tile size and island bounding box size

			Need to make passes for:
				-Elevation
				-Moisture
				-(Possibly "Magic" attr for testing and since Hanging Lakes should have extra differentiation from Normal Lakes?)

			Missing things include:
				-River data (basically a lot of edges going downward from elevations)
				-Road data (either follow Amit's method or make my own, I guess. Splines will definitely be useful, though.)


			It might also benefit me to change save data into:
				-Island World Positions
				-Their data in the form of FColor images
				-Player deltas (maybe just update their images whenever players change something?)

			if I save their data in an image format I could possibly also get away with doing
			world updates via image manipulation algorithms, which iirc are usually pretty well-
			optimized by this point in time.

			I also might chunk the islands "traditionally" instead of on a per-cell or per-several-cells
			basis due to the abrupt shift back into a tiled mindset.

			Also AWorldGenerator::Rasterize miiiight be obsolete after this so I might have to
			get rid of it.
	*/

}

void AWorldGenerator::Rasterize(const FWorldGenerationSettings& WorldSettings)
{
	// TODO : IMPL
}

void AWorldGenerator::ConstructContinentBase(const FWorldGenIslandSettings& CurrIslSettings, const FBox& IslBoundingBox, TArray<UWorldCell*>& OutNonHoleCells)
{
	TArray<FVector> Points;
	float RandX, RandY;
	for (int currPoint = 0; currPoint < CurrIslSettings.NumSamplingPoints; ++currPoint)
	{
		RandX = UWorldGenFuncLib::GetWorldRandom().FRandRange(IslBoundingBox.Min.X, IslBoundingBox.Max.X);
		RandY = UWorldGenFuncLib::GetWorldRandom().FRandRange(IslBoundingBox.Min.Y, IslBoundingBox.Max.Y);
		Points.Add(FVector(RandX, RandY, 0));
	}

	// Create this island's voronoi diagram
	TArray<FSpanTri> Tris;
	TArray<FSpanEdge> Edges;
	UWorldGenFuncLib::Delaunay2D(this, Points, Tris, Edges);

	TArray<FSpanPoly> VoronoiPolys;
	Edges.Empty();
	UWorldGenFuncLib::Delaunay2DToVoronoi2D(this, Tris, VoronoiPolys, Edges);


	TArray<UWorldCell*>& WorldMapCells = WorldMap->WorldCells;

	/*
		NOTE: used to clip polygons back into bounds in previous methods,
		this time it's unnecessary and can be skipped.
	*/
	for (auto &currPoly : VoronoiPolys)
	{
		FWorldGenVoronoiCellData ProcessedPoly;

		for (auto &currEdge : currPoly.Sides)
		{
			ProcessedPoly.ShapeData_Edges.AddUnique(currEdge);

			ProcessedPoly.ShapeData_Verts.AddUnique(currEdge.A);
			ProcessedPoly.ShapeData_Verts.AddUnique(currEdge.B);
		}

		ProcessedPoly.ShapeData_Center = currPoly.Site;

		WorldMapCells[WorldMapCells.Add(NewObject<UWorldCell>())]->CellData = MoveTemp(ProcessedPoly);
	}

	// Set up neighbor cell connections. (TODO MIGHTY NEED TO BE OPTIMIZED)
	for (auto *currCellA : WorldMapCells)
	{
		for (auto *currCellB : WorldMapCells)
		{
			for (auto &currEdgeA : currCellA->CellData.ShapeData_Edges)
			{
				if (currCellB->CellData.ShapeData_Edges.Contains(currEdgeA))
				{
					// They're neighbors, we're done here.

					currCellA->NeighborCells.AddUnique(currCellB);
					currCellB->NeighborCells.AddUnique(currCellA);

					break;
				}
			}
		}
	}

	// Pass to the islandfunction to determine where the ground is
	UWorldShapeFunction* ShapeFunc = NewObject<UWorldShapeFunction>(this, CurrIslSettings.ShapeFunction);
	ShapeFunc->Initialize();
	ShapeFunc->DetermineShape(WorldMapCells);

	// Separating hole and non-hole areas for later ops
	TArray<UWorldCell*> HoleCells;
	for (auto *currCell : WorldMapCells)
	{
		if (currCell->CellData.EnvData.InternalMap.FindRef("BaseType") > -2.0f)
			OutNonHoleCells.Add(currCell);
		else
			HoleCells.Add(currCell);
	}
	
	
	auto HolePredicate = [](UWorldCell* a) { return nullptr != a->CellData.EnvData.InternalMap.Find("IsHole"); };

	// Get rid of the elephant-in-the-room surrounding "hole ocean"
	TArray<UWorldCell*> OuterBoundsHoleCells = FloodFillCells(HoleCells[0], HolePredicate);
	for (auto *curr : OuterBoundsHoleCells)
	{
		HoleCells.Remove(curr);

		// Grab out any island edges while we're at it

		for (auto *currNeigh : curr->NeighborCells)
		{
			if (!HolePredicate(currNeigh))
				WorldMap->IslandEdges.AddUnique(currNeigh);
		}
	}

	while (HoleCells.Num() > 0)
	{
		TArray<UWorldCell*> NextHoleRegion = FloodFillCells(HoleCells[0], HolePredicate);

		if (UWorldGenFuncLib::GetWorldRandom().FRand() < CurrIslSettings.LakeChance)
		{
			bool IsHangingLake = UWorldGenFuncLib::GetWorldRandom().FRand() < CurrIslSettings.HangingLakeChance;

			FName NewAttr((IsHangingLake ? "IsHangingLake" : "IsLake"));

			// This hole becomes a lake.
			for (auto *curr : NextHoleRegion)
			{
				curr->CellData.EnvData.InternalMap.Remove("IsHole");
				curr->CellData.EnvData.InternalMap.Add(NewAttr, 1.0f);

				OutNonHoleCells.Add(curr);
			}


			// Grab its coasts while we can

			TFunction<bool(UWorldCell*)> CoastPred;
			if (IsHangingLake)
				CoastPred = [](UWorldCell* a) {return nullptr != a->CellData.EnvData.InternalMap.Find("IsHangingLake"); };
			else
				CoastPred = [](UWorldCell* a) {return nullptr != a->CellData.EnvData.InternalMap.Find("IsLake"); };

			TArray<TArray<UWorldCell*>>& RelevantCoastOuterArray = (IsHangingLake ? WorldMap->HangingLakeCoasts : WorldMap->NormalLakeCoasts);

			TArray<UWorldCell*>& RelevantCoastInternalArray = RelevantCoastOuterArray[RelevantCoastOuterArray.AddDefaulted()];

			for (auto *curr : NextHoleRegion)
			{
				for (auto *currNeigh : curr->NeighborCells)
				{
					if (!CoastPred(currNeigh))
						RelevantCoastInternalArray.AddUnique(currNeigh);
				}
			}
		}
		else
		{
			// It's an inner hole so we should probably grab its edges while we can

			for (auto *curr : NextHoleRegion)
			{
				for (auto *currNeigh : curr->NeighborCells)
				{
					if (!HolePredicate(currNeigh))
						WorldMap->InnerIslandEdges.AddUnique(currNeigh);
				}
			}
		}

		for (auto *curr : NextHoleRegion)
			HoleCells.Remove(curr);
	}
}

TArray<UWorldCell*> AWorldGenerator::FloodFillCells(UWorldCell* StartingCell, const TFunction<bool(UWorldCell*)>& ConnectedPredicate)
{
	TArray<UWorldCell*> Floodfill;

	TArray<UWorldCell*> queue;
	queue.Add(StartingCell);
	while (queue.Num() > 0)
	{
		for (auto &currNeighbor : queue[0]->NeighborCells)
		{
			if (ConnectedPredicate(currNeighbor))
				queue.Add(currNeighbor);
		}
	}

	return Floodfill;
}
