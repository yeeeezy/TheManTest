#pragma once
#include "Weapons/_Shared/Firearms/Bullets/BulletBase.h"
#include "ExplosionGunBullet.generated.h"

// First impact retains BulletBase damage/feedback. The later blast is cosmetic only.
UCLASS()
class THEMANTEST_API AExplosionGunBullet : public ABulletBase
{
 GENERATED_BODY()
public:
 AExplosionGunBullet();
 UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Bullet|Explosion", meta=(ClampMin="0.0",Units="s"))
 float ExplosionDelay=2.f;
 UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Bullet|Explosion", meta=(Categories="GameplayCue.Weapon.ExplosionGun"))
 FGameplayTag ExplosionCueTag;
 UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Bullet|Explosion", meta=(ClampMin="0.0",Units="cm"))
 float AttachmentOffset=4.f;
 UFUNCTION(BlueprintPure, Category="Bullet|Explosion") bool IsAttachedAndCountingDown() const { return bAttached && !bDetonated; }
 UFUNCTION(BlueprintPure, Category="Bullet|Explosion") float GetRemainingExplosionTime() const;
 virtual void ProcessHit_Implementation(const FHitResult&,AActor*,UAbilitySystemComponent*) override;
protected:
 virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
private:
 void Detonate();
 FTimerHandle ExplosionTimer;
 bool bAttached=false;
 bool bDetonated=false;
 FVector LocalImpactPoint=FVector::ZeroVector;
 FVector LocalImpactNormal=FVector::UpVector;
 TWeakObjectPtr<UAbilitySystemComponent> ExplosionSourceASC;
 TWeakObjectPtr<AActor> ExplosionInstigator;
};
