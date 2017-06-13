// Fill out your copyright notice in the Description page of Project Settings.

#include "MPHorso.h"
#include "ProceduralMapTypes.h"

#include "ANLFacadeLib.h"

#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"

#include "StaticFuncLib.h"

#include "ProceduralMeshComponent.h"


FRandomStream UWorldGenFuncLib::RandStream;

FColor UWorldGenFuncLib::BiomeColors[(int)EBiomeType::MAX] =
{
	FColor(0, 0, 0, 0),			//BIOME_HOLE = 0

	FColor(153, 102, 51),		//BIOME_GROUND

	FColor(51, 102, 153),		//BIOME_WATER
	FColor(0, 153, 204),		//BIOME_HANGINGWATER

	FColor(39, 118, 165),		//BIOME_RIVERWATER


	FColor(47, 102, 102),		//BIOME_MARSH
	FColor(153, 255, 255),		//BIOME_ICE
	FColor(255, 255, 255),		//BIOME_SNOW
	FColor(187, 187, 170),		//BIOME_TUNDRA
	FColor(136, 136, 136),		//BIOME_BARE
	FColor(85, 85, 85),			//BIOME_SCORCHED
	FColor(153, 170, 119),		//BIOME_TAIGA
	FColor(136, 153, 119),		//BIOME_SHRUBLAND
	FColor(201, 210, 155),		//BIOME_TEMP_DESERT
	FColor(68, 136, 85),		//BIOME_TEMP_RAINFOREST
	FColor(103, 148, 89),		//BIOME_TEMP_DECID_FOREST
	FColor(136, 170, 85),		//BIOME_GRASSLAND
	FColor(51, 119, 85),		//BIOME_TROP_RAINFOREST
	FColor(85, 153, 68),		//BIOME_TROP_SEASONAL_FOREST
	FColor(210, 185, 139)		//BIOME_SUBTROP_DESERT


};


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

	{
		// outputting debug images

		TArray<FColor> CurrColorData;

		// elevations
		for (auto &currCell : WorldCells)
		{
			float Elev = currCell.Value.EnvironmentData.Elevation;
			currCell.Value.ShapeData->Color = FLinearColor(Elev, Elev, Elev).ToFColor(true);
		}
		FVoronoiDiagramHelper::GenerateColorArray(Voronoi, CurrColorData);
		UStaticFuncLib::ExportToBitmap(CurrColorData, "TestElevations", ContinentSize.X, ContinentSize.Y);

		// moistures
		for (auto &currCell : WorldCells)
		{
			float Mois = currCell.Value.EnvironmentData.Moisture;
			currCell.Value.ShapeData->Color = FLinearColor(Mois, Mois, Mois).ToFColor(true);
		}
		FVoronoiDiagramHelper::GenerateColorArray(Voronoi, CurrColorData);
		UStaticFuncLib::ExportToBitmap(CurrColorData, "TestMoistures", ContinentSize.X, ContinentSize.Y);

		// biomes
		for (auto &currCell : WorldCells)
		{
			currCell.Value.ShapeData->Color = UWorldGenFuncLib::GetBiomeColor(currCell.Value.Biome);
		}
		FVoronoiDiagramHelper::GenerateColorArray(Voronoi, CurrColorData);
		UStaticFuncLib::ExportToBitmap(CurrColorData, "TestBiomes", ContinentSize.X, ContinentSize.Y);

		UStaticFuncLib::Print("Done", true);
	}

	///*
	//	TODO : 
	//		Need to make passes for:
	//			-(Possibly "Magic" attr for testing and since Hanging Lakes should have extra differentiation from Normal Lakes?)
	//
	//		Missing things include:
	//			-Road data (either follow Amit's method or make my own, I guess. Splines will definitely be useful, though.)
	//*/

}

