#include "Characters/TheExecutive/Drone/ExecutiveDroneAnimInstance.h"
#include "Characters/TheExecutive/Drone/ExecutiveDrone.h"
void UExecutiveDroneAnimInstance::NativeUpdateAnimation(float Dt)
{
 Super::NativeUpdateAnimation(Dt);
 if(const auto* Drone=Cast<AExecutiveDrone>(TryGetPawnOwner()))
 {
  const float Rate=Drone->YawRate;
  if(Rate>18.f) FlightPose=2;
  else if(Rate<-18.f) FlightPose=1;
  else if(FMath::Abs(Rate)<8.f) FlightPose=0;
 }
}
