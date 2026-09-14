#include "Enemy/Boss/CoreMorph/Transformation/CoreMorphReassemblyComponent.h"
#include "Enemy/Boss/CoreMorph/CoreMorphBoss.h"
#include "Enemy/Boss/CoreMorph/Movement/CoreMorphFlightComponent.h"
#include "Core/_Shared/GAS/TheManGameplayTags.h"
#include "AbilitySystemComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"

namespace
{
constexpr int32 MetalCount=11264, SparkCount=1024;
constexpr int32 SandAngles=128, SandRadialSamples=41;
constexpr int32 WindRings=3, WindLayers=3, WindCount=SandAngles*WindRings*WindLayers;
constexpr float SandSampleSpacing=250.f, PeelDuration=.065f;
float Ease(float X){X=FMath::Clamp(X,0.f,1.f);return X*X*X*(X*(X*6-15)+10);}
float Hash(int32 I){const float V=FMath::Sin(I*12.9898f+78.233f)*43758.5453f;return V-FMath::FloorToFloat(V);}
}

UCoreMorphReassemblyComponent::UCoreMorphReassemblyComponent()
{
    PrimaryComponentTick.bCanEverTick=true;
    PrimaryComponentTick.bStartWithTickEnabled=false;
    PrimaryComponentTick.TickGroup=TG_PrePhysics;
}
ACoreMorphBoss* UCoreMorphReassemblyComponent::Boss() const {return Cast<ACoreMorphBoss>(GetOwner());}
bool UCoreMorphReassemblyComponent::CanStart() const
{
    if(!Boss() || Boss()->IsDead() || Boss()->CurrentForm!=ECoreMorphForm::Manta || bMorphing
        || !Layout || Layout->Pieces.Num()!=455 || Boss()->Flight->GetPieces().Num()!=154)return false;
    for(int32 I=0;I<455;++I)if(!Layout->Pieces[I].Mesh || Layout->Pieces[I].Form!=(I<154?0:1))return false;
    return Layout->FragmentMaterial && Layout->SparkMaterial && Layout->ImpactMaterial && Layout->EarthMaterial && Layout->EarthMesh && Layout->DustMaterial && Layout->SandWaveMaterial;
}
bool UCoreMorphReassemblyComponent::Start()
{
    if(!CanStart())return false;
    ResetPreview();
    CapturedRoot=Boss()->GetActorTransform();
    const auto& Motion=Boss()->Flight->GetMotionState();
    const FRotator Heading(0,Motion.GetBody().Rotator().Yaw,0);
    const FVector Landing=ProbeGround(IsValid(LandingReference)?LandingReference->GetActorLocation():Motion.GetBody().GetLocation()+Heading.Vector()*4000);
    FixedTransform=FTransform(Heading,Landing+FVector(0,0,785.72)-Heading.RotateVector(GetFormOffset(1)));
    ReleaseVelocity=FixedTransform.InverseTransformVector(Motion.GetBody().GetUnitAxis(EAxis::X)*Motion.GetSpeed());
    // Capture the approved adaptive flight pose; never snap back to the old timed wing pose.
    for(const auto& Piece:Boss()->Flight->GetPieces()) Captured.Add(Piece->GetComponentTransform().GetRelativeTransform(FixedTransform));
    Components=Boss()->Flight->GetPieces();
    for(int32 I=154;I<Layout->Pieces.Num();++I)
    {
        auto* C=NewObject<UStaticMeshComponent>(Boss());
        C->ComponentTags.Add(TEXT("CoreMorphScorpion"));
        C->SetupAttachment(Boss()->GetRootComponent());C->SetAbsolute(true,true,true);
        C->SetMobility(EComponentMobility::Movable);C->SetStaticMesh(Layout->Pieces[I].Mesh);
        C->SetCanEverAffectNavigation(false);C->SetCollisionObjectType(ECC_Pawn);
        C->SetCollisionResponseToAllChannels(ECR_Ignore);C->SetCollisionResponseToChannel(ECC_GameTraceChannel1,ECR_Block);C->SetCollisionResponseToChannel(ECC_Visibility,ECR_Block);
        C->SetCollisionEnabled(ECollisionEnabled::NoCollision);C->SetVisibility(false);C->RegisterComponent();Components.Add(C);
    }
    Live.SetNum(455);Reveals.SetNumZeroed(455);
    for(int32 I=0;I<455;++I)if(Layout->Pieces[I].Kind!=TEXT("Core"))FormIndices[Layout->Pieces[I].Form].Add(I);
    ResolveGround();PrepareBuildGuides();PrepareStreams();
    bMorphing=true;bPaused=false;CurrentForm=0;Elapsed=13.4f;Progress=ParticleReleaseTime();IdleTime=0;
    SetComponentTickEnabled(true);UpdatePose();return true;
}
FTransform UCoreMorphReassemblyComponent::SourcePose(int32 Index,float Time) const
{
    FTransform Result=Captured[Index];
    Result.AddToTranslation(ReleaseVelocity*((Time-ParticleReleaseTime())*GetMorphDuration()));
    return Result;
}
void UCoreMorphReassemblyComponent::BeginCue()
{
    if(!bMorphing || EffectRoot || !Layout)return;
    EffectRoot=NewObject<USceneComponent>(Boss());EffectRoot->SetupAttachment(Boss()->GetRootComponent());
    EffectRoot->SetAbsolute(true,true,true);EffectRoot->SetMobility(EComponentMobility::Movable);EffectRoot->RegisterComponent();EffectRoot->SetWorldTransform(FixedTransform);
    auto Pool=[&](int32 Count,UMaterialInterface* Material,UStaticMesh* Mesh)
    {
        auto* C=NewObject<UInstancedStaticMeshComponent>(Boss());C->SetupAttachment(EffectRoot);C->SetMobility(EComponentMobility::Movable);
        C->SetStaticMesh(Mesh);C->SetMaterial(0,Material);C->SetCollisionEnabled(ECollisionEnabled::NoCollision);C->SetCanEverAffectNavigation(false);C->SetCastShadow(false);C->RegisterComponent();
        for(int32 I=0;I<Count;++I)C->AddInstance(FTransform(FQuat::Identity,FVector::ZeroVector,FVector(.0001)));
        return C;
    };
    auto* Cube=LoadObject<UStaticMesh>(nullptr,TEXT("/Engine/BasicShapes/Cube.Cube"));
    auto* Sphere=LoadObject<UStaticMesh>(nullptr,TEXT("/Engine/BasicShapes/Sphere.Sphere"));
    Fragments=Pool(MetalCount,Layout->FragmentMaterial,Cube);Sparks=Pool(SparkCount,Layout->SparkMaterial,Cube);
    ImpactDebris=Pool(160,Layout->EarthMaterial,Layout->EarthMesh?Layout->EarthMesh.Get():Sphere);
    ImpactGlow=Pool(WindCount,Layout->SandWaveMaterial,Sphere);ImpactGlow->SetNumCustomDataFloats(2);
    ImpactDust=Pool(256,Layout->DustMaterial,Sphere);ImpactDust->SetNumCustomDataFloats(1);
    ImpactFlash=NewObject<UStaticMeshComponent>(Boss());ImpactFlash->SetupAttachment(EffectRoot);ImpactFlash->SetMobility(EComponentMobility::Movable);
    ImpactFlash->SetStaticMesh(Sphere);ImpactFlash->SetMaterial(0,Layout->ImpactMaterial);ImpactFlash->SetCollisionEnabled(ECollisionEnabled::NoCollision);ImpactFlash->SetCastShadow(false);ImpactFlash->RegisterComponent();
    ImpactLight=NewObject<UPointLightComponent>(Boss());ImpactLight->SetupAttachment(EffectRoot);ImpactLight->SetMobility(EComponentMobility::Movable);
    ImpactLight->SetLightColor(FLinearColor(1,.35f,.06f));ImpactLight->SetAttenuationRadius(6500);ImpactLight->SetCastShadows(false);ImpactLight->RegisterComponent();
    UpdatePose();
}
void UCoreMorphReassemblyComponent::EndCue()
{
    for(USceneComponent* C:{static_cast<USceneComponent*>(Fragments),static_cast<USceneComponent*>(Sparks),static_cast<USceneComponent*>(ImpactDebris),static_cast<USceneComponent*>(ImpactGlow),static_cast<USceneComponent*>(ImpactDust),static_cast<USceneComponent*>(ImpactFlash),static_cast<USceneComponent*>(ImpactLight)})if(IsValid(C))C->DestroyComponent();
    Fragments=Sparks=ImpactDebris=ImpactGlow=ImpactDust=nullptr;ImpactFlash=nullptr;ImpactLight=nullptr;
    if(EffectRoot)EffectRoot->DestroyComponent();EffectRoot=nullptr;
}
void UCoreMorphReassemblyComponent::Cancel()
{
    bMorphing=false;bPaused=false;SetComponentTickEnabled(false);
    Boss()->GetAbilitySystemComponent()->RemoveGameplayCue(TAG_GameplayCue_CoreMorph_Reassembly);
    EndCue();
    if(!Boss()->IsDead() && Boss()->CurrentForm==ECoreMorphForm::Manta)
    {
        for(int32 I=0;I<Captured.Num();++I)
        {
            auto* C=Components[I].Get();C->SetWorldTransform(Captured[I]*FixedTransform);C->SetVisibility(true);
            C->SetCustomPrimitiveDataFloat(0,1.05f);C->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
        }
        if(!Captured.IsEmpty())Boss()->SetActorTransform(CapturedRoot);
        for(int32 I=154;I<Components.Num();++I){Components[I]->SetVisibility(false);Components[I]->SetCollisionEnabled(ECollisionEnabled::NoCollision);}
    }
}
void UCoreMorphReassemblyComponent::ResetPreview()
{
    Cancel();
    for(int32 I=154;I<Components.Num();++I)if(IsValid(Components[I]))Components[I]->DestroyComponent();
    Components.Reset();Captured.Reset();Live.Reset();Reveals.Reset();FlowParticles.Reset();BuildGuides.Reset();SandGroundHeights.Reset();StreamBounds.Init();
    FormIndices[0].Reset();FormIndices[1].Reset();CurrentForm=0;Elapsed=13.4f;Progress=0;IdleTime=0;
}
void UCoreMorphReassemblyComponent::Shutdown()
{
    Cancel();OnReassembled.Clear();
    for(const auto& C:Components)if(IsValid(C))C->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}
