// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "Components/SplineComponent.h"
#include "Components/SplineMeshComponent.h"
#include "ProceduralMapTypes.generated.h"


/*
	TODO :

		Maybe implement "decor connectors"?

		i.e. a decor connector would be a special
		connector which isn't meant for linking to
		corridors. Instead, it spawns a certain
		prebuilt set of decorations fit for the room
		(or maybe even procedurally decorates the room
		based on an implemented function?).
		
		tl;dr it's a way to make the same exact room
		look different so the world doesn't look
		like it's made out of thousands of copy/paste
		operations like it actually is.
	
*/

//	Overview
/*
		Okay so this system has basically been burned to the ground a couple times by now
		but HOPEFULLY this is the last fucking time

		so shit started out like this:
			https://gamedevelopment.tutsplus.com/tutorials/bake-your-own-3d-dungeons-with-procedural-recipes--gamedev-14360

		and it seemed fine... until I actually tried to implement something generic and it kicked my fucking ass

		so now this newer, simpler method that I'm employing will hopefully be a bit cleaner for me to deal with.

		here's how it goes:

		1) Call ProceduralMap's Build() function
		2) ProceduralMap spawns *all* of the ProceduralAreas it intends to spawn
		   very far away from each other (to avoid collisions with
		   outside-of-area rooms)
		3) ProceduralMap then calls every ProceduralArea's Build() at once, after
		   listening for each of their Finished-Building delegates.
		4) The ProceduralAreas begin their own Iterate() loops on their timers,
		   in which they Sequentially spawn all of the (FINITE) number of ProceduralRooms
		   specified in its defaults via GetAreaRandomPoint(), which is to be defined on a
		   per-area basis as a way to allow for more complex types of structures that still
		   actually have *structure*. Room spawning is done using
		   ESpawnActorCollisionHandlingMethod::DontSpawnIfColliding, and if the actor fails
		   to spawn then GetAreaRandomPoint() is just called again until it finally finds
		   a damn place to spawn.
		5) Once all rooms have been placed then the area moves to the average position of
		   the rooms, attaches all of them to itself, and calls its Finished-Building
		   delegate, which causes the ProceduralMap to move it from the iterating list
		   to the finished list.
		6) Once all areas have finished iterating, the ProceduralMap then moves them
		   all into position as well as makes some adjustments to areas that are
		   considered 'malleable' in order to make connections between areas
		   easier to construct, as well as adds some rooms between the areas
		   and binds them with SplineConnectors to other rooms before calling
		   all the finished areas' Finalize() function.
		7) Within Finalize(), all areas take all of the unbound connectors,
		   sorted by room, and connects them all in such a way that there
		   are no or very few rooms unreachable from any other, as well
		   as adding in loops.
		8) Once all of the areas' Finalize() functions are finished, one
		   last sweep over every existing room happens if there's any
		   save data in the form of room deltas (changes in rooms i.e.
		   opening chests, etc.)


		The reason I went through all this trouble of completely burning
		down my system on three separate occasions is because I wanted to
		find that special, as-painless-as-possible system for creating my
		worlds. I can't blow massive spans of time thinking over and
		designing an entire *actually world-sized* playspace; I barely
		have enough time to have a damn coffee. The result is a system
		where I give it relatively few parts to work with and it (probably)
		creates weird noodle systems in fun shapes with additional crappy
		procedural terrain around it to give it a sense of being part of
		a world and not just a pile of room spaghetti like it actually is.

		*also this method allows me to NOT waste time adding corridors to
		 my rooms because the corridors build themselves rofl*

		 really if anything it's probably more like the TinyKeep procedural
		 generation now, except in 3d
		 (link http://www.gamasutra.com/blogs/AAdonaac/20150903/252889/Procedural_Dungeon_Generation_Algorithm.php )

		 which is where I started searching first so rofl I guess I'm the idiot of the 2-AM hour
*/

