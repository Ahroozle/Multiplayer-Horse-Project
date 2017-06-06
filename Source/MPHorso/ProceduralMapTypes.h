// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "Components/SplineComponent.h"
#include "Components/SplineMeshComponent.h"
#include "Components/ChildActorComponent.h"

#include "SimplexNoiseLibrary.h"

#include "ProceduralMapTypes.generated.h"


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
		This contains all of the indices
		that this meshdata was used to
		create within the containing
		splineconnector's SpawnedMeshComponents
		array, and is used to keep track of
		operations like removes and additions
		while maintaining order.
	*/
	UPROPERTY(BlueprintReadWrite, Category = "Procedural Layout Generation|SplineConnectorMeshData")
		TArray<int> Indices;
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


	/*
		The rootcomponent spline.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural Layout Generation|ProceduralSplineConnector")
		USplineComponent* RootSpline;

	/*
		Array of all the splinemeshcomponents
		that were spawned in OnConstruction().
	*/
	UPROPERTY(BlueprintReadWrite, Category = "Procedural Layout Generation|ProceduralSplineConnector")
		TArray<USplineMeshComponent*> SpawnedMeshComponents;

	/*
		The mesh data for the spline, in
		order from start of spline to end
		of spline, distributed evenly.
	*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Procedural Layout Generation|ProceduralSplineConnector")
		TArray<FSplineConnectorMeshData> MeshData;

	/*
		The array of mesh components that
		were instantiated in ConstructMeshes().
	*/
	UPROPERTY(BlueprintReadWrite, Category = "Procedural Layout Generation|ProceduralSplineConnector")
		TArray<class USplineMeshComponent*> CreatedMeshComponents;
	
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

	/*
		Constructs the initial SplineMeshComponents
		which make up the SplineConnector's visible
		and blocking geometry.

		The return is only there so that it registers as
		a function and not an event. It will always
		return true.
	*/
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Procedural Layout Generation|ProceduralSplineConnector")
		bool ConstructMeshes(UPARAM(Ref) FRandomStream& RandStream);
	bool ConstructMeshes_Implementation(UPARAM(Ref) FRandomStream& RandStream) { return true; }

	/*
		Updates the positions of all of the
		pre-created meshes, for use in the
		event that the spline's points change.

		The return is only there so that it registers as
		a function and not an event. It will always
		return true.
	*/
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Procedural Layout Generation|ProceduralSplineConnector")
		bool UpdateMeshes(UPARAM(Ref) FRandomStream& RandStream);
	bool UpdateMeshes_Implementation(UPARAM(Ref) FRandomStream& RandStream) { return true; }

	/*
		Used inside UpdateMeshes() during
		adding operations to distribute newly
		created splinemeshcomponents as evenly
		as possible.

		Gets all mesh type section indices within
		MeshData which are tied for the lowest number
		of indices.
	*/
	UFUNCTION(BlueprintCallable, Category = "Procedural Layout Generation|ProceduralSplineConnector")
		void GetSmallestMeshTypeSectionIndices(TArray<int>& OutSmallest);

	/*
		Used inside UpdateMeshes() during
		removal operations to cull meshes
		as evenly as possible.

		Gets all mesh type section indices within
		MeshData which are tied for the highest number
		of indices.
	*/
	UFUNCTION(BlueprintCallable, Category = "Procedural Layout Generation|ProceduralSplineConnector")
		void GetLargestMeshTypeSectionIndices(TArray<int>& OutLargest);

};


/*
	The ProceduralRoomDecor class is used to
	represent objects within a room that range
	in significance from decorational to actually
	mechanically usable within the space of play.
*/
UCLASS()
class MPHORSO_API AProceduralRoomDecor : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AProceduralRoomDecor(const FObjectInitializer& _init);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;


	/*
	This is imitated from the random stream reference we're given at the start of
	the build.
	*/
	UPROPERTY(BlueprintReadWrite, Category = "Procedural Layout Generation|ProceduralArea")
		FRandomStream RandStream;


	/*
		This function is called by the spawning
		ProceduralRoomDecorComponent after being
		spawned, to set up the decor to allow
		whatever actions it allows.

		This can be overridden on a per-subclass
		basis.
	*/
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Procedural Layout Generation|ProceduralRoomDecorComponent")
		void Initialize(UPARAM(Ref) FRandomStream& RandomStream);
	void Initialize_Implementation(UPARAM(Ref) FRandomStream& RandomStream) { RandStream = RandomStream; }

private:

};

