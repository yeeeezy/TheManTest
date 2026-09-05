#pragma once
#include "Weapons/_Shared/Firearms/Bullets/BulletBase.h"
#include "Core/_Shared/Feedback/BulletTimeSubsystem.h"
#include "ExplosionGunBullet.generated.h"

// First impact retains BulletBase damage/feedback. Both impact types attach and detonate.
UCLASS()
class THEMANTEST_API AExplosionGunBullet : public ABulletBase
{
 GENERATED_BODY()
public:
 AExplosionGunBullet();
 UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Bullet|Explosion|Bullet Time", meta=(ShowOnlyInnerProperties))
 FBulletTimeSettings BulletTime;
 UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Bullet|Explosion|Damage", meta=(ClampMin="0"))
 float ExplosionDamage=20.f;
 UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Bullet|Explosion|Damage", meta=(ClampMin="0", Units="cm"))
 float ExplosionDamageRadius=400.f;
 UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Bullet|Explosion|Damage")
 TSubclassOf<UGameplayEffect> ExplosionDamageEffectClass;
 UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Bullet|Explosion", meta=(ClampMin="0.0",Units="s"))
 float ExplosionDelay=2.f;
 UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Bullet|Explosion", meta=(Categories="GameplayCue.Weapon.ExplosionGun"))
 FGameplayTag ExplosionCueTag;
 UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Bullet|Explosion", meta=(ClampMin="0.0",Units="cm"))
 float AttachmentOffset=4.f;
 UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Bullet|Explosion|Ground", meta=(ClampMin="0",Units="cm"))
 float GroundSearchDistance=2000.f;
 UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Bullet|Explosion|Ground", meta=(ClampMin="0",ClampMax="89",Units="deg"))
 float GroundMaxSlope=45.f;
 UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Bullet|Explosion|Chaos", meta=(ClampMin="0",Units="cm"))
 float ChaosRadius=400.f;
 UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Bullet|Explosion|Chaos", meta=(ClampMin="0"))
 float ChaosStrain=500000.f;
 UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Bullet|Explosion|Chaos", meta=(ClampMin="0"))
 float ChaosImpulse=1200.f;
 UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Bullet|Explosion|Chaos", meta=(ClampMin="0",Units="rad/s"))
 float ChaosAngularSpeed=5.f;
 static const FName ExplosionGroundTag;
 bool FindExplosionGround(const FVector& Origin,FHitResult& OutHit) const;
 bool DidHitEnemy() const { return bHitEnemy; }
 UFUNCTION(BlueprintPure, Category="Bullet|Explosion") bool IsAttachedAndCountingDown() const { return bAttached && !bDetonated; }
 UFUNCTION(BlueprintPure, Category="Bullet|Explosion") float GetRemainingExplosionTime() const;
 virtual void ProcessHit_Implementation(const FHitResult&,AActor*,UAbilitySystemComponent*) override;
protected:
 virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
private:
 void Detonate();
 void TriggerChaos(const FVector& Origin);
 void ApplyExplosionDamage(const FVector& Origin);
 FTimerHandle ExplosionTimer;
 bool bAttached=false;
 bool bDetonated=false;
 bool bHitEnemy=false;
 FVector LocalImpactPoint=FVector::ZeroVector;
 FVector LocalImpactNormal=FVector::UpVector;
 TWeakObjectPtr<UAbilitySystemComponent> ExplosionSourceASC;
 TWeakObjectPtr<AActor> ExplosionInstigator;
};
