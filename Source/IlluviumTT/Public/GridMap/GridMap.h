// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GridMap.generated.h"

class UHierarchicalInstancedStaticMeshComponent;
class UStaticMesh;
class UMaterialInterface;

UCLASS()
class ILLUVIUMTT_API AGridMap : public AActor
{
	GENERATED_BODY()

public:
	AGridMap();
	
	UFUNCTION(CallInEditor, Category="Grid")
	void Rebuild();

	void ClearGrid();
	
protected:
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void BeginPlay() override;

private:
	void BuildGrid();

public:
	/** Grid Params **/
	UPROPERTY(EditAnywhere, Category="Grid", meta=(ClampMin="1"))
	int32 XSize = 100;

	UPROPERTY(EditAnywhere, Category="Grid", meta=(ClampMin="1"))
	int32 YSize = 100;

	UPROPERTY(EditAnywhere, Category="Grid", meta=(ClampMin="1.0"))
	float CellSize = 100.0f;

	UPROPERTY(EditAnywhere, Category="Grid")
	bool bCenterGrid = true;

	/** Grid Mesh **/
	UPROPERTY(EditAnywhere, Category="Mesh")
	UStaticMesh* TileMesh = nullptr;

	UPROPERTY(EditAnywhere, Category="Mesh")
	UMaterialInterface* OverrideMaterial = nullptr;

	// Checker tint 
	UPROPERTY(EditAnywhere, Category="Visual", meta=(ClampMin="0.0", ClampMax="1.0"))
	float CheckerDelta = 0.15f;

private:
	UPROPERTY(VisibleAnywhere, Category="Components")
	UHierarchicalInstancedStaticMeshComponent* InstancedMeshComponent = nullptr;
};
