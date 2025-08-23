// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BattleTypes.h"
#include "GameFramework/GameStateBase.h"
#include "IlluviumTT/Public/Interfaces/GetGridMapInterface.h"
#include "GridGameState.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSimulationStepProduced, const FStepDelta&, StepDelta);

class AGridMap;

UCLASS()
class ILLUVIUMTT_API AGridGameState : public AGameStateBase, public IGetGridMapInterface
{
	GENERATED_BODY()

public:
	AGridGameState();

	virtual AGridMap* GetGridMap() const { return ActiveGridMap; }
	void SetGridMap(AGridMap* NewGridMap) { ActiveGridMap = NewGridMap; }

	const TMap<int32, FSimUnit>& GetUnitsById() const { return UnitsById; }

	UFUNCTION(BlueprintCallable, Category="Simulation")
	void ResetSimulation(int32 Seed);

	UFUNCTION(BlueprintCallable, Category="Simulation")
	void StartSimulation();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

private:
	void DiscoverGridMap();
	void InitializeFromConfig();
	void SpawnInitialTeams();
	FGridCoordinate MakeRandomFreeCell() const;
	bool IsCellOccupied(const FGridCoordinate& Cell, int32 IgnoredUnitId = -1) const;
	int32 FindClosestEnemyUnitId(const FSimUnit& SourceUnit) const;
	bool IsBattleOver() const;

	void RunOneSimulationStep(FStepDelta& OutStepDelta);

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Grid")
	AGridMap* ActiveGridMap = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Simulation")
	FSimConfig SimulationConfig;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Simulation", meta=(ClampMin="1.0"))
	float SimulationStepsPerSecond = 10.f;

	UPROPERTY(BlueprintAssignable, Category="Simulation")
	FOnSimulationStepProduced OnSimulationStepProduced;

private:
	FRandomStream RandomStream;

	TMap<int32, FSimUnit> UnitsById;

	int32 NextUnitId = 1;

	float StepAccumulatorSeconds = 0.f;

	float StepDurationSeconds = 0.1f;
};
