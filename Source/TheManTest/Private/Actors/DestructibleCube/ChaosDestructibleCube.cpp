#include "Actors/DestructibleCube/ChaosDestructibleCube.h"
#include "GeometryCollection/GeometryCollectionComponent.h"
#include "GeometryCollection/GeometryCollectionObject.h"
#if WITH_EDITOR
#include "GeometryCollection/GeometryCollectionEngineConversion.h"
#include "GeometryCollection/GeometryCollectionClusteringUtility.h"
#include "GeometryCollection/GeometryCollection.h"
#include "Engine/StaticMesh.h"
#include "UObject/Package.h"
#include "Misc/PackageName.h"
#include "PlanarCut.h"
#include "Voronoi/Voronoi.h"
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
UGeometryCollection* AChaosDestructibleCube::CreateTestCubeAsset(bool bRebuild)
{
#if WITH_EDITOR
 const TCHAR* Path=TEXT("/Game/Actors/DestructibleCube/Meshes/GC_DestructibleCube");
 auto* Existing=FPackageName::DoesPackageExist(Path)?LoadObject<UGeometryCollection>(nullptr,TEXT("/Game/Actors/DestructibleCube/Meshes/GC_DestructibleCube.GC_DestructibleCube")):nullptr;
 if(Existing&&!bRebuild)return Existing;
 auto* Mesh=LoadObject<UStaticMesh>(nullptr,TEXT("/Engine/BasicShapes/Cube.Cube"));
 if(!Mesh)return nullptr;
 // Build off to the side: a failed cut must not replace the placed asset.
 auto* Working=NewObject<UGeometryCollection>();
 FGeometryCollectionEngineConversion::AppendStaticMesh(Mesh,nullptr,FTransform::Identity,Working,true,true);
 FRandomStream Random(92417);
 TArray<FVector> Sites;
 // Broadly spaced sites make larger fragments; a local concentration supplies small chips.
 for(int I=0;I<24;++I)
 {
  Sites.Emplace(Random.FRandRange(-47.f,47.f),Random.FRandRange(-47.f,47.f),Random.FRandRange(-47.f,47.f));
 }
 for(int I=0;I<18;++I)
  Sites.Add(FVector(-20,12,8)+Random.VRand()*Random.FRandRange(4.f,23.f));
 FVoronoiDiagram Diagram(Sites,FBox(FVector(-51),FVector(51)),.1);
 FPlanarCells Cells(Sites,Diagram);
 FNoiseSettings Noise;
 Noise.Amplitude=.8f;Noise.Frequency=.12f;Noise.Octaves=3;Noise.PointSpacing=5.f;
 Cells.SetNoise(Noise);
 auto Data=Working->GetGeometryCollection();
 if(CutWithPlanarCells(Cells,*Data,0,0.0,5.0,92417)==INDEX_NONE)return nullptr;
 // Cutting converts the source bone into the common cluster; do not introduce a second root.
 Working->UpdateGeometryDependentProperties();
 auto* Collection=Existing?Existing:NewObject<UGeometryCollection>(CreatePackage(Path),TEXT("GC_DestructibleCube"),RF_Public|RF_Standalone|RF_Transactional);
 Collection->Modify();
 Collection->SetGeometryCollection(Data);
 Collection->Materials=Working->Materials;
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