void UCoreMorphReassemblyComponent::EndPlay(const EEndPlayReason::Type Reason){Shutdown();Super::EndPlay(Reason);}
void UCoreMorphReassemblyComponent::TickComponent(float Dt,ELevelTick Tick,FActorComponentTickFunction* Function)
{
    Super::TickComponent(Dt,Tick,Function);
    if(!Boss() || Boss()->IsDead() || bPaused || (!bMorphing && !HasCue()) || !FMath::IsFinite(Dt) || Dt<=0)return;
    Elapsed+=Dt;Progress=FMath::Min(Elapsed/GetMorphDuration(),1.f);IdleTime=FMath::Max(0.f,Elapsed-GetMorphDuration());
    const bool Finished=bMorphing && Progress>=1;
    if(Finished){bMorphing=false;CurrentForm=1;}
    UpdatePose();
    if(Finished)OnReassembled.Broadcast();
    if(Elapsed>=ImpactTime()*GetMorphDuration()+8.f)
    {
        Boss()->GetAbilitySystemComponent()->RemoveGameplayCue(TAG_GameplayCue_CoreMorph_Reassembly);
        EndCue();SetComponentTickEnabled(false);
    }
}
FVector UCoreMorphReassemblyComponent::GetFocus() const {return FixedTransform.TransformPosition(FlowFocus);}

