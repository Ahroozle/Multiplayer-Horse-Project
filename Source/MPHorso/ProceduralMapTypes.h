// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "Components/SplineComponent.h"
#include "Components/SplineMeshComponent.h"
#include "Components/ChildActorComponent.h"

#include "VoronoiDiagram.h"

#include "PolyVox/PagedVolume.h"
#include "PolyVox/MaterialDensityPair.h"

#include "ProceduralMapTypes.generated.h"


/*
	Sources that might be helpful:
		https://gamedevelopment.tutsplus.com/tutorials/bake-your-own-3d-dungeons-with-procedural-recipes--gamedev-14360
		http://www.gamasutra.com/blogs/AAdonaac/20150903/252889/Procedural_Dungeon_Generation_Algorithm.php

	Things to address eventually:
		-Camera. This'll be a mess to deal with, definitely.
*/


UENUM(BlueprintType)
enum class ETileType : uint8
{
	// TODO : IMPL
	PLACEHOLDER
};

// Wrapper struct for attribute map since this version of unreal doesn't support TMaps as function parameters
USTRUCT(BlueprintType)
struct FEnvironmentData
{
	GENERATED_USTRUCT_BODY();

	UPROPERTY(BlueprintReadWrite)
		float Elevation;
	UPROPERTY(BlueprintReadWrite)
		float Moisture;
};

UENUM(BlueprintType)
enum class EBiomeType : uint8
{
	BIOME_HOLE = 0			UMETA(DisplayName = "Hole/No Biome"),

	BIOME_GROUND			UMETA(DisplayName = "Ground"),

	BIOME_WATER				UMETA(DisplayName = "Normal Lake"),
	BIOME_HANGINGWATER		UMETA(DisplayName = "Hanging Lake")
};

UCLASS(BlueprintType)
class MPHORSO_API UBiomeMap : public UObject
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintNativeEvent, BlueprintPure, Category = "Procedural World Generation|Biome Map")
		EBiomeType DetermineBiome(const FEnvironmentData& Cell);
	EBiomeType DetermineBiome_Implementation(const FEnvironmentData& Cell) { return EBiomeType::BIOME_HOLE; }

private:

};

/*
	Structure containing voronoi cell data
	for world generation classes.
*/
USTRUCT(BlueprintType)
struct FWorldCell
{
	GENERATED_USTRUCT_BODY();

	FVoronoiDiagramGeneratedSite* ShapeData;

	/*
		index used to determine what region this cell belongs to
		inside the containing continent.
	*/
	UPROPERTY(BlueprintReadWrite)
		int Region = -1;

	/*
		The index of the biome inside the containing continent's biomemap's
		biomelist, for use looking up relevant data.
	*/
	UPROPERTY(BlueprintReadWrite)
		EBiomeType Biome = EBiomeType::BIOME_HOLE;

	// The environmental data for this cell
	UPROPERTY(BlueprintReadWrite)
		FEnvironmentData EnvironmentData;
};

// multiple-purpose graph point for use during generation
USTRUCT()
struct FGraphPoint
{
	GENERATED_USTRUCT_BODY();

	// the vertex of the graph this struct represents
	FVector2D Point;

	// neighboring points in the graph
	TArray<FVector2D> NeighborVerts;

	// neighboring cells in the graph
	TArray<FWorldCell*> NeighborCells;

	// the point downslope from this one, for river calcs
	FGraphPoint* DownslopePoint = nullptr;

	// This point's environment data.
	FEnvironmentData EnvData;

	EBiomeType Biome = EBiomeType::BIOME_HOLE;

	bool IsIslandEdge = false;
	bool IsBorder = false;

	bool operator==(const FGraphPoint& o) const { return o.Point == Point; }

	friend FORCEINLINE uint32 GetTypeHash(const FGraphPoint& o) { return GetTypeHash(o.Point); }
};

/*
	TODO : struct for roads
*/

