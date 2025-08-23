// Fill out your copyright notice in the Description page of Project Settings.


#include "IlluviumTT/Public/Core/GridGameState.h"

#include "EngineUtils.h"
#include "IlluviumTT/Public/GridMap/GridMap.h"
#include "Navigation/GridAStar.h"

AGridGameState::AGridGameState()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AGridGameState::BeginPlay()
{
	Super::BeginPlay();
	DiscoverGridMap();
	InitializeFromConfig();
	StartSimulation();
}

void AGridGameState::DiscoverGridMap()
{
	for (TActorIterator<AGridMap> It(GetWorld()); It; ++It)
	{
		ActiveGridMap = *It;
		break;
	}

	if (ActiveGridMap)
	{
		SimulationConfig.GridSize = FIntPoint(ActiveGridMap->XSize, ActiveGridMap->YSize);
	}
}

void AGridGameState::InitializeFromConfig()
{
	StepDurationSeconds = 1.f / FMath::Max(1.f, SimulationStepsPerSecond);
	StepAccumulatorSeconds = 0.f;
}

void AGridGameState::StartSimulation()
{
	ResetSimulation(SimulationConfig.Seed);
}

void AGridGameState::ResetSimulation(int32 Seed)
{
	RandomStream.Initialize(Seed);
	UnitsById.Reset();
	NextUnitId = 1;

	DiscoverGridMap();

	SpawnInitialTeams();
}

void AGridGameState::SpawnInitialTeams()
{
	auto SpawnUnit = [&](EBattleTeam Team)
	{
		FSimUnit NewUnit;
		NewUnit.Id = NextUnitId++;
		NewUnit.Team = Team;
		NewUnit.HP = RandomStream.RandRange(SimulationConfig.MinHP, SimulationConfig.MaxHP);
		do { NewUnit.Cell = MakeRandomFreeCell(); }
		while (IsCellOccupied(NewUnit.Cell));
		UnitsById.Add(NewUnit.Id, NewUnit);
	};

	SpawnUnit(EBattleTeam::Red);
	SpawnUnit(EBattleTeam::Blue);
}

FGridCoordinate AGridGameState::MakeRandomFreeCell() const
{
	const int32 X = RandomStream.RandRange(0, SimulationConfig.GridSize.X - 1);
	const int32 Y = RandomStream.RandRange(0, SimulationConfig.GridSize.Y - 1);
	return FGridCoordinate(X, Y);
}

bool AGridGameState::IsCellOccupied(const FGridCoordinate& Cell, int32 IgnoredUnitId) const
{
	for (const auto& Entry : UnitsById)
	{
		const FSimUnit& Unit = Entry.Value;
		if (!Unit.bAlive) continue;
		if (Unit.Id == IgnoredUnitId) continue;
		if (Unit.Cell == Cell) return true;
	}
	return false;
}

int32 AGridGameState::FindClosestEnemyUnitId(const FSimUnit& SourceUnit) const
{
	int32 ClosestEnemyUnitId = -1;
	int32 ShortestGridDistance = TNumericLimits<int32>::Max();

	for (const auto& Entry : UnitsById)
	{
		const FSimUnit& CandidateUnit = Entry.Value;
		if (!CandidateUnit.bAlive || CandidateUnit.Team == SourceUnit.Team)
			continue;

		const int32 GridDistanceToCandidate = Manhattan(SourceUnit.Cell, CandidateUnit.Cell);
		const bool IsStrictlyCloser = GridDistanceToCandidate < ShortestGridDistance;
		const bool IsTieButSmallerId = (GridDistanceToCandidate == ShortestGridDistance) &&
			(CandidateUnit.Id < ClosestEnemyUnitId);

		if (IsStrictlyCloser || IsTieButSmallerId)
		{
			ShortestGridDistance = GridDistanceToCandidate;
			ClosestEnemyUnitId = CandidateUnit.Id;
		}
	}
	return ClosestEnemyUnitId;
}

bool AGridGameState::IsBattleOver() const
{
	bool AnyRed = false, AnyBlue = false;
	for (const auto& Entry : UnitsById)
	{
		const FSimUnit& Unit = Entry.Value;
		if (!Unit.bAlive) continue;
		if (Unit.Team == EBattleTeam::Red) AnyRed = true;
		else AnyBlue = true;
	}
	return !(AnyRed && AnyBlue);
}

