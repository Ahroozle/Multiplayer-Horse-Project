// Fill out your copyright notice in the Description page of Project Settings.

#include "MPHorso.h"
#include "NPCNavigation.h"

#include "Kismet/GameplayStatics.h"

#include "StaticFuncLib.h"

#include "MPHorsoGameInstance.h"


// Sets default values
ANPCNavCluster::ANPCNavCluster(const FObjectInitializer& _init) : Super(_init)
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	RootComponent = _init.CreateDefaultSubobject<USceneComponent>(this, TEXT("RootComponent"));
}

void ANPCNavCluster::OnConstruction(const FTransform & Transform)
{
	Super::OnConstruction(Transform);
	
#if WITH_EDITOR

	//GetWorld()->PersistentLineBatcher->Flush();
	TArray<FBatchedLine>& WorldPersistentBatchedLines = GetWorld()->PersistentLineBatcher->BatchedLines;

	if (DrawnLineCopies.Num() > 0)
	{
		for (FBatchedLine &curr : DrawnLineCopies)
		{
			WorldPersistentBatchedLines.RemoveAllSwap(
				[&curr](FBatchedLine& a) { return a.Start == curr.Start && a.End == curr.End && a.Color == curr.Color; },
				true
			);
		}

		if(!ShowEdges)
			GetWorld()->PersistentLineBatcher->MarkRenderStateDirty();

		DrawnLineCopies.Empty();
	}

	if (ShowEdges)
	{
		int CopyStart = WorldPersistentBatchedLines.Num() - 1;

		TMap<FName, USceneComponent*> FoundComps;
		{
			TArray<USceneComponent*> RetrievedChildren;
			RootComponent->GetChildrenComponents(true, RetrievedChildren);
			for (USceneComponent *curr : RetrievedChildren)
				FoundComps.Add(curr->GetFName(), curr);
		}

		for (auto &currNodePair : ClusterGraph.Nodes)
		{
			{
				USceneComponent* StartComp = FoundComps.FindRef(currNodePair.Key);
				if (nullptr != StartComp)
					DrawDebugCircle(GetWorld(), StartComp->GetComponentLocation(), NodeVisualRadius, 50, FColor::Green, true, -1,
									(uint8)'\000', 5,FVector::ForwardVector,FVector::RightVector,false);
			}

			for (FNPCNavEdge &currEdge : currNodePair.Value.Neighbors)
			{
				FNPCNavNode* Targ = ClusterGraph.Nodes.Find(currEdge.ToNode);
				if (nullptr != Targ)
				{
					USceneComponent* StartComp = FoundComps.FindRef(currNodePair.Key);
					if (nullptr != StartComp)
					{
						USceneComponent* EndComp = FoundComps.FindRef(currEdge.ToNode);
						if(nullptr != EndComp)
						{
							FVector StartLoc = StartComp->GetComponentLocation();
							FVector EndLoc = EndComp->GetComponentLocation();

							FVector ToTarg = (EndLoc - StartLoc).GetSafeNormal();
							FMatrix RotMat = FRotationMatrix::MakeFromXZ(ToTarg, FVector::UpVector);
							FVector TargOffs = RotMat.GetUnitAxis(EAxis::Y);

							DrawDebugDirectionalArrow(GetWorld(),
													  StartLoc + (ToTarg * NodeVisualRadius) + (TargOffs * EdgeOffsetAmount),
													  EndLoc - (ToTarg * NodeVisualRadius) + (TargOffs * EdgeOffsetAmount),
													  200,
													  FColor::Yellow,
													  true,
													  -1,
													  (uint8)'\000',
													  5);
						}
						else
							UStaticFuncLib::Print("ANPCNavCluster::OnConstruction: Couldn't find a component named \'" +
											  currEdge.ToNode.ToString() +
											  "\' to represent the respective node's position! Please fix this, as it will "
											  "cause the World Graph bake to fail.", true);
					}
					else
						UStaticFuncLib::Print("ANPCNavCluster::OnConstruction: Couldn't find a component named \'" +
											  currNodePair.Key.ToString() +
											  "\' to represent the respective node's position! Please fix this, as it will "
											  "cause the World Graph bake to fail.", true);
				}
				else
					UStaticFuncLib::Print("ANPCNavCluster::OnConstruction: Couldn't find a node named \'" +
										  currEdge.ToNode.ToString() +
										  "\' in the cluster's graph!", true);
			}
		}

		while (++CopyStart < WorldPersistentBatchedLines.Num())
			DrawnLineCopies.Add(WorldPersistentBatchedLines[CopyStart]);
	}
#endif
}

