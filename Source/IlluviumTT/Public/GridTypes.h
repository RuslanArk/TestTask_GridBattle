// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GridTypes.generated.h"

USTRUCT(BlueprintType)
struct FGridCoordinate
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 X = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Y = 0;

	FGridCoordinate()
	{
		X = 0;
		Y = 0;
	}
	FGridCoordinate(int32 InX, int32 InY) : X(InX), Y(InY) {}
	
	bool operator==(const FGridCoordinate& R) const { return X==R.X && Y==R.Y; }
};

FORCEINLINE int32 Manhattan(const FGridCoordinate& A, const FGridCoordinate& B)
{
	return FMath::Abs(A.X - B.X) + FMath::Abs(A.Y - B.Y);
}

FORCEINLINE uint32 GetTypeHash(const FGridCoordinate& C)
{
	return HashCombine(GetTypeHash(C.X), GetTypeHash(C.Y));
}