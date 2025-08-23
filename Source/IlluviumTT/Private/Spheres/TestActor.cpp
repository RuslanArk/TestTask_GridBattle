// Fill out your copyright notice in the Description page of Project Settings.


#include "Spheres/TestActor.h"

#include "GridMap/GridMap.h"


ATestActor::ATestActor()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ATestActor::BeginPlay()
{
	Super::BeginPlay();

	const AGridGameState* GS = GetWorld()->GetGameState<AGridGameState>();
	if (!GS)
	{
		UE_LOG(LogTemp, Error, TEXT("No GridGameState"));
		return;
	}
	const AGridMap* Map = GS->GetGridMap();
	if (!Map)
	{
		UE_LOG(LogTemp, Error, TEXT("No GridMap in GameState"));
		return;
	}

	// Prepare request using map size
	FPathRequest Request{
		Start,
		Goal,
		FIntPoint(Map->XSize, Map->YSize)
	};

	// Optional: block a simple “wall” to verify detour
	if (bPlaceSimpleWall)
	{
		for (int32 y = 0; y < Map->YSize; ++y)
		{
			// Make a gap so path must route through it
			if (y == Map->YSize / 2) continue;
			Request.Blocked.Add(FGridCoordinate(WallX, y));
		}
	}

	// Run A*
	TArray<FGridCoordinate> Path;
	const bool bFound = FGridAStar::FindPath(Request, Path);

	UE_LOG(LogTemp, Display, TEXT("A* result: %s, nodes=%d"),
	       bFound ? TEXT("FOUND") : TEXT("NO PATH"),
	       Path.Num());

	// Draw result
	if (bFound && Path.Num() > 0)
	{
		DrawPath(Map, Path);
		DrawGridPoint(Map, Start, FColor::Cyan, 18.f);
		DrawGridPoint(Map, Goal, FColor::Yellow, 18.f);
	}
	else
	{
		// Visualize Start/Goal anyway
		DrawGridPoint(Map, Start, FColor::Red, 18.f);
		DrawGridPoint(Map, Goal, FColor::Red, 18.f);
	}
}

void ATestActor::DrawGridPoint(const AGridMap* Map, const FGridCoordinate& C, const FColor& Color, float Size)
{
	// Re-use GridMap's world mapping (same math it uses to place tiles)
	const float Ox = Map->bCenterGrid ? -((Map->XSize - 1) * Map->CellSize * 0.5f) : 0.f;
	const float Oy = Map->bCenterGrid ? -((Map->YSize - 1) * Map->CellSize * 0.5f) : 0.f;
	const FVector W(Ox + C.X * Map->CellSize, Oy + C.Y * Map->CellSize, DebugZ);
	DrawDebugSolidBox(GetWorld(), W, FVector(8, 8, 2), FQuat::Identity, Color, false, 10.f);
}

void ATestActor::DrawPath(const AGridMap* Map, const TArray<FGridCoordinate>& Path)
{
	for (int32 i = 1; i < Path.Num(); ++i)
	{
		const FGridCoordinate A = Path[i - 1];
		const FGridCoordinate B = Path[i];

		const float Ox = Map->bCenterGrid ? -((Map->XSize - 1) * Map->CellSize * 0.5f) : 0.f;
		const float Oy = Map->bCenterGrid ? -((Map->YSize - 1) * Map->CellSize * 0.5f) : 0.f;

		const FVector WA(Ox + A.X * Map->CellSize, Oy + A.Y * Map->CellSize, DebugZ);
		const FVector WB(Ox + B.X * Map->CellSize, Oy + B.Y * Map->CellSize, DebugZ);
		DrawDebugLine(GetWorld(), WA, WB, PathColor, false, 100.f, 0, 25.f);
	}
}
