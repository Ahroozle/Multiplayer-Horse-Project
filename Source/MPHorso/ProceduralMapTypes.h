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
	BIOME_HOLE = 0						UMETA(DisplayName = "Hole/No Biome"),

	BIOME_GROUND						UMETA(DisplayName = "Ground"),

	BIOME_WATER							UMETA(DisplayName = "Normal Lake"),
	BIOME_HANGINGWATER					UMETA(DisplayName = "Hanging Lake"),

	BIOME_RIVERWATER					UMETA(Hidden),


	// TODO : Determine *actual* biomes. These current ones are ripped straight
	//		  from Amit's model and probably don't align with gameplay purposes
	//		  I want fulfilled, so I'll have to change these sometime.

	BIOME_MARSH							UMETA(DisplayName = "Marshland"),
	BIOME_ICE							UMETA(DisplayName = "Ice"),
	BIOME_SNOW							UMETA(DisplayName = "Snow"),
	BIOME_TUNDRA						UMETA(DisplayName = "Tundra"),
	BIOME_BARE							UMETA(DisplayName = "Bare Land"),
	BIOME_SCORCHED						UMETA(DisplayName = "Scorched Land"),
	BIOME_TAIGA							UMETA(DisplayName = "Taiga"),
	BIOME_SHRUBLAND						UMETA(DisplayName = "Shrubland"),
	BIOME_TEMP_DESERT					UMETA(DisplayName = "Temperate Desert"),
	BIOME_TEMP_RAINFOREST				UMETA(DisplayName = "Temperate Rainforest"),
	BIOME_TEMP_DECID_FOREST				UMETA(DisplayName = "Temperate Deciduous Forest"),
	BIOME_GRASSLAND						UMETA(DisplayName = "Grassland"),
	BIOME_TROP_RAINFOREST				UMETA(DisplayName = "Tropical Rainforest"),
	BIOME_TROP_SEASONAL_FOREST			UMETA(DisplayName = "Tropical Seasonal Forest"),
	BIOME_SUBTROP_DESERT				UMETA(DisplayName = "Subtropical Desert"),




	MAX						UMETA(Hidden)
};

UCLASS(Blueprintable)
class MPHORSO_API UWorldBiomeMap : public UObject
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintNativeEvent, BlueprintPure, Category = "Procedural World Generation|Biome Map")
		EBiomeType DetermineBiome(const EBiomeType& PreBiome, const FEnvironmentData& EnvironmentData);
	EBiomeType DetermineBiome_Implementation(const EBiomeType& PreBiome, const FEnvironmentData& EnvironmentData) { return EBiomeType::BIOME_HOLE; }

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
		static const FColor& GetBiomeColor(EBiomeType Biome) { return BiomeColors[(int)Biome]; }

	// C++ Functions

private:

	static FRandomStream RandStream;

	static FColor BiomeColors[(int)EBiomeType::MAX];

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


	// AssignMoisture() Variables

	/*
		Modifies how far moisture carries out from
		lakes and rivers.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural World Generation|World Continent", meta = (ClampMin = "0.0", ClampMax = "1.0"))
		float MoisturePersistence = 0.9f;


	// DetermineBiomes() Variables

	// The biome map this continent uses when determining its biomes.
	UPROPERTY(EditDefaultsOnly, Category = "Procedural World Generation|World Continent")
		TSubclassOf<UWorldBiomeMap> BiomeMap;



	// The world generation done to determine this continent's attributes.
	UPROPERTY(EditDefaultsOnly, Category = "Procedural World Generation|World Continent")
		TArray<TSubclassOf<UWorldGenPass>> Passes;

	// The created regions
	UPROPERTY(BlueprintReadWrite, Category = "Procedural World Generation|World Continent")
		TArray<FWorldRegion> RegionDetails;

	// Whether or not this continent is "dead" (receives no updates)
	UPROPERTY(BlueprintReadOnly, Category = "Procedural World Generation|World Continent")
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

		Elevation stuff:
			Whatever terrain elevation stuff on top, and then cones on the bottoms
			of cells depending on their elevation?

			Have voronoi cells very obviously switch elevation in steps and then
			build roads that more gradually interpolate between their elevations
			so that they're the easier path to take?

		CHUNK LODS
			PolyVox has *some* chunk LOD stuff in its documentation
			(i.e. http://www.volumesoffun.com/polyvox/documentation/0.2.1/manual/LevelOfDetail.html )
			but I'll definitely say I'm not sure about how to handle LODs with this new
			stuff, if I even have to deal with them at all (although my gut tells me that duh, of
			*course* LODs need to be implemented)

			Thoughts:
				Highest LOD level: Polyvox-constructed mesh
				Lowest LOD level: Polygonal prism of the cell.

				and then of course somehow deduce the in-between
				LODS from these two extremes.

				Also look up how to keep collision consistent
				between LODs.

		CHUNK PAGING
			PolyVox also has stuff for this but I'll need to
			understand it more in-depth to make effective use
			of it. I'm not really finding anything on the
			very-much-finite volumes I'm eventually going to
			be dealing with, although considering dealing with
			infinite volumes is considered a much bigger mess
			to deal with I'm hoping that it won't be a long
			struggle.

		SAVE FILES
			Small change in plan to files, after checking
			how terraria does their save files
			( http://seancode.com/terrafirma/world.html )
			things like RLE were brought to mind, as well
			as it being a much-needed reminder that it's
			fine to splurge on a bit of memory considering
			a lot of what we do is usually only a few
			bytes at worst.

			Of course, save file structure is still being
			thought out.

		AREA DIFFICULTIES
			Originally I was debating whether difficulty
			of an area should be governed by accessibility
			or whether it should be the other way around.
			More recently and very much in part to the
			changes upon changes to how world generation
			is supposed to work, I'm more or less leaning
			toward the former option, as later on in
			development I can see accessibility being very,
			*very* easily manipulated through things like
			NPC placements for generating world histories
			and the like, so I could easily see it continuing
			to lean into the first option.

		WORLD UPDATES
			Updates to terrain are still being considered
			and mulled over, but I'm thinking that I could
			move such a process over to a separate thread.
			Since the end model is that basically every
			continent is updating more or less independently,
			Things hopefully shouldn't get too complicated.

			Another idea could be that rather than updating
			continents via a background system that the player
			neither sees nor meaningfully interacts with, there
			could instead be a much slower "deity" of sorts
			which moves about the continent, updating areas
			in a given sequence and breaking behavior in case of
			certain events transpiring. I'm personally leaning
			more toward a system like this because I believe that
			it would both A) give the player a "justification" as
			for the environment updating so slowly, and B) reaping
			the benefits of such slowness, i.e. having ample time
			to save the world "between" updates.

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