/*
	The ProceduralRoomDecorComponent allows you
	to spawn random decor into rooms from a list
	of possible decor it could be.
*/
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class MPHORSO_API UProceduralRoomDecorComponent : public UChildActorComponent
{
	GENERATED_BODY()

public:

	/*
		The possible types of decor that suit this room,
		used whenever the owning room's RequiredDecor array
		is empty.
	*/
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Procedural Layout Generation|ProceduralRoomDecorComponent")
		TArray<TSubclassOf<AProceduralRoomDecor>> DecorTypes;

	/*
		Sets the child actor to the specified class.

		Called by the owning ProceduralRoom actor
		whenever its RequiredDecor array isn't
		empty.
	*/
	UFUNCTION(BlueprintCallable, Category = "Procedural Layout Generation|ProceduralRoomDecorComponent")
		void SpawnDecor(TSubclassOf<AProceduralRoomDecor> DecorType);

	/*
		The regular function that gets used.

		Called by the owning ProceduralRoom
		actor whenever its RequiredDecor array
		is empty.
	*/
	UFUNCTION(BlueprintCallable, Category = "Procedural Layout Generation|ProceduralRoomDecorComponent")
		void SpawnRandomDecor();

private:
	
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

	/*
		Decor which this room *requires* the existence
		of.

		If this array contains anything, the room goes
		through every piece of decor and picks a random,
		unoccupied UProceduralRoomDecorComponent on itself
		to spawn it at, stopping if there are no unoccupied
		components left.
	*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Procedural Layout Generation|ProceduralRoom")
		TArray<TSubclassOf<AProceduralRoomDecor>> RequiredDecor;

	// The room's geometry, visible or otherwise.
	UPROPERTY(BlueprintReadWrite, Category = "Procedural Layout Generation|ProceduralRoom")
		TArray<USceneComponent*> Geometry;

	// The room's connectors.
	UPROPERTY(BlueprintReadWrite, Category = "Procedural Layout Generation|ProceduralRoom")
		TArray<UProceduralConnector*> Connectors;

	// The room's decor components.
	UPROPERTY(BlueprintReadWrite, Category = "Procedural Layout Generation|ProceduralRoom")
		TArray<UProceduralRoomDecorComponent*> DecorComponents;

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

	/*
		This is imitated from the random stream reference we're given at the start of
		the build.
	*/
	UPROPERTY(BlueprintReadWrite, Category = "Procedural Layout Generation|ProceduralArea")
		FRandomStream RandStream;

	
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
		void Initialize(UPARAM(Ref) FRandomStream& RandomStream);

	/*
		This is called at the beginning of Initialize() as a way
		to allow users to specify that certain objects already belong in certain arrays
		before the function takes it from there.
	*/
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Procedural Layout Generation|ProceduralRoom")
		void PrepopulateInternalArrays();
	void PrepopulateInternalArrays_Implementation() {}

private:

	/*
		This function sets up all of the required
		and random decor for this room.

		Called from within Initialize().
	*/
	UFUNCTION()
		void PrepareDecor();

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
	
	*/
	UPROPERTY(EditDefaultsOnly, Category = "Procedural Layout Generation|ProceduralArea")
		TArray<TSubclassOf<AProceduralSplineConnector>> CorridorTypes;

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























// TESTING SOME SHIT UNDER HERE


//USTRUCT(BlueprintType)
//struct FWorldTile
//{
//	GENERATED_USTRUCT_BODY();
//
//
//	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural World Generation|WorldTile")
//		FVector Position;
//
//	// (-1)->1 value for how corrupt/pure the tile is.
//	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural World Generation|WorldTile")
//		float Purity;
//
//	// (-1)->1 value for how cold/hot the tile is.
//	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural World Generation|WorldTile")
//		float Temperature;
//
//	// 0->1 value for how rainy the average rainfall of this tile is. Also equals the chance that this tile will be rained upon.
//	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural World Generation|WorldTile")
//		float Rainfall;
//
//	// 0->1 value for how easy it is for water to escape this tile.
//	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural World Generation|WorldTile")
//		float Drainage;
//
//	// 0->1 value for how easy it is for plants to grow in this tile.
//	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural World Generation|WorldTile")
//		float Fertility;
//
//	// 0->1 value for how salty this tile is ( on a scale of Not Salty to League of Legends :^) )
//	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural World Generation|WorldTile")
//		float Salinity;
//};

USTRUCT(BlueprintType)
struct FSimplexOctaveSettings
{
	GENERATED_USTRUCT_BODY();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural World Generation|SimplexOctaveSettings")
		int Octaves;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural World Generation|SimplexOctaveSettings")
		float Persistence;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural World Generation|SimplexOctaveSettings")
		float NoiseScale;
};

USTRUCT()
struct FSimplexWorm
{
	GENERATED_USTRUCT_BODY();

	// The current position of the perlin worm in map coordinates.
	FVector Pos;

	// The current direction.
	FVector Dir = FVector(1,1,1);

	/*
		The speed of the worm.

		Adjusting this may cause the worm to create pockets instead
		of tunnels, or not even be able to move at all in some cases.
	*/
	float WormSpeed = 2;

	/*
		this scales the range of the noise to allow for
		the full range of angular shifting or even just
		a fraction.
	*/
	float AngleVariance = 1;

	// The octave settings for this worm.
	FSimplexOctaveSettings WormOctaveSettings;

	// The range of the worm's tunnel scooping size over its lifetime.
	FVector2D SizeRange;

	int CurrSize;

	int SetVal = 0;

	FRandomStream RandStream;

	void Init(const FIntVector& pos, FVector2D& sizeRange, const FSimplexOctaveSettings& OctSettings, const float& AngVariance, UPARAM(Ref) FRandomStream& RandomStream, const int& valToSet)
	{
		Pos = FVector(pos);
		WormOctaveSettings = OctSettings;
		AngleVariance = AngVariance;
		SizeRange = sizeRange;
		SetVal = valToSet;
		RandStream = RandomStream;
	}

	void Step(const float& currLifetime, const FVector& MapExtent)
	{
		// TODO : IMPL WORM STEP

		int octaves = WormOctaveSettings.Octaves;					// 3
		const float& persistence = WormOctaveSettings.Persistence;	// 0.5
		const float& NoiseScale = WormOctaveSettings.NoiseScale;	// 0.01

		FVector MapPoint = (FVector(Pos) - MapExtent) * NoiseScale;

		float frequency = 1, amplitude = 1;
		float AngA = 0, AngB = 0, AngC = 0;
		float MaxVal = 0;
		while (--octaves >= 0)
		{
			float PreA = USimplexNoiseLibrary::SimplexNoise2D(MapPoint.X * frequency, MapPoint.Y * frequency) * amplitude;
			float PreB = USimplexNoiseLibrary::SimplexNoise2D(MapPoint.Y * frequency, MapPoint.Z * frequency) * amplitude;
			float PreC = USimplexNoiseLibrary::SimplexNoise2D(MapPoint.Z * frequency, MapPoint.X * frequency) * amplitude;

			float SignA = FMath::Sign(PreA);
			float SignB = FMath::Sign(PreB);
			float SignC = FMath::Sign(PreC);

			if (SignA + SignB + SignC == 0)
			{
				AngA = Dir.X;
				AngB = Dir.Y;
				AngC = Dir.Z;

				MaxVal = 1;

				break;
			}


			AngA += PreA;
			AngB += PreB;
			AngC += PreC;

			MaxVal += amplitude;
			amplitude *= persistence;
			frequency *= 2;

			
		}
		//AngB = FMath::Acos(1 - (2 * ((AngB* AngleVariance) / MaxVal)));
		//AngA = ((AngA* AngleVariance) / MaxVal) * 2 * PI;
		AngA /= MaxVal;
		AngB /= MaxVal;
		AngC /= MaxVal;

		/*
			random point on sphere calc
			here's a ref http://corysimon.github.io/articles/uniformdistn-on-sphere/

			made sure that was what the OG worms algo was doing
			by finding this link https://stackoverflow.com/questions/9879258/how-can-i-generate-random-points-on-a-circles-circumference-in-javascript

			seems to check out so I'm using it.
		*/
		//Pos.X -= FMath::Sin(AngA) * FMath::Cos(AngB) * WormSpeed;
		//Pos.Y -= FMath::Sin(AngA) * FMath::Sin(AngB) * WormSpeed;
		//Pos.Z -= FMath::Cos(AngA) * WormSpeed;

		Pos.Z += FMath::Cos(AngA*1.9f*PI) * (RandStream.RandRange(0, 1) == 0 ? -1 : 1) * AngleVariance * WormSpeed;
		Pos.X += FMath::Sin(AngB*1.9f*PI) * (RandStream.RandRange(0, 1) == 0 ? -1 : 1) * AngleVariance * WormSpeed;
		Pos.Y += FMath::Cos(AngC*1.9f*PI) * (RandStream.RandRange(0, 1) == 0 ? -1 : 1) * AngleVariance * WormSpeed;

		Dir = FVector(AngA, AngB, AngC);

		CurrSize = FMath::GetMappedRangeValueClamped({ 0.0f, 1.0f }, { SizeRange.X, SizeRange.Y }, currLifetime);
	}

	void Tunnel(UPARAM(Ref) TArray<TArray<TArray<char>>>& Map)
	{
		int furthestOut = FMath::CeilToInt(CurrSize);

		for (int x = -furthestOut; x <= furthestOut; ++x)
		{
			for (int y = -furthestOut; y <= furthestOut; ++y)
			{
				for (int z = -furthestOut; z <= furthestOut; ++z)
				{
					FVector currInspected = Pos + FVector(x, y, z);
					currInspected.X = FMath::RoundToInt(FMath::Clamp(currInspected.X, 0.0f, (float)Map.Num()-1));
					currInspected.Y = FMath::RoundToInt(FMath::Clamp(currInspected.Y, 0.0f, (float)Map[0].Num()-1));
					currInspected.Z = FMath::RoundToInt(FMath::Clamp(currInspected.Z, 0.0f, (float)Map[0][0].Num()-1));

					if (/*0*/SetVal != Map[currInspected.X][currInspected.Y][currInspected.Z] &&
						CurrSize >= FVector::Dist(FVector(Pos), FVector(currInspected)))
					{
						Map[currInspected.X][currInspected.Y][currInspected.Z] = /*0*/SetVal;
					}
				}
			}
		}
	}

};

/*
	The map, in the end, will be made out of things as follows:

	- Topside Height Map (2D)
			This represents everything facing upward on the map.

	- Underside Height Map (2D)
			This represents everything facing downward on the map.

	- Falloff Map (2D)
			This represents where the edges of the map are. The
			further into the falloff the map gets, the closer to
			the cutoff threshold it's guided, until it inevitably
			passes it.

	- Steepness Map (2D)
			This is kind of just an ad-hoc map I'm using to make
			sure that there's always stretches of flat ground
			*somewhere* on the damn map after I realized that the
			generator literally makes nothing but mountains.
			Its sole purpose is to lower some parts of the top
			height map down enough to be hill or plain-like.

	- Offset Map (2D)
			This represents the Z offset of any given tile on the
			map, as a way to add some hilliness variance to the
			average height of the zero on the map, as well as
			add the possibility of smaller floating islands that
			are unattached from the map being at a different
			height than the main map.

	- Base Terrain Map (3D)
			This is interpreted from the combination of the
			Topside Height, Underside Height, Falloff, and
			Offset maps. Any terrain within the bounds specified
			by their intersection is considered solid.

	- Decay Map (3D)
			This is used on the underside to create that
			excellent breakage effect under the islands.
			The more decay that's allowed the more
			lower island gore there'll be.



	- Structure Map (3D)
			This is a map added at the very end which
			represents all of the post-generation
			structures on the map that have been
			placed. These are generally unnatural
			prefabs or places that are generally
			procedurally generated separate from
			the map and added in as a postprocess of
			sorts.

*/

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FGenFinishedNotify);