/*
	Things to keep in mind:
		-Room geometry will have to be constructed a bit weirdly to work with the rest of the game.
		 i.e. since it's paper mario-ish in movement controls, as well as in the camera being 3rd
		 person from-side, that means if I want the Out-of-Bounds support to work then I'll have to
		 craft the rooms in such a way that the visible exterior and visible interior are two separate
		 meshes that can then be faded in/out depending on whether the player is OoB or not.
		-I'm STILL not sure how camera shit will work with this kind of mess but I'll just tackle
		 that when I get to it rofl
		-Depending on how corridors are laid out in an area's RandomlyJunctionSplineConnector() function,
		 there may actually be *splineconnector* collisions to deal with. These should be fairly easy as
		 I could just move points on the spline around until I resolved the collision, but I get the
		 feeling that in practice shit will not be that easy.

	Issues to keep in mind:
		-Collisions between room placements
			(maybe kept track of partially by whether the room has to be rotated >180-deg to align the load zones?)
		-Depth from entrance
			(i.e. how many rooms does it take to get from the entrance to this room?)
		-Whether there needs to be an exit/entrance to another area somewhere
		-Whether camera orientations for load zones should be aligned perfectly or not
			(more about for whether rooms need rotating to fit or not rather than choosing the correct one)


	Other side notes transcribed from stickies (from when this system didn't work like it does now rofl):
		-"Maybe an 'eldritch' dungeon gets rid of the camera direction requirement to make abrupt changes more common in the 'design' for
		  the purpose of subtley disorienting the player with the constantly-changing camera orientations?"

							-NOTE: This one might be a bit weirder to deal with NOW since this idea was made back when camera
								   direction was a thing I was intending on dealing with. However since it's just been pushed
								   to the wayside in favor of *actually getting something done* I'm not so sure how an eldritch
								   area would actually be crafted to *feel* like an eldritch area. This is just going to be
								   up in the air while I finish up implementations, of course.

		-"Maybe have random teleporter rooms that go to rooms in space that link randomly to other teleporter rooms as a crappy way
		  to make the world 'more coherent'?
		  Maybe even make a very rare chance that a teleporter bridge room also branches off into its own spaceship area of sorts?"

							-NOTE: Still maybe. I'd still rather have a more coherent world structure but if it becomes apparent
								   that I'm in over my head this will slowly become the only option.

		-"Separate inside/outside geometry of level so that the outsides of rooms only render / are visible when the player is Out-
		  of-Bounds?
*/


/*
	This class is meant to guide the camera upon entering the collision zone.
	How it goes about this and what the result is depends on blueprint-side implementation.
*/
UCLASS(Blueprintable)
class MPHORSO_API ACameraGuide : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ACameraGuide(const FObjectInitializer& _init);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};

/*
	Specifies spawn data for AMobSpawner subclass instantiations.
*/
USTRUCT(BlueprintType)
struct FMobSpawnData
{
	GENERATED_USTRUCT_BODY();

	// The class of mob to be spawned.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural Layout Generation|MobSpawnData")
		TSubclassOf<AActor> MobClass;

	// The minimum number of mobs of this class to spawn.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural Layout Generation|MobSpawnData")
		int MinToSpawn;

	// The maximum number of mobs of this class to spawn.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural Layout Generation|MobSpawnData")
		int MaxToSpawn;

};

/*
	This class is meant to be a spawner for mobs.
	Whether they are spawned in an area or at a fixed point
	or with other caveat is up to the blueprint-side implementation.
*/
UCLASS(Blueprintable)
class MPHORSO_API AMobSpawner : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AMobSpawner(const FObjectInitializer& _init);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;


	/*
		The spawning data for this mob spawner.

		The list is gone through sequentially when spawning.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural Layout Generation|MobSpawner")
		TArray<FMobSpawnData> SpawnData;

	// Spawns all mobs within the SpawnData list.
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Procedural Layout Generation|MobSpawner")
		void Spawn() const;
	void Spawn_Implementation() const {}
};


/*
	This class is meant to designate positions where rooms can connect,
	and are integral to generation as a result.

	Connectors are meant to have their forward axis point outward, toward
	the place the next room is expected to mount. When connecting, the
	new connector and room are rotated to face the opposite direction.

	Under normal circumstances, every connector should be accounted for by an
	LZ, since connectors are synonymous with pathways. Connectors inherently
	exist to allow rooms to contain corridors that solely exist to maintain
	the realistic structure of an area, and as such allow LZs to be placed in
	such a way to negate players' need to ever traverse them manually, saving
	a ton of camera aches and pains on the developer side.

	However, of course, connectors can be left unpaired for certain situations
	e.g. to keep the realism of a collapsed castle even though you can't normally
	get to the room behind the broken walkway and/or provide a reward to players
	who treat Out-of-Bounds as a mechanic to be used, or to allow for a continuous
	cave system.
*/
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class MPHORSO_API UProceduralConnector : public USceneComponent
{
	GENERATED_BODY()

public:

	/*
		The LZ this Connector has been paired with, set by the room
		during Initialize() when 
	*/
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Procedural Layout Generation|ProceduralConnector")
		UProceduralConnector* BoundTo = nullptr;

	/*
		This connector's attributes, used to determine what kind of
		mesh any splines used to connect the room to others will
		use, among other splineconnector-related things.
	*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Procedural Layout Generation|ProceduralConnector")
		TArray<FName> Attributes;


	/*
		Connects both the connector and the paired LZ to the other's
		respective objects, if present.
	*/
	UFUNCTION(BlueprintCallable, Category = "Procedural Layout Generation|ProceduralConnector")
		void BindConnectorTo(UProceduralConnector* Other) { BoundTo = Other; }
};


