// Fill out your copyright notice in the Description page of Project Settings.

#include "MPHorso.h"
#include "ProceduralMapTypes.h"

#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"

#include "StaticFuncLib.h"

#include "ProceduralMeshComponent.h"


FRandomStream UWorldGenFuncLib::RandStream;

void UWorldGenFuncLib::SetCellAttribute(UPARAM(Ref) TArray<FWorldCell>& Cells, int CellIndex, FName AttributeName, float AttributeValue)
{
	//Cells[CellIndex].EnvironmentAttributes.Add(AttributeName, AttributeValue);
}

void UWorldGenFuncLib::FloodFillCells(int StartCellInd,
									  const TArray<FWorldCell>& Cells,
									  const TFunction<bool(const FWorldCell&)>& Pred,
									  TArray<int>& OutFloodFillInds,
									  bool IncludeStartCellInd)
{
	//int currInd = 0;
	//if (IncludeStartCellInd)
	//{
	//	OutFloodFillInds.AddUnique(StartCellInd);
	//	++currInd;
	//}

	//for (auto &currNeigh : Cells[StartCellInd].Neighbors)
	//{
	//	if (Pred(Cells[currNeigh]))
	//		OutFloodFillInds.AddUnique(currNeigh);
	//}

	//for (; currInd < OutFloodFillInds.Num(); ++currInd)
	//{
	//	for (auto &currNeigh : Cells[OutFloodFillInds[currInd]].Neighbors)
	//	{
	//		if (Pred(Cells[currNeigh]))
	//			OutFloodFillInds.AddUnique(currNeigh);
	//	}
	//}
}


void UWorldShapeFunction::DetermineShape(FIntVector Size, UPARAM(Ref) TMap<int, FWorldCell>& InOutWorldCells)
{
	for (auto &currCell : InOutWorldCells)
	{
		FVector Site = FVector(((currCell.Value.ShapeData->Coordinate / FVector2D(Size.X, Size.Y)) - 0.5f) * 2, 0);

		// As long as it works in the algo and isn't a corner or edge
		if (CellIsGround(Site) && !(currCell.Value.ShapeData->bIsCorner || currCell.Value.ShapeData->bIsEdge))
			currCell.Value.Biome = EBiomeType::BIOME_GROUND;
	}
}


AWorldContinent::AWorldContinent(const FObjectInitializer& _init) : Super(_init)
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	//PrimaryActorTick.bCanEverTick = true;

	bReplicates = true;
	//bReplicateMovement = true;

	RootComponent = _init.CreateDefaultSubobject<USceneComponent>(this, TEXT("RootComponent"));
}

void AWorldContinent::BeginPlay()
{
	Super::BeginPlay();
}