// currently not concerning myself with mem usage; will get back to that later if it's an issue.

UCLASS(Blueprintable)
class MPHORSO_API AProceduralWholeMap : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AProceduralWholeMap(const FObjectInitializer& _init);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	/*
		The random stream, used for basically everything so that maps can be saved efficiently
		with just a seed and some deltas.
	*/
	UPROPERTY(BlueprintReadWrite, Category = "Procedural World Generation|ProceduralWholeMap")
		FRandomStream RandStream;

	/*
		Sets how fast iterations are progressed
	*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Procedural World Generation|ProceduralWholeMap")
		float IterationTime = 0.01f;

	/*
		Tile X/Y/Z width, in UU.
	*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Procedural World Generation|ProceduralWholeMap")
		float TileSize = 100;

	/*
		The X width of the map in tiles
	*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Procedural World Generation|ProceduralWholeMap")
		int MapWidth;

	/*
		the Y width of the map in tiles
	*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Procedural World Generation|ProceduralWholeMap")
		int MapHeight;

	/*
		The maximum >0 Z height of the map in tiles.

		This value gets applied to the Topside height
		map.
	*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Procedural World Generation|ProceduralWholeMap", meta = (ClampMin = "1"))
		int TopMaxDepth;

	// the thickness of the center slice
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Procedural World Generation|ProceduralWholeMap", meta = (ClampMin = "1"))
		int EquatorThickness = 5;

	/*
		The maximum <0 Z height of the map in tiles.

		This value gets applied to the Underside
		height map.
	*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Procedural World Generation|ProceduralWholeMap", meta = (ClampMin = "1"))
		int UnderMaxDepth;

	// stored result of TopMaxDepth + UnderMaxDepth;
	UPROPERTY(BlueprintReadOnly, Category = "Procedural World Generation|ProceduralWholeMap")
		int FullDepth;

	// Half the bounds of the map, stored for wide usage.
	UPROPERTY(BlueprintReadOnly, Category = "Procedural World Generation|ProceduralWholeMap")
		FVector HalfBounds;

	// currently being used for debug; contains all points which are solid after all ops.
	UPROPERTY(BlueprintReadWrite, Category = "Procedural World Generation|ProceduralWholeMap")
		TArray<FVector> SolidPoints;

	/*
		The maximum Z offset the OffsMap is allowed
		to reach, measured as a 0->1 ratio from 
		0->TopMaxDepth.
	*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Procedural World Generation|ProceduralWholeMap", meta = (ClampMin = "0.0", ClampMax = "1.0"))
		float MaxOffs;

	/*
		The maximum Z offset the OffsMap is allowed
		to reach, measured as a (-1)->0 ratio from
		UnderMaxDepth->0.
	*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Procedural World Generation|ProceduralWholeMap", meta = (ClampMin = "-1.0", ClampMax = "0.0"))
		float MinOffs;

	/*
		This represents the (-1)->1 height on the
		Topside Heightmap at which the tile is
		considered a hole.

		at 1 this will erase the entire map.

		at -1 this will leave little to no holes except
		for at the edges of the map, possibly.
	*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Procedural World Generation|ProceduralWholeMap", meta = (ClampMin = "-1.0", ClampMax = "1.0"))
		float HeightMapCutoffThreshold;

	/*
		This represents the 0->1 threshold
		which differentiates flat sections
		from mountains on the Topside
		Heightmap.

		any heights less than this are clamped
		down to 0 while higher ones are allowed
		to persist.
	*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Procedural World Generation|ProceduralWholeMap", meta = (ClampMin = "0.0", ClampMax = "1.0"))
		float SteepnessThreshold = 0.5f;

	/*
		This has some weird effects on mountains
		and is connected to steepness calculations.
	*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Procedural World Generation|ProceduralWholeMap")
		float MountainScale = 3.0f;

	/*
		This represents the maximum variance allowed in island wear and tear,
		and scales the decay value into it.
	*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Procedural World Generation|ProceduralWholeMap", meta = (ClampMin = "0.0"))
		float MaxDecayVariance = 1.0f;

	/*
		The minimum number of worms created for tunnelling.
	*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Procedural World Generation|ProceduralWholeMap", meta = (ClampMin = "0.0"))
		int MinNumWorms = 25;

	/*
		The maximum number of worms created for tunnelling.
	*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Procedural World Generation|ProceduralWholeMap", meta = (ClampMin = "0.0"))
		int MaxNumWorms = 50;

	/*
		The minimum number of iterations the tunnelling worms
		will go before they'll begin to stop.
	*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Procedural World Generation|ProceduralWholeMap", meta = (ClampMin = "0.0"))
		int MinWormIterations = 50;

	/*
		The maximum number of iterations the tunnelling worms
		will go before they are guaranteed to stop.
	*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Procedural World Generation|ProceduralWholeMap", meta = (ClampMin = "0.0"))
		int MaxWormIterations = 100;

	/*
		The range of minimum radii a worm's
		tunnel can be.
	*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Procedural World Generation|ProceduralWholeMap", meta = (ClampMin = "0.0"))
		FVector2D MinWormSizes;

	/*
		The range of maximum radii a worm's
		tunnel can be.
	*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Procedural World Generation|ProceduralWholeMap", meta = (ClampMin = "0.0"))
		FVector2D MaxWormSizes;

	/*
		Determines the "curviness" of caves maybe.

		0 basically means the worms will *always* go straight
		1 is the maximum curviness of the worm's wander.
	*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Procedural World Generation|ProceduralWholeMap", meta = (ClampMin = "0.0", ClampMax = "1.0"))
		float WormAngleVariance = 1.0f;


	/*
		Octave settings for the default
		implementations of their respective
		Determination functions.

		I encourage reuse of these if you're
		implementing your own version of the
		functionality and you still need
		octave settings.
	*/

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Procedural World Generation|ProceduralWholeMap")
		FSimplexOctaveSettings SteepnessMapOctaveSettings = { 16, 0.75f, 0.005f };

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Procedural World Generation|ProceduralWholeMap")
		FSimplexOctaveSettings TopHeightMapOctaveSettings = { 8, 0.5f, 0.01f };

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Procedural World Generation|ProceduralWholeMap")
		FSimplexOctaveSettings BottomHeightMapOctaveSettings = { 4, 2.0f, 0.0025f };

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Procedural World Generation|ProceduralWholeMap")
		FSimplexOctaveSettings OffsetMapOctaveSettings = { 8, 0.5f, 0.0015f };

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Procedural World Generation|ProceduralWholeMap")
		FSimplexOctaveSettings DecayMapOctaveSettings = { 16, 0.8f, 0.01f };

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Procedural World Generation|ProceduralWholeMap")
		FSimplexOctaveSettings WormOctaveSettings = { 3, 0.5f, 0.01f };


	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category = "Procedural World Generation|ProceduralWholeMap")
		FGenFinishedNotify GenerationFinishedDelegate;


	/*
		Begins generation of the map.
	*/
	UFUNCTION(BlueprintCallable, Category = "Procedural World Generation|ProceduralWholeMap")
		void Build(int InitialRandSeed = 0);

	/*
		Iterates the world generation by one piece per frame.
		Also used to allow for progress to be shown.
	*/
	UFUNCTION(BlueprintCallable, Category = "Procedural World Generation|ProceduralWholeMap")
		void Iterate();

	/*
		functions which determine how each of the
		individual piecemeal maps are constructed.
	*/

	UFUNCTION(BlueprintNativeEvent, Category = "Procedural World Generation|ProceduralWholeMap")
		float DetermineFalloff(const FVector& CurrPoint, const FVector& MapExtent);
	float DetermineFalloff_Implementation(const FVector& CurrPoint, const FVector& MapExtent);

	UFUNCTION(BlueprintNativeEvent, Category = "Procedural World Generation|ProceduralWholeMap")
		float DetermineSteepness(const FVector& CurrPoint, const FVector& MapExtent);
	float DetermineSteepness_Implementation(const FVector& CurrPoint, const FVector& MapExtent);

	UFUNCTION(BlueprintNativeEvent, Category = "Procedural World Generation|ProceduralWholeMap")
		float DetermineTopHeight(const FVector& CurrPoint, const FVector& MapExtent);
	float DetermineTopHeight_Implementation(const FVector& CurrPoint, const FVector& MapExtent);

	UFUNCTION(BlueprintNativeEvent, Category = "Procedural World Generation|ProceduralWholeMap")
		float DetermineUndersideHeight(const FVector& CurrPoint, const FVector& MapExtent);
	float DetermineUndersideHeight_Implementation(const FVector& CurrPoint, const FVector& MapExtent);

	UFUNCTION(BlueprintNativeEvent, Category = "Procedural World Generation|ProceduralWholeMap")
		float DetermineOffset(const FVector& CurrPoint, const FVector& MapExtent);
	float DetermineOffset_Implementation(const FVector& CurrPoint, const FVector& MapExtent);

	UFUNCTION(BlueprintNativeEvent, Category = "Procedural World Generation|ProceduralWholeMap")
		float DetermineDecay(const FVector& CurrPoint, const FVector& MapExtent);
	float DetermineDecay_Implementation(const FVector& CurrPoint, const FVector& MapExtent);


