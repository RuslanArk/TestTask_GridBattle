// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BattleTypes.h"
#include "GameFramework/GameModeBase.h"
#include "BattleSimGameMode.generated.h"

class ASimulatedSphere;
class AGridGameState;
class AGridMap;

UCLASS()
class ILLUVIUMTT_API ABattleSimGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ABattleSimGameMode();

	UFUNCTION(BlueprintCallable, Category="Simulation")
	void ResetSimulationWithSeed(int32 Seed);

protected:
	virtual void BeginPlay() override;

private:
	UFUNCTION()
	void HandleSimulationStepProduced(const FStepDelta& StepDelta);

	void SyncMissingVisuals();
	void ApplyStepDeltaToVisuals(const FStepDelta& StepDelta);
	void CleanupDeadVisuals();

	FVector CellToWorld(const FGridCoordinate& Cell) const;

public:
	UPROPERTY(EditAnywhere, Category="Visual")
	TSubclassOf<ASimulatedSphere> SimulatedSphereClass;

	UPROPERTY(EditAnywhere, Category="Visual")
	float CellZOffsetUU = 40.f;

private:
	UPROPERTY()
	AGridGameState* GridGameState = nullptr;
	UPROPERTY()
	AGridMap* ActiveGridMap = nullptr;

	TMap<int32, ASimulatedSphere*> VisualByUnitId;
};