void AWorldContinent::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AWorldContinent::Build(FIntVector MapSize, int NumSamplingPoints, bool IsDead)
{
	Dead = IsDead;
	ContinentSize = MapSize;

	ConstructVoronoi(NumSamplingPoints);

	Annotate();

	///*
	//	TODO : 
	//		Need to make passes for:
	//			-Elevation
	//			-Moisture
	//			-(Possibly "Magic" attr for testing and since Hanging Lakes should have extra differentiation from Normal Lakes?)

	//		Missing things include:
	//			-River data (basically a lot of edges going downward from elevations)
	//			-Road data (either follow Amit's method or make my own, I guess. Splines will definitely be useful, though.)


	//		It might also benefit me to change save data into:
	//			-Island World Positions
	//			-Their data in the form of FColor images
	//			-Player deltas (maybe just update their images whenever players change something?)

	//		if I save their data in an image format I could possibly also get away with doing
	//		world updates via image manipulation algorithms, which iirc are usually pretty well-
	//		optimized by this point in time.

	//		I also might chunk the islands "traditionally" instead of on a per-cell or per-several-cells
	//		basis due to the abrupt shift back into a tiled mindset.

	//		Also AWorldGenerator::Rasterize miiiight be obsolete after this so I might have to
	//		get rid of it.
	//*/


	//{
	//	TArray<FColor> BiomeCols;
	//	for (auto &currPx : Biomes)
	//	{
	//		int ind = currPx - 1;
	//		if (ind >= 0)
	//			BiomeCols.Add(BiomeMap.GetDefaultObject()->BiomeList.GetDefaultObject()->Biomes[ind].BiomeColor);
	//		else
	//			BiomeCols.Add(FColor::Black);
	//	}
	//	UStaticFuncLib::ExportToBitmap(BiomeCols, "TestBiomes", MapSize.X, MapSize.Y);

	//	TArray<FColor> RegionCols;
	//	for (auto &currPx : Regions)
	//	{
	//		float pxrat = float(currPx) / Regions.Num();
	//		RegionCols.Add(FLinearColor(pxrat, pxrat, pxrat).ToFColor(true));
	//	}
	//	UStaticFuncLib::ExportToBitmap(RegionCols, "TestRegions", MapSize.X, MapSize.Y);

	//	for (auto &curr : Attributes)
	//	{
	//		TArray<FColor> currAttrCols;

	//		for (auto &currPx : curr.Value)
	//		{
	//			//UStaticFuncLib::Print(FString::SanitizeFloat(currPx), true);
	//			float InRange = FMath::GetMappedRangeValueClamped({ -1,1 }, { 0,1 }, currPx);
	//			currAttrCols.Add(FLinearColor(InRange, InRange, InRange).ToFColor(false));
	//		}

	//		UStaticFuncLib::ExportToBitmap(currAttrCols, "Test" + curr.Key.ToString(), MapSize.X, MapSize.Y);
	//	}
	//}

	//UStaticFuncLib::Print("Done", true);
}

void AWorldContinent::ConstructVoronoi(int SamplingPoints)
{
	const FRandomStream& WorldRand = UWorldGenFuncLib::GetWorldRandom();

	Voronoi = MakeShareable(new FVoronoiDiagram(FIntRect(0, 0, ContinentSize.X, ContinentSize.Y)));

	TArray<FIntPoint> Points;

	while (--SamplingPoints >= 0)
		Points.Add(FIntPoint(WorldRand.RandRange(1, ContinentSize.X - 1), WorldRand.RandRange(1, ContinentSize.Y - 1)));

	Voronoi->AddPoints(Points);
	Voronoi->GenerateSites(2);

	TMap<FVector2D, TArray<FVector2D>> VertsToNeighborVerts;
	TMap<FVector2D, TArray<FWorldCell*>> VertsToNeighborCells;

	for (auto &currSite : Voronoi->GeneratedSites)
	{
		FWorldCell NewCell;
		NewCell.ShapeData = &currSite.Value;

		FWorldCell& NewFinalized = WorldCells.Add(currSite.Key, MoveTemp(NewCell));

		if (currSite.Value.bIsCorner)
			CornerInds.Add(currSite.Value.Index);
		else if (currSite.Value.bIsEdge)
			EdgeInds.Add(currSite.Value.Index);

		for (auto &currEdge : NewFinalized.ShapeData->Edges)
		{
			VertsToNeighborVerts.FindOrAdd(currEdge.LeftEndPoint).AddUnique(currEdge.RightEndPoint);
			VertsToNeighborVerts.FindOrAdd(currEdge.RightEndPoint).AddUnique(currEdge.LeftEndPoint);

			VertsToNeighborCells.FindOrAdd(currEdge.LeftEndPoint).AddUnique(&NewFinalized);
			VertsToNeighborCells.FindOrAdd(currEdge.RightEndPoint).AddUnique(&NewFinalized);
		}
	}

	for (auto &currVert : VertsToNeighborVerts)
	{
		FGraphPoint NextPoint;

		NextPoint.Point = currVert.Key;
		NextPoint.NeighborVerts = currVert.Value;
		NextPoint.NeighborCells = VertsToNeighborCells[currVert.Key];

		for (auto *currCell : NextPoint.NeighborCells)
		{
			if (CornerInds.Contains(currCell->ShapeData->Index) || EdgeInds.Contains(currCell->ShapeData->Index))
			{
				NextPoint.IsBorder = true;
				break;
			}
		}

		PointGraph.Add(MoveTemp(NextPoint));
	}

}