protected:

	UPROPERTY(BlueprintReadWrite, Category = "Procedural World Generation|ProceduralWholeMap")
		UTexture2D* TopHeightMapColorTexture;
	UPROPERTY(BlueprintReadWrite, Category = "Procedural World Generation|ProceduralWholeMap")
		UTexture2D* BottomHeightMapColorTexture;

	UPROPERTY(BlueprintReadWrite, Category = "Procedural World Generation|ProceduralWholeMap")
		UTexture2D* TopHeightMapTexture;
	UPROPERTY(BlueprintReadWrite, Category = "Procedural World Generation|ProceduralWholeMap")
		UTexture2D* BottomHeightMapTexture;

	UPROPERTY(BlueprintReadWrite, Category = "Procedural World Generation|ProceduralWholeMap")
		UTexture2D* FalloffMapTexture;

	UPROPERTY(BlueprintReadWrite, Category = "Procedural World Generation|ProceduralWholeMap")
		UTexture2D* OffsetMapTexture;

	UPROPERTY(BlueprintReadWrite, Category = "Procedural World Generation|ProceduralWholeMap")
		UTexture2D* SteepnessMapTexture;

	UPROPERTY(BlueprintReadWrite, Category = "Procedural World Generation|ProceduralWholeMap")
		TArray<UTexture2D*> BaseTerrainMapSlices;


	// MIP data pointers for the textures above.

	FColor* TopColorMipData;
	FColor* BotColorMipData;
	FColor* TopMipData;
	FColor* BotMipData;
	FColor* FallMipData;
	FColor* OffsMipData;
	FColor* SteepMipData;


	UFUNCTION(BlueprintCallable, Category = "Procedural World Generation|ProceduralWholeMap")
		void GetTerrainIsosurface(UPARAM(Ref) class UProceduralMeshComponent*& ProcMeshComp);

