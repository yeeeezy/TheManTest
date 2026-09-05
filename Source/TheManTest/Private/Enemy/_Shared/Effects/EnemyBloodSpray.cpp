#include "Enemy/_Shared/Effects/EnemyBloodSpray.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Kismet/GameplayStatics.h"
#include "Camera/PlayerCameraManager.h"
#include "UObject/ConstructorHelpers.h"

AEnemyBloodSpray::AEnemyBloodSpray()
{
 PrimaryActorTick.bCanEverTick=true;
 SetRootComponent(CreateDefaultSubobject<USceneComponent>(TEXT("Root")));
 static ConstructorHelpers::FObjectFinder<UStaticMesh> Plane(TEXT("/Engine/BasicShapes/Plane.Plane"));
 for(int32 Index=0;Index<9;++Index)
 {
  auto* Mesh=CreateDefaultSubobject<UStaticMeshComponent>(*FString::Printf(TEXT("Droplet%d"),Index));
  Mesh->SetupAttachment(RootComponent);
  Mesh->SetStaticMesh(Plane.Object);
  Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
  Mesh->SetCastShadow(false);
  Mesh->SetReceivesDecals(false);
  Droplets.Add(Mesh);
 }
}

void AEnemyBloodSpray::Initialize(UMaterialInterface* Material,const FVector& Direction,float Scale)
{
 SprayMaterial=UMaterialInstanceDynamic::Create(Material,this);
 const FVector Normal=Direction.GetSafeNormal(UE_SMALL_NUMBER,FVector::UpVector);
 for(int32 Index=0;Index<Droplets.Num();++Index)
 {
  UStaticMeshComponent* Mesh=Droplets[Index];
  Mesh->SetMaterial(0,SprayMaterial);
  const float Size=(Index==0 ? FMath::FRandRange(.18f,.3f) : FMath::FRandRange(0.035f,0.09f))*Scale;
  Mesh->SetWorldScale3D(FVector(Size));
  Velocities.Add((Normal*FMath::FRandRange(80.f,180.f)+FMath::VRand()*65.f+FVector(0,0,65.f))*Scale);
  Rolls.Add(FMath::FRandRange(0.f,2.f*PI));
 }
 SetLifeSpan(Duration);
}

void AEnemyBloodSpray::Tick(float DeltaSeconds)
{
 Super::Tick(DeltaSeconds);
 Age+=DeltaSeconds;
 if(SprayMaterial)SprayMaterial->SetScalarParameterValue(TEXT("Fade"),1.f-FMath::Clamp(Age/Duration,0.f,1.f));
 APlayerCameraManager* Camera=UGameplayStatics::GetPlayerCameraManager(this,0);
 for(int32 Index=0;Index<Velocities.Num();++Index)
 {
  Velocities[Index].Z-=380.f*DeltaSeconds;
  Droplets[Index]->AddWorldOffset(Velocities[Index]*DeltaSeconds);
  if(Camera)Droplets[Index]->SetWorldRotation(FRotationMatrix::MakeFromZ(Camera->GetCameraLocation()-Droplets[Index]->GetComponentLocation()).ToQuat()*FQuat(FVector::UpVector,Rolls[Index]));
 }
}
