// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GridTypes.h"
#include "UObject/Object.h"
#include "BattleTypes.generated.h"

UENUM(BlueprintType)
enum class EBattleTeam : uint8
{
	Red,
	Blue
};

UENUM(BlueprintType)
enum class EEventType : uint8
{
	Attack,
	Hit,
	Die
};

USTRUCT(BlueprintType)
struct FSimConfig
{
	GENERATED_USTRUCT_BODY()
	// TODO: [RUSLAN.A] As it is used in many places should have one source, refactor if have time
	FIntPoint GridSize = {100, 100};

	// Core rules
	UPROPERTY(EditAnywhere)
	int32 MoveSquaresPerStep = 1;
	UPROPERTY(EditAnywhere)
	int32 AttackRangeSquares = 1;
	UPROPERTY(EditAnywhere)
	int32 AttackPeriodSteps = 2;
	UPROPERTY(EditAnywhere)
	int32 MinHP = 2;
	UPROPERTY(EditAnywhere)
	int32 MaxHP = 5;

	// Random seed
	UPROPERTY(EditAnywhere)
	int32 Seed = 1337;
};

USTRUCT()
struct FSimEvent
{
	GENERATED_USTRUCT_BODY()
	
	UPROPERTY()	
	EEventType EventType = EEventType::Attack;
	UPROPERTY()
	int32 ActorId = -1;
	UPROPERTY()
	int32 OtherId = -1;
};

USTRUCT()
struct FSimMove
{
	GENERATED_USTRUCT_BODY()
	
	UPROPERTY()
	int32 ActorId = -1;
	UPROPERTY()
	FGridCoordinate From;
	UPROPERTY()
	FGridCoordinate To;
};

USTRUCT()
struct FStepDelta
{
	GENERATED_USTRUCT_BODY()
	
	UPROPERTY()
	TArray<FSimMove> Moves;
	UPROPERTY()
	TArray<FSimEvent> Events;
};

USTRUCT()
struct FSimUnit
{
	GENERATED_USTRUCT_BODY()

	int32 Id = -1;
	EBattleTeam Team = EBattleTeam::Red;
	int32 HP = 1;
	FGridCoordinate Cell;
	int32 AttackCooldown = 0;
	bool bAlive = true;
};