private:

	UPROPERTY()
		FTimerHandle IterationTimerHandle;

	// The "iterator" of sorts that goes over every section
	// of the map one at a time.
	FIntVector MapIterator = FIntVector::ZeroValue;

	// percent completion of each step

	int TerrainMapProgress		= -1;

	int WormProgress			= -1;


	/*
		Maps for keeping the data during
		generation. These get dealloc'd
		once paged into chunks for the
		session
	*/

	/*
		The main terrain map that takes
		the brunt of the operations.
	*/
	TArray<TArray<TArray<char>>> TerrainMap;

	/*
		Starts for perlin worms, chosen
		randomly out of the lowest and highest
		surfaces.

		The perlin worms I use to dig tunnels!

		These are placed randomly at top and
		underside height map locations first
		and then iterated through until they've
		all reached their limit.
	*/
	TArray<FSimplexWorm> Worms;

	int OGNumWorms;
	int currWormIterations; // current iteration all worms are on.


	/*
		marching cubes impl

		returns false if it didn't make any new tris
	*/
	bool March(const FVector(&CubeVerts)[8], const char(&CubeVals)[8], TArray<FVector>& OutVerts);
};

/*
	TODO : Perlin worms for tunnels!
	https://www.youtube.com/watch?v=qxAHrLZ3COY

	also wire up bridges with some kind of weighted point system?
	use elevation changes to find where places don't meet up?
	or a flood fill? idk rn rofl
*/

























// TRYIN EVEN *MORE* STUFF DOWN HERE

// Edge struct type, used during delaunay and MST calcs
USTRUCT(BlueprintType)
struct FSpanEdge
{
	GENERATED_USTRUCT_BODY();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FVector A;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FVector B;

	FSpanEdge(){}

	FSpanEdge(FVector a, FVector b) : A(a), B(b) {}

	bool operator==(const FSpanEdge& o) const { return (A == o.A && B == o.B) || (B == o.A && A == o.B); }
};

// triangle struct type, used during delaunay and MST calcs
USTRUCT(BlueprintType)
struct FSpanTri
{
	GENERATED_USTRUCT_BODY();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FVector A;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FVector B;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FVector C;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FSpanEdge AB;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FSpanEdge BC;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FSpanEdge CA;

	FSpanTri() {}

	FSpanTri(FVector a, FVector b, FVector c)
		: A(a), B(b), C(c), AB(a, b), BC(b, c), CA(c, a) {}

	bool HasVert(FVector vert) const { return (A == vert || B == vert || C == vert); }

	bool HasEdge(const FSpanEdge& edge) const { return (AB == edge || BC == edge || CA == edge); }

	void GetCircumcircle(FVector& outCircumcenter, float& outCircumradius) const
	{
		FVector ab = FVector(FVector2D(B) - FVector2D(A), 0);
		FVector bc = FVector(FVector2D(C) - FVector2D(B), 0);
		FVector ca = FVector(FVector2D(A) - FVector2D(C), 0);

		float s = (ab.Size() + bc.Size() + ca.Size()) / 2;
		float KSq = s * (s - ab.Size())*(s - bc.Size())*(s - ca.Size());

		outCircumcenter = (0.5f * (A + C)) + ((FVector::DotProduct(ab, bc) / (8 * KSq))*FVector::CrossProduct(ca, FVector::CrossProduct(ab, bc)));
		outCircumradius = (ab.Size() * bc.Size() * ca.Size()) / (4 * FMath::Sqrt(KSq));
	}

	bool CircumContains(const FVector& Point) const
	{
		FVector Circumcenter;
		float Circumradius;

		GetCircumcircle(Circumcenter, Circumradius);

		return ((Point - FVector(0, 0, Point.Z)) - Circumcenter).Size() <= Circumradius;
	}

	bool operator==(const FSpanTri& o) const { return (A == o.A || A == o.B || A == o.C ) &&
													  (B == o.A || B == o.B || B == o.C ) &&
													  (C == o.A || C == o.B || C == o.C ); }
};

// tetrahedron struct type, used during delaunay and MST calcs
USTRUCT(BlueprintType)
struct FSpanTetra
{
	GENERATED_USTRUCT_BODY();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FVector A;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FVector B;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FVector C;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FVector D;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FSpanTri ABC;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FSpanTri ABD;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FSpanTri BCD;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FSpanTri ACD;

	FSpanTetra() {}

	FSpanTetra(FVector a, FVector b, FVector c, FVector d)
		: A(a), B(b), C(c), D(d), ABC(a, b, c), ABD(a, b, d), BCD(b, c, d), ACD(a, c, d) {}

	bool HasVert(FVector vert) const { return (A == vert || B == vert || C == vert || D == vert); }

	bool CircumContains(FVector Vert) const
	{
		// Finding the circumcenter of tetrahedron ABCD via the normals and circumcenters of ABC and BCD

		// getting the normals of ABC and BCD early
		FVector NormalABC = FVector::CrossProduct((C - A).GetSafeNormal(), (B - A).GetSafeNormal());
		FVector NormalBCD = FVector::CrossProduct((C - B).GetSafeNormal(), (D - B).GetSafeNormal());


		// getting the circumcenters of ABC and BCD
		// formulas ripped from https://www.google.com/url?sa=t&rct=j&q=&esrc=s&source=web&cd=1&ved=0ahUKEwjP3_WFlJfUAhViwlQKHfZqCxcQFggmMAA&url=http%3A%2F%2Fwww.kurtnalty.com%2FTriangle&usg=AFQjCNEmZ9PvvbjT0pVq1EYHJADvjIqJpA
		// (pdf format)

		FVector CircumcenterABC, CircumcenterBCD;
		float CircumradiusABC, CircumradiusBCD;

		ABC.GetCircumcircle(CircumcenterABC, CircumradiusABC);
		BCD.GetCircumcircle(CircumcenterBCD, CircumradiusBCD);

		NormalABC *= CircumradiusABC * 100000;
		NormalBCD *= CircumradiusBCD * 100000;


		FVector TetraCircumcenter;

		{
			// line intersection, grabbed from https://answers.unrealengine.com/questions/363361/intersection-between-two-lines.html

			FVector da = CircumcenterBCD - CircumcenterABC;
			FVector db = (CircumcenterBCD + NormalBCD) - (CircumcenterABC + NormalABC);
			FVector dc = NormalABC;

			FVector crossDaDb = FVector::CrossProduct(da, db);
			float prod = FVector::DotProduct(crossDaDb, crossDaDb);

			float res = FVector::DotProduct(FVector::CrossProduct(dc, db), FVector::CrossProduct(da, db) / prod);
			TetraCircumcenter = CircumcenterABC + da * FVector(res, res, res);
		}

		float TetraCircumradius = (A - TetraCircumcenter).Size();


		// and now we can *finally* do the god damn check

		return (Vert - TetraCircumcenter).Size() <= TetraCircumradius;

	}

