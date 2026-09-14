#include "Enemy/Boss/CoreMorph/Movement/CoreMorphFlightPath.h"
#include "Enemy/Boss/CoreMorph/Movement/CoreMorphTailMotion.h"
#include "Enemy/Boss/CoreMorph/Data/CoreMorphVisualLayout.h"

// Source: UE58Blank FEAT058. Preserve trajectory, wings and lateral turn shape;
// tail undulation now receives measured movement instead of a timed dive pulse.
namespace
{
float Ease(float X){X=FMath::Clamp(X,0.f,1.f);return X*X*X*(X*(X*6-15)+10);}
constexpr float DiveTurnLead=1.30f,DiveTurnDuration=1.95f,DiveSpeed=11000.f;
FVector DiveTurnDisplacement(const FVector& InitialVelocity,const FVector& Rates,const FVector& Direction,float Seconds)
{
 // Integrate a smooth heading turn at sustained speed, avoiding the mid-turn stop of velocity lerp.
 static constexpr float Nodes[]={-.96028986f,-.79666648f,-.52553241f,-.18343464f,.18343464f,.52553241f,.79666648f,.96028986f};
 static constexpr float Weights[]={.10122854f,.22238103f,.31370665f,.36268378f,.36268378f,.31370665f,.22238103f,.10122854f};
 const FRotator From=InitialVelocity.Rotation(),To=Direction.Rotation();
 // Keep the orbit's positive turn direction across the +/-180 seam while solving the endpoint.
 const float YawDelta=FMath::Fmod(float(To.Yaw-From.Yaw)+360.f,360.f);
 FVector Sum=FVector::ZeroVector;
 for(int32 I=0;I<8;++I)
 {
  const float U=FMath::Clamp(Seconds*.5f*(Nodes[I]+1)/DiveTurnDuration,0.f,1.f),Blend=Ease(U);
  const float Tangent=DiveTurnDuration*(U-6*U*U*U+8*U*U*U*U-3*U*U*U*U*U);
  const FRotator Facing(FMath::Lerp(From.Pitch,To.Pitch,Blend)+Rates.X*Tangent,From.Yaw+YawDelta*Blend+Rates.Y*Tangent,0);
  Sum+=Facing.Vector()*(FMath::Lerp(float(InitialVelocity.Size()),DiveSpeed,Blend)+Rates.Z*Tangent)*Weights[I];
 }
 return Sum*(Seconds*.5f);
}
}

