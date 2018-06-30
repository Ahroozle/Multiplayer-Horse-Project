// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "MPHorsoCameraGuideTypes.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent), BlueprintType)
class MPHORSO_API UWeightedArrowComponent : public UArrowComponent
{
	GENERATED_BODY()

public:
		UWeightedArrowComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.001", ClampMax = "1.0", UIMin = "0.001", UIMax = "1.0"))
		float Weight = 1;

	// How close to this component can you get before its influence completely governs your rotation?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", UIMin = "0.0"))
		float ThresholdDistance = 100;


};

USTRUCT(BlueprintType)
struct FCameraGuideLine
{
	GENERATED_USTRUCT_BODY();

	UPROPERTY(EditInstanceOnly)
		FName EndA;

	UPROPERTY(EditInstanceOnly)
		FName EndB;

};

USTRUCT(BlueprintType)
struct FCameraGuideTri
{
	GENERATED_USTRUCT_BODY();

	UPROPERTY(EditInstanceOnly)
		FName VertA;

	UPROPERTY(EditInstanceOnly)
		FName VertB;

	UPROPERTY(EditInstanceOnly)
		FName VertC;
};

UCLASS(BlueprintType)
class MPHORSO_API AMPHorsoCameraGuideCluster : public AActor
{
	GENERATED_BODY()

#if WITH_EDITOR
	TArray<FBatchedLine> DrawnLineCopies;
#endif

	FCameraGuideLine* GetClosestGuideLine(const FVector& Pos, FVector& ClosestSegPoint, float& BestDist);
	FCameraGuideTri* GetClosestGuideTri(const FVector& Pos, FVector& ClosestTriPoint, float& BestDist);

	void GetFinalRotAndScaleFromLine(UArrowComponent* InStart, UArrowComponent* InEnd, FVector Pos,
									 FRotator& OutFinalRot, FVector& OutFinalScale);
	void GetFinalRotAndScaleFromMesh(UArrowComponent* InVertA, UArrowComponent* InVertB, UArrowComponent* InVertC, FVector Pos,
									 FRotator& OutFinalRot, FVector& OutFinalScale);

public:
	// Sets default values for this actor's properties
	AMPHorsoCameraGuideCluster(const FObjectInitializer& _init);

	virtual void OnConstruction(const FTransform & Transform) override;

	virtual void Destroyed() override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;



public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;
	virtual void NotifyActorEndOverlap(AActor* OtherActor) override;

	void GuideCamera();

	UPROPERTY(EditInstanceOnly)
		bool Visualize = true;

	UPROPERTY(EditInstanceOnly, BlueprintReadWrite)
		bool Enabled = true;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		TMap<FName, UArrowComponent*> Arrows;

	UPROPERTY(EditInstanceOnly)
		TArray<FCameraGuideLine> GuideLines;

	UPROPERTY(EditInstanceOnly)
		TArray<FCameraGuideTri> GuideMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		APawn* StoredObserver;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		FTimerHandle TimerHandle;

	UPROPERTY(EditInstanceOnly)
		float DefaultThresholdDistance = 100;

	UFunction* RetrievedRotFunc;
	UFloatProperty* RetrievedNextZoom;

};

UCLASS(BlueprintType)
class MPHORSO_API AMPHorsoCameraConformer : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AMPHorsoCameraConformer(const FObjectInitializer& _init);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	virtual void OnConstruction(const FTransform& Transform) override;

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;
	virtual void NotifyActorEndOverlap(AActor* OtherActor) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		APawn* StoredObserver;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FName ConformComponentName;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		UShapeComponent* ConformShape;

	UObjectProperty* RetrievedConformTo;

};
