// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BattleTypes.h"
#include "GameFramework/Actor.h"
#include "SimulatedSphere.generated.h"

UCLASS()
class ILLUVIUMTT_API ASimulatedSphere : public AActor
{
	GENERATED_BODY()

public:
	ASimulatedSphere();

	virtual void Tick(float DeltaTime) override;

	void Init(int32 InId, EBattleTeam InTeam, const FVector& StartWorld, float InStepDuration);
	void OnNewCell(const FVector& FromWorld, const FVector& ToWorld);
	void OnAttack();
	void OnHit();
	void OnDie();

protected:
	virtual void BeginPlay() override;

private:
	void ApplyTeamColor();

public:
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* SphereMesh = nullptr;

	UPROPERTY(EditAnywhere, Category="Visual")
	float StepDurationSec = 0.1f;
	UPROPERTY(EditAnywhere, Category="Visual")
	float EmissiveDecay = 12.f;

private:
	int32 UnitId = -1;
	EBattleTeam Team = EBattleTeam::Red;

	FVector FromPos, ToPos;
	float LerpAlpha = 1.f;
	float StepDuration = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	UMaterialInstanceDynamic* MaterialInstance = nullptr;

	FName BaseColorParam = TEXT("BaseColor");
	FName EmissiveParam = TEXT("Emissive");
	float EmissiveCurrent = 0.f;
	float EmissiveTarget = 0.f;
	bool bDying = false;
	float DieTimer = 0.f;
	float DieFadeTime = 0.55f;
};