void ANPCNavCluster::Destroyed()
{
#if WITH_EDITOR
	if (DrawnLineCopies.Num() > 0)
	{
		for (FBatchedLine &curr : DrawnLineCopies)
		{
			GetWorld()->PersistentLineBatcher->BatchedLines.RemoveAllSwap(
				[&curr](FBatchedLine& a) { return a.Start == curr.Start && a.End == curr.End && a.Color == curr.Color; },
				true
			);
		}

		GetWorld()->PersistentLineBatcher->MarkRenderStateDirty();

		DrawnLineCopies.Empty();
	}
#endif

	Super::Destroyed();
}

// Called when the game starts or when spawned
void ANPCNavCluster::BeginPlay()
{
	Super::BeginPlay();
	
	this->Destroy();
}

// Called every frame
void ANPCNavCluster::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}


// Sets default values
ANPCNavManager::ANPCNavManager(const FObjectInitializer& _init) : Super(_init)
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	RootComponent = _init.CreateDefaultSubobject<USceneComponent>(this, TEXT("RootComponent"));
}

void ANPCNavManager::OnConstruction(const FTransform & Transform)
{
	Super::OnConstruction(Transform);

#if WITH_EDITOR
	if (Bake)
	{
		Bake = false;

		TArray<AActor*> FoundClusters;
		UGameplayStatics::GetAllActorsOfClass(this, ANPCNavCluster::StaticClass(), FoundClusters);
		WorldGraph.Nodes.Empty();
		FString NodesAdded = "ANPCNavManager::OnConstruction:\n";
		int NumClusters = 0;
		int NumNodes = 0;
		for (AActor *currCluster : FoundClusters)
		{
			++NumClusters;
			ANPCNavCluster* castedCurr = Cast<ANPCNavCluster>(currCluster);

			TMap<FName, USceneComponent*> FoundComps;
			{
				TArray<USceneComponent*> RetrievedChildren;
				currCluster->GetRootComponent()->GetChildrenComponents(true, RetrievedChildren);
				for (USceneComponent *curr : RetrievedChildren)
					FoundComps.Add(curr->GetFName(), curr);
			}

			for (auto &currPair : castedCurr->ClusterGraph.Nodes)
			{
				++NumNodes;
				if (!WorldGraph.Nodes.Contains(currPair.Key))
				{
					USceneComponent* RelevantComp = FoundComps.FindRef(currPair.Key);
					if (nullptr != RelevantComp)
					{
						FNPCNavNode& NextNode = WorldGraph.Nodes.Add(currPair.Key, currPair.Value);
						NextNode.Location = RelevantComp->GetComponentLocation();

						NodesAdded += "\tAdded Node \'" + currPair.Key.ToString() +
							"\' from cluster\'" + currCluster->GetName() +
							"\' to the World Graph.\n";
					}
					else
					{
						UStaticFuncLib::Print(NodesAdded);
						UStaticFuncLib::Print("ANPCNavManager::OnConstruction: Bake of World Graph has failed!\n"
											  "\tNode \'" + currPair.Key.ToString() + "\' does not have a respective scenecomponent "
											  "within its cluster. Clearing the World Graph, and returning early.", true);
						WorldGraph.Nodes.Empty();
						return;
					}
				}
				else
				{
					UStaticFuncLib::Print(NodesAdded);
					UStaticFuncLib::Print("ANPCNavManager::OnConstruction: Bake of World Graph has failed!\n"
										  "\tCluster \'" + currCluster->GetName() + "\' Contains a node named \'" +
										  currPair.Key.ToString() + "\', which was already added to the map by a "
										  "Previous node or cluster. Clearing the World Graph, and returning early.", true);
					WorldGraph.Nodes.Empty();
					return;
				}
			}
		}

		// TODO : Base traversal speeds on Navmesh path distance rather than direct distance.
		for (auto &currPair : WorldGraph.Nodes)
		{
			for (FNPCNavEdge& currNeighbor : currPair.Value.Neighbors)
			{
				FNPCNavNode* RetrievedTo = WorldGraph.Nodes.Find(currNeighbor.ToNode);

				if (nullptr != RetrievedTo)
				{
					if (currNeighbor.IsLoadingZone)
						currNeighbor.TraversalTime = 0;
					else
						currNeighbor.TraversalTime =
							(RetrievedTo->Location - currPair.Value.Location).Size() / FMath::Max(0.001f, TraversalSpeed);
				}
				else
					UStaticFuncLib::Print("ANPCNavManager::OnConstruction: (Warning) Couldn't find node \'" +
										  currNeighbor.ToNode.ToString() +
										  "\'. Did you forget to create it?");

				currNeighbor.RequiredAttributes.PrepForUse();
				currNeighbor.DesiredAttributes.PrepForUse();
			}
		}

		UStaticFuncLib::Print(NodesAdded);
		UStaticFuncLib::Print("ANPCNavManager::OnConstruction: Bake of World Graph succeeded! Added "
							  + FString::FromInt(NumNodes) + " nodes across "
							  + FString::FromInt(NumClusters) + " clusters.",
							  true);
	}
#endif

}

