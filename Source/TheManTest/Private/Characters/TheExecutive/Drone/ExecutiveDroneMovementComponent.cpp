#include "Characters/TheExecutive/Drone/ExecutiveDroneMovementComponent.h"
#include "Characters/TheExecutive/Drone/ExecutiveDrone.h"
UExecutiveDroneMovementComponent::UExecutiveDroneMovementComponent() {PrimaryComponentTick.bCanEverTick=true;}
void UExecutiveDroneMovementComponent::TickComponent(float Dt,ELevelTick Type,FActorComponentTickFunction* Fn)
{
 Super::TickComponent(Dt,Type,Fn);
 auto* Drone=Cast<AExecutiveDrone>(PawnOwner);
 if(!Drone || !UpdatedComponent || ShouldSkipUpdate(Dt)) return;
 if(Drone->IsHidden()) {Velocity=FVector::ZeroVector;UpdateComponentVelocity();return;}
 const FVector Desired=((Drone->FlightGoal-UpdatedComponent->GetComponentLocation())*PositionGain).GetClampedToMaxSize(MaxSpeed);
 Velocity=FMath::VInterpConstantTo(Velocity,Desired,Dt,Acceleration);
 FHitResult Hit;
 SafeMoveUpdatedComponent(Velocity*Dt,UpdatedComponent->GetComponentQuat(),true,Hit);
 bBlocked=Hit.IsValidBlockingHit();
 if(bBlocked) {SlideAlongSurface(Velocity*Dt,1.f-Hit.Time,Hit.Normal,Hit,true);Velocity=FVector::VectorPlaneProject(Velocity,Hit.Normal);}
 UpdateComponentVelocity();
}