void AGridGameState::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	StepAccumulatorSeconds += DeltaSeconds;
	while (StepAccumulatorSeconds >= StepDurationSeconds && !IsBattleOver())
	{
		FStepDelta ProducedStepDelta;
		RunOneSimulationStep(ProducedStepDelta);

		OnSimulationStepProduced.Broadcast(ProducedStepDelta);

		StepAccumulatorSeconds -= StepDurationSeconds;
	}
}

void AGridGameState::RunOneSimulationStep(FStepDelta& OutStepDelta)
{
	OutStepDelta.Moves.Reset();
	OutStepDelta.Events.Reset();

	TSet<FGridCoordinate> OccupiedCells;
	for (const auto& Entry : UnitsById)
	{
		const FSimUnit& Unit = Entry.Value;
		if (Unit.bAlive) OccupiedCells.Add(Unit.Cell);
	}

	TArray<int32> AliveUnitIds;
	for (const auto& Entry : UnitsById) if (Entry.Value.bAlive) AliveUnitIds.Add(Entry.Key);
	AliveUnitIds.Sort([](int32 A, int32 B) { return A < B; });

	struct FPlannedMove
	{
		int32 UnitId;
		FGridCoordinate FromCell;
		FGridCoordinate ToCell;
	};
	TArray<FPlannedMove> PlannedMoves;

	for (int32 UnitId : AliveUnitIds)
	{
		FSimUnit& ActingUnit = UnitsById[UnitId];

		if (ActingUnit.AttackCooldown > 0) { ActingUnit.AttackCooldown--; }

		const int32 ClosestEnemyUnitId = FindClosestEnemyUnitId(ActingUnit);
		if (ClosestEnemyUnitId < 0) continue;

		FSimUnit& TargetUnit = UnitsById[ClosestEnemyUnitId];
		const int32 GridDistanceToTarget = Manhattan(ActingUnit.Cell, TargetUnit.Cell);

		if (GridDistanceToTarget <= SimulationConfig.AttackRangeSquares)
		{
			const bool IsAttackReady = (ActingUnit.AttackCooldown == 0);
			if (IsAttackReady)
			{
				ActingUnit.AttackCooldown = SimulationConfig.AttackPeriodSteps;

				OutStepDelta.Events.Add({EEventType::Attack, ActingUnit.Id, TargetUnit.Id});

				TargetUnit.HP -= 1;
				OutStepDelta.Events.Add({EEventType::Hit, TargetUnit.Id, ActingUnit.Id});

				if (TargetUnit.HP <= 0 && TargetUnit.bAlive)
				{
					TargetUnit.bAlive = false;
					OccupiedCells.Remove(TargetUnit.Cell);
					OutStepDelta.Events.Add({EEventType::Die, TargetUnit.Id, ActingUnit.Id});
				}
			}
			continue;
		}

		FPathRequest PathRequest;
		PathRequest.Start = ActingUnit.Cell;
		PathRequest.Goal = TargetUnit.Cell;
		PathRequest.GridSize = SimulationConfig.GridSize;
		PathRequest.Blocked = OccupiedCells;
		PathRequest.Blocked.Remove(ActingUnit.Cell);

		TArray<FGridCoordinate> Path;
		const bool bFound = FGridAStar::FindPath(PathRequest, Path);
		if (bFound && Path.Num() >= 2)
		{
			const int32 MaxCellsThisStep = FMath::Clamp(SimulationConfig.MoveSquaresPerStep, 1, 8);
			const int32 TargetPathIndex = FMath::Min(1 + (MaxCellsThisStep - 1), Path.Num() - 1);
			const FGridCoordinate NextCell = Path[TargetPathIndex];

			if (!OccupiedCells.Contains(NextCell))
			{
				PlannedMoves.Add({ActingUnit.Id, ActingUnit.Cell, NextCell});
				OccupiedCells.Remove(ActingUnit.Cell);
				OccupiedCells.Add(NextCell); // reserve
			}
		}
	}

	PlannedMoves.Sort([](const FPlannedMove& L, const FPlannedMove& R) { return L.UnitId < R.UnitId; });
	for (const FPlannedMove& Move : PlannedMoves)
	{
		FSimUnit& MovingUnit = UnitsById[Move.UnitId];
		if (!MovingUnit.bAlive) continue;
		if (MovingUnit.Cell != Move.FromCell) continue;

		MovingUnit.Cell = Move.ToCell;
		OutStepDelta.Moves.Add({MovingUnit.Id, Move.FromCell, Move.ToCell});
	}
}
