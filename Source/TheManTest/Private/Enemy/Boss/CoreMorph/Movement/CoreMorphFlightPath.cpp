#include "Enemy/Boss/CoreMorph/Movement/CoreMorphFlightPath.h"

// Original FEAT058 reference route: position only, no presentation ownership.
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
float FCoreMorphFlightPath::Schedule(float C) const
{
 return (FMath::Max(ConstructionExtension,0.f)*FMath::Clamp((C-.70f)/.30f,0.f,1.f)+C*FMath::Max(Duration,3.f)+FMath::Max(AscentExtension,0.f)*FMath::Clamp(C/.36f,0.f,1.f)+(C>=.70f?FMath::Max(AirflowDuration,0.f)+FMath::Max(CrashDuration,.2f)+FMath::Max(FeedDuration,0.f):0))/GetMorphDuration();
}