FVector FCoreMorphFlightPath::OrbitPosition(float T) const
{
 const float Time=FMath::Max(0.f,T*GetMorphDuration());
 // A distant approach ends tangent to the circle; the first downstroke starts propulsion.
 constexpr float Entry=3.55f,Speed=11215.f;
 if(Time<Entry)return FMath::CubicInterp(GetFormOffset(0),FVector::ZeroVector,FVector(0,0,1600),FVector(Speed*3,0,0),FMath::Clamp((Time-.55f)/3.f,0.f,1.f));
 const float ClimbEnd=Schedule(.36f)*GetMorphDuration();
 const float Angle=(Time-Entry)*Speed/10000.f;
 // Rise into a rounded crest before pitching down. The position derivative drives the real heading.
 const float Crest=4000*FMath::Exp(-FMath::Square((Time-10.8f)/.9f));
 return FVector(10000*FMath::Sin(Angle),10000*(1-FMath::Cos(Angle)),1600+42000*Ease((Time-Entry)/(ClimbEnd-Entry))+Crest);
}
FVector FCoreMorphFlightPath::OrbitVelocity(float T) const
{
 const float Time=T*GetMorphDuration();
 if(Time<3.55f)return (OrbitPosition(T+.002f/GetMorphDuration())-OrbitPosition(FMath::Max(0.f,T-.002f/GetMorphDuration())))/.004f;
 const float ClimbSpan=Schedule(.36f)*GetMorphDuration()-3.55f;
 const float U=FMath::Clamp((Time-3.55f)/ClimbSpan,0.f,1.f),Angle=(Time-3.55f)*11215.f/10000.f;
 const float HeightSpeed=42000*30*U*U*(U-1)*(U-1)/ClimbSpan-8000*(Time-10.8f)/.81f*FMath::Exp(-FMath::Square((Time-10.8f)/.9f));
 return FVector(11215*FMath::Cos(Angle),11215*FMath::Sin(Angle),HeightSpeed);
}
void FCoreMorphFlightPath::PrepareDive()
{
 const float End=Schedule(.40f)-DiveTurnLead/GetMorphDuration();
 DiveTurnStart=OrbitPosition(End);
 DiveTurnVelocity=OrbitVelocity(End);
 const FVector Before=OrbitVelocity(End-.01f/GetMorphDuration());
 const FVector After=OrbitVelocity(End+.01f/GetMorphDuration());
 const FRotator BeforeHeading=Before.Rotation(),AfterHeading=After.Rotation();
 DiveTurnRates=FVector((AfterHeading.Pitch-BeforeHeading.Pitch)/.02f,FMath::FindDeltaAngleDegrees(BeforeHeading.Yaw,AfterHeading.Yaw)/.02f,(After.Size()-Before.Size())/.02f);
 DiveDirection=(GroundPoint-DiveTurnStart).GetSafeNormal();
 // The turn itself moves the body. Solve the heading from its resulting endpoint, not its old position.
 for(int32 I=0;I<8;++I)
 {
  DiveTurnEnd=DiveTurnStart+DiveTurnDisplacement(DiveTurnVelocity,DiveTurnRates,DiveDirection,DiveTurnDuration);
  DiveDirection=(GroundPoint-DiveTurnEnd).GetSafeNormal();
 }
 DiveTurnEnd=DiveTurnStart+DiveTurnDisplacement(DiveTurnVelocity,DiveTurnRates,DiveDirection,DiveTurnDuration);
}
FVector FCoreMorphFlightPath::FlightPosition(float T) const
{
 const float Seconds=T*GetMorphDuration(),End=Schedule(.40f)*GetMorphDuration()-DiveTurnLead;
 if(Seconds<=End)return OrbitPosition(T);
 const float DownTime=Seconds-End;
 if(DownTime<DiveTurnDuration)return DiveTurnStart+DiveTurnDisplacement(DiveTurnVelocity,DiveTurnRates,DiveDirection,DownTime);
 const float Sprint=DownTime-DiveTurnDuration;
 // Translation, nose and subsequent particle corridor share the same actual landing point.
 return DiveTurnEnd+DiveDirection*(DiveSpeed*Sprint+200*Sprint*Sprint);
}
FTransform FCoreMorphFlightPath::DivePose(float T) const
{
 const float Seconds=T*GetMorphDuration();
 const FVector Direction=OrbitVelocity(T);
 FRotator Facing=Direction.IsNearlyZero()?FRotator::ZeroRotator:Direction.Rotation();
 const float TurnStart=Schedule(.40f)*GetMorphDuration()-DiveTurnLead;
 if(Seconds>=TurnStart)
 {
  // Read the integrated heading directly; differentiating sampled positions twice made the nose jitter at joins.
  const float U=FMath::Clamp((Seconds-TurnStart)/DiveTurnDuration,0.f,1.f),Blend=Ease(U);
  const float Tangent=DiveTurnDuration*(U-6*U*U*U+8*U*U*U*U-3*U*U*U*U*U);
  const FRotator From=DiveTurnVelocity.Rotation(),To=DiveDirection.Rotation();
  Facing=FRotator(FMath::Lerp(From.Pitch,To.Pitch,Blend)+DiveTurnRates.X*Tangent,From.Yaw+FMath::Fmod(float(To.Yaw-From.Yaw)+360.f,360.f)*Blend+DiveTurnRates.Y*Tangent,0);
 }
 // Banking changes wing height, never the direction the nose travels.
 const float DiveBank=Ease((Seconds-Schedule(.40f)*GetMorphDuration()+DiveTurnLead)/DiveTurnDuration);
 // Positive yaw turns toward +Y; positive UE roll lowers that inside wing.
 Facing.Roll=55.f*Ease((Seconds-3.f)/1.3f)*(1-DiveBank);
 return FTransform(Facing.Quaternion(),FlightPosition(T));
}
FTransform FCoreMorphFlightPath::SourcePose(int32 Index,float T,const FCoreMorphTailMotion& TailMotion) const
{
 const auto& P=Layout->Pieces[Index];FVector Local=SourcePositions.IsValidIndex(Index)?SourcePositions[Index]:P.Position;
 const float Seconds=T*GetMorphDuration(),C=ChoreographyTime(T);
 const float Phase=(Seconds+.30f*(1-Ease((Seconds-.8f)/1.75f)))*2*PI*.85f,Start=Ease(C/.04f),Dive=Ease((Seconds-Schedule(.40f)*GetMorphDuration()+DiveTurnLead)/DiveTurnDuration);
 const float Launch=1-Ease((Seconds-1.f)/1.8f);
 const float Gain=FMath::Lerp(Start*Ease((Seconds-1.3f)/1.5f),.85f*Ease(Seconds/.28f),Launch)*(1-.80f*Dive);FRotator Bend=FRotator::ZeroRotator;
 if(P.Kind==TEXT("Wing"))
 {
  const float Side=Local.Y<0?-1.f:1.f,Span=FMath::Clamp(float(FMath::Abs(Local.Y)/2540.),0.f,1.f);
  const float Wave=Phase-1.8f*Span+.25f*float(Local.X/1000.);
  const float Lift=780*Gain*FMath::Pow(Span,1.7f)*FMath::Sin(Wave);
  Local.Z+=Lift-300*Dive*Span*Span;
  Local.Y-=Side*180*Gain*Span*Span*FMath::Square(FMath::Sin(Wave));
  Local.X-=600*Dive*Span*Span;
  Bend.Roll=Side*(32*Gain*Span*FMath::Sin(Wave)-16*Dive*Span);
  Bend.Yaw=Side*12*Dive*Span;Bend.Pitch=6*Gain*Span*FMath::Cos(Wave);
 }
 else if(P.Kind==TEXT("Tail"))
 {
  TailMotion.Apply(P.Order,Local,Bend);
  // Bend the trailing spine along the circular wake instead of rotating a rigid straight tail.
  // Let the long tail trail the heading change, then straighten along the locked dive.
  const float Turn=Ease((Seconds-3.f)/1.3f)*(1-Ease((Seconds-Schedule(.40f)*GetMorphDuration()-.65f)/1.f));
  const float Radius=10000/GetFormScale(0),Arc=-float(Local.X)/Radius;
  Local.X=FMath::Lerp(float(Local.X),-Radius*FMath::Sin(Arc),Turn);
  Local.Y+=Radius*(1-FMath::Cos(Arc))*Turn;Bend.Yaw-=FMath::RadiansToDegrees(Arc)*Turn;
 }
 const FTransform Body=DivePose(T);
 const float Size=GetFormScale(0);
 return FTransform(Body.GetRotation()*Bend.Quaternion(),Body.TransformPosition(Local*Size),FVector(Size));
}
float FCoreMorphFlightPath::Schedule(float C) const
{
 return (FMath::Max(ConstructionExtension,0.f)*FMath::Clamp((C-.70f)/.30f,0.f,1.f)+C*FMath::Max(Duration,3.f)+FMath::Max(AscentExtension,0.f)*FMath::Clamp(C/.36f,0.f,1.f)+(C>=.70f?FMath::Max(AirflowDuration,0.f)+FMath::Max(CrashDuration,.2f)+FMath::Max(FeedDuration,0.f):0))/GetMorphDuration();
}
float FCoreMorphFlightPath::ChoreographyTime(float T) const
{
 const float Base=FMath::Max(Duration,3.f),Extra=FMath::Max(AscentExtension,0.f),ElapsedSeconds=T*GetMorphDuration();
 if(ElapsedSeconds<.36f*Base+Extra)return ElapsedSeconds/(Base+Extra/.36f);
 const float Seconds=ElapsedSeconds-Extra,At=.70f*Base,Hold=FMath::Max(AirflowDuration,0.f)+FMath::Max(CrashDuration,.2f)+FMath::Max(FeedDuration,0.f);
 return Seconds<=At?Seconds/Base:Seconds<At+Hold?.70f:.70f+(Seconds-At-Hold)/(Base+FMath::Max(ConstructionExtension,0.f)/.30f);
}
