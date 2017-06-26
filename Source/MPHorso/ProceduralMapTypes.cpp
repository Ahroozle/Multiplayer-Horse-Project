// Fill out your copyright notice in the Description page of Project Settings.

#include "MPHorso.h"
#include "ProceduralMapTypes.h"

#include "ANLFacadeLib.h"

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

	// Construct Island Diagram
	ConstructVoronoi(NumSamplingPoints);

	//Annotate();

	// Determine Ground/Lakes
	ShapeIsland();

	// Pre-River Attribute Pass/es
	AssignElevations();

	//return;

	// River Pass/es
	GenerateRivers();

	// Create the raster data
	Rasterize();

	// Apply bloom to rasterization data based around the rivers
	// in order to model the spread of moisture from water
	AssignMoisture();

	// Determine the biomes on a per-pixel basis.
	DetermineBiomes();


	// debug stuff
	TArray<FColor> Elvs;
	TArray<FColor> Mois;
	TArray<FColor> Bios;

	for (int currInd = 0; currInd < Biomes.Num(); ++currInd)
	{
		uint8& relevantElev = Elevations[currInd];
		uint8& relevantMois = Moistures[currInd];
		
		Elvs.Add(FColor(relevantElev, relevantElev, relevantElev));
		Mois.Add(FColor(relevantMois, relevantMois, relevantMois));

		if (!RiversRasterized[currInd])
			Bios.Add(UWorldGenFuncLib::GetBiomeColor(Biomes[currInd]));
		else
			Bios.Add(UWorldGenFuncLib::GetBiomeColor(EBiomeType::BIOME_RIVERWATER));
	}

	UStaticFuncLib::ExportToBitmap(Elvs, "TestElevations", Voronoi->Bounds.Width(), Voronoi->Bounds.Height());
	UStaticFuncLib::ExportToBitmap(Mois, "TestMoistures", Voronoi->Bounds.Width(), Voronoi->Bounds.Height());
	UStaticFuncLib::ExportToBitmap(Bios, "TestBiomes", Voronoi->Bounds.Width(), Voronoi->Bounds.Height());

	BiomeTexture		=	UTexture2D::CreateTransient(Voronoi->Bounds.Width(),Voronoi->Bounds.Height());
	ElevationTexture	=	UTexture2D::CreateTransient(Voronoi->Bounds.Width(),Voronoi->Bounds.Height());
	MoistureTexture		=	UTexture2D::CreateTransient(Voronoi->Bounds.Width(),Voronoi->Bounds.Height());

	//FColor* MipData = static_cast<FColor*>(GeneratedTexture->PlatformData->Mips[0].BulkData.Lock(LOCK_READ_WRITE));
	FColor* BiomeData		= static_cast<FColor*>(BiomeTexture->PlatformData->Mips[0].BulkData.Lock(LOCK_READ_WRITE));
	FColor* ElevationData	= static_cast<FColor*>(ElevationTexture->PlatformData->Mips[0].BulkData.Lock(LOCK_READ_WRITE));
	FColor* MoistureData	= static_cast<FColor*>(MoistureTexture->PlatformData->Mips[0].BulkData.Lock(LOCK_READ_WRITE));

	FMemory::Memcpy(BiomeData, Bios.GetData(), Bios.Num() * sizeof(FColor));
	FMemory::Memcpy(ElevationData, Elvs.GetData(), Elvs.Num() * sizeof(FColor));
	FMemory::Memcpy(MoistureData, Mois.GetData(), Mois.Num() * sizeof(FColor));

	//for (int curr = 0; curr < Elvs.Num(); ++curr)
	//{
	//	FMemory::MemCopy()

	//	Biome
	//	ElevationData[0]
	//}

	BiomeTexture		->PlatformData->Mips[0].BulkData.Unlock();
	ElevationTexture	->PlatformData->Mips[0].BulkData.Unlock();
	MoistureTexture		->PlatformData->Mips[0].BulkData.Unlock();

	BiomeTexture		->UpdateResource();
	ElevationTexture	->UpdateResource();
	MoistureTexture		->UpdateResource();


	//GeneratedTexture->PlatformData->Mips[0].BulkData.Unlock();
	//GeneratedTexture->UpdateResource();

	UStaticFuncLib::Print("Done", true);

	BuildFinishedDelegate.Broadcast();

	/*
		TODO : 
			Need to make passes for:
				-(Possibly "Magic" attr for testing and since Hanging Lakes should have extra differentiation from Normal Lakes?)
	
			Missing things include:
				-Road data (either follow Amit's method or make my own, I guess. Splines will definitely be useful, though.)
	
			REMEMBER TO FREE MEMORY THAT'S NOT NEEDED ANYMORE AFTERWARD!
	*/

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

	const float KeyScale = 100 * PI;
	for (auto &currSite : Voronoi->GeneratedSites)
	{
		FWorldCell NewCell;
		NewCell.ShapeData = &currSite.Value;

		WorldCells.Add(currSite.Key, MoveTemp(NewCell));

		if (currSite.Value.bIsCorner)
			CornerInds.Add(currSite.Value.Index);
		else if (currSite.Value.bIsEdge)
			EdgeInds.Add(currSite.Value.Index);

		for (auto &currVert : currSite.Value.Vertices)
		{
			FVector CurrFocus = FVector(currVert, 0);
			auto PointPred = [&CurrFocus](const FGraphPoint& a) { return UKismetMathLibrary::EqualEqual_VectorVector(FVector(a.Pt, 0), CurrFocus); };

			FGraphPoint* currFound;
			currFound = PointGraph.FindByPredicate(PointPred);

			if (nullptr == currFound)
				currFound = &PointGraph[PointGraph.AddDefaulted()];

			currFound->Pt = currVert;

			for (auto &currEdge : currSite.Value.Edges)
			{
				if (currEdge.LeftEndPoint == currVert)
					CurrFocus = FVector(currEdge.RightEndPoint, 0);
				else if (currEdge.RightEndPoint == currVert)
					CurrFocus = FVector(currEdge.LeftEndPoint, 0);

				int neigh = PointGraph.IndexOfByPredicate(PointPred);

				if (INDEX_NONE == neigh)
				{
					neigh = PointGraph.AddDefaulted();
					PointGraph[neigh].Pt = FVector2D(CurrFocus);
				}

				currFound->NeighborVertIndices.AddUnique(neigh);
			}

			currFound->NeighborCellIndices.AddUnique(currSite.Key);
		}
	}

}