// Stream and construction equations retain the accepted UE58Blank implementation.
// Impact sand fronts are revised into concentric wind walls for the target project.
float UCoreMorphReassemblyComponent::Schedule(float C) const
{
 return (FMath::Max(ConstructionExtension,0.f)*FMath::Clamp((C-.70f)/.30f,0.f,1.f)+C*FMath::Max(Duration,3.f)+FMath::Max(AscentExtension,0.f)*FMath::Clamp(C/.36f,0.f,1.f)+(C>=.70f?FMath::Max(AirflowDuration,0.f)+FMath::Max(CrashDuration,.2f)+FMath::Max(FeedDuration,0.f):0))/GetMorphDuration();
}
float UCoreMorphReassemblyComponent::PeelStart(const FCoreMorphVisualPiece& P) const
{
 // Keep the silhouette intact through the first diving sprint; release the wings only after gaining depth.
 if(P.Kind==TEXT("Core"))return FMath::Max(ParticleReleaseTime(),Schedule(.635f));
 if(P.Kind==TEXT("Body"))return FMath::Max(ParticleReleaseTime(),Schedule(.61f));
 return FMath::Max(ParticleReleaseTime(),Schedule(.56f+.035f*P.Order+.005f*Hash(P.Limb+3)));
}
float UCoreMorphReassemblyComponent::BuildStart(const FCoreMorphVisualPiece& P) const
{
 const float Window=(1-ImpactTime())*GetMorphDuration()-.06f;
 float Start=0;
 if(P.Kind==TEXT("Body") || P.Kind==TEXT("Core"))Start=.025f;
 else if(P.Kind==TEXT("Leg"))Start=.28f*(1-FMath::Clamp((P.Order-.15f)/.85f,0.f,1.f));
 else if(P.Kind==TEXT("Tail"))Start=.28f+.58f*FMath::Clamp((P.Order-.18f)/.82f,0.f,1.f);
 else if(P.Kind==TEXT("Claw"))Start=.25f;
 return ImpactTime()+(.06f+Start*Window)/GetMorphDuration();
}
float UCoreMorphReassemblyComponent::BuildSpan(const FCoreMorphVisualPiece& P) const
{
 const float Window=(1-ImpactTime())*GetMorphDuration()-.06f;
 const float Span=P.Kind==TEXT("Tail")?.12f:P.Kind==TEXT("Leg")?.22f:P.Kind==TEXT("Claw")?.35f:.32f;
 return Span*Window/GetMorphDuration();
}
void UCoreMorphReassemblyComponent::PrepareBuildGuides()
{
 BuildGuides.SetNum(Layout->Pieces.Num());
 for(int32 I=0;I<Layout->Pieces.Num();++I)
 {
  const auto& P=Layout->Pieces[I];if(P.Form!=1 || !P.Mesh)continue;
  auto& G=BuildGuides[I];G.Path.Reset();G.Path.Add(GetFormOffset(1));
  const bool FromFoot=P.Kind==TEXT("Leg");
  TArray<const FCoreMorphVisualPiece*> Chain;
  for(const auto& J:Layout->Pieces)if(J.Form==1 && J.Limb==P.Limb && (FromFoot?J.Order>P.Order+.001f:J.Order<P.Order-.001f))
  {
   bool Found=false;for(const auto* K:Chain)Found|=FMath::IsNearlyEqual(K->Order,J.Order,.001f);
   if(!Found)Chain.Add(&J);
  }
  Chain.Sort([FromFoot](const FCoreMorphVisualPiece& A,const FCoreMorphVisualPiece& B){return FromFoot?A.Order>B.Order:A.Order<B.Order;});
  if(FromFoot)
  {
   FVector Foot=(Chain.IsEmpty()?P.Anchor:Chain[0]->Anchor)+GetFormOffset(1);Foot.Z=GroundPoint.Z;
   G.Path[0]=Foot;
  }
  for(const auto* J:Chain)G.Path.Add(J->Anchor+GetFormOffset(1));
  const FVector Previous=G.Path.Last()-GetFormOffset(1);
  G.Axis=(P.Anchor-Previous).GetSafeNormal();
  if(P.Kind==TEXT("Core") || P.Kind==TEXT("Body"))G.Axis=FVector::UpVector;
  if(G.Axis.IsNearlyZero())G.Axis=P.RevealAxis;
  G.Path.Add(P.Anchor+GetFormOffset(1));
  const FBox Box=P.Mesh->GetBoundingBox();float Min=MAX_flt,Max=-MAX_flt;
  for(int32 Corner=0;Corner<8;++Corner)
  {
   const FVector V(Corner&1?Box.Max.X:Box.Min.X,Corner&2?Box.Max.Y:Box.Min.Y,Corner&4?Box.Max.Z:Box.Min.Z);
   const float D=FVector::DotProduct(V,G.Axis);Min=FMath::Min(Min,D);Max=FMath::Max(Max,D);
  }
  G.Min=Min;G.Span=FMath::Max(1.f,Max-Min);
  Components[I]->SetCustomPrimitiveDataVector3(1,G.Axis);Components[I]->SetCustomPrimitiveDataFloat(5,G.Min);Components[I]->SetCustomPrimitiveDataFloat(6,1/G.Span);
 }
}
void UCoreMorphReassemblyComponent::ResolveGround()
{
 GroundPoint=GetFormOffset(1);GroundPoint.Z=-FixedTransform.GetLocation().Z;
 if(auto* W=GetWorld())
 {
  const FVector Center=FixedTransform.TransformPosition(GetFormOffset(1));FHitResult Hit;FCollisionQueryParams Params(SCENE_QUERY_STAT(CoreMorphGround),false,GetOwner());
  if(W->LineTraceSingleByChannel(Hit,Center+FVector(0,0,5000),Center-FVector(0,0,20000),ECC_WorldStatic,Params))GroundPoint=FixedTransform.InverseTransformPosition(Hit.ImpactPoint);
  // Cache terrain profiles once per placement/start; wave expansion does no per-frame traces.
  SandGroundHeights.SetNum(SandAngles*SandRadialSamples);
  for(int32 AngleIndex=0;AngleIndex<SandAngles;++AngleIndex)
  {
   const float Angle=2*PI*AngleIndex/128.f;
   for(int32 RadiusIndex=0;RadiusIndex<SandRadialSamples;++RadiusIndex)
   {
    const FVector Local=GroundPoint+FVector(FMath::Cos(Angle),FMath::Sin(Angle),0)*(RadiusIndex*SandSampleSpacing);
    const FVector Ground=FixedTransform.InverseTransformPosition(ProbeGround(FixedTransform.TransformPosition(Local)));
    SandGroundHeights[AngleIndex*SandRadialSamples+RadiusIndex]=Ground.Z;
   }
  }
 }
}

