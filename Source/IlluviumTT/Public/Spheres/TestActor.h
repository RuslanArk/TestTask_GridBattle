// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "IlluviumTT/Public/Navigation/GridAStar.h"
#include "IlluviumTT/Public/Core/GridGameState.h"
#include "TestActor.generated.h"

class AGridMap;

UCLASS()
class ILLUVIUMTT_API ATestActor : public AActor
{
	GENERATED_BODY()

public:
	ATestActor();

	UPROPERTY(EditAnywhere, Category="Test")
	FGridCoordinate Start{2, 2};
	UPROPERTY(EditAnywhere, Category="Test")
	FGridCoordinate Goal{25, 18};
	UPROPERTY(EditAnywhere, Category="Test")
	bool bPlaceSimpleWall = true;
	UPROPERTY(EditAnywhere, Category="Test")
	int32 WallX = 10; // vertical wall at X = WallX
	UPROPERTY(EditAnywhere, Category="Test")
	float DebugZ = 140.f;
	UPROPERTY(EditAnywhere, Category="Test")
	FColor PathColor = FColor::Green;

protected:
	virtual void BeginPlay() override;

private:
	void DrawGridPoint(const AGridMap* Map, const FGridCoordinate& C, const FColor& Color, float Size = 12.f);
	void DrawPath(const AGridMap* Map, const TArray<FGridCoordinate>& Path);
};
