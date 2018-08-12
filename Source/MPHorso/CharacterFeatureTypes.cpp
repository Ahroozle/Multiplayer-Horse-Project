// Fill out your copyright notice in the Description page of Project Settings.

#include "MPHorso.h"
#include "CharacterFeatureTypes.h"

#include "StaticFuncLib.h"

#include "Kismet/KismetMathLibrary.h"


void UCharFeatureTypesFuncLib::GetOutlineEdges(const FCharFeatureGraph& Graph, TArray<FCharFeatureEdge>& OutOutlineEdges)
{
	if (Graph.Loops.Num() > 1)
	{
		// only truly non-shared/unique edges indicate what should be outlined.

		TSet<FCharFeatureEdge> AllEdges = Graph.Loops[0].Edges;
		TSet<FCharFeatureEdge> SharedEdges;

		for (int i = 0; i < Graph.Loops.Num(); ++i)
		{
			AllEdges = AllEdges.Union(Graph.Loops[i].Edges);

			for (int j = i + 1; j < Graph.Loops.Num(); ++j)
				SharedEdges = SharedEdges.Union(Graph.Loops[i].Edges.Intersect(Graph.Loops[j].Edges));
		}

		OutOutlineEdges = AllEdges.Difference(SharedEdges).Array();
	}
	else if (Graph.Loops.Num() > 0)
		OutOutlineEdges = Graph.Loops[0].Edges.Array();
}

bool UCharFeatureTypesFuncLib::GetContainingLoop(const FCharFeatureGraph& Graph, FCharFeatureEdge Edge, FCharFeatureLoop& FoundLoop)
{
	for (const FCharFeatureLoop& currLoop : Graph.Loops)
	{
		if (currLoop.Edges.Contains(Edge))
		{
			FoundLoop = currLoop;
			return true;
		}
	}

	return false;
}

void UCharFeatureTypesFuncLib::FillShapes(const TArray<FVector2D>& Verts, const FCharFeatureGraph& Graph, UCanvas* FocusCanvas, int StrokeThickness)
{
	if (StrokeThickness < 1 || nullptr == FocusCanvas)
		return;

	FVector2D YRange = { 100000,-100000 };
	for (const FVector2D& currVert : Verts)
	{
		YRange.X = FMath::Min(YRange.X, currVert.Y);
		YRange.Y = FMath::Max(YRange.Y, currVert.Y);
	}

	for (const FCharFeatureLoop& currLoop : Graph.Loops)
	{
		if (UCharFeatureTypesFuncLib::IsLoopClosed(currLoop))
		{
			int NumSteps = FMath::CeilToInt((YRange.Y - YRange.X) / StrokeThickness);

			for (int i = 0; i < NumSteps; ++i)
			{
				float currentY = YRange.X + (StrokeThickness * i);
				FVector SegStart = { 0, currentY, 0 }, SegEnd = { (float)FocusCanvas->SizeX, currentY, 0 };

				TArray<FVector2D> IntersectPoints;
				for (const FCharFeatureEdge& currEdge : currLoop.Edges)
				{
					const FVector2D& VertA = Verts[currEdge.Verts.X], VertB = Verts[currEdge.Verts.Y];
					FVector EdgeIntersect;
					if (FMath::SegmentIntersection2D(SegStart, SegEnd, FVector(VertA, 0), FVector(VertB, 0), EdgeIntersect))
						IntersectPoints.Add(FVector2D(EdgeIntersect));
				}

				IntersectPoints.Sort([](const FVector2D& a, const FVector2D& b) { return a.X < b.X; });

				if (IntersectPoints.Num() % 2 != 0)
					UStaticFuncLib::Print("UCharFeatureTypesFuncLib::FillShapes: Number of intersect points was detected to be odd, which "
						"implies that the loop is not closed! I'm not sure how this happened.");

				while (IntersectPoints.Num() > 0)
				{
					if (IntersectPoints.Num() >= 2)
					{
						FocusCanvas->K2_DrawLine(IntersectPoints[0], IntersectPoints[1], StrokeThickness, currLoop.LoopColor);
						IntersectPoints.RemoveAt(0, 2, true);
					}
					else
					{
						FocusCanvas->K2_DrawLine(IntersectPoints[0], IntersectPoints[0], StrokeThickness, currLoop.LoopColor);
						IntersectPoints.RemoveAt(0);
					}
				}
			}
		}
	}
}

