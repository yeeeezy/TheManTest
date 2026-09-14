#include "Enemy/Boss/CoreMorph/Effects/CoreMorphTailEffects.h"
#include "Enemy/Boss/CoreMorph/CoreMorphBoss.h"
#include "Enemy/Boss/CoreMorph/Combat/CoreMorphScorpionCombat.h"
#include "Core/_Shared/GAS/TheManGameplayTags.h"
#include "AbilitySystemComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/DecalComponent.h"
#include "Components/PointLightComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/StaticMesh.h"
namespace
{
 template<class T> void DestroyFX(TObjectPtr<T>& C){if(C){C->DestroyComponent();C=nullptr;}}
 void Line(UInstancedStaticMeshComponent* Pool,const FVector& A,const FVector& B,float Width)
 {const FVector D=B-A;Pool->AddInstance(FTransform(D.Rotation(),(A+B)*.5,FVector(D.Size(),Width,Width)/100),true);}
}
UCoreMorphTailEffects::UCoreMorphTailEffects()
{PrimaryComponentTick.bCanEverTick=true;PrimaryComponentTick.bStartWithTickEnabled=false;PrimaryComponentTick.TickGroup=TG_PostUpdateWork;}
UInstancedStaticMeshComponent* UCoreMorphTailEffects::MakePool(const TCHAR* Mesh,UMaterialInterface* Material)
{
 auto* C=NewObject<UInstancedStaticMeshComponent>(GetOwner(),NAME_None,RF_Transient);
 C->SetupAttachment(GetOwner()->GetRootComponent());C->SetAbsolute(true,true,true);C->SetWorldTransform(FTransform::Identity);
 C->SetStaticMesh(LoadObject<UStaticMesh>(nullptr,Mesh));C->SetMaterial(0,Material);C->SetCollisionEnabled(ECollisionEnabled::NoCollision);
 C->SetCastShadow(false);C->SetCanEverAffectNavigation(false);C->RegisterComponent();return C;
}
UPointLightComponent* UCoreMorphTailEffects::MakeLight()
{auto* L=NewObject<UPointLightComponent>(GetOwner(),NAME_None,RF_Transient);L->SetupAttachment(GetOwner()->GetRootComponent());L->SetAbsolute(true,true,true);L->SetCastShadows(false);L->SetLightColor(FLinearColor(.12,.48,1));L->SetAttenuationRadius(2600);L->RegisterComponent();return L;}
void UCoreMorphTailEffects::BeginCharge(const FGameplayCueParameters& P)
{
 EndCharge();bCharging=true;ChargeClock=0;WarningCenter=P.Location;WarningRadius=FMath::Max(100.f,P.RawMagnitude);
 Orbs=MakePool(TEXT("/Engine/BasicShapes/Sphere.Sphere"),EnergyMaterial);
 ChargeArcs=MakePool(TEXT("/Engine/BasicShapes/Cube.Cube"),LightningMaterial);ChargeLight=MakeLight();
 Warning=NewObject<UDecalComponent>(GetOwner(),NAME_None,RF_Transient);Warning->SetupAttachment(GetOwner()->GetRootComponent());Warning->SetAbsolute(true,true,true);
 Warning->DecalSize=FVector(180,WarningRadius,WarningRadius);Warning->SetWorldLocation(WarningCenter+FVector(P.Normal)*15);
 Warning->SetWorldRotation((-FVector(P.Normal)).Rotation());Warning->SetDecalMaterial(WarningMaterial);Warning->RegisterComponent();WarningMID=Warning->CreateDynamicMaterialInstance();
 SetComponentTickEnabled(true);DrawCharge(0);
}
void UCoreMorphTailEffects::EndCharge()
{bCharging=false;DestroyFX(Orbs);DestroyFX(ChargeArcs);DestroyFX(Warning);DestroyFX(ChargeLight);WarningMID=nullptr;if(!bBlasting)SetComponentTickEnabled(false);}
void UCoreMorphTailEffects::BeginBlast(const FGameplayCueParameters& P)
{EndBlast();bBlasting=true;BlastClock=0;BlastCenter=P.Location;BlastNormal=FVector(P.Normal).GetSafeNormal();BlastRadius=FMath::Max(100.f,P.RawMagnitude);BlastArcs=MakePool(TEXT("/Engine/BasicShapes/Cube.Cube"),LightningMaterial);BlastCore=MakePool(TEXT("/Engine/BasicShapes/Sphere.Sphere"),EnergyMaterial);BlastCoreMID=BlastCore->CreateDynamicMaterialInstance(0);BlastLight=MakeLight();BlastLight->SetWorldLocation(BlastCenter+BlastNormal*250);SetComponentTickEnabled(true);DrawBlast(0);}
void UCoreMorphTailEffects::EndBlast()
{bBlasting=false;DestroyFX(BlastArcs);DestroyFX(BlastCore);BlastCoreMID=nullptr;DestroyFX(BlastLight);if(!bCharging)SetComponentTickEnabled(false);}
void UCoreMorphTailEffects::Shutdown()
{if(auto* B=Cast<ACoreMorphBoss>(GetOwner()))if(auto* ASC=B->GetAbilitySystemComponent()){ASC->RemoveGameplayCue(TAG_GameplayCue_CoreMorph_TailCharge);ASC->RemoveGameplayCue(TAG_GameplayCue_CoreMorph_TailBlast);}EndCharge();EndBlast();}
void UCoreMorphTailEffects::EndPlay(const EEndPlayReason::Type Reason){Shutdown();Super::EndPlay(Reason);}
void UCoreMorphTailEffects::TickComponent(float Dt,ELevelTick Tick,FActorComponentTickFunction* Function)
{
 Super::TickComponent(Dt,Tick,Function);auto* B=Cast<ACoreMorphBoss>(GetOwner());if(!B || B->IsDead()){Shutdown();return;}if(B->ScorpionCombat->IsPaused())return;
 if(bCharging)DrawCharge(Dt);
 if(bBlasting){BlastClock+=Dt;if(BlastClock>=1.25f){B->GetAbilitySystemComponent()->RemoveGameplayCue(TAG_GameplayCue_CoreMorph_TailBlast);EndBlast();}else DrawBlast(Dt);}
}
void UCoreMorphTailEffects::DrawCharge(float Dt)
{
 auto* B=CastChecked<ACoreMorphBoss>(GetOwner());ChargeClock+=Dt;
 const float Progress=FMath::Clamp(ChargeClock/FMath::Max(.05f,B->ScorpionCombat->WindupDuration),0.f,1.f);
 const FVector Tip=B->ScorpionCombat->TipPosition;Orbs->ClearInstances();ChargeArcs->ClearInstances();
 const float OrbRadius=45+230*Progress;Orbs->AddInstance(FTransform(FQuat::Identity,Tip,FVector(OrbRadius/50)),true);
 for(int32 I=0;I<48;++I)
 {
  const float Phase=FMath::Frac(I*.618034f+ChargeClock*(.38f+.25f*Progress));const float A=I*2.39996f+ChargeClock*(3+Progress*6);
  const float Radius=OrbRadius+(1-Phase)*(650+120*FMath::Sin(I*1.7f));
  const FVector Offset=FVector(FMath::Cos(A),FMath::Sin(A),FMath::Sin(A*.61f+I)*.75f)*Radius;
  Orbs->AddInstance(FTransform(FQuat::Identity,Tip+Offset,FVector((9+Phase*12)/50)),true);
  const FVector Tangent=FVector(-FMath::Sin(A),FMath::Cos(A),.25f)*70;
  Line(ChargeArcs,Tip+Offset,Tip+Offset*.85f+Tangent,3+Progress*6);
 }
 ChargeLight->SetWorldLocation(Tip);ChargeLight->SetIntensity(40000+Progress*180000);
 if(WarningMID)WarningMID->SetScalarParameterValue(TEXT("Pulse"),.42f+.58f*FMath::Pow(.5f+.5f*FMath::Sin(ChargeClock*(10+Progress*8)),2));
}
void UCoreMorphTailEffects::DrawBlast(float Dt)
{
 BlastArcs->ClearInstances();const float Life=FMath::Clamp(BlastClock/1.25f,0.f,1.f),Fade=1-Life;
 const FQuat Frame=FQuat::FindBetweenNormals(FVector::UpVector,BlastNormal);
 const int32 Flicker=FMath::FloorToInt(BlastClock*24);FRandomStream Random(7183+Flicker*7919);
 for(int32 I=0;I<18;++I)
 {
  const float A=I*UE_TWO_PI/18;FVector Previous=BlastCenter+BlastNormal*25;
  for(int32 J=1;J<=12;++J)
  {
   const float Radius=BlastRadius*J/12.f;const float Angle=A+Random.FRandRange(-.07f,.07f);
   FVector Next=BlastCenter+Frame.RotateVector(FVector(FMath::Cos(Angle)*Radius,FMath::Sin(Angle)*Radius,25+Random.FRandRange(0,100)*Fade));
   Line(BlastArcs,Previous,Next,(7+Random.FRandRange(0,8))*Fade);Previous=Next;
  }
 }
 for(int32 I=0;I<9;++I)
 {
  FVector Previous=BlastCenter;const float A=I*UE_TWO_PI/9;
  for(int32 J=1;J<=10;++J){const float H=J*105.f*Fade;const FVector Next=BlastCenter+Frame.RotateVector(FVector(FMath::Cos(A)*J*24+Random.FRandRange(-65,65),FMath::Sin(A)*J*24+Random.FRandRange(-65,65),H));Line(BlastArcs,Previous,Next,12*Fade);Previous=Next;}
 }
 const float RingRadius=BlastRadius*FMath::Clamp(BlastClock/.45f,0.f,1.f);
 for(int32 I=0;I<96;++I){const float A=I*UE_TWO_PI/96,B=(I+1)*UE_TWO_PI/96;Line(BlastArcs,BlastCenter+Frame.RotateVector(FVector(FMath::Cos(A)*RingRadius,FMath::Sin(A)*RingRadius,20)),BlastCenter+Frame.RotateVector(FVector(FMath::Cos(B)*RingRadius,FMath::Sin(B)*RingRadius,20)),18*Fade);}
 BlastCore->ClearInstances();BlastCore->AddInstance(FTransform(FQuat::Identity,BlastCenter+BlastNormal*65,FVector((80+BlastRadius*.65f*(1-FMath::Exp(-BlastClock*10)))/50)),true);
 if(BlastCoreMID)BlastCoreMID->SetScalarParameterValue(TEXT("EnergyFade"),FMath::Exp(-BlastClock*12));
 BlastLight->SetIntensity(900000*FMath::Exp(-BlastClock*7));
}