// Called when the game starts or when spawned
void ANPCNavManager::BeginPlay()
{
	Super::BeginPlay();

}

// Called every frame
void ANPCNavManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ANPCNavManager::GetClosestNodeToPoint(FVector Point, FName& PointName)
{
	float BestDist = 10000000000000000;
	for (auto &currPair : WorldGraph.Nodes)
	{
		float NextDist = (currPair.Value.Location - Point).Size();
		if (BestDist > NextDist)
		{
			BestDist = NextDist;
			PointName = currPair.Key;
		}
	}
}


int UNPCNavVisitor::DetermineEdgeCost(const FName& StartNodeName, const FNPCNavEdge& Edge)
{
	if (nullptr != MemoryStateHandle.ToState)
	{
		FNPCFact* foundBlacklist = MemoryStateHandle.ToState->Facts.Find("EdgeBlacklist");
		if (nullptr != foundBlacklist)
		{
			TArray<FString> BlacklistedEdges;
			foundBlacklist->Data.GetValue<FString>().ParseIntoArray(BlacklistedEdges, TEXT(","));
			if (BlacklistedEdges.Contains(StartNodeName.ToString() + "_" + Edge.ToNode.ToString()))
				return 10000;
		}
	}

	int FinalCost = 1;

	if (!Edge.IsLoadingZone)
	{
		for (auto &currRequired : Edge.RequiredAttributes.InternalState.Facts)
		{
			FNPCFact* FoundFact = nullptr;
			if (nullptr != MemoryStateHandle.ToState)
				FoundFact = MemoryStateHandle.ToState->Facts.Find(currRequired.Key);
			if (nullptr == FoundFact && nullptr != PersonalityStateHandle.ToState)
				FoundFact = PersonalityStateHandle.ToState->Facts.Find(currRequired.Key);

			if (nullptr == FoundFact)
				FinalCost += 1000;
			else
			{
				if(FoundFact->FactType!=currRequired.Value.FactType)
					UStaticFuncLib::Print("UNPCNavVisitor::DetermineEdgeCost: Required Attribute \'" + currRequired.Key.ToString() +
										  "\' in the current edge has a different fact type than the actual attribute found. "
										  "It will be ignored.", true);
				else
				{
					if (FoundFact->FactType == ENPCFactType::String && FoundFact->Data.GetValue<FString>().Contains(","))
					{
						TArray<FString> ArrayPieces;
						FoundFact->Data.GetValue<FString>().ParseIntoArray(ArrayPieces, TEXT(","));
						if (ArrayPieces.Contains(currRequired.Value.Data.GetValue<FString>()))
							FinalCost += 1000;
					}
					else if (FoundFact->Data != currRequired.Value.Data)
						FinalCost += 1000;
				}
			}
		}

		for (auto &currDesired : Edge.DesiredAttributes.InternalState.Facts)
		{
			FNPCFact* FoundFact = nullptr;
			if (nullptr != MemoryStateHandle.ToState)
				FoundFact = MemoryStateHandle.ToState->Facts.Find(currDesired.Key);
			if (nullptr == FoundFact && nullptr != PersonalityStateHandle.ToState)
				FoundFact = PersonalityStateHandle.ToState->Facts.Find(currDesired.Key);

			if (nullptr == FoundFact)
				FinalCost += 1;
			else
			{
				if (FoundFact->FactType != currDesired.Value.FactType)
					UStaticFuncLib::Print("UNPCNavVisitor::DetermineEdgeCost: Required Attribute \'" + currDesired.Key.ToString() +
						"\' in the current edge has a different fact type than the actual attribute found. "
						"It will be ignored.", true);
				else
				{
					if (FoundFact->FactType == ENPCFactType::String && FoundFact->Data.GetValue<FString>().Contains(","))
					{
						TArray<FString> ArrayPieces;
						FoundFact->Data.GetValue<FString>().ParseIntoArray(ArrayPieces, TEXT(","));
						if (ArrayPieces.Contains(currDesired.Value.Data.GetValue<FString>()))
							FinalCost += 1;
					}
					else if (FoundFact->Data != currDesired.Value.Data)
						FinalCost += 1;
				}
			}
		}
	}

	return FinalCost;
}