bool UCharFeatureTypesFuncLib::IsLoopClosed(const FCharFeatureLoop& Loop)
{
	if (Loop.Edges.Num() <= 2)
		return false;

	TMap<int, int> IndexCounts;

	for (const FCharFeatureEdge& currEdge : Loop.Edges)
	{
		++IndexCounts.FindOrAdd(currEdge.Verts.X);
		++IndexCounts.FindOrAdd(currEdge.Verts.Y);
	}

	for (auto &currPair : IndexCounts)
	{
		if (currPair.Value != 2)
			return false;
	}

	return true;
}

bool UCharFeatureTypesFuncLib::GetClosestGraphPartsToPoint(const TArray<FVector2D>& Verts, FVector2D NewVert, const FCharFeatureGraph& Graph, float EdgeSnapDistance,
	FCharFeatureEdge& OutClosestEdge, int& OutClosestLoopIndex)
{
	float ClosestDist = 10000000000000000;
	OutClosestLoopIndex = -1;

	for (int i = 0; i < Graph.Loops.Num(); ++i)
	{
		for (const FCharFeatureEdge& currEdge : Graph.Loops[i].Edges)
		{
			FVector2D ClosestPoint = FMath::ClosestPointOnSegment2D(NewVert, Verts[currEdge.Verts.X], Verts[currEdge.Verts.Y]);

			float NewDist = FVector2D::Distance(ClosestPoint, NewVert);

			if (NewDist < EdgeSnapDistance)
			{
				if (NewDist < ClosestDist)
				{
					ClosestDist = NewDist;
					OutClosestEdge = currEdge;
					OutClosestLoopIndex = i;
				}
			}
		}
	}

	return OutClosestLoopIndex > -1;
}

void UCharFeatureTypesFuncLib::AddVertToCharGraph(UPARAM(Ref) TArray<FVector2D>& Verts, FVector2D NewVert, UPARAM(Ref) FCharFeatureGraph& Graph, float EdgeSnapDistance)
{
	FCharFeatureEdge ClosestEdge;
	int ClosestLoopInd = -1;

	bool OnEdge =
		UCharFeatureTypesFuncLib::GetClosestGraphPartsToPoint(Verts, NewVert, Graph, EdgeSnapDistance, ClosestEdge, ClosestLoopInd);

	int NewVertInd = Verts.Add(NewVert);

	if (OnEdge)
	{
		FCharFeatureLoop& ClosestLoop = Graph.Loops[ClosestLoopInd];

		FCharFeatureEdge NewEdgeA = { { ClosestEdge.Verts.X, NewVertInd } },
			NewEdgeB = { { NewVertInd, ClosestEdge.Verts.Y } };

		ClosestLoop.Edges.Remove(ClosestEdge);
		ClosestLoop.Edges.Add(NewEdgeA);
		ClosestLoop.Edges.Add(NewEdgeB);
	}
}