void AWorldContinent::Annotate()
{
	// Determine Ground/Lakes
	ShapeIsland();

	// Pre-River Attribute Pass/es
	AssignElevations();

	// River Pass/es
	GenerateRivers();
	
	// Post-River Attribute Pass/es
	AssignMoisture();

	// Biome Pass
	DetermineBiomes();

	// Continent Naming Pass
	NameContinent();

	// Region Pass
	DetermineRegions();

	// Region Naming Pass/es
	NameRegions();
	
	// Structure Pass/es
	/*
		Note: tips on placing cities ( https://www.youtube.com/watch?v=3PWWtqfwacQ ) ?
	*/
	PlaceStructures();

	// Structure Naming Pass/es
	NameStructures();

	// Road Pass/es (Make Roads, Make Signs At Junctions)
	GenerateRoads();


	/*
		TODO :
			-Caves how and when?
	*/
}

void AWorldContinent::ShapeIsland()
{
	UWorldShapeFunction* SpawnedShapeFunc = NewObject<UWorldShapeFunction>(this, ShapeFunc);
	SpawnedShapeFunc->Initialize();
	SpawnedShapeFunc->DetermineShape(ContinentSize, WorldCells);

	TArray<int> NotHoles;
	TArray<int> Holes;

	for (auto &currCell : WorldCells)
	{
		if (EBiomeType::BIOME_HOLE == currCell.Value.Biome)
			Holes.Add(currCell.Key);
		else
			NotHoles.Add(currCell.Key);
	}

	if (Holes.Num() > 0)
	{
		TArray<int> OuterBoundsHoleCells;

		auto HolePred = [](const FWorldCell& a) { return EBiomeType::BIOME_HOLE == a.Biome; };

		{
			// Getting rid of the outer hole

			TArray<int> OuterHole;
			OuterHole.Append(CornerInds);
			OuterHole.Append(EdgeInds);

			for (int currInd = 0; currInd < OuterHole.Num(); ++currInd)
			{
				Holes.Remove(OuterHole[currInd]);
				for (auto &currNeigh : WorldCells[currInd].ShapeData->NeighborSites)
				{
					if (HolePred(WorldCells[currNeigh]))
						OuterHole.AddUnique(currNeigh);
				}
			}
		}

		while (Holes.Num() > 0)
		{
			TArray<int> NextHoleRegion;
			NextHoleRegion.Add(Holes[0]);
			for (int currInd = 0; currInd < NextHoleRegion.Num(); ++currInd)
			{
				for (auto &currNeigh : WorldCells[currInd].ShapeData->NeighborSites)
				{
					if (HolePred(WorldCells[currNeigh]))
						NextHoleRegion.Add(currNeigh);
				}
			}

			if (UWorldGenFuncLib::GetWorldRandom().FRand() < LakeChance)
			{
				bool IsHangingLake = UWorldGenFuncLib::GetWorldRandom().FRand() < HangingLakeChance;

				EBiomeType NewBiome = (IsHangingLake ? EBiomeType::BIOME_HANGINGWATER : EBiomeType::BIOME_WATER);

				// This hole becomes a lake.
				for (auto &currInd : NextHoleRegion)
				{
					WorldCells[currInd].Biome = NewBiome;

					NotHoles.Add(currInd);
				}
			}
		}

		if (NotHoles.Num() < 1)
			UStaticFuncLib::Print("AWorldContinent::ShapeIsland: No non-hole cells found! Did something go awry with the shape function?", true);
		else
			NonHoleIndices = MoveTemp(NotHoles);
	}
	else
		UStaticFuncLib::Print("AWorldContinent::ShapeIsland: No holes found! Did something go awry with the shape function?", true);
}