/*
	Contains data used during construction
	of the splineconnector
*/
USTRUCT(BlueprintType)
struct FSplineConnectorMeshData
{
	GENERATED_USTRUCT_BODY();

	/*
		all the variants for this type
		of segment, chosen at random
		during splineconnector construction.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural Layout Generation|SplineConnectorMeshData")
		TArray<UStaticMesh*> SegmentMeshes;

	/*
		the value of T at and after
		which these meshes start to
		get used.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural Layout Generation|SplineConnectorMeshData")
		float Time;
};

/*
	These are used during the finalization
	stages of world generation to connect all
	of the free-floating rooms together!
*/
UCLASS(Blueprintable)
class MPHORSO_API AProceduralSplineConnector : public AActor
{
	GENERATED_BODY()

public:

	AProceduralSplineConnector(const FObjectInitializer& _init);

	void OnConstruction(const FTransform& Transform) override;

	/*
		The rootcomponent spline.
	*/
	UPROPERTY(BlueprintReadWrite, Category = "Procedural Layout Generation|ProceduralSplineConnector")
		USplineComponent* RootSpline;

	/*
		Array of all the splinemeshcomponents
		that were spawned in OnConstruction().
	*/
	UPROPERTY(BlueprintReadWrite, Category = "Procedural Layout Generation|ProceduralSplineConnector")
		TArray<USplineMeshComponent*> SpawnedMeshComponents;

	/*
		The mesh data for the spline, intended
		to be in chronological order.
	*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Procedural Layout Generation|ProceduralSplineConnector")
		TArray<FSplineConnectorMeshData> MeshData;
	
	/*
		The attributes that are supported for connection
		to the start of the spline.
	*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Procedural Layout Generation|ProceduralSplineConnector")
		TArray<FName> StartAttributes;

	/*
		The attributes that are supported for connection
		to the end of the spline.
	*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Procedural Layout Generation|ProceduralSplineConnector")
		TArray<FName> EndAttributes;


};


/*
	This class contains the base functionality for
	procedural rooms.
*/
UCLASS(Blueprintable)
class MPHORSO_API AProceduralRoom : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AProceduralRoom(const FObjectInitializer& _init);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	/*
		The name of this room.

		This is used during world reloading to recreate the
		world from saved data.

		Names during play and within save files are in the given format:
			<original_name>_<index(0-N)>

		This is a product of the order of creation of the rooms and used to
		identify between duplicates.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural Layout Generation|ProceduralRoom")
		FName RoomName;

	/*
		This room's attributes, used when considering placement
		in AProceduralArea::GetAreaRandomLocation().
	*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Procedural Layout Generation|ProceduralRoom")
		TArray<FName> Attributes;

	// The room's geometry, visible or otherwise.
	UPROPERTY(BlueprintReadWrite, Category = "Procedural Layout Generation|ProceduralRoom")
		TArray<USceneComponent*> Geometry;

	// The room's connectors.
	UPROPERTY(BlueprintReadWrite, Category = "Procedural Layout Generation|ProceduralRoom")
		TArray<UProceduralConnector*> Connectors;

	// The childactorcomponents that create the camera guides, in case they're needed.
	UPROPERTY(BlueprintReadWrite, Category = "Procedural Layout Generation|ProceduralRoom")
		TArray<UChildActorComponent*> CameraGuideComponents;

	// The room's camera guides
	UPROPERTY(BlueprintReadWrite, Category = "Procedural Layout Generation|ProceduralRoom")
		TArray<ACameraGuide*> CameraGuides;

	// The childactorcomponents that create the mob spawners, in case they're needed.
	UPROPERTY(BlueprintReadWrite, Category = "Procedural Layout Generation|ProceduralRoom")
		TArray<UChildActorComponent*> MobSpawnerComponents;

	// The room's mob spawners.
	UPROPERTY(BlueprintReadWrite, Category = "Procedural Layout Generation|ProceduralRoom")
		TArray<AMobSpawner*> MobSpawners;

	/*
		Other significant objects within the room.

		NOTE: This array is meant to be filled separately from the others,
			  for discrimination between 'important other objects'
			  and other categories like geometry.
			  Please provide an override to PrepopulateInternalArrays()
			  in your subclass to add specific objects to this
			  array
	*/
	// 
	UPROPERTY(BlueprintReadWrite, Category = "Procedural Layout Generation|ProceduralRoom")
		TArray<USceneComponent*> OtherObjects;

	
	// Gets all connectors in this room whose paired LZs' ToZone variable is null.
	UFUNCTION(BlueprintCallable, Category = "Procedural Layout Generation|ProceduralRoom")
		void GetUnboundConnectors(TArray<UProceduralConnector*>& OutUnboundConnectors, bool ClearArray = true);