UCLASS()
class MPHORSO_API UWorldGenFuncLib : public UObject
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable)
		static void InitializeWorldRandom(int RandSeed) { RandStream.Initialize(RandSeed); }

	UFUNCTION(BlueprintCallable)
		static const FRandomStream& GetWorldRandom() { return RandStream; }

	UFUNCTION(BlueprintCallable)
		static void SetCellAttribute(UPARAM(Ref) TArray<FWorldCell>& Cells, int CellIndex, FName AttributeName, float AttributeValue);

	// C++ Functions

	static void FloodFillCells(int StartCellInd,
							   const TArray<FWorldCell>& Cells,
							   const TFunction<bool(const FWorldCell&)>& Pred,
							   TArray<int>& OutFloodFillInds,
							   bool IncludeStartCellInd);

private:

	static FRandomStream RandStream;

};


/*
	This class represents a function used to determine
	which parts of a world/island are land, which parts
	are water, and which parts are holes.
*/
UCLASS(Blueprintable)
class MPHORSO_API UWorldShapeFunction : public UObject
{
	GENERATED_BODY()

public:

	// Initializes any internal variables this function to perform its ops.
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Procedural World Generation|World Shape Function")
		void Initialize();
	virtual void Initialize_Implementation() {}

	//The actual function which determines whether a cell is ground or not.
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Procedural World Generation|World Shape Function")
		bool CellIsGround(FVector Pt);
	virtual bool CellIsGround_Implementation(FVector Pt) { return true; }

	void DetermineShape(FIntVector Size, UPARAM(Ref) TMap<int, FWorldCell>& InOutWorldCells);

private:

};

/*
	A world generation pass
*/
UCLASS(Blueprintable)
class MPHORSO_API UWorldGenPass : public UObject
{
	GENERATED_BODY()

public:

	// Initializes any internal variables this function to perform its ops.
	UFUNCTION(BlueprintNativeEvent, Category = "Procedural World Generation|World Generation Pass")
		void Initialize();
	void Initialize_Implementation() {}

	// Function that applies attribute/s to the cells
	UFUNCTION(BlueprintNativeEvent, Category = "Procedural World Generation|World Generation Pass")
		bool Apply(UPARAM(Ref) TArray<FWorldCell>& Cells, FVector MapSize);
	virtual bool Apply_Implementation(UPARAM(Ref) TArray<FWorldCell>& Cells, FVector MapSize) { return false; }

private:

};

USTRUCT()
struct FWorldRegion
{
	GENERATED_USTRUCT_BODY();

	// The biome of the region
	UPROPERTY(BlueprintReadWrite)
		int Biome;

	// How big the region is in cells
	UPROPERTY(BlueprintReadWrite)
		int NumCells;

	// Region's Name
	UPROPERTY(BlueprintReadWrite)
		FString Name;
};


DECLARE_DYNAMIC_MULTICAST_DELEGATE(FContinentBuildFinished);