void AWorldContinent::AssignElevations()
{
	// marks all of the edges of the island
	for (auto &currPoint : PointGraph)
	{
		bool HasGround = false;
		bool HasHole = false;
		for (auto *currCell : currPoint.NeighborCells)
		{
			currPoint.Biome = FMath::Max(currPoint.Biome, currCell->Biome);

			if (currCell->Biome == EBiomeType::BIOME_GROUND)
				HasGround = true;
			if (currCell->Biome == EBiomeType::BIOME_HOLE)
				HasHole = true;

			if (HasGround && HasHole)
			{
				currPoint.IsIslandEdge = true;
				break;
			}
		}
	}


	// follows assignCornerElevations() and redistributeElevations()
	// a la https://github.com/amitp/mapgen2/blob/master/Map.as

	TArray</*int*/FGraphPoint*> queue;
	const float ElevEpsilon = 0.01f;

	TArray<FGraphPoint*> LandPoints;

	for (auto &curr : PointGraph)
	{
		if (curr.IsBorder)
		{
			curr.EnvData.Elevation = 0;
			queue.Add(&curr);
		}
		else
			curr.EnvData.Elevation = INFINITY;

		if (curr.Biome == EBiomeType::BIOME_GROUND)
			LandPoints.Add(&curr);
	}

	while (queue.Num())
	{
		FGraphPoint* curr = queue[0];
		queue.RemoveAt(0);

		for (auto &currNeigh : curr->NeighborVerts)
		{
			FGraphPoint dummy;
			dummy.Point = currNeigh;
			FGraphPoint* neighborPoint = PointGraph.Find(dummy);

			float NewElev = ElevEpsilon + curr->EnvData.Elevation;
			if (curr->Biome == EBiomeType::BIOME_GROUND)
				NewElev += 1 + (UWorldGenFuncLib::GetWorldRandom().FRand()*ElevationRandScale);

			if (NewElev < neighborPoint->EnvData.Elevation)
			{
				neighborPoint->EnvData.Elevation = NewElev;
				queue.Add(neighborPoint);
			}
		}
	}


	// redistribution

	LandPoints.Sort([](FGraphPoint& a, FGraphPoint& b) { return a.EnvData.Elevation < b.EnvData.Elevation; });

	// saved result of sqrt(ElevationScaleFactor) to cut ops from 2xSqrt per loop to 1xSqrt per loop
	const float ElevScaleFactorRoot = FMath::Sqrt(ElevationScaleFactor);

	float adjustedElev, currHeightRatio;
	int currInd = 0;
	for (auto *curr : LandPoints)
	{
		currHeightRatio = float(currInd) / (LandPoints.Num() - 1);

		adjustedElev = FMath::Clamp(ElevScaleFactorRoot - (ElevScaleFactorRoot * FMath::Sqrt(1 - currHeightRatio)), 0.0f, 1.0f);
		curr->EnvData.Elevation = adjustedElev;

		++currInd;
	}


	// Setting polygon elevations

	TArray<FWorldCell*> RelevantCells;
	for (auto *currVert : LandPoints)
	{
		for (auto *currCell : currVert->NeighborCells)
		{
			if (currCell->Biome != EBiomeType::BIOME_HOLE)
			{
				currCell->EnvironmentData.Elevation += currVert->EnvData.Elevation;
				RelevantCells.Add(currCell);
			}
		}
	}

	for (auto *currCell : RelevantCells)
		currCell->EnvironmentData.Elevation /= currCell->ShapeData->Vertices.Num();
}

