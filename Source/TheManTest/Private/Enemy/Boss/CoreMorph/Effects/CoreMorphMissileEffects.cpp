#include "Enemy/Boss/CoreMorph/Effects/CoreMorphMissileEffects.h"
#include "Enemy/Boss/CoreMorph/Combat/CoreMorphMissileCombat.h"
#include "Enemy/Boss/CoreMorph/CoreMorphBoss.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/DecalComponent.h"
#include "Components/PointLightComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/StaticMesh.h"
namespace
{
 void MissileInstance(UInstancedStaticMeshComponent* P,const FTransform& T,float Fade=1)
 {const int32 I=P->AddInstance(T,true);P->SetCustomDataValue(I,0,Fade,false);}
 void MissileLine(UInstancedStaticMeshComponent* P,const FVector& A,const FVector& B,float Width,float Fade)
 {const FVector D=B-A;MissileInstance(P,FTransform(D.Rotation(),(A+B)*.5,FVector(D.Size(),Width,Width)/100),Fade);}
}
UCoreMorphMissileEffects::UCoreMorphMissileEffects(){PrimaryComponentTick.bCanEverTick=true;PrimaryComponentTick.bStartWithTickEnabled=false;PrimaryComponentTick.TickGroup=TG_PostUpdateWork;}
UInstancedStaticMeshComponent* UCoreMorphMissileEffects::MakePool(const TCHAR* Mesh,UMaterialInterface* Material)
{
 auto* P=NewObject<UInstancedStaticMeshComponent>(GetOwner(),NAME_None,RF_Transient);P->SetupAttachment(GetOwner()->GetRootComponent());P->SetAbsolute(true,true,true);P->SetWorldTransform(FTransform::Identity);P->SetStaticMesh(LoadObject<UStaticMesh>(nullptr,Mesh));P->SetMaterial(0,Material);P->SetNumCustomDataFloats(1);P->SetCollisionEnabled(ECollisionEnabled::NoCollision);P->SetCastShadow(false);P->SetCanEverAffectNavigation(false);P->RegisterComponent();Pools.Add(P);return P;
}
void UCoreMorphMissileEffects::BeginCue()
{
 EndCue();auto* B=Cast<ACoreMorphBoss>(GetOwner());if(!B || B->IsDead() || !B->MissileCombat->IsActive())return;
 MakePool(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"),HullMaterial);MakePool(TEXT("/Engine/BasicShapes/Cone.Cone"),EnergyMaterial);MakePool(TEXT("/Engine/BasicShapes/Cube.Cube"),HullMaterial);
 MakePool(TEXT("/Engine/BasicShapes/Sphere.Sphere"),EnergyMaterial);MakePool(TEXT("/Engine/BasicShapes/Cube.Cube"),EnergyMaterial);MakePool(TEXT("/Engine/BasicShapes/Sphere.Sphere"),BlastMaterial);MakePool(TEXT("/Engine/BasicShapes/Cube.Cube"),BlastMaterial);
 const float R=B->MissileCombat->GetLockedRadius();
 for(const auto& M:B->MissileCombat->GetMissiles())
 {
  auto* D=NewObject<UDecalComponent>(B,NAME_None,RF_Transient);D->SetupAttachment(B->GetRootComponent());D->SetAbsolute(true,true,true);D->DecalSize=FVector(180,R,R);D->SetWorldLocation(M.Ground+M.Normal*10);D->SetWorldRotation((-M.Normal).Rotation());D->SetDecalMaterial(WarningMaterial);D->RegisterComponent();Warnings.Add(D);WarningMIDs.Add(D->CreateDynamicMaterialInstance());
  auto* L=NewObject<UPointLightComponent>(B,NAME_None,RF_Transient);L->SetupAttachment(B->GetRootComponent());L->SetAbsolute(true,true,true);L->SetWorldLocation(M.Ground+M.Normal*220);L->SetLightColor(FLinearColor(1,.25f,.025f));L->SetAttenuationRadius(R*2);L->SetIntensity(0);L->SetCastShadows(false);L->RegisterComponent();Lights.Add(L);
 }
 bActive=true;SetComponentTickEnabled(true);Draw();
}
void UCoreMorphMissileEffects::EndCue()
{
 for(const auto& P:Pools)if(P)P->DestroyComponent();for(const auto& D:Warnings)if(D)D->DestroyComponent();for(const auto& L:Lights)if(L)L->DestroyComponent();Pools.Reset();Warnings.Reset();WarningMIDs.Reset();Lights.Reset();bActive=false;SetComponentTickEnabled(false);
}
void UCoreMorphMissileEffects::EndPlay(const EEndPlayReason::Type Reason){EndCue();Super::EndPlay(Reason);}
int32 UCoreMorphMissileEffects::GetWarningCount() const {int32 N=0;for(const auto& D:Warnings)N+=IsValid(D)?1:0;return N;}
void UCoreMorphMissileEffects::TickComponent(float Dt,ELevelTick Tick,FActorComponentTickFunction* Function)
{Super::TickComponent(Dt,Tick,Function);if(!bActive)return;auto* B=Cast<ACoreMorphBoss>(GetOwner());if(!B || B->IsDead() || !B->MissileCombat->IsActive()){EndCue();return;}if(!B->MissileCombat->IsPaused())Draw();}
void UCoreMorphMissileEffects::Draw()
{
 auto* C=CastChecked<ACoreMorphBoss>(GetOwner())->MissileCombat.Get();for(const auto& P:Pools)P->ClearInstances();
 const float Clock=C->GetClock(),R=C->GetLockedRadius();bool Pending=false;
 const auto& Missiles=C->GetMissiles();
 for(int32 I=0;I<Missiles.Num();++I)
 {
  const auto& M=Missiles[I];Pending|=!M.bLaunched;
  if(M.bResolved && Warnings[I]){Warnings[I]->DestroyComponent();Warnings[I]=nullptr;WarningMIDs[I]=nullptr;}
  else if(WarningMIDs[I])WarningMIDs[I]->SetScalarParameterValue(TEXT("Pulse"),.45f+.55f*FMath::Square(.5f+.5f*FMath::Sin(Clock*12+I)));
  if(M.bLaunched && !M.bResolved)
  {
   const FQuat Q=FQuat::FindBetweenNormals(FVector::UpVector,M.Direction);
   MissileInstance(Pools[0],FTransform(Q,M.Position,FVector(1.3,1.3,5)));
   MissileInstance(Pools[1],FTransform(Q,M.Position+M.Direction*310,FVector(1.3,1.3,1.6)));
   for(int32 F=0;F<4;++F){const FVector Fin=Q.RotateVector(FVector(FMath::Cos(F*UE_HALF_PI),FMath::Sin(F*UE_HALF_PI),0));MissileInstance(Pools[2],FTransform((Q*FQuat(FVector::UpVector,F*UE_HALF_PI)),M.Position-M.Direction*170+Fin*100,FVector(1.8,.16,1.7)));}
   MissileInstance(Pools[3],FTransform(Q,M.Position-M.Direction*320,FVector(1.7,1.7,5)),.9f);
  }
  if(M.bLaunched)
  {
   const float Fade=M.bResolved?FMath::Max(0.f,1-(Clock-M.ImpactAt)/.4f):1;
   for(int32 J=1;J<M.Trail.Num();++J)MissileLine(Pools[4],M.Trail[J-1],M.Trail[J],20+J*2,Fade*J/FMath::Max(1,M.Trail.Num()));
  }
  if(M.bImpact)
  {
   const float Age=Clock-M.ImpactAt,Fade=FMath::Clamp(1-Age/1.1f,0.f,1.f);const FQuat Plane=FQuat::FindBetweenNormals(FVector::UpVector,M.Normal);
   MissileInstance(Pools[5],FTransform(FQuat::Identity,M.Ground+M.Normal*100,FVector((80+R*.7f*(1-FMath::Exp(-Age*9)))/50)),FMath::Exp(-Age*8));
   for(int32 J=0;J<16;++J){const float A=J*UE_TWO_PI/16;FVector D=Plane.RotateVector(FVector(FMath::Cos(A),FMath::Sin(A),.25+.2*FMath::Sin(J*2.1)));MissileLine(Pools[6],M.Ground+D*(100+Age*R),M.Ground+D*(180+Age*R*1.5f),18*Fade,Fade);}
   const float Ring=R*FMath::Clamp(Age/.4f,0.f,1.f);for(int32 J=0;J<64;++J){const float A=J*UE_TWO_PI/64,Z=(J+1)*UE_TWO_PI/64;MissileLine(Pools[6],M.Ground+Plane.RotateVector(FVector(FMath::Cos(A)*Ring,FMath::Sin(A)*Ring,20)),M.Ground+Plane.RotateVector(FVector(FMath::Cos(Z)*Ring,FMath::Sin(Z)*Ring,20)),22*Fade,Fade);}
   Lights[I]->SetIntensity(650000*FMath::Exp(-Age*9));
  }
 }
 if(Pending)MissileInstance(Pools[3],FTransform(FQuat::Identity,C->GetCoreLocation(),FVector(2.5+.8*FMath::Sin(Clock*12))),.8f);
 for(const auto& P:Pools)P->MarkRenderStateDirty();
}
