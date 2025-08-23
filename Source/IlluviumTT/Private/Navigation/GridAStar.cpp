// Fill out your copyright notice in the Description page of Project Settings.


#include "IlluviumTT/Public/Navigation/GridAStar.h"

namespace
{
    const FGridCoordinate NeighbourOffsets[4] {
        {+1,0},
        {0,+1},
        {-1,0},
        {0,-1}
    };
    
    FORCEINLINE bool InBounds(const FGridCoordinate& C, const FIntPoint& S)
    {
        return C.X >= 0 && C.X < S.X && C.Y >= 0 && C.Y < S.Y;
    }
}

bool FGridAStar::FindPath(const FPathRequest& PathRequest, TArray<FGridCoordinate>& OutPath)
{
	OutPath.Reset();

    if (!IsWithinGridBounds(PathRequest.Start, PathRequest.GridSize) ||
        !IsWithinGridBounds(PathRequest.Goal,  PathRequest.GridSize))
    {
        return false;
    }

    if (PathRequest.Start == PathRequest.Goal)
    {
        OutPath.Add(PathRequest.Goal);
        return true;
    }

    auto EstimateRemainingCost = [&](const FGridCoordinate& From) -> int32
    {
        return Manhattan(From, PathRequest.Goal);
    };

    TSet<FGridCoordinate> BlockedCoordinates = PathRequest.Blocked;
    BlockedCoordinates.Remove(PathRequest.Goal);

    TArray<FSearchNode> OpenListNodes;
    TSet<FGridCoordinate> ClosedCoordinates;
    TMap<FGridCoordinate, FSearchNode> BestNodeByCoordinate;

    int32 InsertionCounter = 0;

    FSearchNode StartNode{
        PathRequest.Start,
        0,
        EstimateRemainingCost(PathRequest.Start),
        { INT32_MIN, INT32_MIN },
        InsertionCounter++
    };

    OpenListNodes.Add(StartNode);
    BestNodeByCoordinate.Add(PathRequest.Start, StartNode);

    while (!OpenListNodes.IsEmpty())
    {
        OpenListNodes.Sort([](const FSearchNode& A, const FSearchNode& B) { return A < B; });
        const FSearchNode CurrentNode = OpenListNodes[0];
        OpenListNodes.RemoveAt(0);

        if (CurrentNode.Coordinate == PathRequest.Goal)
        {
            TArray<FGridCoordinate> ReconstructedPathBackwards;

            FGridCoordinate CurrentCoordinate = CurrentNode.Coordinate;

            while (CurrentCoordinate.X != INT32_MIN)
            {
                ReconstructedPathBackwards.Add(CurrentCoordinate);

                const FSearchNode* NodeForCurrentCoordinate = BestNodeByCoordinate.Find(CurrentCoordinate);
                if (!NodeForCurrentCoordinate)
                {
                    break;
                }

                CurrentCoordinate = NodeForCurrentCoordinate->ParentCoordinate;
            }

            Algo::Reverse(ReconstructedPathBackwards);

            OutPath = MoveTemp(ReconstructedPathBackwards);
            return true;
        }

        ClosedCoordinates.Add(CurrentNode.Coordinate);

        for (const FGridCoordinate& Offset : NeighbourOffsets)
        {
            const FGridCoordinate NeighborCoordinate{
                CurrentNode.Coordinate.X + Offset.X,
                CurrentNode.Coordinate.Y + Offset.Y
            };

            if (!IsWithinGridBounds(NeighborCoordinate, PathRequest.GridSize))
                continue;

            if (BlockedCoordinates.Contains(NeighborCoordinate))
                continue;

            if (ClosedCoordinates.Contains(NeighborCoordinate))
                continue;

            const int32 TentativeCostFromStart  = CurrentNode.CostFromStart + 1;
            const int32 TentativeEstimatedTotal = TentativeCostFromStart + EstimateRemainingCost(NeighborCoordinate);

            FSearchNode* ExistingBestForNeighbor = BestNodeByCoordinate.Find(NeighborCoordinate);

            const bool IsBetterPath =
                (!ExistingBestForNeighbor) ||
                (TentativeCostFromStart <  ExistingBestForNeighbor->CostFromStart) ||
                (TentativeCostFromStart == ExistingBestForNeighbor->CostFromStart &&
                 TentativeEstimatedTotal  <  ExistingBestForNeighbor->EstimatedTotalCost);

            if (IsBetterPath)
            {
                FSearchNode UpdatedNeighborNode {
                NeighborCoordinate,
                TentativeCostFromStart,
                TentativeEstimatedTotal,
                CurrentNode.Coordinate,
                InsertionCounter++
                };
                
                BestNodeByCoordinate.Add(NeighborCoordinate, UpdatedNeighborNode);

                const int32 ExistingIndex =
                    OpenListNodes.IndexOfByPredicate([&](const FSearchNode& Node){ return Node.Coordinate == NeighborCoordinate; });

                if (ExistingIndex != INDEX_NONE)
                {
                    OpenListNodes[ExistingIndex] = UpdatedNeighborNode;
                }
                else
                {
                    OpenListNodes.Add(UpdatedNeighborNode);
                }
            }
        }
    }

    return false;
}
