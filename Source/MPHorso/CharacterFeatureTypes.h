// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "UObject/NoExportTypes.h"
#include "CharacterFeatureTypes.generated.h"


// TODO HOW ADD COLOR OFFSET TO THINGS i.e. add shadows to hair? An extra graph specifically for shadows?

USTRUCT(BlueprintType, meta = (DisplayName = "Character Feature Edge"))
struct FCharFeatureEdge
{
	GENERATED_USTRUCT_BODY();

	// Indices to verts in the outer graph structure
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FIntPoint Verts;

	friend uint32 GetTypeHash(const FCharFeatureEdge& FocusEdge)
	{
		const FIntPoint& FocusVerts = FocusEdge.Verts;
		FIntPoint corrected = { FMath::Min(FocusVerts.X, FocusVerts.Y), FMath::Max(FocusVerts.X,FocusVerts.Y) };
		return GetTypeHash(corrected);
	}
};

static FORCEINLINE bool operator==(const FCharFeatureEdge& a, const FCharFeatureEdge& b)
{
	return (a.Verts.X == b.Verts.X && a.Verts.Y == b.Verts.Y) || (a.Verts.X == b.Verts.Y && a.Verts.Y == b.Verts.X);
}

USTRUCT(BlueprintType, meta = (DisplayName = "Character Feature Loop"))
struct FCharFeatureLoop
{
	GENERATED_USTRUCT_BODY();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TSet<FCharFeatureEdge> Edges;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FLinearColor LoopColor;
};

static FORCEINLINE bool operator==(const FCharFeatureLoop& a, const FCharFeatureLoop& b)
{
	return (a.Edges.Difference(b.Edges)).Num() <= 0 && a.LoopColor == b.LoopColor;
}

USTRUCT(BlueprintType, meta = (DisplayName = "Character Feature Graph"))
struct FCharFeatureGraph
{
	GENERATED_USTRUCT_BODY();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TArray<FCharFeatureLoop> Loops;

};

/**
 * 
 */
UCLASS(meta = (DisplayName = "Character Feature Types Func Lib"))
class MPHORSO_API UCharFeatureTypesFuncLib : public UObject
{
	GENERATED_BODY()
	
public:

	UFUNCTION(BlueprintPure)
		static void GetOutlineEdges(const FCharFeatureGraph& Graph, TArray<FCharFeatureEdge>& OutOutlineEdges);

	/*
		Gets the first loop in the given graph that contains the specified edge.
		Returns whether one was found or not.
	*/
	UFUNCTION(BlueprintPure)
		static bool GetContainingLoop(const FCharFeatureGraph& Graph, FCharFeatureEdge Edge, FCharFeatureLoop& FoundLoop);

	UFUNCTION(BlueprintCallable)
		static void FillShapes(const TArray<FVector2D>& Verts, const FCharFeatureGraph& Graph, UCanvas* FocusCanvas, int StrokeThickness);

	UFUNCTION(BlueprintPure)
		static bool IsLoopClosed(const FCharFeatureLoop& Loop);

	UFUNCTION(BlueprintPure)
		static bool GetClosestGraphPartsToPoint(const TArray<FVector2D>& Verts, FVector2D NewVert, const FCharFeatureGraph& Graph, float EdgeSnapDistance,
			FCharFeatureEdge& OutClosestEdge, int& OutClosestLoopIndex);

	UFUNCTION(BlueprintCallable)
		static void AddVertToCharGraph(UPARAM(Ref) TArray<FVector2D>& Verts, FVector2D NewVert, UPARAM(Ref) FCharFeatureGraph& Graph, float EdgeSnapDistance);

	UFUNCTION()
		static void SplitLoop(const TArray<FVector2D>& Verts, FCharFeatureEdge NewEdge, const FCharFeatureLoop& LoopToSplit,
			FCharFeatureLoop& NewLoopA, FCharFeatureLoop& NewLoopB);

	UFUNCTION()
		static void SplitOutlineIntoLoops(const TArray<FVector2D>& Verts, FCharFeatureEdge NewEdge, const TArray<FCharFeatureEdge>& OutlineEdges,
			FCharFeatureLoop& NewLoopA, FCharFeatureLoop& NewLoopB, float& AreaLoopA, float& AreaLoopB);

	UFUNCTION(BlueprintCallable)
		static void AddEdgeToCharGraph(const TArray<FVector2D>& Verts, int NewEdgeStart, int NewEdgeEnd, UPARAM(Ref) FCharFeatureGraph& Graph);

	UFUNCTION(BlueprintCallable)
		static void AddEdgesToCharGraph(const TArray<FVector2D>& Verts, const TArray<int>& EdgePoints, UPARAM(Ref) FCharFeatureGraph& Graph);

	UFUNCTION(BlueprintCallable)
		static void RemoveVertFromCharGraph(UPARAM(Ref) TArray<FVector2D>& Verts, UPARAM(Ref) FCharFeatureGraph& Graph, int VertToRemove);

	UFUNCTION(BlueprintCallable)
		static void RemoveEdgeFromCharGraph(UPARAM(Ref) FCharFeatureGraph& Graph, FCharFeatureEdge EdgeToRemove);

	/*
		Uses the shoelace algorithm to get the area of an arbitrary 2D shape.
		NOTE: Assumes vertices are in order around the shape! Direction doesn't matter,
		but if they aren't consecutive I can't guarantee that this algorithm will work.
	*/
	UFUNCTION(BlueprintPure)
		static float GetShapeArea(const TArray<FVector2D>& Vertices);
};