void AWorldContinent::ConstructVoronoi(int SamplingPoints)
{
	const FRandomStream& WorldRand = UWorldGenFuncLib::GetWorldRandom();

	Voronoi = MakeShareable(new FVoronoiDiagram(FIntRect(0, 0, ContinentSize.X, ContinentSize.Y)));

	TArray<FIntPoint> Points;

	while (--SamplingPoints >= 0)
		Points.AddUnique(FIntPoint(WorldRand.RandRange(1, ContinentSize.X - 1), WorldRand.RandRange(1, ContinentSize.Y - 1)));

	Voronoi->AddPoints(Points);
	Voronoi->GenerateSites(2);

	TMap<FVector2D, TArray<FVector2D>> VertsToNeighborVerts;
	TMap<FVector2D, TArray<FWorldCell*>> VertsToNeighborCells;

	for (auto &currSite : Voronoi->GeneratedSites)
	{
		FWorldCell NewCell;
		NewCell.ShapeData = &currSite.Value;

		WorldCells.Add(currSite.Key, MoveTemp(NewCell));

		if (currSite.Value.bIsCorner)
			CornerInds.Add(currSite.Value.Index);
		else if (currSite.Value.bIsEdge)
			EdgeInds.Add(currSite.Value.Index);
	}

	for (auto &currCellPair : WorldCells)
	{
		FWorldCell& currCell = currCellPair.Value;
		for (auto &currEdge : currCell.ShapeData->Edges)
		{
			VertsToNeighborVerts.FindOrAdd(currEdge.LeftEndPoint).AddUnique(currEdge.RightEndPoint);
			VertsToNeighborVerts.FindOrAdd(currEdge.RightEndPoint).AddUnique(currEdge.LeftEndPoint);

			VertsToNeighborCells.FindOrAdd(currEdge.LeftEndPoint).AddUnique(&currCell);
			VertsToNeighborCells.FindOrAdd(currEdge.RightEndPoint).AddUnique(&currCell);
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


	// all of this stuff past this line
	// is under revision.

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
	// follows assignCornerElevations() and redistributeElevations()
	// a la https://github.com/amitp/mapgen2/blob/master/Map.as

	TArray<FGraphPoint*> queue;
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
	{
		if(nullptr != curr.DownslopePoint)
			SortedPoints.Add(&curr);
	}

	if (SortedPoints.Num() < 1)
	{
		UStaticFuncLib::Print("AWorldContinent::GenerateRivers: No valid downslopes found! "
							  "Did something go awry with the shape function?", true);
		return; // return before any exceptions or infinite loops can happen
	}

	int NumRivers = int((ContinentSize.GetMax() / 2) * RiverCoefficient);
	for (int currRiver = 0; currRiver < NumRivers; ++currRiver)
	{
		FGraphPoint* currChoice = SortedPoints[UWorldGenFuncLib::GetWorldRandom().RandRange(0, SortedPoints.Num() - 1)];
		//while(nullptr == currChoice->DownslopePoint)
		//	currChoice = SortedPoints[UWorldGenFuncLib::GetWorldRandom().RandRange(0, SortedPoints.Num() - 1)];

		while (currChoice->DownslopePoint != nullptr)
		{
			currChoice->Biome = EBiomeType::BIOME_RIVERWATER;
			FVector4 NewEdge(currChoice->Point, currChoice->DownslopePoint->Point);
			Rivers.FindOrAdd(NewEdge) += 1;

			currChoice = currChoice->DownslopePoint;
		}
	}
}

void AWorldContinent::AssignMoisture()
{
	TArray<FGraphPoint*> queue;

	TArray<FGraphPoint*> LandPoints;

	for (auto &curr : PointGraph)
	{
		if (curr.Biome == EBiomeType::BIOME_RIVERWATER) // if it's a river
		{
			curr.EnvData.Moisture = FMath::Min(3.0f, 0.2f * Rivers.FindRef(FVector4(curr.Point, curr.DownslopePoint->Point)));
			queue.Add(&curr);
		}
		else if (curr.Biome == EBiomeType::BIOME_WATER || curr.Biome == EBiomeType::BIOME_HANGINGWATER) // if it's a lake
		{
			curr.EnvData.Moisture = 1.0f;
			queue.Add(&curr);
		}
		else // otherwise
		{
			curr.EnvData.Moisture = 0.0f;

			if (curr.Biome == EBiomeType::BIOME_GROUND)
				LandPoints.Add(&curr);
		}
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

			float NewMoist = curr->EnvData.Moisture * MoisturePersistence;

			if (NewMoist > neighborPoint->EnvData.Moisture)
			{
				neighborPoint->EnvData.Moisture = NewMoist;
				queue.Add(neighborPoint);
			}
		}
	}


	// redistribution

	LandPoints.Sort([](FGraphPoint& a, FGraphPoint& b) { return a.EnvData.Moisture < b.EnvData.Moisture; });

	for (int currInd = 0; currInd < LandPoints.Num(); ++currInd)
		LandPoints[currInd]->EnvData.Moisture = currInd / (LandPoints.Num() - 1);


	// Setting polygon moistures

	TArray<FWorldCell*> RelevantCells;
	for (auto *currVert : LandPoints)
	{
		for (auto *currCell : currVert->NeighborCells)
		{
			if (currCell->Biome != EBiomeType::BIOME_HOLE)
			{
				currCell->EnvironmentData.Moisture += currVert->EnvData.Moisture;
				RelevantCells.Add(currCell);
			}
		}
	}

	for (auto *currCell : RelevantCells)
		currCell->EnvironmentData.Moisture /= currCell->ShapeData->Vertices.Num();

}

void AWorldContinent::DetermineBiomes()
{
	for (auto &currCellInd : NonHoleIndices)
	{
		FWorldCell& currCell = WorldCells[currCellInd];
		currCell.Biome = BiomeMap.GetDefaultObject()->DetermineBiome(currCell.Biome, currCell.EnvironmentData);
	}
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

	FWorldType* Chosen = WorldTypes.Find(WorldTypeToBuild);

	if (nullptr != Chosen)
	{
		FActorSpawnParameters params;
		params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		AWorldContinent* CurrentSpawned;
		for (FWorldContinentPacket &currContinent : Chosen->ContinentsToSpawn)
		{
			CurrentSpawned = GetWorld()->SpawnActor<AWorldContinent>(currContinent.Continent,
				FVector::ZeroVector,
				FRotator::ZeroRotator,
				params);

			CurrentSpawned->Build(currContinent.BoxDimensions, currContinent.NumSamplingPoints, currContinent.IsDead);
		}
	}
	else
		UStaticFuncLib::Print("AWorldGenerator::Build: WorldType \'" +
							  WorldTypeToBuild.ToString() + 
							  "\' Wasn't in the WorldTypes map! Building was not completed successfully.", true);

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