	bool operator==(const FSpanTetra& o) const { return (A == o.A || A == o.B || A == o.C || A == o.D) &&
														(B == o.A || B == o.B || B == o.C || B == o.D) &&
														(C == o.A || C == o.B || C == o.C || C == o.D) &&
														(D == o.A || D == o.B || D == o.C || D == o.D); }
};

// point returned from some delaunay/MST calcs, usually specifies how many connections feed into this point.
USTRUCT(BlueprintType)
struct FRankedPoint
{
	GENERATED_USTRUCT_BODY();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FVector Point;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float Rank = 1;

};

// structure created when obtaining a voronoi graph from a delaunay triangulation
USTRUCT(BlueprintType)
struct FSpanPoly
{
	GENERATED_USTRUCT_BODY();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TArray<FSpanEdge> Sides;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FVector Site;

	FSpanPoly() {}

	FSpanPoly(FVector site) : Site(site) {}

};

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
		static void ConnectPoints(UObject* WorldContext, const TArray<FVector>& InPoints, TArray<FSpanEdge>& OutEdges, TArray<FRankedPoint>& OutRankedPoints);

	UFUNCTION(BlueprintCallable)
		static void Delaunay3D(UObject* WorldContext, const TArray<FVector>& Points, TArray<FSpanTetra>& Tetras, TArray<FSpanEdge>& Edges);

	UFUNCTION(BlueprintCallable)
		static void Delaunay2D(UObject* WorldContext, const TArray<FVector>& Points, TArray<FSpanTri>& Tris, TArray<FSpanEdge>& Edges);

	// attempted impl of kruskal's greedy MST algorithm
	UFUNCTION(BlueprintCallable)
		static void KruskalMST(const TArray<FVector>& Points, UPARAM(Ref) TArray<FSpanEdge>& InOutEdges);

	UFUNCTION(BlueprintCallable)
		static void Delaunay2DToVoronoi2D(UObject* WorldContext, const TArray<FSpanTri>& InDelaunay, TArray<FSpanPoly>& OutPolygons, TArray<FSpanEdge>& OutEdges, float MaxCircumcenterDistance = 0);

	// Flood fills cells until it can gather no more and returns the result
	//UFUNCTION(BlueprintCallable)
	//	static void FloodFillAmong(UWorldCell* StartingCell, const TArray<class UWorldCell*>& BoundsArray, TArray<class UWorldCell*>& OutFloodFill);

	UFUNCTION(BlueprintCallable)
		static void FloodFillIfHasAttribute(UWorldCell* StartingCell, FName AttributeName, TArray<class UWorldCell*>& OutFloodFill);

private:

	static FRandomStream RandStream;

};


#pragma region Biome Classes

/*
	Generic struct for environmental
	data, manipulated during passes
	world generation passes.
*/
USTRUCT(BlueprintType)
struct FWorldGenEnvironmentData
{
	GENERATED_USTRUCT_BODY();

	/*
		Environmental data values by name.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TMap<FName, float> InternalMap;
};

/*
	rather naive N-Dimensional Box implementation,
	used for determining 
*/
USTRUCT(BlueprintType)
struct FBiomeBox
{
	GENERATED_USTRUCT_BODY();

	/*
		Name of the biome this box represents.

		If this is none then it is expected that
		this box actually leads to a subtable.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FName BiomeName;

	/*
		Name of the subtable that this
		box points to, if any.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FName TableName;

	/*
		N-dimensional ranges which this
		box occupies.

		Should be the same size as the
		TableAxes array in its respective
		Biome Table. If there are less
		axes then they are just assumed to
		be zeros, while any axes above the
		amount required are ignored.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TArray<FVector2D> AxisRanges;


	bool Contains(const TArray<float>& InAxisData) const
	{
		for (int currInd = 0; currInd < InAxisData.Num(); ++currInd)
		{
			const float& currAxisData = InAxisData[currInd];
			const FVector2D& InAxisRange = AxisRanges[currInd];

			if (!FMath::IsWithinInclusive(currAxisData, InAxisRange.X, InAxisRange.Y))
				return false;
		}
		return true;
	}
};

USTRUCT(BlueprintType)
struct FBiomeTable
{
	GENERATED_USTRUCT_BODY();

	// The axis attributes used by this table.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TArray<FName> TableAxes;


	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TArray<FBiomeBox> TableBoxes;

	bool GetOverlappingBox(const TMap<FName, float>& InData, FBiomeBox& OutFound) const
	{
		TArray<float> RelevantAxisData;
		for (auto &currAxis : TableAxes)
		{
			const float* currGrabbed = InData.Find(currAxis);

			if (nullptr != currGrabbed)
				RelevantAxisData.Add(*currGrabbed);
			else
				RelevantAxisData.Add(0); // any axis data that doesn't exist is just assumed to be its default.
		}

		TArray<FBiomeBox> WorkingBoxes = TableBoxes.FilterByPredicate(
																		[&RelevantAxisData](const FBiomeBox& a)
																		{
																			return a.Contains(RelevantAxisData);
																		}
																	 );

		if (WorkingBoxes.Num() < 1)
			return false;

		// if there's more than one box that works it just chooses randomly.
		OutFound = WorkingBoxes[UWorldGenFuncLib::GetWorldRandom().RandRange(0, WorkingBoxes.Num() - 1)];

		return true;
	}
};

/*
	Biome map class!

	This class attempts to capture a multiple-layered
	'table' of values based on attributes expected
	to be on a cell, and uses them to determine a
	cell's biome from its data.
*/
UCLASS(Blueprintable)
class MPHORSO_API UBiomeMap : public UObject
{
	GENERATED_BODY()

public:

	// The biome table always used as the root by GetBiome()

	FBiomeTable StartingTable;

	// all of the tables stemming off from the starting table and further down.
	TMap<FName, FBiomeTable> BiomeSubtables;

	/*
		Get the biome associated with the current
		set of environmental data values.

		Returns NONE when no biome was found.
	*/
	FName GetBiome(const FWorldGenEnvironmentData& InEnvironmentalData, TMap<FName, FVector2D>& OutBiomeRanges);

private:
	