/*
	class that represents a single "continent" in the world
	i.e. a neighborhood of connected world cells.

	Separate islands are separate "continents."
*/
UCLASS(BlueprintType)
class MPHORSO_API AWorldContinent : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AWorldContinent(const FObjectInitializer& _init);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// The voronoi diagram.
	TSharedPtr<FVoronoiDiagram> Voronoi;

	// Size, kept for internal use during generation
	FIntVector ContinentSize;

	TMap<int, FWorldCell> WorldCells;

	// Indices of non-hole cells in WorldCells
	TArray<int> NonHoleIndices;

	// indices into worldcells which represent corners and edges.
	TArray<int> CornerInds;
	TArray<int> EdgeInds;

	// Collection of more specific per-point data
	TSet<FGraphPoint> PointGraph;

	/*
		Map of all created rivers

		NOTE: the FVector4 key is actually
		two FVector2Ds mashed into one
		struct.

		tl;dr I'm just too lazy to overload
		GetTypeHash for FEdges right now lol
	*/
	TMap<FVector4, float> Rivers;


	// ShapeIsland() Variables

	// The shaping function used to determine what's ground and what isn't.
	UPROPERTY(EditDefaultsOnly, Category = "Procedural World Generation|World Continent")
		TSubclassOf<UWorldShapeFunction> ShapeFunc;

	// The chance that an internal hole in the island will instead become an internal lake.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural World Generation|World Continent", meta = (ClampMin = "0.0", ClampMax = "1.0"))
		float LakeChance = 0.75f;

	// The chance that a generated lake will actually be a hanging lake (i.e. no ground underneath).
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural World Generation|World Continent", meta = (ClampMin = "0.0", ClampMax = "1.0"))
		float HangingLakeChance = 0.25f;


	// AssignElevations() Variables

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural World Generation|World Continent", meta = (ClampMin = "0.0", ClampMax = "1.0"))
		float ElevationRandScale = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural World Generation|World Continent", meta = (ClampMin = "0.0"))
		float ElevationScaleFactor = 1.1f;
	

	// GenerateRivers() Variables

	/*
		Modifies how many rivers will spawn on the continent.
		The higher the coefficient, the more rivers.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural World Generation|World Continent", meta = (ClampMin = "0.0", ClampMax = "5.0"))
		float RiverCoefficient = 1;


	// The world generation done to determine this continent's attributes.
	UPROPERTY(EditDefaultsOnly, Category = "Procedural World Generation|World Continent")
		TArray<TSubclassOf<UWorldGenPass>> Passes;

	// The biome map this continent uses when determining its biomes.
	UPROPERTY(EditDefaultsOnly, Category = "Procedural World Generation|World Continent")
		TSubclassOf<UBiomeMap> BiomeMap;


	// The created regions
	UPROPERTY(BlueprintReadWrite, Category = "Procedural World Generation|World Continent")
		TArray<FWorldRegion> RegionDetails;

	// Whether or not this continent is "dead" (receives no updates)
	UPROPERTY(BlueprintReadWrite, Category = "Procedural World Generation|World Continent")
		bool Dead = false;


	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category = "Procedural World Generation|World Continent")
		FContinentBuildFinished BuildFinishedDelegate;


	UFUNCTION(BlueprintCallable, Category = "Procedural World Generation|World Continent")
		void Build(FIntVector MapSize, int NumSamplingPoints, bool IsDead);

private:

	/*
		These two main functions are called during Build().
	*/
	void ConstructVoronoi(int SamplingPoints);

	void Annotate();
		void ShapeIsland();
		void AssignElevations();
		void GenerateRivers();
		void AssignMoisture();
		void DetermineBiomes();
		void NameContinent();
		void DetermineRegions();
		void NameRegions();
		void PlaceStructures();
		void NameStructures();
		void GenerateRoads();

	void Rasterize();

	void Voxelize();
};


USTRUCT(BlueprintType)
struct FWorldContinentPacket
{
	GENERATED_USTRUCT_BODY();

	UPROPERTY(EditAnywhere, Category = "Procedural World Generation|World Continent Packet")
		bool IsDead;

	// The dimensions of this continent, in tiles.
	UPROPERTY(EditDefaultsOnly, Category = "Procedural World Generation|World Continent")
		FIntVector BoxDimensions;

	// The number of sampling points used to create this continent's voronoi diagram.
	UPROPERTY(EditDefaultsOnly, Category = "Procedural World Generation|World Continent")
		int NumSamplingPoints = 1000;

	UPROPERTY(EditAnywhere, Category = "Procedural World Generation|World Continent Packet")
		TSubclassOf<AWorldContinent> Continent;

};

USTRUCT(BlueprintType)
struct FWorldType
{
	GENERATED_USTRUCT_BODY();

	UPROPERTY(EditAnywhere, Category = "Procedural World Generation|World Type")
		TArray<FWorldContinentPacket> ContinentsToSpawn;

	// The index of the starting island in ContinentsToSpawn
	UPROPERTY(EditAnywhere, Category = "Procedural World Generation|World Type")
		int StartingIsland = 0;

