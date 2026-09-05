#include "Enemy/_Shared/GAS/GameplayCues/GCN_EnemyHit.h"
#include "Core/_Shared/GAS/TheManGameplayTags.h"
#include "Enemy/_Shared/Effects/EnemyBloodSpray.h"
#include "Components/DecalComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

UGCN_EnemyHit::UGCN_EnemyHit()
{
	GameplayCueTag = TAG_GameplayCue_Character_Enemy_Hit;
}

bool UGCN_EnemyHit::OnExecute_Implementation(AActor* Target,const FGameplayCueParameters& Parameters) const
{
	if(!IsValid(Target)||!Target->GetWorld()||Target->GetNetMode()==NM_DedicatedServer)return false;
	Super::OnExecute_Implementation(Target,Parameters);
	UWorld* World=Target->GetWorld();
	const FVector Point=Parameters.Location;
	const FVector Normal=FVector(Parameters.Normal).GetSafeNormal(UE_SMALL_NUMBER,FVector::UpVector);
	if(BloodSprayMaterial)
	{
		FActorSpawnParameters Spawn;
		Spawn.SpawnCollisionHandlingOverride=ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		if(auto* Spray=World->SpawnActor<AEnemyBloodSpray>(Point+Normal*3.f,FRotator::ZeroRotator,Spawn))
			Spray->Initialize(BloodSprayMaterial,Normal,BloodScale);
	}
	if(BloodStainMaterial)
	{
		auto Fade=[this](UDecalComponent* Decal)
		{
			if(Decal){RandomizeDecalMaterial(Decal);Decal->SetFadeScreenSize(0.0001f);Decal->SetFadeOut(FMath::Max(0.f,BloodStainLifeSpan-2.f),FMath::Min(2.f,BloodStainLifeSpan),false);}
		};
		if(ACharacter* Character=Cast<ACharacter>(Target);Character && Character->GetMesh())
		{
			USkeletalMeshComponent* Mesh=Character->GetMesh();
			FHitResult MeshHit;
			FCollisionQueryParams Query(SCENE_QUERY_STAT(EnemyBloodMesh),true);
			const bool bSurface=Mesh->LineTraceComponent(MeshHit,Point+Normal*50.f,Point-Normal*100.f,Query);
			// Never project from a capsule hit in mid-air. If the surface trace misses, retry toward the closest bone.
			bool bBodyHit=bSurface;
			if(!bBodyHit)
			{
				FVector BonePoint;Mesh->FindClosestBone(Point,&BonePoint);
				const FVector Toward=(BonePoint-Point).GetSafeNormal(UE_SMALL_NUMBER,-Normal);
				bBodyHit=Mesh->LineTraceComponent(MeshHit,Point-Toward*30.f,BonePoint+Toward*30.f,Query);
			}
			if(bBodyHit)
			{
				const FVector SurfaceNormal=MeshHit.ImpactNormal.GetSafeNormal(UE_SMALL_NUMBER,Normal);
				const FName Bone=MeshHit.BoneName.IsNone()?Mesh->FindClosestBone(MeshHit.ImpactPoint):MeshHit.BoneName;
				const float Size=BloodScale*FMath::FRandRange(1.f-BloodSizeVariation,1.f+BloodSizeVariation);
				Fade(UGameplayStatics::SpawnDecalAttached(BloodStainMaterial,FVector(BodyStainProjectionDepth,10.f*Size,10.f*Size*FMath::FRandRange(.75f,1.25f)),Mesh,Bone,MeshHit.ImpactPoint+SurfaceNormal*.5f,
					MakeDecalRotation(SurfaceNormal,true),EAttachLocation::KeepWorldPosition,BloodStainLifeSpan));
			}
		}
		// A nearby wall behind the target or floor receives a stain; never put bullet holes on the enemy.
		FCollisionQueryParams Query(SCENE_QUERY_STAT(EnemyBloodWorld),true,Target);
		if(Parameters.Instigator.IsValid())Query.AddIgnoredActor(Parameters.Instigator.Get());
		if(Parameters.EffectCauser.IsValid())Query.AddIgnoredActor(Parameters.EffectCauser.Get());
		FHitResult Surface;
		const FVector Start=Point+Normal*4.f;
		bool bHit=World->LineTraceSingleByObjectType(Surface,Start,Point-Normal*180.f,FCollisionObjectQueryParams(ECC_WorldStatic),Query);
		if(!bHit)bHit=World->LineTraceSingleByObjectType(Surface,Start,Start-FVector(0,0,220.f),FCollisionObjectQueryParams(ECC_WorldStatic),Query);
		if(bHit)
		{
			const float Size=BloodScale*FMath::FRandRange(1.f-BloodSizeVariation,1.f+BloodSizeVariation);
			Fade(UGameplayStatics::SpawnDecalAtLocation(World,BloodStainMaterial,FVector(4.f,18.f*Size,18.f*Size*FMath::FRandRange(.75f,1.25f)),
				Surface.ImpactPoint+Surface.ImpactNormal,MakeDecalRotation(Surface.ImpactNormal,true),BloodStainLifeSpan));
		}
	}
	return true;
}
