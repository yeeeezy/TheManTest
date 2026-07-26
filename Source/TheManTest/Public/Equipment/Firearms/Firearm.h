#pragma once

#include "CoreMinimal.h"
#include "Equipment/WeaponBase/WeaponBase.h"
#include "Equipment/Firearms/Bullets/BulletBase.h"
#include "GameplayAbilitySpec.h"
#include "Firearm.generated.h"

class UAnimMontage;
class USoundBase;
class UGameplayAbility;
class UAbilitySystemComponent;

UCLASS()
class THEMANTEST_API AFirearm : public AWeaponBase
{
	GENERATED_BODY()

public:
	AFirearm();

	virtual void Equip(AActor* NewOwner) override;
	virtual void Unequip() override;

	void GrantAbilities(UAbilitySystemComponent* ASC);
	void RevokeAbilities(UAbilitySystemComponent* ASC);

	/* ===== 射击参数（蓝图 Defaults 配置） ===== */

	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Shooting")
	bool bIsHitscan = false;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Shooting", meta = (ClampMin = "0.0"))
	float HitscanRange = 10000.f;

	// 枪口 Socket 名称，射线视觉连线从此处出发
	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Shooting")
	FName MuzzleSocketName = TEXT("Muzzle");

	// 子弹类型：命中逻辑（伤害/治疗/特效）由子弹负责
	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Shooting")
	TSubclassOf<ABulletBase> BulletClass;

	// 最小射击间隔（秒），后续可用 Cooldown GE 替代
	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Shooting", meta = (ClampMin = "0.0"))
	float FireRate = 0.1f;

	/* ===== 动画（蓝图 Defaults 配置） ===== */

	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Animation")
	UAnimMontage* FireMontage = nullptr;

	/* ===== 音效 ===== */

	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Audio")
	USoundBase* FireSound = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Audio", meta = (ClampMin = "0.0"))
	float FireSoundVolumeMultiplier = 1.f;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Audio", meta = (ClampMin = "0.0"))
	float FireSoundPitchMultiplier = 1.f;

	/* ===== 后坐力 ===== */

	// 开火时给予的仰角初始角速度（度/秒），总偏移量 ≈ Pitch / Damping
	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Recoil", meta = (ClampMin = "0.0"))
	float RecoilPitch = 20.f;

	// 水平随机初始角速度范围（度/秒）
	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Recoil")
	float RecoilYawMin = -6.f;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Recoil")
	float RecoilYawMax = 6.f;

	// 角速度衰减系数：越大收得越快，总偏移 ≈ Pitch / Damping 度
	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Recoil", meta = (ClampMin = "0.1"))
	float RecoilDamping = 18.f;


	/* ===== GAS 技能（蓝图 Defaults 配置） ===== */

	UPROPERTY(EditDefaultsOnly, Category = "Weapon|GAS")
	TSubclassOf<UGameplayAbility> PrimaryFireAbilityClass;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon|GAS")
	TSubclassOf<UGameplayAbility> SecondaryFireAbilityClass;

	/* ===== 枪口偏移（AimIK 用） ===== */
	UPROPERTY(EditDefaultsOnly, Category = "Equipment|Animation")
	FTransform MuzzleLocalTransform;

	/* ===== Getters ===== */
	FORCEINLINE bool IsHitscan() const { return bIsHitscan; }
	FORCEINLINE float GetHitscanRange() const { return HitscanRange; }
	FORCEINLINE FName GetMuzzleSocketName() const { return MuzzleSocketName; }
	FORCEINLINE TSubclassOf<ABulletBase> GetBulletClass() const { return BulletClass; }
	FORCEINLINE float GetFireRate() const { return FireRate; }

private:
	FGameplayAbilitySpecHandle PrimaryFireHandle;
	FGameplayAbilitySpecHandle SecondaryFireHandle;

	// 授予技能时所用的 ASC（位于持久的 PlayerState 上）。切换角色销毁旧角色时，
	// 旧角色已被 UnPossess、PlayerState 置空，GetAbilitySystemComponent() 返回 null，
	// 此时回收技能必须靠这个缓存指针，否则技能规格泄漏累积 → 开火多颗子弹炸膛。
	TWeakObjectPtr<UAbilitySystemComponent> GrantedASC;
};
