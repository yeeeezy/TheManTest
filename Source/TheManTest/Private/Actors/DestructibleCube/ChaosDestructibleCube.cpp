#include "Actors/DestructibleCube/ChaosDestructibleCube.h"
#include "GeometryCollection/GeometryCollectionComponent.h"
#include "GeometryCollection/GeometryCollectionObject.h"
#if WITH_EDITOR
#include "GeometryCollection/GeometryCollectionEngineConversion.h"
#include "GeometryCollection/GeometryCollectionClusteringUtility.h"
#include "GeometryCollection/GeometryCollection.h"
#include "Engine/StaticMesh.h"
#include "UObject/Package.h"
#endif

AChaosDestructibleCube::AChaosDestructibleCube()
{
 PrimaryActorTick.bCanEverTick=false;
 GeometryCollection=CreateDefaultSubobject<UGeometryCollectionComponent>(TEXT("GeometryCollection"));
 SetRootComponent(GeometryCollection);
 GeometryCollection->SetCollisionObjectType(ECC_Destructible);
 GeometryCollection->SetCollisionResponseToAllChannels(ECR_Block);
 GeometryCollection->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
 GeometryCollection->SetEnableDamageFromCollision(false);
}
void AChaosDestructibleCube::OnConstruction(const FTransform& Transform)
{
 Super::OnConstruction(Transform);
 GeometryCollection->SetRestCollection(FractureAsset);
 GeometryCollection->SetDamageThreshold({FMath::Max(1.f,Toughness)});
 GeometryCollection->bUseSizeSpecificDamageThreshold=false;
 GeometryCollection->SetEnableDamageFromCollision(false);
}
UGeometryCollection* AChaosDestructibleCube::CreateTestCubeAsset()
{
#if WITH_EDITOR
 const TCHAR* Path=TEXT("/Game/Actors/DestructibleCube/Meshes/GC_DestructibleCube");
 if(auto* Existing=LoadObject<UGeometryCollection>(nullptr,TEXT("/Game/Actors/DestructibleCube/Meshes/GC_DestructibleCube.GC_DestructibleCube")))
 {
  Existing->UpdateGeometryDependentProperties();
  Existing->InvalidateCollection();
  Existing->CreateSimulationData();
  Existing->MarkPackageDirty();
  return Existing;
 }
 auto* Mesh=LoadObject<UStaticMesh>(nullptr,TEXT("/Engine/BasicShapes/Cube.Cube"));
 if(!Mesh)return nullptr;
 auto* Collection=NewObject<UGeometryCollection>(CreatePackage(Path),TEXT("GC_DestructibleCube"),RF_Public|RF_Standalone|RF_Transactional);
 // 27 closed chunks, assembled into a single 100cm clustered cube (no runtime mesh generation).
 for(int X=0;X<3;++X)for(int Y=0;Y<3;++Y)for(int Z=0;Z<3;++Z)
 {
  const FTransform Chunk(FQuat::Identity,FVector(X-1,Y-1,Z-1)*(100.f/3.f),FVector(1.f/3.f));
  FGeometryCollectionEngineConversion::AppendStaticMesh(Mesh,nullptr,Chunk,Collection,true,false);
 }
 auto Data=Collection->GetGeometryCollection();
 FGeometryCollectionClusteringUtility::ClusterAllBonesUnderNewRoot(Data.Get());
 Collection->EnableClustering=true;
 Collection->DamageThreshold={100000.f};
 Collection->bUseSizeSpecificDamageThreshold=false;
 Collection->UpdateGeometryDependentProperties();
 Collection->InvalidateCollection();
 Collection->CreateSimulationData();
 Collection->MarkPackageDirty();
 return Collection;
#else
 return nullptr;
#endif
}