void UCharFeatureTypesFuncLib::SplitLoop(const TArray<FVector2D>& Verts, FCharFeatureEdge NewEdge, const FCharFeatureLoop& LoopToSplit,
	FCharFeatureLoop& NewLoopA, FCharFeatureLoop& NewLoopB)
{
	// Split focus loop into two with the edge

	FVector2D ComparePoint = Verts[NewEdge.Verts.X];
	FVector2D CompareDir = (Verts[NewEdge.Verts.Y] - Verts[NewEdge.Verts.X]).GetRotated(90).GetSafeNormal();

	for (const FCharFeatureEdge& currEdge : LoopToSplit.Edges)
	{
		int FocusInd = (currEdge.Verts.X != NewEdge.Verts.X && currEdge.Verts.X != NewEdge.Verts.Y ? currEdge.Verts.X : currEdge.Verts.Y);

		float DotRes = FVector2D::DotProduct(CompareDir, ComparePoint - Verts[FocusInd]);

		if (DotRes > 0)
			NewLoopA.Edges.Add(currEdge);
		else
			NewLoopB.Edges.Add(currEdge);
	}

	if (NewLoopA.Edges.Num() > 0) NewLoopA.Edges.Add(NewEdge);
	if (NewLoopB.Edges.Num() > 0) NewLoopB.Edges.Add(NewEdge);
}

void UCharFeatureTypesFuncLib::SplitOutlineIntoLoops(const TArray<FVector2D>& Verts, FCharFeatureEdge NewEdge,
	const TArray<FCharFeatureEdge>& OutlineEdges, FCharFeatureLoop& NewLoopA, FCharFeatureLoop& NewLoopB, float& AreaLoopA, float& AreaLoopB)
{
	// Make new loop

	FVector2D CurrVertA = Verts[NewEdge.Verts.X];
	FVector2D ForwardDirA = (CurrVertA - Verts[NewEdge.Verts.Y]).GetSafeNormal();
	FVector2D RightDirA = ForwardDirA.GetRotated(90).GetSafeNormal();
	FVector2D CurrVertB = CurrVertA, ForwardDirB = ForwardDirA, RightDirB = RightDirA;

	int IndA = NewEdge.Verts.X, IndB = NewEdge.Verts.X;

	TArray<FVector2D> GrabbedVertsA = { CurrVertA }, GrabbedVertsB = { CurrVertB };
	TSet<FCharFeatureEdge> GrabbedEdgesA = { NewEdge }, GrabbedEdgesB = { NewEdge };

	while (IndA > -1 || IndB > -1)
	{
		if (IndA > -1)
		{
			auto PredA = [&IndA, &GrabbedEdgesA](const FCharFeatureEdge& a)
			{
				return (a.Verts.X == IndA || a.Verts.Y == IndA) && !GrabbedEdgesA.Contains(a);
			};
			TArray<FCharFeatureEdge> RelevantEdges = OutlineEdges.FilterByPredicate(PredA);

			if (RelevantEdges.Num() > 0)
			{
				float lowestDeg = 100000;
				FCharFeatureEdge* lowestEdge = nullptr;
				FVector2D lowestVert;
				int lowestInd = 0;
				for (FCharFeatureEdge& currEdge : RelevantEdges)
				{
					int NextInd = (currEdge.Verts.X == IndA ? currEdge.Verts.Y : currEdge.Verts.X);
					FVector2D NextVert = Verts[NextInd];
					FVector2D currEdgeDir = (NextVert - CurrVertA).GetSafeNormal();

					float NewDeg = UKismetMathLibrary::DegAcos(FVector2D::DotProduct(currEdgeDir, ForwardDirA));
					NewDeg *= FMath::Sign(FVector2D::DotProduct(currEdgeDir, RightDirA));

					if (NewDeg < lowestDeg)
					{
						lowestDeg = NewDeg;
						lowestEdge = &currEdge;
						lowestVert = NextVert;
						lowestInd = NextInd;
					}
				}

				if (nullptr != lowestEdge)
				{
					GrabbedEdgesA.Add(*lowestEdge);
					GrabbedVertsA.Add(lowestVert);
					IndA = lowestInd;

					ForwardDirA = (lowestVert - CurrVertA).GetSafeNormal();
					RightDirA = ForwardDirA.GetRotated(90).GetSafeNormal();
					CurrVertA = lowestVert;

					if (IndA == NewEdge.Verts.Y)
						IndA = -1;
				}
				else
					IndA = -1;
			}
			else
			{
				GrabbedVertsA.Empty();
				IndA = -1;
			}
		}

		if (IndB > -1)
		{
			auto PredB = [&IndB, &GrabbedEdgesB](const FCharFeatureEdge& a)
			{
				return (a.Verts.X == IndB || a.Verts.Y == IndB) && !GrabbedEdgesB.Contains(a);
			};
			TArray<FCharFeatureEdge> RelevantEdges = OutlineEdges.FilterByPredicate(PredB);

			if (RelevantEdges.Num() > 0)
			{
				float highestDeg = -100000;
				FCharFeatureEdge* highestEdge = nullptr;
				FVector2D highestVert;
				int highestInd = 0;
				for (FCharFeatureEdge& currEdge : RelevantEdges)
				{
					int NextInd = (currEdge.Verts.X == IndB ? currEdge.Verts.Y : currEdge.Verts.X);
					FVector2D NextVert = Verts[NextInd];
					FVector2D currEdgeDir = (NextVert - CurrVertB).GetSafeNormal();

					float NewDeg = UKismetMathLibrary::DegAcos(FVector2D::DotProduct(currEdgeDir, ForwardDirB));
					NewDeg *= FMath::Sign(FVector2D::DotProduct(currEdgeDir, RightDirB));

					if (NewDeg > highestDeg)
					{
						highestDeg = NewDeg;
						highestEdge = &currEdge;
						highestVert = NextVert;
						highestInd = NextInd;
					}
				}

				if (nullptr != highestEdge)
				{

					GrabbedEdgesB.Add(*highestEdge);
					GrabbedVertsB.Add(highestVert);
					IndB = highestInd;

					ForwardDirA = (highestVert - CurrVertB).GetSafeNormal();
					RightDirA = ForwardDirB.GetRotated(90).GetSafeNormal();
					CurrVertA = highestVert;

					if (IndB == NewEdge.Verts.Y)
						IndB = -1;
				}
				else
					IndB = -1;
			}
			else
			{
				GrabbedVertsB.Empty();
				IndB = -1;
			}
		}
	}

	AreaLoopA = (GrabbedVertsA.Num() > 0 ? UCharFeatureTypesFuncLib::GetShapeArea(GrabbedVertsA) : -1);
	AreaLoopB = (GrabbedVertsB.Num() > 0 ? UCharFeatureTypesFuncLib::GetShapeArea(GrabbedVertsB) : -1);

	NewLoopA.Edges = GrabbedEdgesA;
	NewLoopB.Edges = GrabbedEdgesB;
}

