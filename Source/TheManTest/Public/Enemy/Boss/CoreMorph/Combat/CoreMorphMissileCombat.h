#pragma once
#include "Components/ActorComponent.h"
#include "CoreMorphMissileCombat.generated.h"
class UStaticMeshComponent;
struct FCoreMorphMissile
{
 FVector Ground=FVector::ZeroVector,Normal=FVector::UpVector;
 FVector Launch=FVector::ZeroVector,Control=FVector::ZeroVector,Position=FVector::ZeroVector,Direction=FVector::UpVector;
 TArray<FVector> Trail;
 float LaunchAt=0,FlightTime=0,ImpactAt=-1,Damage=0;
 bool bLaunched=false,bResolved=false,bImpact=false;
};
UCLASS(ClassGroup=(AI),meta=(BlueprintSpawnableComponent))
class THEMANTEST_API UCoreMorphMissileCombat : public UActorComponent
{
 GENERATED_BODY()
public:
 UCoreMorphMissileCombat();
 virtual void TickComponent(float Dt,ELevelTick Tick,FActorComponentTickFunction* Function) override;
 UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Missiles",meta=(ClampMin="1",ClampMax="8")) int32 MissileCount=4;
 UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Missiles",meta=(ClampMin="100")) float ImpactRadius=1000;
 UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Missiles",meta=(ClampMin="100")) float ScatterRadius=3500;
 UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Missiles",meta=(ClampMin="0.3")) float WarningLead=.8f;
 UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Missiles",meta=(ClampMin="0.05")) float LaunchInterval=.3f;
 UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Missiles",meta=(ClampMin="0.5")) float TravelDuration=2.2f;
 UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Missiles") float BaseDamage=20;
 UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Missiles") float CooldownDuration=6;
 UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Missiles") float MaxRange=60000;
 UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Missiles") int32 RandomSeed=0;
 UPROPERTY(Transient) TObjectPtr<AActor> Target;
 bool CanFire() const;
 bool StartSalvo();
 void StopSalvo();
 bool IsActive() const {return bActive;}
 bool IsPaused() const;
 float GetClock() const {return Clock;}
 float GetLockedRadius() const {return LockedRadius;}
 FVector GetCoreLocation() const;
 const TArray<FCoreMorphMissile>& GetMissiles() const {return Missiles;}
 FSimpleMulticastDelegate OnFinished,OnInvalidated;
 DECLARE_MULTICAST_DELEGATE_OneParam(FMissileImpact,int32);
 FMissileImpact OnImpact;
private:
 TWeakObjectPtr<UStaticMeshComponent> Core;
 TArray<FCoreMorphMissile> Missiles;
 float Clock=0,LockedRadius=0;
 int32 SalvoNumber=0;
 bool bActive=false;
};
