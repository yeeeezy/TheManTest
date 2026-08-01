#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GA_EnemyShoot.generated.h"

class ABulletBase;
class UAnimMontage;
class UAnimSequenceBase;
class USoundBase;
class AHumanoidEnemy;
class UNiagaraSystem;

/**
 * UGA_EnemyShoot
 * 人形怪射击技能基类，复用玩家同一套子弹+伤害管线（ABulletBase + GE_BulletDamage）。
 *
 * 设计要点：
 * - 「技能与子弹绑定」：BulletClass 等是本技能的 UPROPERTY。一种子弹建一个蓝图子类各填一种，
 *   同一敌人可同时拥有多个本类的蓝图子类（各绑不同子弹），归入敌人的 PhaseSkillSets（按阶段+近/中/远分组），
 *   由 BTTask_UseCombatSkill → AEnemyBase::UseRandomSkill 按类激活，互不干扰（故本类不注册 GameplayEvent 触发器）。
 * - 「逻辑复用」：算枪口/方向/播蒙太奇/音效/结束流程都在本基类。只有子弹「怎么发」
 *   （单发 / 散射 / hitscan…）不同时，才写 C++ 子类重写 SpawnProjectiles()，其余照旧复用。
 * - 不依赖玩家专属的 AFirearm / EquipmentManager / 相机 / 后坐力。
 */
UCLASS()
class THEMANTEST_API UGA_EnemyShoot : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UGA_EnemyShoot();

	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

	UFUNCTION(BlueprintPure, Category = "EnemyShoot|Accuracy")
	float GetCurrentSpreadDegrees() const { return CurrentSpreadDegrees; }

	UFUNCTION(BlueprintPure, Category = "EnemyShoot|VFX")
	UNiagaraSystem* GetMuzzleEffect() const { return MuzzleEffect; }
	FVector GetMuzzleEffectScale() const { return MuzzleEffectScale; }

	// Shared accuracy entry point for automatic-fire variants and future humanoid weapons.
	UFUNCTION(BlueprintCallable, Category = "EnemyShoot|Accuracy")
	FVector CalculateShotDirection(AHumanoidEnemy* Enemy, const FVector& FireDir);

protected:
	// 执行一发完整射击表现；供三连发/持续扫射子类按节奏复用。
	bool FireSingleRound(AHumanoidEnemy* Enemy);

	// 子弹生成扩展点：默认从枪口单发 BulletClass。
	// 不同开火逻辑（散射 / 连发 / hitscan）由 C++ 子类重写本函数，复用基类其余流程。
	virtual void SpawnProjectiles(AHumanoidEnemy* Enemy, const FVector& MuzzleLocation, const FVector& FireDir);

	// 本技能发射的子弹（技能与子弹绑定）。可复用玩家伤害弹 BP，或配本技能专属伤害值的 BP。
	UPROPERTY(EditDefaultsOnly, Category = "EnemyShoot")
	TSubclassOf<ABulletBase> BulletClass;

	// 武器 StaticMesh 上的枪口 socket 名；不存在则退回武器组件原点
	UPROPERTY(EditDefaultsOnly, Category = "EnemyShoot")
	FName MuzzleSocketName = TEXT("Muzzle");

	// 武器没有枪口 Socket 时的可配置局部生成点，避免把子弹生成在角色/武器原点。
	UPROPERTY(EditDefaultsOnly, Category = "EnemyShoot")
	FVector MuzzleRelativeOffset = FVector(70.f, 0.f, 10.f);

	// 开火蒙太奇（可选，留空不播）。FEAT-027 上半身插槽就绪前播全身会盖住移动，按需配置。
	UPROPERTY(EditDefaultsOnly, Category = "EnemyShoot")
	UAnimMontage* FireMontage = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "EnemyShoot")
	TObjectPtr<UAnimSequenceBase> FireAnimation;

	UPROPERTY(EditDefaultsOnly, Category = "EnemyShoot")
	FName FireAnimationSlot = TEXT("DefaultSlot");

	UPROPERTY(EditDefaultsOnly, Category = "EnemyShoot")
	USoundBase* FireSound = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "EnemyShoot", meta = (ClampMin = "0.0"))
	float FireSoundVolumeMultiplier = 1.f;

	UPROPERTY(EditDefaultsOnly, Category = "EnemyShoot", meta = (ClampMin = "0.0"))
	float FireSoundPitchMultiplier = 1.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "EnemyShoot|VFX")
	TObjectPtr<UNiagaraSystem> MuzzleEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "EnemyShoot|VFX")
	FRotator MuzzleEffectRotation = FRotator::ZeroRotator;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "EnemyShoot|VFX")
	FVector MuzzleEffectScale = FVector::OneVector;

	// Reusable humanoid accuracy model: first shots are controlled, sustained fire blooms,
	// and movement adds a separate penalty. Values are overridable per ability Blueprint.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "EnemyShoot|Accuracy", meta = (ClampMin = "0.0", ClampMax = "20.0"))
	float BaseSpreadDegrees = 1.5f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "EnemyShoot|Accuracy", meta = (ClampMin = "0.0", ClampMax = "20.0"))
	float SpreadPerShotDegrees = 0.45f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "EnemyShoot|Accuracy", meta = (ClampMin = "0.0", ClampMax = "30.0"))
	float MaxSpreadDegrees = 5.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "EnemyShoot|Accuracy", meta = (ClampMin = "0.0"))
	float SpreadRecoveryDegreesPerSecond = 3.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "EnemyShoot|Accuracy", meta = (ClampMin = "0.0", ClampMax = "10.0"))
	float MovingSpreadPenaltyDegrees = 1.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "EnemyShoot|Accuracy", meta = (ClampMin = "1.0"))
	float MovingSpeedThreshold = 25.f;

private:
	float CurrentSpreadDegrees = 0.f;
	double LastShotTimeSeconds = -1.0;
};
