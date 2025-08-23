// Fill out your copyright notice in the Description page of Project Settings.

#include "Core/BattleSimGameMode.h"
#include "Core/GridGameState.h"
#include "IlluviumTT/Public/GridMap/GridMap.h"
#include "IlluviumTT/Public/Spheres/SimulatedSphere.h"

ABattleSimGameMode::ABattleSimGameMode()
{
	PrimaryActorTick.bCanEverTick = false; // visuals update via delegate, not per-tick
}

void ABattleSimGameMode::BeginPlay()
{
	Super::BeginPlay();

	GridGameState = GetWorld()->GetGameState<AGridGameState>();
	check(GridGameState);

	ActiveGridMap = GridGameState->ActiveGridMap;

	GridGameState->OnSimulationStepProduced.AddDynamic(this, &ABattleSimGameMode::HandleSimulationStepProduced);

	SyncMissingVisuals();
}

void ABattleSimGameMode::ResetSimulationWithSeed(int32 Seed)
{
	if (!GridGameState) return;

	for (auto& Entry : VisualByUnitId)
	{
		if (ASimulatedSphere* Visual = Entry.Value)
		{
			Visual->Destroy();
		}
	}
	VisualByUnitId.Reset();

	GridGameState->ResetSimulation(Seed);
	SyncMissingVisuals();
}

void ABattleSimGameMode::HandleSimulationStepProduced(const FStepDelta& StepDelta)
{
	ApplyStepDeltaToVisuals(StepDelta);
}

void ABattleSimGameMode::SyncMissingVisuals()
{
	if (!SimulatedSphereClass || !GridGameState) return;

	const TMap<int32, FSimUnit>& Units = GridGameState->GetUnitsById();
	for (const auto& UnitEntry : Units)
	{
		const int32 UnitId = UnitEntry.Key;
		const FSimUnit& SimulationUnit = UnitEntry.Value;

		if (!SimulationUnit.bAlive) continue;
		if (VisualByUnitId.Contains(UnitId)) continue;

		const FVector SpawnLocation = CellToWorld(SimulationUnit.Cell);
		ASimulatedSphere* SpawnedVisual = GetWorld()->SpawnActor<ASimulatedSphere>(
			SimulatedSphereClass, SpawnLocation, FRotator::ZeroRotator);
		if (SpawnedVisual)
		{
			SpawnedVisual->Init(UnitId, SimulationUnit.Team, SpawnLocation, /*StepDuration*/
			                    1.f / FMath::Max(1.f, GridGameState->SimulationStepsPerSecond));
			VisualByUnitId.Add(UnitId, SpawnedVisual);
		}
	}
}

void ABattleSimGameMode::ApplyStepDeltaToVisuals(const FStepDelta& StepDelta)
{
	for (const FSimMove& MoveRecord : StepDelta.Moves)
	{
		ASimulatedSphere* Visual = VisualByUnitId.FindRef(MoveRecord.ActorId);
		if (!Visual) continue;

		const FVector FromWorld = CellToWorld(MoveRecord.From);
		const FVector ToWorld = CellToWorld(MoveRecord.To);
		Visual->OnNewCell(FromWorld, ToWorld);
	}

	for (const FSimEvent& SimulationEvent : StepDelta.Events)
	{
		ASimulatedSphere* Visual = VisualByUnitId.FindRef(SimulationEvent.ActorId);
		if (!Visual) continue;

		switch (SimulationEvent.EventType)
		{
		case EEventType::Attack: Visual->OnAttack();
			break;
		case EEventType::Hit: Visual->OnHit();
			break;
		case EEventType::Die: Visual->OnDie();
			break;
		default: break;
		}
	}

	CleanupDeadVisuals();
	SyncMissingVisuals();
}

void ABattleSimGameMode::CleanupDeadVisuals()
{
	const TMap<int32, FSimUnit>& Units = GridGameState->GetUnitsById();

	for (auto It = VisualByUnitId.CreateIterator(); It; ++It)
	{
		const int32 UnitId = It->Key;
		const FSimUnit* Unit = Units.Find(UnitId);
		if (!Unit || !Unit->bAlive)
		{
			It.RemoveCurrent();
		}
	}
}

FVector ABattleSimGameMode::CellToWorld(const FGridCoordinate& Cell) const
{
	if (!ActiveGridMap) return FVector::ZeroVector;

	const float CellSize = ActiveGridMap->CellSize;
	const int32 GridX = ActiveGridMap->XSize;
	const int32 GridY = ActiveGridMap->YSize;
	const bool bCenter = ActiveGridMap->bCenterGrid;

	const float OriginX = bCenter ? -((GridX - 1) * CellSize * 0.5f) : 0.f;
	const float OriginY = bCenter ? -((GridY - 1) * CellSize * 0.5f) : 0.f;

	return ActiveGridMap->GetActorLocation()
		+ FVector(OriginX + Cell.X * CellSize, OriginY + Cell.Y * CellSize, CellZOffset);
}