//void AWorldContinent::Annotate()
//{
//	
//	// Post-River Attribute Pass/es
//	AssignMoisture();
//
//	// Biome Pass
//	DetermineBiomes();
//
//
//	// all of this stuff past this line
//	// is under revision.
//
//	// Continent Naming Pass
//	NameContinent();
//
//	// Region Pass
//	DetermineRegions();
//
//	// Region Naming Pass/es
//	NameRegions();
//	
//	// Structure Pass/es
//	/*
//		Note: tips on placing cities ( https://www.youtube.com/watch?v=3PWWtqfwacQ ) ?
//			this could also be useful https://www.youtube.com/watch?v=ThNeIT7aceI
//
//			this ( https://www.rockpapershotgun.com/tag/generation-next/ ) could also
//			be useful for generating things in the cracks like "lore" from the times
//			before the destruction, or at least something to think about for other
//			later projects
//	*/
//	PlaceStructures();
//
//	// Structure Naming Pass/es
//	NameStructures();
//
//	// Road Pass/es (Make Roads, Make Signs At Junctions)
//	GenerateRoads();
//
//
//	/*
//		TODO :
//			-Caves how and when?
//	*/
//}

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
				for (auto &currNeigh : WorldCells[OuterHole[currInd]].ShapeData->NeighborSites)
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
				for (auto &currNeigh : WorldCells[NextHoleRegion[currInd]].ShapeData->NeighborSites)
				{
					if (HolePred(WorldCells[currNeigh]))
						NextHoleRegion.AddUnique(currNeigh);
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

			for (auto &currInd : NextHoleRegion)
				Holes.Remove(currInd);
		}

		if (NotHoles.Num() < 1)
			UStaticFuncLib::Print("AWorldContinent::ShapeIsland: No non-hole cells found! Did something go awry with the shape function?", true);
		else
		{
			NonHoleIndices = MoveTemp(NotHoles);

			for (auto &currPt : PointGraph)
			{
				bool AdjacentToHole = false;
				bool AdjacentToBorder = false;
				for (auto &currCellInd : currPt.NeighborCellIndices)
				{
					FWorldCell& currNeigh = WorldCells[currCellInd];

					currPt.Biome = FMath::Max(currPt.Biome, currNeigh.Biome);

					if (!AdjacentToHole && currPt.Biome != EBiomeType::BIOME_HOLE && currNeigh.Biome == EBiomeType::BIOME_HOLE)
						AdjacentToHole = true;

					if (!AdjacentToBorder && (currNeigh.ShapeData->bIsCorner || currNeigh.ShapeData->bIsEdge))
						AdjacentToBorder = true;
				}

				currPt.IsIslandEdge = AdjacentToHole;
				currPt.IsBorder = AdjacentToBorder;
			}
		}
	}
	else
		UStaticFuncLib::Print("AWorldContinent::ShapeIsland: No holes found! Did something go awry with the shape function?", true);

	/*
		TODO :
			I'm considering making a shape tool that allows me to specify a shape for the island
			to take.

			as far as I can tell from a distance this should be relatively easy due to the tools
			I already have at my disposal.

			i.e. if you consider the canvas to be "negative" in the first place, and each shape
			included to switch the sign at a current location, this means that all you have to do is
			make sure the point is within any "positive" shape and outside of any "negative" shape
			(excluding the canvas, of course). if anything I'm sure the hardest part of this will be
			determining which shapes are negative and which are positive.
	*/
}

