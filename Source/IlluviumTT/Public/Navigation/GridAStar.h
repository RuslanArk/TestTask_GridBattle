// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GridTypes.h"
#include "GridAStar.generated.h"

struct FPathRequest
{
	FGridCoordinate Start;
	FGridCoordinate Goal;
	FIntPoint  GridSize { 100,100 };
	
	TSet<FGridCoordinate> Blocked;
};

struct FSearchNode
{
	FGridCoordinate Coordinate;
	int32 CostFromStart { 0 };
	int32 EstimatedTotalCost { 0 };
	FGridCoordinate ParentCoordinate { INT32_MIN, INT32_MIN };
	int32  InsertionOrderForTies { 0 };

	bool operator<(const FSearchNode& Other) const
	{
		if (EstimatedTotalCost != Other.EstimatedTotalCost)
			return EstimatedTotalCost < Other.EstimatedTotalCost;

		if (CostFromStart != Other.CostFromStart)
			return CostFromStart > Other.CostFromStart;

		return InsertionOrderForTies < Other.InsertionOrderForTies;
	}
};

FORCEINLINE bool IsWithinGridBounds(const FGridCoordinate& Coordinate, const FIntPoint& GridSize)
{
	return Coordinate.X >= 0 && Coordinate.X < GridSize.X &&
			Coordinate.Y >= 0 && Coordinate.Y < GridSize.Y;
}

USTRUCT()
struct ILLUVIUMTT_API FGridAStar
{
	GENERATED_USTRUCT_BODY()

	static bool FindPath(const FPathRequest& Req, TArray<FGridCoordinate>& OutPath);
};