void AWorldContinent::GenerateRivers()
{
	// TODO : IMPL

	// constructing the downslopes from preexisting elevation data
	FGraphPoint* LowestNeighbor;
	for (auto &currVert : PointGraph)
	{
		if (currVert.Biome != EBiomeType::BIOME_HOLE)
		{
			LowestNeighbor = &currVert;

			for (auto &currNeigh : currVert.NeighborVerts)
			{
				FGraphPoint dummy;
				dummy.Point = currNeigh;
				FGraphPoint* neighborPoint = PointGraph.Find(dummy);

				if (neighborPoint->EnvData.Elevation < LowestNeighbor->EnvData.Elevation)
					LowestNeighbor = neighborPoint;
			}

			if (LowestNeighbor != &currVert)
				currVert.DownslopePoint = LowestNeighbor;
		}
	}


	// Generating rivers

	PointGraph.Sort([](const FGraphPoint& a, const FGraphPoint& b) {return a.EnvData.Elevation > b.EnvData.Elevation; });
	TArray<FGraphPoint*> SortedPoints;
	for (auto &curr : PointGraph)
		SortedPoints.Add(&curr);

	int NumRivers = int((ContinentSize.GetMax() / 2) * RiverCoefficient);
	for (int currRiver = 0; currRiver < NumRivers; ++currRiver)
	{
		FGraphPoint* currChoice = SortedPoints[UWorldGenFuncLib::GetWorldRandom().RandRange(0, SortedPoints.Num() - 1)];
		while(nullptr == currChoice->DownslopePoint)
			currChoice = SortedPoints[UWorldGenFuncLib::GetWorldRandom().RandRange(0, SortedPoints.Num() - 1)];

		while (currChoice->DownslopePoint != nullptr)
		{
			FVector4 NewEdge(currChoice->Point, currChoice->DownslopePoint->Point);
			Rivers.FindOrAdd(NewEdge) += 1;

			currChoice = currChoice->DownslopePoint;
		}
	}
}

void AWorldContinent::AssignMoisture()
{
	// TODO : IMPL
}

void AWorldContinent::DetermineBiomes()
{
	// TODO : IMPL
}

void AWorldContinent::NameContinent()
{
	// TODO : IMPL
}

void AWorldContinent::DetermineRegions()
{
	// TODO : IMPL
}

void AWorldContinent::NameRegions()
{
	// TODO : IMPL
}

void AWorldContinent::PlaceStructures()
{
	// TODO : IMPL
}

void AWorldContinent::NameStructures()
{
	// TODO : IMPL
}

void AWorldContinent::GenerateRoads()
{
	// TODO : IMPL
}

void AWorldContinent::Rasterize()
{
	// TODO : IMPL
	/*
		Also remember to add some noise during this phase
		as some last-minute stuff to make the map more interesting
	*/
}

void AWorldContinent::Voxelize()
{
	// TODO : IMPL
	/*
		also this would probably be the most appropriate point to add
		caves and stuff
	*/
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

	UWorldGenFuncLib::InitializeWorldRandom(InitialRandSeed);

	//FWorldType* ChosenWorldType = WorldTypes.Find(WorldTypeToBuild);

	//if (nullptr != ChosenWorldType)
	//{
	//	FActorSpawnParameters params;
	//	params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	//	AWorldContinent* CurrentSpawned;
	//	for (FWorldContinentPacket &currContinent : ChosenWorldType->ContinentsToSpawn)
	//	{
	//		CurrentSpawned = GetWorld()->SpawnActor<AWorldContinent>(currContinent.Continent,
	//																 FVector::ZeroVector,
	//																 FRotator::ZeroRotator,
	//																 params);

	//		CurrentSpawned->Build(currContinent.BoxDimensions, currContinent.NumSamplingPoints, currContinent.IsDead);
	//	}
	//}
	//else
	//	UStaticFuncLib::Print("AWorldGenerator::Build: WorldType \'" +
	//						  WorldTypeToBuild.ToString() +
	//						  "\' Wasn't in the WorldTypes map! Building was not completed successfully.", true);


	/*
		TODO :
			1) Generate the continents
			2) Once all continents have generated, sprinkle the NPCs + Guide
			3) Run history passes.
	*/
}