void AWorldContinent::AssignElevations()
{
	// follows assignCornerElevations() and redistributeElevations()
	// a la https://github.com/amitp/mapgen2/blob/master/Map.as


	TQueue<FGraphPoint*> queue;

	const float ElevEpsilon = 0.01f;

	TArray<FGraphPoint*> LandPoints;

	for (auto &curr : PointGraph)
	{
		if (curr.IsBorder)
		{
			curr.EnvData.Elevation = 0;
			queue.Enqueue(&curr);
		}
		else
			curr.EnvData.Elevation = INFINITY;

		if (curr.Biome == EBiomeType::BIOME_GROUND)
			LandPoints.Add(&curr);
	}

	while (!queue.IsEmpty())
	{
		FGraphPoint* curr;
		queue.Dequeue(curr);

		for (auto &currNeigh : curr->NeighborVertIndices)
		{
			FGraphPoint* neighborPoint = &PointGraph[currNeigh];

			float NewElev = ElevEpsilon + curr->EnvData.Elevation;
			if (curr->Biome == EBiomeType::BIOME_GROUND)
				NewElev += 1 + (UWorldGenFuncLib::GetWorldRandom().FRand() * ElevationRandScale);

			if (NewElev < neighborPoint->EnvData.Elevation)
			{
				neighborPoint->EnvData.Elevation = NewElev;
				queue.Enqueue(neighborPoint);
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
		for (auto &currCellInd : currVert->NeighborCellIndices)
		{
			FWorldCell* currCell = &WorldCells[currCellInd];
			if (currCell->Biome != EBiomeType::BIOME_HOLE)
			{
				currCell->EnvironmentData.Elevation += currVert->EnvData.Elevation;
				RelevantCells.AddUnique(currCell);
			}
			else
				currCell->EnvironmentData.Elevation = 0;
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

			for (auto &currNeigh : currVert.NeighborVertIndices)
			{
				FGraphPoint* neighborPoint = &PointGraph[currNeigh];

				if (neighborPoint->EnvData.Elevation < LowestNeighbor->EnvData.Elevation)
					LowestNeighbor = neighborPoint;
			}

			if (LowestNeighbor != &currVert)
				currVert.DownslopePoint = LowestNeighbor;

			
		}
	}


	// Generating rivers

	//PointGraph.Sort([](const FGraphPoint& a, const FGraphPoint& b) {return a.EnvData.Elevation > b.EnvData.Elevation; });
	TArray<FGraphPoint*> SortedPoints;
	for (auto &curr : PointGraph)
	{
		if (nullptr != curr.DownslopePoint && &curr != curr.DownslopePoint)
			SortedPoints.Add(&curr);
	}
	SortedPoints.Sort([](const FGraphPoint& a, const FGraphPoint& b) {return a.EnvData.Elevation > b.EnvData.Elevation; });

	if (SortedPoints.Num() < 1)
	{
		UStaticFuncLib::Print("AWorldContinent::GenerateRivers: No valid downslopes found! "
							  "Did something go awry with the shape function?", true);
		return; // return before any exceptions or infinite loops can happen
	}

	int NumRivers = int((ContinentSize.GetMax() / 2) * RiverCoefficient);
	for (int currRiver = 0; currRiver < NumRivers; ++currRiver)
	{
		int chosenInd = UWorldGenFuncLib::GetWorldRandom().RandRange(0, (SortedPoints.Num() / 2) - 1);
		FGraphPoint* currChoice = SortedPoints[chosenInd];
		//while(nullptr == currChoice->DownslopePoint)
		//	currChoice = SortedPoints[UWorldGenFuncLib::GetWorldRandom().RandRange(0, SortedPoints.Num() - 1)];

		while (currChoice->DownslopePoint != nullptr && currChoice->DownslopePoint != currChoice)
		{
			currChoice->Biome = EBiomeType::BIOME_RIVERWATER;
			FVector4 NewEdge(currChoice->Pt, currChoice->DownslopePoint->Pt);
			Rivers.FindOrAdd(PointGraph.Find(*currChoice)) += 1;

			currChoice = currChoice->DownslopePoint;
		}
	}
}

void AWorldContinent::Rasterize()
{
	/*
		TODO :

		Also remember to add some noise during this phase or others
		as some last-minute stuff to make the map more interesting
	*/

	int MapDims = Voronoi->Bounds.Width() * Voronoi->Bounds.Height();


	// Rasterize Cells
	Elevations.SetNumZeroed(MapDims, false);
	Moistures.SetNumZeroed(MapDims, false);
	Biomes.SetNumZeroed(MapDims, false);
	RiversRasterized.SetNumZeroed(MapDims, false);
	TempMoistures.SetNumZeroed(MapDims, false);
	CellInds.SetNumZeroed(MapDims, false);
	for (auto &currPair : WorldCells)
	{
		FWorldCell& currCell = currPair.Value;
		FVoronoiDiagramGeneratedSite* currShapeData = currCell.ShapeData;
	
		FVector2D Min(Voronoi->Bounds.Width(), Voronoi->Bounds.Height()), Max(0,0);
		for (auto &currVert : currShapeData->Vertices)
		{
			Min.X = FMath::Min(Min.X, currVert.X);
			Min.Y = FMath::Min(Min.Y, currVert.Y);
	
			Max.X = FMath::Max(Max.X, currVert.X);
			Max.Y = FMath::Max(Max.Y, currVert.Y);
		}
	
		Min.X = FMath::Clamp(Min.X, 0.0f, (float)Voronoi->Bounds.Width() - 1);
		Min.Y = FMath::Clamp(Min.Y, 0.0f, (float)Voronoi->Bounds.Height() - 1);
	
		Max.X = FMath::Clamp(Max.X, 0.0f, (float)Voronoi->Bounds.Width() - 1);
		Max.Y = FMath::Clamp(Max.Y, 0.0f, (float)Voronoi->Bounds.Height() - 1);
	
		if (Min < Max)
		{
			FEnvironmentData& currEnvData = currCell.EnvironmentData;
	
			for (int x = Min.X; x <= Max.X; ++x)
			{
				for (int y = Min.Y; y <= Max.Y; ++y)
				{
					if (FVoronoiDiagramHelper::PointInVertices(FIntPoint(x, y), currShapeData->Vertices))
					{
						int Index = x + y * Voronoi->Bounds.Width();

						Elevations[Index] = (uint8)(currEnvData.Elevation * 255); // should be 0->1 => 0->255
						/*
							TODO : Bottomside elevation map?
						*/
						Biomes[Index] = currCell.Biome;
						CellInds[Index] = currPair.Key;

						if (EBiomeType::BIOME_WATER == currCell.Biome)
							TempMoistures[Index] = LakeMoisture;
						else if (EBiomeType::BIOME_HANGINGWATER == currCell.Biome)
						{
							TempMoistures[Index] = LakeMoisture;
							/*
								TODO : Magic/Holiness/Spiritual Aligmnent as an attribute of the map?
							*/
						}
					}
				}
			}
		}
	}

	// Rasterize Rivers
	for (auto &currRivPair : Rivers)
	{
		FVector RivPtA(PointGraph[currRivPair.Key].Pt, 0);
		FVector RivPtB(PointGraph[currRivPair.Key].DownslopePoint->Pt, 0);
		float RivWidth = FMath::Sqrt(currRivPair.Value) * RiverWidthMultiplier;
		FVector Perp = (RivPtB - RivPtA).GetSafeNormal().RotateAngleAxis(90, FVector::UpVector) * (RivWidth / 2);
	
		TArray<FVector2D> Points;
		Points.Add(FVector2D(RivPtA + Perp));
		Points.Add(FVector2D(RivPtB + Perp));
		Points.Add(FVector2D(RivPtB - Perp));
		Points.Add(FVector2D(RivPtA - Perp));
	
		FVector2D Min, Max;
	
		Min.X = FMath::Clamp(FMath::Min3(Points[0].X, Points[1].X, FMath::Min(Points[2].X, Points[3].X)), 0.0f, (float)Voronoi->Bounds.Width() - 1);
		Min.Y = FMath::Clamp(FMath::Min3(Points[0].Y, Points[1].Y, FMath::Min(Points[2].Y, Points[3].Y)), 0.0f, (float)Voronoi->Bounds.Height() - 1);
	
		Max.X = FMath::Clamp(FMath::Max3(Points[0].X, Points[1].X, FMath::Max(Points[2].X, Points[3].X)), 0.0f, (float)Voronoi->Bounds.Width() - 1);
		Max.Y = FMath::Clamp(FMath::Max3(Points[0].Y, Points[1].Y, FMath::Max(Points[2].Y, Points[3].Y)), 0.0f, (float)Voronoi->Bounds.Height() - 1);
	
		if (Min < Max)
		{
			const FColor& RivCol = UWorldGenFuncLib::GetBiomeColor(EBiomeType::BIOME_RIVERWATER);
	
			for (int x = Min.X; x <= Max.X; ++x)
			{
				for (int y = Min.Y; y <= Max.Y; ++y)
				{
					if (FVoronoiDiagramHelper::PointInVertices(FIntPoint(x, y), Points))
					{
						const int Index = x + y*Voronoi->Bounds.Width();

						bool InHole = EBiomeType::BIOME_HOLE == Biomes[Index];
						bool InLake = EBiomeType::BIOME_WATER == Biomes[Index] || EBiomeType::BIOME_HANGINGWATER == Biomes[Index];

						if ((RiversRasterized[Index] = !(InHole || InLake)))
							TempMoistures[Index] = FMath::Clamp(0.2f * currRivPair.Value, /*1*/0.0f, 3.0f);

						/*
							NOTE AND POSSIBLY TODO :
								This method results of course in some rivers being
								cut off and stuff as well as moving along an edge
								in a weird way.

								I feel like this'll make excellent waterfalls at some
								locations so I'm keeping it.

								Also I could just make a way to settle liquids and then
								just settle them after generation if I'm so concerned about
								having weird unrealistic waterfalls I guess rofl
						*/
					}
				}
			}
		}
	}
}

void AWorldContinent::AssignMoisture()
{
	TQueue<FIntPoint> queue;
	for (int x = 0; x < Voronoi->Bounds.Width(); ++x)
	{
		for (int y = 0; y < Voronoi->Bounds.Height(); ++y)
		{
			if (TempMoistures[x + y * Voronoi->Bounds.Width()])
				queue.Enqueue({ x,y });
		}
	}

	float MaxMoisture = 0;
	while (!queue.IsEmpty())
	{
		FIntPoint Pt;
		queue.Dequeue(Pt);

		for (int xOffs = -1; xOffs <= 1; ++xOffs)
		{
			for (int yOffs = -1; yOffs <= 1; ++yOffs)
			{
				if ((xOffs != 0 && yOffs != 0) &&
					FMath::IsWithin(Pt.X+xOffs,0,Voronoi->Bounds.Width()) &&
					FMath::IsWithin(Pt.Y + yOffs, 0, Voronoi->Bounds.Height()))
				{
					int currInd = Pt.X + Pt.Y * Voronoi->Bounds.Width();
					MaxMoisture = FMath::Max(MaxMoisture, TempMoistures[currInd]);
					float NewMoist = TempMoistures[currInd] * MoisturePersistence;

					int currNeighInd = (Pt.X + xOffs) + (Pt.Y + yOffs) * Voronoi->Bounds.Width();
					if (NewMoist > TempMoistures[currNeighInd])
					{
						TempMoistures[currNeighInd] = NewMoist;
						queue.Enqueue({ Pt.X + xOffs,Pt.Y + yOffs });
					}
				}
			}
		}
	}

	if (DownLayers > 0)
	{
		TArray<TArray<float>> BuffsA;
		TArray<TArray<float>> BuffsB;
		TArray<float> HalfPows;

		int Downsleft = DownLayers;
		while (--Downsleft >= 0)
		{
			int ind = BuffsA.AddDefaulted();
			BuffsB.AddDefaulted();

			float currPow = FMath::Pow(0.5f, ind);
			HalfPows.Add(currPow);
			BuffsA[ind].SetNumZeroed((Voronoi->Bounds.Width() * currPow) * (Voronoi->Bounds.Height() * currPow), false);
			BuffsB[ind].SetNumZeroed(BuffsA[ind].Num(), false);

			if (BuffsA.Last().Num() <= 1)
			{
				BuffsA.RemoveAt(BuffsA.Num() - 1);
				BuffsB.RemoveAt(BuffsB.Num() - 1);
				break;
			}
		}

		for (int x = 0; x < Voronoi->Bounds.Width(); ++x)
		{
			for (int y = 0; y < Voronoi->Bounds.Height(); ++y)
			{
				float normX = float(x) / Voronoi->Bounds.Width();
				float normY = float(y) / Voronoi->Bounds.Height();

				float& currVal = TempMoistures[x + y*Voronoi->Bounds.Width()];
				for (int currInd = 0; currInd < BuffsA.Num(); ++currInd)
				{
					float& currPow = HalfPows[currInd];
					int localX = normX * (Voronoi->Bounds.Width() * currPow);
					int localY = normY * (Voronoi->Bounds.Height() * currPow);
					BuffsA[currInd][localX + localY * (Voronoi->Bounds.Width() * currPow)] += currVal;
				}
			}
		}

		for (int currA = 0; currA < BuffsA.Num(); ++currA)
		{
			TArray<float>& currBuff = BuffsA[currA];
			float quadPow = FMath::Pow(4, currA);
			for (int currInd = 0; currInd < currBuff.Num(); ++currInd)
				currBuff[currInd] /= quadPow;
		}

		auto GaussPred =
			[](FIntPoint Pt, FIntPoint Offs, TArray<float>& From, TArray<float>& To, int ArrWidth)
		{
			float Res = 0;

			if (FMath::IsWithin(Pt.X - Offs.X, 0, ArrWidth) && FMath::IsWithin(Pt.Y - Offs.Y, 0, From.Num() / ArrWidth))
				Res += 5.0f * From[(Pt.X - Offs.X) + (Pt.Y - Offs.Y) * ArrWidth];

			Res += 6.0f * From[Pt.X + Pt.Y * ArrWidth];

			if (FMath::IsWithin(Pt.X + Offs.X, 0, ArrWidth) && FMath::IsWithin(Pt.Y + Offs.Y, 0, From.Num() / ArrWidth))
				Res += 5.0f * From[(Pt.X + Offs.X) + (Pt.Y + Offs.Y) * ArrWidth];

			To[Pt.X + Pt.Y * ArrWidth] = Res / 16.0f;
		};

		for (int currOuter = 0; currOuter < BuffsA.Num(); ++currOuter)
		{
			TArray<float>& currA = BuffsA[currOuter];
			TArray<float>& currB = BuffsB[currOuter];

			int currWidth = Voronoi->Bounds.Width() * HalfPows[currOuter];
			int currHeight = Voronoi->Bounds.Height() * HalfPows[currOuter];

			int offs = 1.2f / currWidth;
			for (int x = 0; x < currWidth; ++x)
			{
				for (int y = 0; y < currHeight; ++y)
					GaussPred(FIntPoint(x, y), FIntPoint(offs, 0), currA, currB, currWidth);
			}

			offs = 1.2f / currHeight;
			for (int x = 0; x < currWidth; ++x)
			{
				for (int y = 0; y < currHeight; ++y)
					GaussPred(FIntPoint(x, y), FIntPoint(0, offs), currB, currA, currWidth);
			}
		}

		MaxMoisture = 0;
		for (int x = 0; x < Voronoi->Bounds.Width(); ++x)
		{
			for (int y = 0; y < Voronoi->Bounds.Height(); ++y)
			{
				int Index = x + y * Voronoi->Bounds.Width();

				float normX = float(x) / Voronoi->Bounds.Width();
				float normY = float(y) / Voronoi->Bounds.Height();

				float currRes = 0;
				for (int currInd = 0; currInd < BuffsA.Num(); ++currInd)
				{
					float& currPow = HalfPows[currInd];
					int localX = normX * (Voronoi->Bounds.Width() * currPow);
					int localY = normY * (Voronoi->Bounds.Height() * currPow);
					currRes += BuffsA[currInd][localX + localY * (Voronoi->Bounds.Width() * currPow)];
				}
				TempMoistures[Index] = FMath::Clamp(currRes, 0.0f, 1.0f);
				MaxMoisture = FMath::Max(MaxMoisture, TempMoistures[Index]);
			}
		}
	}

	for (int currInd = 0; currInd < Moistures.Num(); ++currInd)
		Moistures[currInd] = (uint8)((TempMoistures[currInd] / MaxMoisture) * 255); // should be 0->1 => 0->255

	TempMoistures.Empty(); // won't be needing this anymore
}

void AWorldContinent::DetermineBiomes()
{
	for (int currInd = 0; currInd < Biomes.Num(); ++currInd)
	{
		FEnvironmentData currEnvData = { float(Elevations[currInd]) / 255.0f, float(Moistures[currInd]) / 255.0f };
		Biomes[currInd] = BiomeMap.GetDefaultObject()->DetermineBiome(Biomes[currInd], currEnvData);
	}
}

void AWorldContinent::NameContinent()
{
	// TODO : IMPL
	/*
		Pour through raster data and determine a name
		based on it + some randomization
	*/
}

void AWorldContinent::DetermineRegions()
{
	// TODO : IMPL
	/*
		Compare cells and raster data.
		The majority biome in the cell's raster
		data is considered the cell's biome. This
		information is then used to group cells into
		regions that can then be named later.

		EDIT: or maybe just use the raster data and
		segment the image into regions based on their
		biome values alone (and then find a way to deal
		with very small regions)?
	*/
}

void AWorldContinent::NameRegions()
{
	// TODO : IMPL
}

void AWorldContinent::PlaceStructures()
{
	// TODO : IMPL
	/*
		Might need to rename this to PlaceSurfaceStructures
		whenever I get around to voxelizing the map?
		this is probably only going to make structures on
		the surface if there's only raster data to work
		with and not also respective voxel data.
	*/
}

void AWorldContinent::NameStructures()
{
	// TODO : IMPL
}

void AWorldContinent::GenerateRoads()
{
	// TODO : IMPL
	/*
		bump this upward, probably. Roads
		are more likely a sooner-constructed
		feature than a later-constructed one.
		and besides who makes roads underground?
		Asinine, really. :^)
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
