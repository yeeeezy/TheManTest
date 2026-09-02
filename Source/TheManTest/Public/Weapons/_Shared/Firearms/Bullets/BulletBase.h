#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayTagContainer.h"
#include "BulletBase.generated.h"

class UAbilitySystemComponent;
class UGameplayEffect;
class USphereComponent;
class UStaticMeshComponent;
class UProjectileMovementComponent;

UCLASS(Abstract)
class THEMANTEST_API ABulletBase : public AActor
{
	GENERATED_BODY()

public:
	ABulletBase();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Bullet|Components")
	USphereComponent* CollisionSphere;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Bullet|Components")
	UStaticMeshComponent* BulletMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Bullet|Components")
	UProjectileMovementComponent* ProjectileMovement;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Bullet|GAS")
	TSubclassOf<UGameplayEffect> HitEffectClass;

	// 伤害数值，命中时通过 SetByCaller(Data.Damage) 传入 HitEffectClass；0 = 不造成伤害
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Bullet|GAS", meta = (ClampMin = "0.0"))
	float Damage = 0.f;

	// 由具体武器/弹体配置。有效命中时在攻击者 ASC 上执行；为空则不产生武器命中表现。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Bullet|Feedback", meta = (Categories = "GameplayCue.Weapon"))
	FGameplayTag ImpactCueTag;

	// 命中后是否销毁子弹。默认 true（普通伤害弹打到任何东西即停下销毁）。
	// 自管生命周期的子类（如 ARepairGunBullet 命中后膨胀）在构造函数置 false。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Bullet")
	bool bDestroyOnHit = true;

	void InitBullet(AActor* HitInstigator, UAbilitySystemComponent* SourceASC);

	UFUNCTION(BlueprintNativeEvent, Category = "Bullet")
	void ProcessHit(const FHitResult& HitResult, AActor* HitInstigator, UAbilitySystemComponent* SourceASC);
	virtual void ProcessHit_Implementation(const FHitResult& HitResult, AActor* HitInstigator, UAbilitySystemComponent* SourceASC);

private:
	TWeakObjectPtr<UAbilitySystemComponent> CachedSourceASC;
	TWeakObjectPtr<AActor>                  CachedInstigator;

	bool bHasProcessedHit = false;

	UFUNCTION()
	void OnBulletHit(UPrimitiveComponent* HitComponent, AActor* OtherActor,
	                 UPrimitiveComponent* OtherComp, FVector NormalImpulse,
	                 const FHitResult& Hit);
};