void UCharFeatureTypesFuncLib::AddEdgeToCharGraph(const TArray<FVector2D>& Verts, int NewEdgeStart, int NewEdgeEnd, UPARAM(Ref) FCharFeatureGraph& Graph)
{
	// TODO DELETE FUNC

	FCharFeatureEdge NewEdge = { { NewEdgeStart, NewEdgeEnd } };

	FVector VertA(Verts[NewEdgeStart], 0), VertB(Verts[NewEdgeEnd], 0);

	TSet<FCharFeatureLoop*> VertALoops, VertBLoops;

	for (FCharFeatureLoop& currLoop : Graph.Loops)
	{
		if (currLoop.Edges.Contains(NewEdge))
			return;

		for (FCharFeatureEdge& currEdge : currLoop.Edges)
		{
			FVector OtherA(Verts[currEdge.Verts.X], 0), OtherB(Verts[currEdge.Verts.Y], 0);

			FVector DummyOut;

			if (FMath::SegmentIntersection2D(VertA, VertB, OtherA, OtherB, DummyOut))
			{
				if (!DummyOut.Equals(VertA) && !DummyOut.Equals(VertB))
					return;
			}

			if (currEdge.Verts.X == NewEdgeStart || currEdge.Verts.Y == NewEdgeStart)
				VertALoops.Add(&currLoop);

			if (currEdge.Verts.X == NewEdgeEnd || currEdge.Verts.Y == NewEdgeEnd)
				VertBLoops.Add(&currLoop);
		}
	}

	if (VertALoops.Num() > 0 && VertBLoops.Num() > 0)
	{
		TArray<FCharFeatureLoop*> BothTouched = VertALoops.Intersect(VertBLoops).Array();

		if (BothTouched.Num() > 0)
		{
			// Split focus loop into two with the edge
			FCharFeatureLoop NewLoopA = { {}, FLinearColor::MakeRandomColor() } , NewLoopB = { {}, FLinearColor::MakeRandomColor() };
			UCharFeatureTypesFuncLib::SplitLoop(Verts, NewEdge, *BothTouched[0], NewLoopA, NewLoopB);

			FCharFeatureLoop LoopProxy = *BothTouched[0];
			Graph.Loops.Remove(LoopProxy);
			if (NewLoopA.Edges.Num() > 0)
				Graph.Loops.Add(NewLoopA);
			if (NewLoopB.Edges.Num() > 0)
				Graph.Loops.Add(NewLoopB);
		}
		else
		{
			// Make new loop

			TArray<FCharFeatureEdge> OutlineEdges;
			UCharFeatureTypesFuncLib::GetOutlineEdges(Graph, OutlineEdges);

			FCharFeatureLoop NewLoopA = { {}, FLinearColor::MakeRandomColor() }, NewLoopB = { {}, FLinearColor::MakeRandomColor() };
			float AreaLoopA, AreaLoopB;
			UCharFeatureTypesFuncLib::SplitOutlineIntoLoops(Verts, NewEdge, OutlineEdges, NewLoopA, NewLoopB, AreaLoopA, AreaLoopB);

			if (AreaLoopA >= 0 && AreaLoopB >= 0)
			{
				if (AreaLoopA < AreaLoopB)
					Graph.Loops.Add(NewLoopA);
				else
					Graph.Loops.Add(NewLoopB);
			}
			else if (AreaLoopA >= 0)
				Graph.Loops.Add(NewLoopA);
			else if (AreaLoopB >= 0)
				Graph.Loops.Add(NewLoopB);
			else
				Graph.Loops.Add({ { NewEdge }, FLinearColor::MakeRandomColor() });
		}
	}
	else
	{
		TArray<FCharFeatureLoop*> AnyTouched = VertALoops.Union(VertBLoops).Array();

		TArray<FCharFeatureLoop> LoopsToRemove;
		TArray<FCharFeatureLoop> LoopsToAdd;

		if (AnyTouched.Num() > 0)
		{
			bool MakeNewLoop = false;

			for (FCharFeatureLoop* currLoop : AnyTouched)
			{
				if (!UCharFeatureTypesFuncLib::IsLoopClosed(*currLoop))
				{
					FCharFeatureLoop NewLoopA, NewLoopB;
					UCharFeatureTypesFuncLib::SplitLoop(Verts, NewEdge, *currLoop, NewLoopA, NewLoopB);

					LoopsToRemove.Add(*currLoop);
					if (NewLoopA.Edges.Num() > 0)
						LoopsToAdd.Add(NewLoopA);
					if (NewLoopA.Edges.Num() > 0)
						LoopsToAdd.Add(NewLoopB);
				}
				else
					MakeNewLoop = true;
			}

			if (MakeNewLoop)
				LoopsToAdd.Add({ { NewEdge }, FLinearColor::Red });
		}
		else
			LoopsToAdd.Add({ { NewEdge }, FLinearColor::Red });


		for (FCharFeatureLoop& currToDel : LoopsToRemove)
			Graph.Loops.Remove(currToDel);

		Graph.Loops.Append(LoopsToAdd);
	}
}