	// Recursive function used inside GetBiome().
	FName GetBiomeFromTable(const FBiomeTable& Table, const TMap<FName, float>& InEnvironmentalData, TMap<FName, FVector2D>& OutBiomeRanges);

};

#pragma endregion


/*
	Structure containing voronoi cell data
	for world generation classes.
*/
USTRUCT(BlueprintType)
struct FWorldGenVoronoiCellData
{
	GENERATED_USTRUCT_BODY();

	// The vertices of the region shape
	UPROPERTY(BlueprintReadWrite)
		TArray<FVector> ShapeData_Verts;

	// The edges of the region shape
	UPROPERTY(BlueprintReadWrite)
		TArray<FSpanEdge> ShapeData_Edges;

	// The voronoi centerpoint of the region shape.
	UPROPERTY(BlueprintReadWrite)
		FVector ShapeData_Center;

	// The environmental data for this cell
	UPROPERTY(BlueprintReadWrite)
		FWorldGenEnvironmentData EnvData;

	/*
		The biome of this cell, for use looking up relevant data in dictionaries.

		NOTE: Cells which represent holes should be tagged as 'HOLE' on their biome.
			  This is what the generation function is looking for when determining
			  what is a hole and what is not a hole.
	*/
	UPROPERTY(BlueprintReadWrite)
		FName Biome;

	/*
		The environmental data value
		ranges within which this cell
		will still remain the current
		biome.

		Every time the cell is updated,
		its new environmental data values
		are compared to the values within
		this map, and if it's outside any
		of them now its biome is recalculated.
	*/
	UPROPERTY(BlueprintReadWrite)
		TMap<FName, FVector2D> BiomeRanges;

	bool BoundsFound = false;
	FVector MinBounds;
	FVector MaxBounds;
	void GetBounds(FVector& OutMin, FVector& OutMax)
	{
		if (!BoundsFound)
		{
			MinBounds = FVector(100000000, 100000000, 100000000);
			MaxBounds = FVector(-100000000, -100000000, -100000000);
			for (auto &curr : ShapeData_Verts)
			{
				if (curr.X < MinBounds.X)
					MinBounds.X = curr.X;
				if (curr.Y < MinBounds.Y)
					MinBounds.Y = curr.Y;

				if (curr.X > MaxBounds.X)
					MaxBounds.X = curr.X;
				if (curr.Y > MaxBounds.Y)
					MaxBounds.Y = curr.Y;
			}
			BoundsFound = true;
		}

		OutMin = MinBounds;
		OutMax = MaxBounds;
	}

	bool FoundAvgPt = false;
	FVector AvgPt;
	bool ContainsPoint(FVector Pt)
	{
		if (!FoundAvgPt)
		{
			AvgPt = FVector::ZeroVector;
			for (auto &curr : ShapeData_Verts)
				AvgPt += curr;
			AvgPt /= ShapeData_Verts.Num();
		}

		float PtToAvgDist = (AvgPt - Pt).Size2D();

		for (auto &curr : ShapeData_Verts)
		{
			if ((AvgPt - curr).Size() < PtToAvgDist)
				return false;
		}

		return true;
	}
};

/*
	class that represents a single voronoi cell of the world map.
*/
UCLASS(BlueprintType)
class MPHORSO_API UWorldCell : public UObject
{
	GENERATED_BODY()

public:

	// The voronoi data for this world cell.
	UPROPERTY(BlueprintReadWrite, Category = "Procedural World Generation|World Cell")
		FWorldGenVoronoiCellData CellData;

	// All cells that share an edge with this one
	UPROPERTY(BlueprintReadWrite, Category = "Procedural World Generation|World Cell")
		TArray<UWorldCell*> NeighborCells;

private:

};

/*
	This is an extra step in between continents and
	cells used to segregate biomes from one another.
*/
UCLASS(BlueprintType)
class MPHORSO_API UWorldRegion : public UObject
{
	GENERATED_BODY()

public:

	// The world cells that make up this world region.
	UPROPERTY(BlueprintReadWrite, Category = "Procedural World Generation|World Continent")
		TArray<UWorldCell*> Cells;

private:

};

/*
	class that represents a single "continent" in the world
	i.e. a neighborhood of connected world cells.

	Separate islands are separate "continents."
*/
UCLASS(BlueprintType)
class MPHORSO_API UWorldContinent : public UObject
{
	GENERATED_BODY()

public:

	// The world regions which make up this continent
	UPROPERTY(BlueprintReadWrite, Category = "Procedural World Generation|World Continent")
		TArray<UWorldRegion*> Regions;

	/*
		The biomes of the map, by color.
	*/
	UPROPERTY(BlueprintReadWrite, Category = "Procedural World Generation|World Continent")
		TArray<FColor> BiomeMap;

	/*
		The regions of the map, colored by "index"
		into an array of names specific to the
		continent.
	*/
	UPROPERTY(BlueprintReadWrite, Category = "Procedural World Generation|World Continent")
		TArray<FColor> RegionMap;

	/*
		The generic attribute maps.
	*/
	TMap<FName, TArray<FColor>> AttributeMaps;

	/*
		Whether or not this continent is "dead."

		Dead continents receive no updates.
	*/
	UPROPERTY(BlueprintReadWrite, Category = "Procedural World Generation|World Continent")
		bool IsDead = false;


	UFUNCTION(BlueprintCallable, Category = "Procedural World Generation|World Continent")
		bool GetAttributeMap(FName Attribute, TArray<FColor>& OutAttributeMap)
		{
			TArray<FColor>* Attr = AttributeMaps.Find(Attribute);

			if (nullptr != Attr)
			{
				OutAttributeMap = *Attr;
				return true;
			}

			return false;
		}

private:

};

/*
	Class used to store the unit-unbound world
	data.

	Contains redundant data in multiple degrees
	of granularity for ease of manipulation.
*/
UCLASS(BlueprintType)
class MPHORSO_API UWorldMap : public UObject
{
	GENERATED_BODY()

public:

	// The world cells that make up the entire map.
	UPROPERTY(BlueprintReadWrite, Category = "Procedural World Generation|World Map")
		TArray<UWorldCell*> WorldCells;

	// The world regions/biomes, for biome-wide operatopns.
	UPROPERTY(BlueprintReadWrite, Category = "Procedural World Generation|World Map")
		TArray<UWorldRegion*> WorldRegions;

	// The world cells' continents, for continent-wide operations.
	UPROPERTY(BlueprintReadWrite, Category = "Procedural World Generation|World Map")
		TArray<UWorldContinent*> WorldContinents;

	/*
		Additional specific information arrays
	*/

	// The outer edges of the island
	UPROPERTY(BlueprintReadWrite, Category = "Procedural World Generation|World Map")
		TArray<UWorldCell*> IslandEdges;

	// All edges of islands around inner holes
	UPROPERTY(BlueprintReadWrite, Category = "Procedural World Generation|World Map")
		TArray<UWorldCell*> InnerIslandEdges;

	// The coasts of all normal lakes on the island.
		TArray<TArray<UWorldCell*>> NormalLakeCoasts;