	/*
		Initializes the room for building.

		i.e. this populates the arrays:
			-Geometry
			-LoadingZoneComponents
			-LoadingZones
			-CameraGuideComponents
			-CameraGuides
			-MobSpawnerComponents
			-MobSpawners

		As well as listens for appropriate internal events among other initialization procedures.

		NOTE: You should not use rooms for proper operations before calling this, nor should you use
			  the CDO. Doing either of these things will cause glitches and/or crashes.
	*/
	UFUNCTION(BlueprintCallable, Category = "Procedural Layout Generation|ProceduralRoom")
		void Initialize();

	/*
		This is called at the beginning of Initialize() as a way
		to allow users to specify that certain objects already belong in certain arrays
		before the function takes it from there.
	*/
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Procedural Layout Generation|ProceduralRoom")
		void PrepopulateInternalArrays();
	void PrepopulateInternalArrays_Implementation() {}

};


/*
	Specifies room archetype data for AProceduralArea subclass instantiations.
*/
USTRUCT(BlueprintType)
struct FProceduralAreaRoomTypeData
{
	GENERATED_USTRUCT_BODY();

	// The type of room to be spawned
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural Layout Generation|ProceduralAreaRoomTypeData")
		TSubclassOf<AProceduralRoom> RoomType;

	/*
		The number of this type of room to put into the map.

		setting to 0 means none of this type of room spawns.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural Layout Generation|ProceduralAreaRoomTypeData", meta = (ClampMin = 0))
		int NumInstances = 0;
};


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAreaFinishedBuildNotify, class AProceduralArea*, FinishedArea);

/*
	This class contains the base functionality for
	procedural areas.
*/
UCLASS(Blueprintable)
class MPHORSO_API AProceduralArea : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AProceduralArea(const FObjectInitializer& _init);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	/*
		Is the overarcing ProceduralMap allowed to manipulate the
		positions of this area's rooms for the benefit of more
		even room distribution while connecting areas?
	*/
	UPROPERTY(EditDefaultsOnly, Category = "Procedural Layout Generation|ProceduralArea")
		bool IsMalleable = true;

	// All of the types of room to be used when this type of area is generated.
	UPROPERTY(EditDefaultsOnly, Category = "Procedural Layout Generation|ProceduralArea")
		TArray<FProceduralAreaRoomTypeData> RoomArchetypes;

	/*
		These room archetypes are considered "special" and are
		spawned before any others to guarantee their existence.

		Use this for storing rooms that represent things like
		essential landmarks and other important structures and
		geography.
	*/
	UPROPERTY(EditDefaultsOnly, Category = "Procedural Layout Generation|ProceduralArea")
		TArray<FProceduralAreaRoomTypeData> SpecialRoomArchetypes;

	/*
		The maximum number of times the area will try to place a
		failed room before it discards the room entirely.
	*/
	UPROPERTY(EditDefaultsOnly, Category = "Procedural Layout Generation|ProceduralArea")
		int MaxRoomPlacementAttempts = 10000;

	/*
		Areas that this area is willing to connecting to.

		Used by the overarcing ProceduralMap when manipulating
		pre-finalized areas to allow for and create connections
		between areas.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural Layout Generation|ProceduralArea")
		TArray<TSubclassOf<AProceduralArea>> ConnectableAreas;

	/*
		Contains all rooms which have been spawned.
	*/
	UPROPERTY(BlueprintReadWrite, Category = "Procedural Layout Generation|ProceduralArea")
		TArray<AProceduralRoom*> SpawnedRooms;

	/*
		Delegate called when building has finished and Iterate() will stop being called.
	*/
	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category = "Procedural Layout Generation|ProceduralArea")
		FAreaFinishedBuildNotify FinishedBuildingDelegate;

	/*
		This is imitated from the random stream reference we're given at the start of
		the build.
	*/
	UPROPERTY(BlueprintReadWrite, Category = "Procedural Layout Generation|ProceduralArea")
		FRandomStream RandStream;


