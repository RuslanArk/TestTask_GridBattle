// Fill out your copyright notice in the Description page of Project Settings.


#include "Spheres/SimulatedSphere.h"


ASimulatedSphere::ASimulatedSphere()
{
	PrimaryActorTick.bCanEverTick = true;

	PrimaryActorTick.bCanEverTick = true;
	SphereMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SphereMesh"));
	SetRootComponent(SphereMesh);
	SphereMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ASimulatedSphere::BeginPlay()
{
	Super::BeginPlay();

	if (SphereMesh && SphereMesh->GetMaterial(0))
	{
		MaterialInstance = SphereMesh->CreateDynamicMaterialInstance(0);
	}

	ApplyTeamColor();
}

void ASimulatedSphere::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (LerpAlpha < 1.f && StepDuration > 0.f)
	{
		LerpAlpha = FMath::Min(1.f, LerpAlpha + DeltaTime / StepDuration);
		const FVector P = FMath::Lerp(FromPos, ToPos, LerpAlpha);
		SetActorLocation(P);
	}

	if (MaterialInstance)
	{
		EmissiveCurrent = FMath::FInterpTo(EmissiveCurrent, EmissiveTarget, DeltaTime, EmissiveDecay);
		MaterialInstance->SetScalarParameterValue(EmissiveParam, EmissiveCurrent);
		EmissiveTarget = FMath::FInterpTo(EmissiveTarget, 0.f, DeltaTime, EmissiveDecay * 0.5f);
	}

	if (bDying)
	{
		DieTimer += DeltaTime;
		const float Opacity = 1.f - FMath::Clamp(DieTimer / DieFadeTime, 0.f, 1.f);
		SphereMesh->SetScalarParameterValueOnMaterials(TEXT("Opacity"), Opacity);
		if (DieTimer >= DieFadeTime) Destroy();
	}
}

void ASimulatedSphere::ApplyTeamColor()
{
	if (!MaterialInstance) return;
	const FLinearColor Color = (Team == EBattleTeam::Red) ? FLinearColor(1,0,0) : FLinearColor(0,0.2f,1);
	MaterialInstance->SetVectorParameterValue(BaseColorParam, Color);
}

void ASimulatedSphere::Init(int32 InId, EBattleTeam InTeam, const FVector& StartWorld, float InStepDuration)
{
	UnitId = InId;
	Team = InTeam;
	StepDuration = InStepDuration;
	FromPos = ToPos = StartWorld;
	SetActorLocation(StartWorld);
	ApplyTeamColor();
}

void ASimulatedSphere::OnNewCell(const FVector& FromWorld, const FVector& ToWorld)
{
	FromPos = FromWorld;
	ToPos = ToWorld;
	LerpAlpha = 0.f;
}

void ASimulatedSphere::OnAttack()
{
	EmissiveTarget = 25.f;
}

void ASimulatedSphere::OnHit()
{
	EmissiveTarget = 40.f;
}

void ASimulatedSphere::OnDie()
{
	bDying = true;
	DieTimer = 0.f;
	EmissiveTarget = 80.f;
}