	// The coasts of all hanging lakes on the island.
		TArray<TArray<UWorldCell*>> HangingLakeCoasts;

	UFUNCTION(BlueprintPure, Category = "Procedural World Generation|World Map")
		int GetNumNormalLakeCoasts() { return NormalLakeCoasts.Num(); }
	UFUNCTION(BlueprintPure, Category = "Procedural World Generation|World Map")
		TArray<UWorldCell*>& GetNormalLakeCoast(int index) { return NormalLakeCoasts[index]; }

	UFUNCTION(BlueprintPure, Category = "Procedural World Generation|World Map")
		int GetNumHangingLakeCoasts() { return HangingLakeCoasts.Num(); }
	UFUNCTION(BlueprintPure, Category = "Procedural World Generation|World Map")
		TArray<UWorldCell*>& GetHangingLakeCoast(int index) { return HangingLakeCoasts[index]; }

private:

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

	/*
		Initializes any internal variables this function to perform its ops.
	*/
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Procedural World Generation|World Shape Function")
		void Initialize();
	virtual void Initialize_Implementation() {}

	/*
		The actual function which determines whether a cell is ground or not.
	*/
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Procedural World Generation|World Shape Function")
		bool CellIsGround(UWorldCell* Cell);
	virtual bool CellIsGround_Implementation(UWorldCell* Cell) { return false; }

	void DetermineShape(UPARAM(Ref) TArray<UWorldCell*>& InOutWorldCells);

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

	/*
		The applying function of the pass

		NOTE: boolean return is just to make it
			a function and not an event blueprint-side!
			the return value is ignored.
	*/
	UFUNCTION(BlueprintNativeEvent, Category = "Procedural World Generation|World Generation Pass")
		bool Apply(UWorldMap* InWorldMap);
	virtual bool Apply_Implementation(UWorldMap* InWorldMap) { return false; }

private:

};

/*
	struct for abstracting island-specific generation
	settings into a convenient format.
*/
USTRUCT(BlueprintType)
struct FWorldGenIslandSettings
{
	GENERATED_USTRUCT_BODY();

	/*
		Whether this continent is generated
		as a dead continent or not.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural World Generation|WorldGen Island Settings")
		bool IsDead = false;

	/*
		The dimensions of this island, in tiles.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural World Generation|WorldGen Island Settings")
		FIntVector BoxDimensions;

	/*
		The shaping function for this island.

		This determines which parts of the island
		are considered ground and which parts are
		to be evaluated by later passes to be
		considered either holes or bodies of
		water.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural World Generation|WorldGen Island Settings")
		TSubclassOf<UWorldShapeFunction> ShapeFunction;

	/*
		The chance that an internal hole in the island will instead become an
		internal lake.

		0 means no lakes ever, while 1 guarantees that all internal holes will
		become lakes.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural World Generation|WorldGen Island Settings", meta = (ClampMin = "0.0", ClampMax = "1.0"))
		float LakeChance = 0.75f;

	/*
		The chance that a generated lake
		will actually be a hanging lake
		(i.e. no ground underneath).
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural World Generation|WorldGen Island Settings", meta = (ClampMin = "0.0", ClampMax = "1.0"))
		float HangingLakeChance = 0.25;

	/*
		The number of sampling points used to
		create this island's voronoi diagram.

		the more points, the more fine the
		shape of this island will become.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural World Generation|WorldGen Island Settings")
		int NumSamplingPoints = 1000;
};

/*
	World generation settings struct, abstracted
	from the main class itself to afford the ability
	to slot settings in and out for convenience and
	expansion of capability.
*/
USTRUCT(BlueprintType)
struct FWorldGenerationSettings
{
	GENERATED_USTRUCT_BODY();

	/*
		The bounds of the ellipsoid within
		which island centers *must* be.
		X value is for X/Y distance while Y
		value is for Z distance.

		Their actual positions get rounded to
		the closest integer value coordinate
		for ease of creation.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural World Generation|World Generation Settings")
		FVector2D MaxIslandDistances;

	/*
		An array which represents both the number of
		islands to spawn and the parameters by which
		to create each island.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural World Generation|World Generation Settings")
		TArray<FWorldGenIslandSettings> Islands;

	/*
		The index of the island the player is intended to start on.

		if this is <0 or >Islands.Num() then the player will start on a random island.

		As for the exact position on the island, the algorithm attempts to place the
		player in the most hospitable region on the island in terms of its attributes
		and/or biome. This will usually result in being placed in grasslands
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural World Generation|World Generation Settings")
		int StartingIsland = 0;

	/*
		The generation passes, in order of
		occurrence.

		These are done for every island in
		sequence, after their respective
		shaping functions, and serve the
		purpose of assigning attributes like
		elevation, moisture, etc. for later
		use during biome placement and
		regioning.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural World Generation|World Generation Settings")
		TArray<TSubclassOf<UWorldGenPass>> Passes;

	// The biome map used for this world type. Translates environmental data into a concrete biome.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural World Generation|World Generation Settings")
		TSubclassOf<UBiomeMap> BiomeMap;

	// The size of a tile in UU, clamped to integer values for convenience of creation.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural World Generation|World Generation Settings")
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


	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Procedural World Generation|World Generator")
		UWorldMap* WorldMap;

	/*
		This map contains all of the different layouts of worlds which can
		have been saved as generation types.

		By default this is most likely just going to be small/medium/large
		but implementing it this way allows me to append user-created settings
		to the map whenever I feel like getting around to something like that.
	*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Procedural World Generation|World Generator")
		TMap<FName, FWorldGenerationSettings> WorldTypes;

	// Function which both generates and rasterizes the world.
	UFUNCTION(BlueprintCallable, Category = "Procedural World Generation|World Generator")
		void Build(FName WorldTypeToBuild, int InitialRandSeed = 0);

	// Function which generates the world and populates the WorldMap member.
	void Generate(const FWorldGenerationSettings& WorldSettings, int InitialRandSeed);

	// Function which rasterizes the pre-generated world for actual gameplay use.
	void Rasterize(const FWorldGenerationSettings& WorldSettings);

private:

	void GenerateIslandMaps();

	/*
		Helper function for Generate().

		Constructs the basic structure of the continent
		given the current island settings; Determines
		what is land, water, and hole.
	*/
	void ConstructContinentBase(const FWorldGenIslandSettings& CurrIslSettings, const FBox& IslBoundingBox, TArray<UWorldCell*>& OutNonHoleCells);

	/*
		General helper function that flood fills cells.

		If the predicate returns true for the current cell it is added to the
		flood fill.
	*/
	TArray<UWorldCell*> FloodFillCells(UWorldCell* StartingCell, const TFunction<bool(UWorldCell*)>& ConnectedPredicate);

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