	/*
		Initializes the area for building.

		i.e. Instantiates all of the room types present
			 in the RoomArchetypes array, as well as
			 populates the CurrentEnds array with all
			 of the given root rooms' unbound LZs.
	*/
	UFUNCTION(BlueprintCallable, Category = "Procedural Layout Generation|ProceduralArea")
		void Build(UPARAM(Ref) FRandomStream& RandomStream);

	/*
		Spawns all of the rooms specified within SpecialRoomArchetypes.

		called within Build().
	*/
	UFUNCTION(BlueprintCallable, Category = "Procedural Layout Generation|ProceduralArea")
		void SpawnSpecialRooms();

	/*
		Performs an iteration of the area's building algorithm.

		i.e. For every LZ in CurrentEnds, it finds an appropriate
			 room to bind given the current amount of rooms, current
			 depth of the rooms, the current LZ's preferences and needs,
			 and whether the area will have to connect to other areas.

		Returns false when there are no more rooms left to add and/or when all branches have been capped,
		i.e. the area is effectively finished.
	*/
	UFUNCTION(BlueprintCallable, Category = "Procedural Layout Generation|ProceduralArea")
		void Iterate();

	/*
		Spawns a room of the given archetype.

		returns false if the room has still not been spawned after the maximum number of attempts has been reached.
	*/
	UFUNCTION(BlueprintCallable, Category = "Procedural Layout Generation|ProceduralArea")
		bool SpawnRoom(FProceduralAreaRoomTypeData ArchetypeToSpawn, AProceduralRoom*& OutSpawned);

	/*
		This is called inside Iterate() while placing rooms.
	*/
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Procedural Layout Generation|ProceduralArea")
		void GetAreaRandomPoint(TSubclassOf<AProceduralRoom> RoomArchetype, FVector& OutRandomLoc, FRotator& OutRandomRot);
	void GetAreaRandomPoint_Implementation(TSubclassOf<AProceduralRoom> RoomArchetype, FVector& OutRandomLoc, FRotator& OutRandomRot) {}

	/*
		Called in Iterate() when all rooms have been spawned.

		Adjusts the position of this area to the average position
		of its spawned rooms, and then attaches them.
	*/
	UFUNCTION(BlueprintCallable, Category = "Procedural Layout Generation|ProceduralArea")
		void Fasten();

	/*
		Finalizes the area after generation and ProceduralMap
		manipulation has been completed.

		This function takes all of the rooms' unbound connectors
		and works out a way to connect them all with the fewest
		rooms unreachable, with the best and intended situation
		being that all rooms are reachable. Another side goal
		of the process is to create at least one loop, maybe.
		we'll have to see how often it just naturally makes
		loops before doing anything to make them more common.
	*/
	UFUNCTION(BlueprintCallable, Category = "Procedural Layout Generation|ProceduralArea")
		void Finalize();

private:

	/*
		The handle to the timer that calls Iterate().
	*/
	UPROPERTY()
		FTimerHandle IterationTimer;

};


/*
	TODO : ACTUALLY DEFINE THE WAY TO SAVE THESE DAMN MAPS SOMEHOW
*/

UCLASS(Blueprintable)
class MPHORSO_API AProceduralMap : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AProceduralMap(const FObjectInitializer& _init);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	
	/*
		All the areas to be created, spawned
		when Build() is called.
	*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Procedural Layout Generation|ProceduralMap")
		TArray<TSubclassOf<AProceduralArea>> Areas;

	/*
		The random stream for the entire map generation process.

		NOTE: this is exclusively the rand used for generation of
			  the map. other objects will have different rands.
	*/
	UPROPERTY(BlueprintReadWrite, Category = "Procedural Layout Generation|ProceduralArea")
		FRandomStream RandStream;


	/*
		Begins the map build cycle.
	*/
	UFUNCTION(BlueprintCallable, Category = "Procedural Layout Generation|ProceduralMap")
		void Build(int InitialRandStreamSeed = 0);


	UFUNCTION(BlueprintCallable, Category = "Procedural Layout Generation|ProceduralMap")
		void OnAreaFinishedBuilding(AProceduralArea* FinishedArea);

	/*
		Called in OnAreaFinishedBuilding() once
		all areas have finished building.

		Manipulates the positions of some rooms
		in 'malleable' areas to better support
		connection to other areas before adding
		non-areabound rooms and splineconnectors
		to finish the job.
	*/
	UFUNCTION(BlueprintCallable, Category = "Procedural Layout Generation|ProceduralMap")
		void PreFinalize();

private:

	TArray<AProceduralArea*> IteratingAreas;

	TArray<AProceduralArea*> FinishedAreas;

};
