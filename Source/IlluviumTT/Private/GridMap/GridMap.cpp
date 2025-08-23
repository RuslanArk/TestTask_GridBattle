// Fill out your copyright notice in the Description page of Project Settings.


#include "IlluviumTT/Public/GridMap/GridMap.h"

#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "IlluviumTT/Public/Core/GridGameState.h"


AGridMap::AGridMap()
{
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;

	InstancedMeshComponent = CreateDefaultSubobject<UHierarchicalInstancedStaticMeshComponent>(TEXT("GridInstancedMeshComponent"));
	SetRootComponent(InstancedMeshComponent);

	InstancedMeshComponent->NumCustomDataFloats = 1;
	InstancedMeshComponent->SetMobility(EComponentMobility::Static);
	InstancedMeshComponent->SetCastShadow(true);
}

void AGridMap::Rebuild()
{
	if (!TileMesh) return;

	InstancedMeshComponent->SetStaticMesh(TileMesh);
	if (OverrideMaterial)
	{
		InstancedMeshComponent->SetMaterial(0, OverrideMaterial);
	}

	InstancedMeshComponent->bEnableDensityScaling = false;

	BuildGrid();	
}

void AGridMap::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	Rebuild();
}

void AGridMap::BeginPlay()
{
	Super::BeginPlay();
	// Obviouslu shouldn't be here, just for testing and proof of concept simplify it 
	if (AGridGameState* GameState = GetWorld()->GetGameState<AGridGameState>())
	{
		GameState->SetGridMap(this);
	}
}

void AGridMap::BuildGrid()
{
	ClearGrid();
	if (!InstancedMeshComponent || !TileMesh) return;

	const int32 Total = XSize * YSize;
	InstancedMeshComponent->PreAllocateInstancesMemory(Total);

	const float OriginX = bCenterGrid ? -((XSize - 1) * CellSize * 0.5f) : 0.0f;
	const float OriginY = bCenterGrid ? -((YSize - 1) * CellSize * 0.5f) : 0.0f;

	int32 InstanceIndex = 0;
	for (int32 y = 0; y < YSize; ++y)
	{
		for (int32 x = 0; x < XSize; ++x)
		{
			const FVector Location(OriginX + x * CellSize, OriginY + y * CellSize, 0.0f);
			const FTransform Xform(FRotator::ZeroRotator, Location, FVector(1.0f));

			const int32 Idx = InstancedMeshComponent->AddInstance(Xform);

			const float Checker = (x + y) % 2 ? 1 : 0;

			InstancedMeshComponent->SetCustomDataValue(Idx, 0, Checker, false);

			InstanceIndex++;
		}
	}

	InstancedMeshComponent->MarkRenderStateDirty();
}

void AGridMap::ClearGrid()
{
	if (InstancedMeshComponent)
	{
		InstancedMeshComponent->ClearInstances();
	}
}
