#include "Characters/TheExecutive/Drone/ExecutiveDroneComponent.h"
#include "Characters/TheExecutive/Drone/ExecutiveDrone.h"
#include "Characters/CharacterBase/FPSCharacterBase/FPSCharacterBase.h"
#include "Characters/CharacterBase/TheManAttributeSetBase.h"
#include "AbilitySystemComponent.h"
#include "Engine/World.h"
UExecutiveDroneComponent::UExecutiveDroneComponent() {PrimaryComponentTick.bCanEverTick=true;PrimaryComponentTick.TickInterval=.1f;}
void UExecutiveDroneComponent::TickComponent(float Dt,ELevelTick Type,FActorComponentTickFunction* Fn)
{
 Super::TickComponent(Dt,Type,Fn);
 AActor* Leader=GetOwner();
 bool bEligible=bLobbyPresentation;
 if(auto* Character=Cast<AFPSCharacterBase>(Leader))
  if(auto* ASC=Character->GetAbilitySystemComponent()) bEligible=Character->IsPlayerControlled() && ASC->GetNumericAttribute(UTheManAttributeSetBase::GetHealthAttribute())>0.f;
 if(!bEligible) {ReleaseDrone();return;}
 if(!IsValid(Drone) && DroneClass && !Leader->IsHidden())
 {
  const auto* Defaults=DroneClass->GetDefaultObject<AExecutiveDrone>();
  const FVector Offset=bLobbyPresentation?Defaults->LobbyOffset:Defaults->FollowOffset;
  const FRotator Heading(0,Leader->GetActorRotation().Yaw,0);
  const FVector Position=Leader->GetActorLocation()+Heading.RotateVector(Offset);
  const FTransform Transform(FRotator(0,Heading.Yaw+(bLobbyPresentation?Defaults->LobbyYawOffset:0.f),0),Position);
  Drone=GetWorld()->SpawnActorDeferred<AExecutiveDrone>(DroneClass,Transform,Leader,Cast<APawn>(Leader),ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding);
  if(Drone) {Drone->InitializeCompanion(Leader,bLobbyPresentation);Drone->FinishSpawning(Transform);}
 }
 if(IsValid(Drone))
 {
  Drone->SetActorHiddenInGame(Leader->IsHidden());
  Drone->SetActorTickEnabled(!Leader->IsHidden());
 }
}
void UExecutiveDroneComponent::ReleaseDrone() {if(IsValid(Drone)) Drone->Destroy();Drone=nullptr;}
void UExecutiveDroneComponent::EndPlay(const EEndPlayReason::Type Reason) {ReleaseDrone();Super::EndPlay(Reason);}