bool UNPCNavVisitor::NodeIsGoal(const FNPCNavNode& Node)
{
	for (FName &curr : StoredDestAttributes)
	{
		if (!Node.Attributes.Contains(curr))
			return false;
	}

	return true;
}

void UNPCNavVisitor::BeginTraversal(UObject* WorldContext, FName StartNodeName, TArray<FName> DestNodeAttributes)
{
	if (TraversalHandle.IsValid())
		UStaticFuncLib::Print("UNPCNavVisitor::BeginTraversal: This visitor was already traversing! The previous traversal will be "
							  "overwritten.", true);

	NodeQueue.Empty();
	NodeParentQueue.Empty();
	QueueIterator = 0;
	VisitedNodes.Empty();

	NodeQueue.Add(StartNodeName);
	NodeParentQueue.Add(-1);
	VisitedNodes.Add(StartNodeName, 0);
	StoredDestAttributes = DestNodeAttributes;

	if (nullptr == StoredWorldContext)
		StoredWorldContext = WorldContext;

	if (nullptr == StoredNavManager)
		StoredNavManager = UNPCNavFuncLib::GetNavManager(StoredWorldContext);

	WorldContext->GetWorld()->GetTimerManager().SetTimer(TraversalHandle, this, &UNPCNavVisitor::Traverse, 0.01f, true);
}

void UNPCNavVisitor::Traverse()
{
	if (QueueIterator < NodeQueue.Num())
	{
		FNPCNavNode* currNode = StoredNavManager->WorldGraph.Nodes.Find(NodeQueue[QueueIterator]);

		if (NodeIsGoal(*currNode))
		{
			FinishTraversal();
			return;
		}

		if (nullptr != currNode)
		{
			int* currNodeCost = VisitedNodes.Find(NodeQueue[QueueIterator]);

			for (FNPCNavEdge &currNeighbor : currNode->Neighbors)
			{
				int NewCost = DetermineEdgeCost(NodeQueue[QueueIterator], currNeighbor) + *currNodeCost;

				int* LastCostOfNeigh = VisitedNodes.Find(currNeighbor.ToNode);
				if (nullptr != LastCostOfNeigh) // if the neighbor was already visited before
				{
					if (*LastCostOfNeigh < NewCost)
						continue;
				}
				else
					LastCostOfNeigh = &VisitedNodes.Add(currNeighbor.ToNode);

				*LastCostOfNeigh = NewCost;

				NodeQueue.Add(currNeighbor.ToNode);
				NodeParentQueue.Add(QueueIterator);
			}

			++QueueIterator;
		}
		else
		{
			UStaticFuncLib::Print("UNPCNavVisitor::Traverse: A node named \'" + NodeQueue[QueueIterator].ToString() +
								  "\' was not found in the World Graph! Cancelling traversal.", true);
			StoredWorldContext->GetWorld()->GetTimerManager().ClearTimer(TraversalHandle);
		}
	}
	else
	{
		FString AttrStr;
		for (FName &curr : StoredDestAttributes)
			AttrStr += "\t" + curr.ToString() + "\n";

		UStaticFuncLib::Print("UNPCNavVisitor::Traverse: Traversal finished, but did not find a node which had "
							  "the required goal attributes.\nAttributes were:" + AttrStr, true);
		StoredWorldContext->GetWorld()->GetTimerManager().ClearTimer(TraversalHandle);
	}
}

void UNPCNavVisitor::FinishTraversal()
{
	TArray<FName> FoundPath;

	int CurrNodeInd = QueueIterator;

	while (CurrNodeInd >= 0)
	{
		FoundPath.Insert(NodeQueue[CurrNodeInd], 0);
		CurrNodeInd = NodeParentQueue[CurrNodeInd];
	}

	OnFinishedTraversal.Broadcast(FoundPath);

	StoredWorldContext->GetWorld()->GetTimerManager().ClearTimer(TraversalHandle);
}

void UNPCNavVisitor::CancelTraversal(UObject* WorldContext)
{
	WorldContext->GetWorld()->GetTimerManager().ClearTimer(TraversalHandle);
}


ANPCNavManager* UNPCNavFuncLib::GetNavManager(UObject* WorldContext)
{
	UMPHorsoGameInstance* gameInst = UStaticFuncLib::RetrieveGameInstance(WorldContext);

	if (nullptr != gameInst)
		return gameInst->GetNavManager();
	else
		UStaticFuncLib::Print("UNPCNavFuncLib::GetNavManager: Couldn't retrieve the game instance!", true);

	return nullptr;
}