FVector UCoreMorphReassemblyComponent::ProbeGround(FVector Position) const
{
 FHitResult Hit;FCollisionQueryParams Params(SCENE_QUERY_STAT(CoreMorphPlacement),false,GetOwner());
 if(GetWorld() && GetWorld()->LineTraceSingleByObjectType(Hit,Position+FVector(0,0,30000),Position-FVector(0,0,30000),FCollisionObjectQueryParams(ECC_WorldStatic),Params))return Hit.ImpactPoint;
 return FVector(Position.X,Position.Y,-30000);
}
int32 UCoreMorphReassemblyComponent::StreamFor(const FCoreMorphVisualPiece& P) const
{
 if(P.Form==0){if(P.Kind==TEXT("Wing"))return P.Limb*2+(P.Order>.55f?1:0);return P.Kind==TEXT("Tail")?4:5;}
 if(P.Kind==TEXT("Leg"))return FMath::Clamp(P.Limb/2,0,3);
 return P.Kind==TEXT("Tail")?4:5;
}
void UCoreMorphReassemblyComponent::PrepareStreams()
{
 FlowParticles.Reset();FlowParticles.Reserve(MetalCount+SparkCount);
 TArray<int32> Sources[6];for(int32 I:FormIndices[CurrentForm])Sources[StreamFor(Layout->Pieces[I])].Add(I);
 const auto& Targets=FormIndices[1-CurrentForm];
 for(int32 I=0;I<MetalCount+SparkCount;++I)
 {
  const int32 DI=Targets[I%Targets.Num()];const auto& D=Layout->Pieces[DI];const int32 Strand=I%6;
  const auto& Pool=Sources[Strand].IsEmpty()?FormIndices[CurrentForm]:Sources[Strand];const int32 SI=Pool[(I*37+I/Targets.Num())%Pool.Num()];const auto& S=Layout->Pieces[SI];
  const FVector ALocal=S.SurfacePoints.IsEmpty()?FVector::ZeroVector:S.SurfacePoints[(I*17+I/7)%S.SurfacePoints.Num()];
  const FVector DLocal=D.SurfacePoints.IsEmpty()?FVector::ZeroVector:D.SurfacePoints[(I*31+I/11)%D.SurfacePoints.Num()];
  const float SF=FMath::Clamp(float((FVector::DotProduct(ALocal,S.RevealAxis)-S.RevealMin)/FMath::Max(1.f,S.RevealSpan)),0.f,1.f);
  const auto& Guide=BuildGuides[DI];
  const float DF=FMath::Clamp(float((FVector::DotProduct(DLocal,Guide.Axis)-Guide.Min)/Guide.Span),0.f,1.f);
  const float ClockScale=FMath::Max(Duration,3.f)/GetMorphDuration();
  FFlowParticle P;P.BuildPath=Guide.Path;P.Strand=Strand;P.Birth=PeelStart(S)+PeelDuration*ClockScale*(1-SF);P.Arrival=BuildStart(D)+BuildSpan(D)*DF;
  P.A=SourcePose(SI,P.Birth).TransformPosition(ALocal);P.D=D.Position+GetFormOffset(D.Form)+DLocal;
  const float LandingAngle=Strand*PI/3+(Hash(I+391)-.5f)*.08f;
  P.Impact=GroundPoint+FVector(FMath::Cos(LandingAngle),FMath::Sin(LandingAngle),0)*(380+40*Hash(I+827));
  // Early destination fronts start receiving immediately; trailing matter keeps falling
  // while legs and underside are already growing. No wait for the last strand.
  const float Available=FMath::Max(0.f,(P.Arrival-ImpactTime())*GetMorphDuration()-.045f);
  P.Land=ImpactTime()+FMath::Min(Available,FMath::Max(FeedDuration,0.f)*(.06f*Strand+.67f*Hash(I+911)))/GetMorphDuration();
  const FVector Velocity=(SourcePose(SI,P.Birth+.001f).TransformPosition(ALocal)-SourcePose(SI,P.Birth-.001f).TransformPosition(ALocal))/.002f;
  P.Axis=(P.Impact-P.A).GetSafeNormal();
  P.ReleaseTangent=FVector::VectorPlaneProject(Velocity*(P.Arrival-P.Birth),P.Axis).GetClampedToMaxSize(3500);
  // Monotone Hermite travel preserves the forward component of release momentum.
  P.Speed=FMath::Clamp(float(FVector::DotProduct(Velocity,P.Axis)*(P.Arrival-P.Birth)/FMath::Max(1.,FVector::Dist(P.A,P.D))),.6f,2.4f);
  FlowParticles.Add(P);
 }
 FVector SharedStart=FVector::ZeroVector;for(const auto& P:FlowParticles)SharedStart+=P.A;SharedStart/=FlowParticles.Num();

 for(auto& P:FlowParticles)
 {
  // Gather only across the travel corridor; never pull a fragment back along it.
  P.StreamStart=SharedStart;P.StreamEnd=GroundPoint;
  P.LaneStart=FVector::VectorPlaneProject(SharedStart-P.A,P.Axis);
  P.LaneEnd=FVector::VectorPlaneProject(GroundPoint-P.Impact,P.Axis);
 }
}
FVector UCoreMorphReassemblyComponent::CoilOffset(const FFlowParticle& P,float T) const
{
 const FVector Axis=(P.StreamEnd-P.StreamStart).GetSafeNormal();
 FVector N=FVector::CrossProduct(Axis,FVector::RightVector).GetSafeNormal();if(N.IsNearlyZero())N=FVector::UpVector;
 const FVector M=FVector::CrossProduct(Axis,N).GetSafeNormal();
 const float Phase=P.Strand*1.73f;
 const float Angle=2*PI*(.65f+.09f*P.Strand)*T+Phase+.7f*FMath::Sin(2*PI*T+Phase);
 const float Radius=240+270*FMath::Square(FMath::Sin(PI*T*1.4f+Phase));
 // Broad nonperiodic rolls within uneven, overlapping sheet bundles.
 return (N*FMath::Cos(Angle)+M*FMath::Sin(Angle)*.6f)*Radius;
}
FVector UCoreMorphReassemblyComponent::FlowPoint(const FFlowParticle& P,float U,int32 Seed) const
{
 U=FMath::Clamp(U,0.f,1.f);
 // Spread material continuously along each ribbon; the derivative stays positive.
 const float FlowClock=U+(Hash(Seed+617)-.5f)*.54f*FMath::Sin(PI*U);
 const float Travel=.35f*FlowClock+.65f*FlowClock*FlowClock;
 const FVector Center=FMath::Lerp(P.A,P.Impact,Travel),Axis=P.Axis;
 FVector N=FVector::CrossProduct(Axis,FVector(0,1,0)).GetSafeNormal();if(N.IsNearlyZero())N=FVector::UpVector;
 const FVector M=FVector::CrossProduct(Axis,N).GetSafeNormal();const int32 S=P.Strand;
 // Use a shared spatial phase within each strand so coils remain legible as a shape.
 const float Envelope=Ease(U/.12f)*(1-Ease((U-.96f)/.04f));
 const float Gather=Ease(U/.20f)*(1-Ease((U-.96f)/.04f));
 const FVector Lane=FMath::Lerp(P.LaneStart,P.LaneEnd,Travel)*Gather;
 const FVector StreamAxis=(P.StreamEnd-P.StreamStart).GetSafeNormal();
 const float StreamTravel=FVector::DotProduct(Center+Lane-P.StreamStart,StreamAxis)/FMath::Max(1.,FVector::Dist(P.StreamStart,P.StreamEnd));
 const float Phase=S*1.73f;
 const float Radius=(1050+650*FMath::Sin(StreamTravel*PI*1.6f+Phase))*Envelope;
 const float Angle=StreamTravel*2*PI*(.60f+.085f*S)+Phase+.5f*FMath::Sin(StreamTravel*2*PI+Phase);
 const FVector Radial=N*FMath::Cos(Angle)+M*FMath::Sin(Angle),Across=-N*FMath::Sin(Angle)+M*FMath::Cos(Angle);
 const float Bulge=.45f+.9f*FMath::Square(FMath::Sin(StreamTravel*PI*2.3f+Phase));
 const float Fringe=Hash(Seed+75)>.90f?1.8f:1.f;
 const float Width=(Hash(Seed+21)-.5f)*650*Bulge*Envelope*Fringe,Thickness=(Hash(Seed+43)-.5f)*170*Bulge*Envelope*Fringe;
 const FVector Coil=FVector::VectorPlaneProject(CoilOffset(P,StreamTravel),Axis)*Envelope;
 const FVector Momentum=P.ReleaseTangent*U*(1-Ease(U/.22f));
 return Center+Lane+Momentum+Radial*(Radius+Thickness)+Across*Width+Coil;

}
FVector UCoreMorphReassemblyComponent::GroundFlowPoint(const FFlowParticle& P,float U) const
{
 U=FMath::Clamp(U,0.f,1.f);
 const FVector Root=P.BuildPath.IsEmpty()?GetFormOffset(1):P.BuildPath[0];FVector Floor=Root;Floor.Z=GroundPoint.Z;
 TArray<FVector> Path;Path.Add(P.Impact);Path.Add(Floor);Path.Append(P.BuildPath);Path.Add(P.D);
 float Length=0;for(int32 I=1;I<Path.Num();++I)Length+=FVector::Dist(Path[I-1],Path[I]);
 // Constant arc-length motion through contact: no ease-to-zero or floor hold.
 float Remaining=Length*U;
 for(int32 I=1;I<Path.Num();++I){const float Segment=FVector::Dist(Path[I-1],Path[I]);if(Remaining<=Segment)return FMath::Lerp(Path[I-1],Path[I],Remaining/FMath::Max(1.f,Segment));Remaining-=Segment;}
 return P.D;

}
FVector UCoreMorphReassemblyComponent::ParticlePoint(const FFlowParticle& P,float T,int32 Seed) const
{
 const float Impact=P.Land;
 return T<Impact?FlowPoint(P,(T-P.Birth)/(Impact-P.Birth),Seed):GroundFlowPoint(P,(T-Impact)/FMath::Max(.001f,P.Arrival-Impact));
}
void UCoreMorphReassemblyComponent::UpdateImpact()
{
 if(!ImpactFlash || !ImpactLight || !ImpactDebris || !ImpactGlow || !ImpactDust)return;
 const float Seconds=(Progress-ImpactTime())*GetMorphDuration()+(CurrentForm==1?IdleTime:0);
 const bool Active=(bMorphing || CurrentForm==1) && Seconds>=0 && Seconds<8.f;
 ImpactFlash->SetVisibility(Active && Seconds<.25f);ImpactLight->SetVisibility(Active && Seconds<.35f);
 ImpactDebris->SetVisibility(Active && Seconds<3.8f);ImpactGlow->SetVisibility(Active && Seconds<4.6f);ImpactDust->SetVisibility(Active);
 if(!Active)return;
 ImpactFlash->SetRelativeLocation(GroundPoint+FVector(0,0,15));ImpactFlash->SetRelativeScale3D(FVector(18+Seconds*20,18+Seconds*20,.12));
 ImpactLight->SetRelativeLocation(GroundPoint+FVector(0,0,150));ImpactLight->SetIntensity(180000*FMath::Square(FMath::Max(0.f,1-Seconds/.35f)));
 TArray<FTransform> Debris,Glow,Dust;Debris.Reserve(160);Glow.Reserve(WindCount);Dust.Reserve(256);
 for(int32 I=0;I<160;++I)
 {
  const float H=Hash(I+1401),Angle=2*PI*(I+.8f*Hash(I+771))/160.f,Age=FMath::Max(0.f,Seconds-.35f*Hash(I+141));
  const bool Large=I%10<2,Medium=I%10<6;
  const float RockSize=.45f*(Large?3.5f+2.8f*H:Medium?1.1f+1.6f*H:.30f+.60f*H);
  const float RadialSpeed=.48f*(450+1700*H)*(Large?.70f:1.f),Up=.70f*(450+1100*Hash(I+512))*(Large?.85f:1.f),Flight=2*Up/980;
  const float T=FMath::Min(Age,Flight),GroundAge=FMath::Max(0.f,Age-Flight);
  const float LaunchRadius=550+650*FMath::Sqrt(Hash(I+923));
  const float Distance=LaunchRadius+RadialSpeed*T+RadialSpeed*.16f*(1-FMath::Exp(-GroundAge*3));
  FVector P=GroundPoint+FVector(FMath::Cos(Angle),FMath::Sin(Angle),0)*Distance;
  P.Z+=20+RockSize*27.5f+FMath::Max(0.f,Up*T-490*T*T);
  const float Size=RockSize*Ease(Seconds/.08f)*(1-Ease((Seconds-3.f)/.8f));
  Debris.Add(FTransform(FRotator(I+T*140,T*170,I*17).Quaternion(),P,FVector(Size,Size*(.78f+.22f*Hash(I+77)),Size*(.72f+.24f*Hash(I+81)))+FVector(.0001)));
 }
 // Three equal-speed fronts preserve the empty space between concentric walls.
 // Each sector has a grounded body, rolling crest and trailing curl (sphere diameter: 100 cm).
 for(int32 I=0;I<WindCount;++I)
 {
  const int32 Sector=I%SandAngles,Layer=(I/SandAngles)%WindLayers,Ring=I/(SandAngles*WindLayers);
  const float Angle=2*PI*Sector/SandAngles,Age=Seconds-Ring*.65f;
  const float Travel=FMath::Clamp(Age,0.f,3.2f),Grow=Ease(Travel/.20f);
  const float Radius=650+2450*Travel;
  const float Fade=Age>=0?Grow*(1-Ease((Travel-2.0f)/1.2f)):0;
  // Coherent angular billows close the seam without turning the wall into separate pillars.
  const float Billow=1+.10f*FMath::Sin(Angle*7-Travel*2.4f+Ring)+.055f*FMath::Sin(Angle*13+Travel*3);
  const float Height=2000*Billow*Grow;
  const float Curl=FMath::Sin(Angle*9-Travel*3+Ring);
  const float Offset=Layer==0?0:Layer==1?-120-45*Curl:-330-65*Curl;
  const float WallRadius=FMath::Max(0.f,Radius+Offset*Grow);
  const float RadialSample=FMath::Clamp(WallRadius/SandSampleSpacing,0.f,float(SandRadialSamples-1));
  const int32 GroundIndex=FMath::Min(SandRadialSamples-2,FMath::FloorToInt(RadialSample));
  const int32 SampleIndex=Sector*SandRadialSamples+GroundIndex;
  const float GroundZ=SandGroundHeights.Num()==SandAngles*SandRadialSamples?FMath::Lerp(SandGroundHeights[SampleIndex],SandGroundHeights[SampleIndex+1],RadialSample-GroundIndex):float(GroundPoint.Z);
  FVector P=GroundPoint+FVector(FMath::Cos(Angle),FMath::Sin(Angle),0)*WallRadius;
  P.Z=GroundZ+Height*(Layer==0?.39f:Layer==1?.85f:.69f);
  const float Width=(Layer==0?300+70*Travel:Layer==1?520:380)*Grow;
  const float Tangent=FMath::Max(100.f,2*PI*WallRadius/SandAngles*1.8f)*Grow;
  const float Vertical=Height*(Layer==0?.82f:Layer==1?.34f:.44f);
  Glow.Add(FTransform(FRotator(0,FMath::RadiansToDegrees(Angle),0).Quaternion(),P,FVector(Width,Tangent,Vertical)/100+FVector(.0001)));
  ImpactGlow->SetCustomDataValue(I,0,Fade*(Layer==2?.65f:1.f),false);
  // Component time also drives shader advection, so review pause freezes the entire wall.
  ImpactGlow->SetCustomDataValue(I,1,Travel,false);
 }
 for(int32 I=0;I<256;++I)
 {
  const bool Inner=I<144;
  const float H=Hash(I+320),Angle=2*PI*Hash(I+981),Delay=(Inner?.26f:.55f)*Hash(I+419),Age=FMath::Max(0.f,Seconds-Delay);
  const float Radius=Inner?FMath::Sqrt(H)*(650+1500*(1-FMath::Exp(-Age*1.3f))):850+(1600+1800*H)*(1-FMath::Exp(-Age*1.4f));
  const float Grow=Ease(Age/.28f),Fade=Inner && Seconds>=Delay?Grow*(1-Ease((Age-2.6f)/4.6f))*(.45f+.55f*Ease((Seconds-1.4f)/1.4f)):0;
  const float Size=(7.f+8*Hash(I+783))*(1+.16f*Age)*Grow;
  const float Height=Inner?200+(350+950*Hash(I+61))*(1-FMath::Exp(-Age*2.f))+95*Age:90+100*Age;
  FVector P=GroundPoint+FVector(FMath::Cos(Angle)*Radius+80*Age,FMath::Sin(Angle)*Radius,Height);
  Dust.Add(FTransform(FRotator(0,I*137,0).Quaternion(),P,FVector(Size*1.35f,Size,Size*(Inner?1.05f:.60f))+FVector(.0001)));
  ImpactDust->SetCustomDataValue(I,0,Fade,false);
 }
 ImpactDebris->BatchUpdateInstancesTransforms(0,Debris,false,true,true);ImpactGlow->BatchUpdateInstancesTransforms(0,Glow,false,true,true);ImpactDust->BatchUpdateInstancesTransforms(0,Dust,false,true,true);
}
void UCoreMorphReassemblyComponent::UpdatePose()
{
 if(!Layout || Components.Num()!=Layout->Pieces.Num())return;
 for(int32 I=0;I<Components.Num();++I)
 {
  const auto& P=Layout->Pieces[I];FVector Pos=P.Position+GetFormOffset(P.Form);FQuat Q=FQuat::Identity;float Reveal=0;
  if(!bMorphing){Reveal=P.Form==CurrentForm?1:0;Pos=P.Position+GetFormOffset(P.Form);}
  else if(P.Form==CurrentForm)
  {
   const FTransform Swim=SourcePose(I,Progress);Pos=Swim.GetLocation();Q=Swim.GetRotation();
   Reveal=1-FMath::Clamp((Progress-PeelStart(P))/(PeelDuration*FMath::Max(Duration,3.f)/GetMorphDuration()),0.f,1.f);
  }
  else Reveal=FMath::Clamp((Progress-BuildStart(P))/BuildSpan(P),0.f,1.f);
  // Destination clusters always occupy their final pose. Only the local construction front advances.
  Live[I]=FTransform(Q,Pos,FVector(GetFormScale(P.Form)));Reveals[I]=Reveal;
  Components[I]->SetWorldTransform(Live[I]*FixedTransform);
  Components[I]->SetCollisionEnabled(Reveal>0 && !Boss()->IsDead()?ECollisionEnabled::QueryOnly:ECollisionEnabled::NoCollision);
  Components[I]->SetVisibility(Reveal>0);
  Components[I]->SetCustomPrimitiveDataFloat(0,Reveal>=1?1.05f:Reveal<=0?-.05f:Reveal);
 }
 const float Handoff=Ease((Progress-ParticleReleaseTime())/FMath::Max(.001f,ImpactTime()-ParticleReleaseTime()));
 Boss()->SetActorLocation(FMath::Lerp(CapturedRoot.GetLocation(),FixedTransform.TransformPosition(GetFormOffset(1)),Handoff),false,nullptr,ETeleportType::TeleportPhysics);
 UpdateImpact();if(!Fragments || !Sparks)return;
 const bool Released=bMorphing && Progress>=ParticleReleaseTime();
 Fragments->SetVisibility(Released);Sparks->SetVisibility(Released);if(!bMorphing || FlowParticles.Num()!=MetalCount+SparkCount)return;
 TArray<FTransform> Metal,Glow;Metal.Reserve(MetalCount);Glow.Reserve(SparkCount);
 FVector WeightedFocus=FVector::ZeroVector;float FocusWeight=0;StreamBounds.Init();
 for(int32 I=0;I<FlowParticles.Num();++I)
 {
  const auto& P=FlowParticles[I];const float U=FMath::Clamp((Progress-P.Birth)/FMath::Max(.01f,P.Arrival-P.Birth),0.f,1.f);
  const FVector Pos=ParticlePoint(P,Progress,I);const FVector Direction=(ParticlePoint(P,Progress+.0002f,I)-ParticlePoint(P,Progress-.0002f,I)).GetSafeNormal();
  const float H=Hash(I+100),Visible=Ease(U/.035f)*(1-Ease((U-.99f)/.01f));const bool Spark=I>=MetalCount;
  WeightedFocus+=Pos*Visible;FocusWeight+=Visible;
  if(Visible>.01f)StreamBounds+=FBox(Pos-FVector(100),Pos+FVector(100));
  const FVector Scale=(Spark?FVector(.50+.55*H,.025,.025):FVector(.85+.90*H,.50+.58*Hash(I+17),.06+.10*Hash(I+29)))*Visible+FVector(.0001);
  const FQuat Rotation=Direction.Rotation().Quaternion()*FQuat(FVector::ForwardVector,Progress*PI*(2+H*3)+I);
  (Spark?Glow:Metal).Add(FTransform(Rotation,Pos,Scale));
 }
 Fragments->BatchUpdateInstancesTransforms(0,Metal,false,true,true);Sparks->BatchUpdateInstancesTransforms(0,Glow,false,true,true);
 FlowFocus=FocusWeight>0?WeightedFocus/FocusWeight:GetFormOffset(1);
}