void UCharFeatureTypesFuncLib::AddEdgesToCharGraph(const TArray<FVector2D>& Verts, const TArray<int>& EdgePoints, UPARAM(Ref) FCharFeatureGraph& Graph)
{
	FCharFeatureLoop UserLoop = { {}, FLinearColor::MakeRandomColor() };

	for (int i = 0; i < EdgePoints.Num() - 1; ++i)
		UserLoop.Edges.Add({ { EdgePoints[i], EdgePoints[i + 1] } });

	if (UserLoop.Edges.Num() > 1)
	{
		TArray<FCharFeatureLoop> LoopCopies = Graph.Loops;

		auto SortPred = [&UserLoop](const FCharFeatureLoop& a, const FCharFeatureLoop& b)
		{
			return a.Edges.Intersect(UserLoop.Edges).Num() > b.Edges.Intersect(UserLoop.Edges).Num();
		};

		LoopCopies.Sort(SortPred);

		// TODO Expect user is trying to make a new loop. Check if there already is a loop that the user has drawn.
		// If all but one edge belong to a single preexisting loop, and that loop is broken, expect that the user is trying to finish the loop.
		// Otherwise just make a whole new loop.

		if (LoopCopies.Num() > 0)
		{
			if (!LoopCopies[0].Edges.Includes(UserLoop.Edges))
			{
				int TopDiff = UserLoop.Edges.Num() - LoopCopies[0].Edges.Intersect(UserLoop.Edges).Num();
				if (TopDiff == 1)
				{
					if (UCharFeatureTypesFuncLib::IsLoopClosed(LoopCopies[0]))
					{
						// TODO Expect user is trying to seal a loop, see if what they're doing seals it.
					}
				}
				else if (TopDiff > 1)
				{
					// Expect user is trying to make a new loop. Simply make a new loop.
					Graph.Loops.Add(UserLoop);
				}
			}
		}
		else
		{
			// user trying to make new loop, just add it.

			Graph.Loops.Add(UserLoop);
		}
	}
	else if (UserLoop.Edges.Num() > 0 && UserLoop.Edges.Array()[0].Verts.X != UserLoop.Edges.Array()[0].Verts.Y)
	{
		// TODO Expect that the user is trying to split or finish a loop. Search for the loop and if you find one split it.
	}
}

