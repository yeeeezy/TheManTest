#include "Core/_Shared/Feedback/BulletTimeSubsystem.h"
#include "Engine/World.h"
#include "GameFramework/WorldSettings.h"
#include "GameFramework/PlayerController.h"
#include "Camera/PlayerCameraManager.h"
#include "HAL/PlatformTime.h"

float FBulletTimeSettings::GetStrength(float Distance) const
{
 const float Inner=FMath::Max(0.f,InnerRadius),Outer=FMath::Max(Inner+1.f,OuterRadius);
 return bEnabled?1.f-FMath::Clamp((Distance-Inner)/(Outer-Inner),0.f,1.f):0.f;
}
float FBulletTimeSettings::Evaluate(float Seconds) const
{
 const auto Smooth=[](float T){T=FMath::Clamp(T,0.f,1.f);return T*T*(3.f-2.f*T);};
 if(Seconds<SlowInDuration) return FMath::Lerp(1.f,TimeScale,Smooth(Seconds/FMath::Max(.01f,SlowInDuration)));
 return FMath::Lerp(TimeScale,1.f,Smooth((Seconds-SlowInDuration-HoldDuration)/FMath::Max(.01f,RecoveryDuration)));
}
bool UBulletTimeSubsystem::RequestBulletTimeAtLocation(FVector Location,const FBulletTimeSettings& Settings)
{
 if(!GetWorld()||Location.ContainsNaN())return false;
 float Strength=0.f;
 for(auto It=GetWorld()->GetPlayerControllerIterator();It;++It)
 {
  auto* PC=It->Get();
  if(PC&&PC->IsLocalController()&&PC->PlayerCameraManager)
   Strength=FMath::Max(Strength,Settings.GetStrength(FVector::Distance(Location,PC->PlayerCameraManager->GetCameraLocation())));
 }
 FBulletTimeSettings Local=Settings;
 Local.TimeScale=FMath::Lerp(1.f,Settings.TimeScale,Strength);
 return Strength>UE_KINDA_SMALL_NUMBER&&RequestBulletTime(Local);
}
bool UBulletTimeSubsystem::RequestBulletTime(const FBulletTimeSettings& Settings)
{
 if(!GetWorld()||GetWorld()->GetNetMode()!=NM_Standalone||!Settings.bEnabled
  ||!FMath::IsFinite(Settings.TimeScale)||!FMath::IsFinite(Settings.SlowInDuration)
  ||!FMath::IsFinite(Settings.HoldDuration)||!FMath::IsFinite(Settings.RecoveryDuration)
  ||Settings.TimeScale>=1.f||Settings.SlowInDuration<=0||Settings.RecoveryDuration<=0||Settings.HoldDuration<0)return false;
 if(bActive)Tick(0);
 if(bActive||FPlatformTime::Seconds()<RecoveryUntil)return false;
 ActiveSettings=Settings;
 ActiveSettings.TimeScale=FMath::Clamp(Settings.TimeScale,.01f,1.f);
 ActiveSettings.SlowInDuration=FMath::Clamp(Settings.SlowInDuration,.01f,2.f);
 ActiveSettings.HoldDuration=FMath::Clamp(Settings.HoldDuration,0.f,2.f);
 ActiveSettings.RecoveryDuration=FMath::Clamp(Settings.RecoveryDuration,.01f,2.f);
 OriginalDilation=AppliedDilation=GetWorld()->GetWorldSettings()->TimeDilation;
 StartRealTime=FPlatformTime::Seconds();bActive=true;
 return true; // Start at the previous speed; never jump straight into slow motion.
}
void UBulletTimeSubsystem::Tick(float DeltaTime)
{
 if(!bActive)return;
 auto* WS=GetWorld()->GetWorldSettings();
 const double Now=FPlatformTime::Seconds();
 if(!FMath::IsNearlyEqual(WS->TimeDilation,AppliedDilation))
 {bActive=false;RecoveryUntil=Now+.1;return;}
 if(Now-StartRealTime>=ActiveSettings.GetDuration())
 {CancelBulletTime();RecoveryUntil=Now+.1;return;}
 AppliedDilation=WS->SetTimeDilation(OriginalDilation*ActiveSettings.Evaluate(float(Now-StartRealTime)));
}
void UBulletTimeSubsystem::CancelBulletTime()
{
 if(bActive&&GetWorld())
 {
  auto* WS=GetWorld()->GetWorldSettings();
  if(WS&&FMath::IsNearlyEqual(WS->TimeDilation,AppliedDilation))WS->SetTimeDilation(OriginalDilation);
 }
 bActive=false;
}
void UBulletTimeSubsystem::OnWorldEndPlay(UWorld& World){CancelBulletTime();Super::OnWorldEndPlay(World);}
void UBulletTimeSubsystem::Deinitialize(){CancelBulletTime();Super::Deinitialize();}
