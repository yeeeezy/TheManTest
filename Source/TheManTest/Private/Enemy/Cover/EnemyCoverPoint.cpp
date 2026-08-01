#include "Enemy/Cover/EnemyCoverPoint.h"
#include "Components/BoxComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "EngineUtils.h"
#include "Engine/World.h"

AEnemyCoverPoint::AEnemyCoverPoint()
{
	PrimaryActorTick.bCanEverTick = false;
	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);
	CoverMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CoverMesh"));
	CoverMesh->SetupAttachment(Root);
	CoverCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("CoverCollision"));
	CoverCollision->SetupAttachment(Root);
	CoverCollision->SetBoxExtent(FVector(60.f, 20.f, 90.f));
	CoverCollision->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	CoverCollision->SetCollisionObjectType(ECC_WorldStatic);
	CoverCollision->SetCollisionResponseToAllChannels(ECR_Block);
	StandPoint = CreateDefaultSubobject<USceneComponent>(TEXT("StandPoint"));
	StandPoint->SetupAttachment(Root);
	StandPoint->SetRelativeLocation(FVector(-100.f, 0.f, 0.f));
}

FVector AEnemyCoverPoint::GetStandLocation() const
{
	return StandPoint->GetComponentLocation();
}

AEnemyCoverPoint* AEnemyCoverPoint::FindBestCover(const UObject* WorldContextObject,
	const FVector& SeekerLocation, const FVector& ThreatLocation, float MaxDistance)
{
	const UWorld* World = WorldContextObject ? WorldContextObject->GetWorld() : nullptr;
	if (!World) return nullptr;

	AEnemyCoverPoint* Best = nullptr;
	float BestScore = TNumericLimits<float>::Max();
	for (TActorIterator<AEnemyCoverPoint> It(World); It; ++It)
	{
		AEnemyCoverPoint* Candidate = *It;
		const FVector Stand = Candidate->GetStandLocation();
		const float Distance = FVector::Dist2D(SeekerLocation, Stand);
		if (Distance > MaxDistance) continue;

		FHitResult Hit;
		FCollisionQueryParams Params(SCENE_QUERY_STAT(EnemyCoverQuery), false);
		const bool bBlocked = World->LineTraceSingleByChannel(Hit, ThreatLocation + FVector(0,0,60),
			Stand + FVector(0,0,60), ECC_Visibility, Params);
		if (!bBlocked) continue;

		const FVector ThreatToCover = (Candidate->GetActorLocation() - ThreatLocation).GetSafeNormal2D();
		const FVector CoverToStand = (Stand - Candidate->GetActorLocation()).GetSafeNormal2D();
		const float BehindPenalty = (1.f - FVector::DotProduct(ThreatToCover, CoverToStand)) * 250.f;
		const float Score = Distance + BehindPenalty;
		if (Score < BestScore) { BestScore = Score; Best = Candidate; }
	}
	return Best;
}