void UCharFeatureTypesFuncLib::RemoveVertFromCharGraph(UPARAM(Ref) TArray<FVector2D>& Verts, UPARAM(Ref) FCharFeatureGraph& Graph, int VertToRemove)
{
	TArray<FCharFeatureLoop> LoopsToRemove;

	auto Pred = [&VertToRemove](const FCharFeatureEdge& a) { return a.Verts.X == VertToRemove || a.Verts.Y == VertToRemove; };
	for (FCharFeatureLoop& currLoop : Graph.Loops)
	{
		TArray<FCharFeatureEdge> RelevantEdges = currLoop.Edges.Array().FilterByPredicate(Pred);

		if (RelevantEdges.Num() > 1)
		{
			FCharFeatureEdge ReplacementEdge;
			ReplacementEdge.Verts.X = (RelevantEdges[0].Verts.X == VertToRemove ? RelevantEdges[0].Verts.Y : RelevantEdges[0].Verts.X);
			ReplacementEdge.Verts.Y = (RelevantEdges[1].Verts.X == VertToRemove ? RelevantEdges[1].Verts.Y : RelevantEdges[1].Verts.X);

			currLoop.Edges.Remove(RelevantEdges[0]);
			currLoop.Edges.Remove(RelevantEdges[1]);
			currLoop.Edges.Add(ReplacementEdge);
		}
		else if (RelevantEdges.Num() > 0)
			currLoop.Edges.Remove(RelevantEdges[0]);

		TArray<FCharFeatureEdge> ToRem, ToAdd;

		for (FCharFeatureEdge& currEdge : currLoop.Edges)
		{
			FCharFeatureEdge Altered = currEdge;
			if (Altered.Verts.X > VertToRemove) --Altered.Verts.X;
			if (Altered.Verts.Y > VertToRemove) --Altered.Verts.Y;

			if (!(Altered == currEdge))
			{
				ToRem.Add(currEdge);
				ToAdd.Add(Altered);
			}
		}

		for (FCharFeatureEdge& currToRem : ToRem)
			currLoop.Edges.Remove(currToRem);

		currLoop.Edges.Append(ToAdd);

		if (currLoop.Edges.Num() < 1)
			LoopsToRemove.Add(currLoop);
	}

	Verts.RemoveAt(VertToRemove);

	for (FCharFeatureLoop& currToRem : LoopsToRemove)
		Graph.Loops.Remove(currToRem);
}