	// The size of a tile in UU
	UPROPERTY(EditAnywhere, Category = "Procedural World Generation|World Type")
		int TileSize = 200;

};

UCLASS(Blueprintable)
class MPHORSO_API AWorldGenerator : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AWorldGenerator(const FObjectInitializer& _init);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// The island types to be spawned
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Procedural World Generation|World Generator")
		TMap<FName, FWorldType> WorldTypes;

	// Function which both generates and rasterizes the world.
	UFUNCTION(BlueprintCallable, Category = "Procedural World Generation|World Generator")
		void Build(FName WorldTypeToBuild, int InitialRandSeed = 0);

private:

	UPROPERTY()
		TArray<AWorldContinent*> SpawnedContinents;

};


/*
	TODO (+ writing downs):

		Switch terrain generation system over from simplex-based to voronoi-based

		Delaunay
			|
			V
		Voronoi (Cut edges to bounds)
			|
			V
		Determine Holes VS Water
			|
			V
		Assign Elevations
			|
			V
		Create Rivers
			|
			V
		(Special Attr Passes?)
			|
			V
		Create Biomes
			|
			V
		Create Places
			|
			V
		Create Roads


		Elevation stuff:
			Whatever terrain elevation stuff on top, and then cones on the bottoms
			of cells depending on their elevation?

			Have voronoi cells very obviously switch elevation in steps and then
			build roads that more gradually interpolate between their elevations
			so that they're the easier path to take?

		Store Voronoi Cells as the chunks?

		Worldgen takes arbitrary passes and such.
		void Pass::Apply(InOut WorldMap&)

		Built-in "inherent" tile attributes as specific variables, and then
		Map<FName, Variant> for special-er attributes?

		FIND A WAY TO MAKE CHUNK LODS
			either downsample the map or maybe even slowly
			transition between more and more just the voronoi shape?

			ofc will need to destroy/create mesh sections more ofte
			than just updating them to do any good

			Thought:
				Highest LOD level: Marching cubes of the cell.
				Lowest LOD level: Polygonal prism of the cell.

				and then of course somehow deduce the in-between
				LODS from these two extremes.

				Also look up how to keep collision consistent
				between LODs.

		CHUNK PAGING
			Maybe just temporary paging for the duration of a game
			so that save files can *also* take up as little space
			as possible at the downside of some longer loading
			times for worlds since it has to regenerate them?

		SAVE FILES
			Obviously want to just make a save file that only
			contains the world seed (and maybe settings but only 
			if I make them user-changeable) and various deltas
			(probably chunk-space).

		ENVIRONMENT DAEMONS
			What if I put real-time environmental changes on
			separate worker threads with eternal loops until
			broken by some condition (either game ending or
			otherwise).

			... I could even link them to stop working or
			completely change how they work depending on
			in-game stuff. Imagine killing the god of
			wind and all the wind literally stops.
			*Now you're thinking with incredibly bad
			ideas* :^)

		AREA DIFFICULTIES
			Should difficulty of area be based on ease of
			accessibility of said area, or should ease of
			accessibility be based on a pre-determined
			difficulty for the type of area?

		WORLD UPDATES
			maybe also have LODs for terrain updates?
			i.e. the more relevant a given chunk/cell
			is to the player the more frequently and/or
			granularly it's updated, while further out,
			less-relevant chunks are updated less
			frequently and as a wholesale chunk value
			which then the details are 'caught up'
			with when the chunk becomes relevant sometime
			in the future.

		WORLD EVENTS
			i.e. things like meteor impacts and stuff.
			I was thinking of making some sort of
			'predestining' system which either sometime
			before or at the beginning of the event
			all of its required details are calculated
			in the background. e.g. for a meteor impact
			the first two things I'd need to precalc
			are the starting position and the ending
			position. Then, if I wanted to (which I
			most likely will) I could also precalc
			the resulting crater and where meteor
			pieces would end up, so that none of
			those calcs would need to happen in
			real time and I can still make a wowing
			gameplay spectacle for players to react
			to and enjoy.

			This would be in stark contrast to how
			terraria does *its* meteors, where you
			literally never see or are around for
			the meteor impact and it just spawns in
			the middle of nowhere. It feels rather
			anticlimactic, although maybe there's
			some kind of design principle behind it
			that I'm not picking up that makes it
			a genius move, who knows.

		AI STUFF
			Even though I really, *really* like thinking
			up more complex AI systems the fact is that
			I'm running on a very empty budget and I need
			to create systems that are cheap but effective.

			So I'll have to simplify the system a bit,
			definitely.

			Rule Databases *might* still be used for things
			like NPC dialogue and whatnot but their overall
			behavior ultimately should probably be much
			simpler, albeit at a larger scale that should
			make the world feel alive and connected.

			once roads are created I also might need to
			assign them an area type cost that's lower
			than most other ways so that NPCs prefer
			to stay near or on roads. Maybe not even
			that, maybe just steer them from relevant
			point-to-point on the road with a steering
			algorithm and depending on other factors
			make them veer off or take shortcuts.

			idk rofl, I have stuff to do right now and
			this isn't relevant to development yet.

			
			On the subject of enemy AI I still don't
			really know which way to go. I'll have to
			research more methods of AI handling to
			find a method I prefer above the others
			or just mix some together in some weird
			way.

			MAYBE some kind of rule system but really
			idk since that seems a bit much for a
			most likely finite set of unique player
			actions, as well as might be tedious to
			generate content for. Might find some
			way to piggy back off of behavior trees
			in a way which allows backing out of a
			current tree if something of a 'higher
			priority' pops up and then an enemy's
			'intelligence' could just be chalked
			up to how many kinds of player actions
			an enemy has the ability to react to
			or counteract.

		Also couple things:
			A) Unless marching cubes is for some reason
			   exiting the building then players will
			   need their max slope changed to 45 or so
			   degrees to deal with the steep changes in
			   slope caused by the algorithm.
			B) Stemming off from that slightly, remember
			   to devise a system to allow disorientation
			   of players' skeletalspritebodies while
			   not accidentally breaking the game. i.e.
			   for purposes of aligning to terrain, or
			   allowing players to "tumble" or be sent
			   flying with their body at an angle
			   without hindering gameplay.

			Initial thoughts on this: Allow the
			player to cancel the "animation" of
			sorts through their own input. If
			input is available to the player
			while they are tumbling, their
			action takes priority over the
			gameplay-caused one and their
			body snaps back into the proper
			orientation, preferrably with a
			special animation and maybe a
			sound as extra feedback for their
			exiting of the situation. An
			animation snap should be relatively
			unnoticeable with said proper 
			feedback and the fact that the
			game's graphical style already
			constitutes snappiness of
			animation.

		BIOME STUFF
			I want to be able to name continents,
			biomes, etc. in a fashion which suits
			their attributes and in such a way
			as to be easily referenceable by NPCs
			so that they can give directions.
			I'll have to think on this for a while
			before I know a good method by which to
			do this, since it's a rather tough
			cookie to crack and I'm not sure what
			the best course of action would be yet.


		other things:
			Allow the camera to be tilted up/down
			in addition to the current L/R stuff
			so that a wider range of vision can
			be achieved? Might need to rip off
			how Journey handles looking upward
			(i.e. zoom the camera inward along
			a curve as to avoid colliding with
			the ground as well as avoiding the
			jarring effect of hitting the ground
			and *suddenly* being zoomed in).
			Might have to think about this for
			a bit since zooming is also a thing
			that can happen and stuff. Maybe just
			have the camera zoomed out a bit
			further as a default as well since it
			might be a bit close-to-character for
			a default zoom as of right now?

			Also, rework the chat to be simpler
			and not take up half the screen in
			the "pause" menu. the game's
			direction has changed so much since
			the start that it's not even
			appropriate to have such a complex
			chat system anymore.
*/