void UCharFeatureTypesFuncLib::RemoveEdgeFromCharGraph(UPARAM(Ref) FCharFeatureGraph& Graph, FCharFeatureEdge EdgeToRemove)
{
	TArray<FCharFeatureLoop> LoopsToRemove;

	TArray<FCharFeatureLoop> LoopsToAdd;

	for (FCharFeatureLoop& currLoop : Graph.Loops)
	{
		if (currLoop.Edges.Contains(EdgeToRemove))
		{
			if (UCharFeatureTypesFuncLib::IsLoopClosed(currLoop))
			{
				currLoop.Edges.Remove(EdgeToRemove);

				if (currLoop.Edges.Num() <= 0)
					LoopsToRemove.Add(currLoop);
			}
			else
			{
				LoopsToRemove.Add(currLoop);

				int NextA = EdgeToRemove.Verts.X, NextB = EdgeToRemove.Verts.Y;
				FCharFeatureLoop NewLoopA = { {}, currLoop.LoopColor }, NewLoopB = { {}, FLinearColor::MakeRandomColor() };
				while (NextA > -1 || NextB > -1)
				{
					if (NextA > -1)
					{
						auto PredA = [&NextA, &EdgeToRemove, &NewLoopA](const FCharFeatureEdge& a)
						{
							return !(a == EdgeToRemove) && !NewLoopA.Edges.Contains(a) && (a.Verts.X == NextA || a.Verts.Y == NextA);
						};
						TArray<FCharFeatureEdge> Relevant = currLoop.Edges.Array().FilterByPredicate(PredA);

						if (Relevant.Num() > 0)
						{
							NewLoopA.Edges.Add(Relevant[0]);
							NextA = (Relevant[0].Verts.X == NextA ? Relevant[0].Verts.Y : Relevant[0].Verts.X);
						}
						else
							NextA = -1;
					}

					if (NextB > -1)
					{
						auto PredB = [&NextB, &EdgeToRemove, &NewLoopB](const FCharFeatureEdge& b)
						{
							return !(b == EdgeToRemove) && !NewLoopB.Edges.Contains(b) && (b.Verts.X == NextB || b.Verts.Y == NextB);
						};
						TArray<FCharFeatureEdge> Relevant = currLoop.Edges.Array().FilterByPredicate(PredB);

						if (Relevant.Num() > 0)
						{
							NewLoopB.Edges.Add(Relevant[0]);
							NextB = (Relevant[0].Verts.X == NextB ? Relevant[0].Verts.Y : Relevant[0].Verts.X);
						}
						else
							NextB = -1;
					}
				}

				if (NewLoopA.Edges.Num() > 0) LoopsToAdd.Add(NewLoopA);
				if (NewLoopB.Edges.Num() > 0) LoopsToAdd.Add(NewLoopB);
			}
		}
	}

	for (FCharFeatureLoop& currToRemove : LoopsToRemove)
		Graph.Loops.Remove(currToRemove);

	Graph.Loops.Append(LoopsToAdd);
}

float UCharFeatureTypesFuncLib::GetShapeArea(const TArray<FVector2D>& Vertices)
{
	float RollingArea = 0;
	for (int i = 0; i < Vertices.Num(); ++i)
		RollingArea += FVector2D::CrossProduct(Vertices[i], Vertices[(i + 1) % Vertices.Num()]);
	return 0.5f * FMath::Abs(RollingArea);
}
