#pragma once

#include "CoreMinimal.h"
#include "Weapons/_Shared/WeaponBase/WeaponBase.h"
#include "Weapons/_Shared/Firearms/Bullets/BulletBase.h"
#include "GameplayAbilitySpec.h"
#include "Firearm.generated.h"

class UAnimMontage;
class USoundBase;
class UGameplayAbility;
class UAbilitySystemComponent;
class UNiagaraSystem;
class UCameraShakeBase;
class UStaticMeshComponent;
class UPointLightComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(
	FPlayerAmmoChanged,
	int32, CurrentAmmo,
	int32, MagazineCapacity,
	int32, SpareMagazineCount);

UCLASS()
class THEMANTEST_API AFirearm : public AWeaponBase
{
	GENERATED_BODY()

public:
	AFirearm();
	virtual void BeginPlay() override;

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

	/* ===== 弹药（蓝图 Defaults 配置） ===== */

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Ammo", meta = (ClampMin = "1"))
	int32 MagazineCapacity = 30;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Ammo", meta = (ClampMin = "0"))
	int32 SpareMagazineCount = 3;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Weapon|Ammo")
	int32 CurrentAmmo = 30;

	UPROPERTY(BlueprintAssignable, Category = "Weapon|Ammo")
	FPlayerAmmoChanged OnAmmoChanged;

	UFUNCTION(BlueprintCallable, Category = "Weapon|Ammo")
	bool ConsumeRound();

	UFUNCTION(BlueprintCallable, Category = "Weapon|Ammo")
	bool ReloadMagazine();

	UFUNCTION(BlueprintPure, Category = "Weapon|Ammo")
	bool CanFire() const { return CurrentAmmo > 0; }

	UFUNCTION(BlueprintPure, Category = "Weapon|Ammo")
	bool CanReload() const { return CurrentAmmo < MagazineCapacity && SpareMagazineCount > 0; }

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

	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Audio")
	USoundBase* DryFireSound = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Audio", meta = (ClampMin = "0.0"))
	float DryFireSoundVolumeMultiplier = 1.f;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Audio", meta = (ClampMin = "0.0"))
	float DryFireSoundPitchMultiplier = 1.f;

	/* ===== 枪口特效 ===== */

	UPROPERTY(EditDefaultsOnly, Category = "Weapon|VFX")
	TObjectPtr<UNiagaraSystem> MuzzleEffect;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon|VFX")
	FRotator MuzzleEffectRotation = FRotator::ZeroRotator;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon|VFX")
	FVector MuzzleEffectScale = FVector(2.f);

	// Optional fire-time light. Disabled by default so existing firearms keep their current look.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|VFX|Muzzle Light")
	bool bEnableMuzzleFlashLight = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|VFX|Muzzle Light")
	FLinearColor MuzzleFlashLightColor = FLinearColor(0.075319f, 1.f, 0.652928f, 1.f);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|VFX|Muzzle Light", meta = (ClampMin = "0.0"))
	float MuzzleFlashLightIntensity = 600.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|VFX|Muzzle Light", meta = (ClampMin = "0.0"))
	float MuzzleFlashLightAttenuationRadius = 200.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|VFX|Muzzle Light", meta = (ClampMin = "0.0"))
	float MuzzleFlashLightSourceRadius = 0.f;

	// Offset in muzzle-local space; useful for moving the light out of a shadow-casting barrel.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|VFX|Muzzle Light")
	FVector MuzzleFlashLightLocalOffset = FVector::ZeroVector;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|VFX|Muzzle Light", meta = (ClampMin = "0.01"))
	float MuzzleFlashLightDuration = 0.1f;

	void PlayMuzzleFlashLight();

	/* ===== 相机反馈 ===== */

	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Camera")
	TSubclassOf<UCameraShakeBase> FireCameraShake;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Camera", meta = (ClampMin = "0.0"))
	float FireCameraShakeScale = 1.f;

	/* ===== 后坐力 ===== */

	// 开火时给予的仰角初始角速度（度/秒），总偏移量 ≈ Pitch / Damping
	// Temporary gameplay-test switch: camera shake remains active while view kick is disabled.
	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Recoil")
	bool bEnableViewRecoil = false;

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

	UPROPERTY(EditDefaultsOnly, Category = "Weapon|GAS")
	TSubclassOf<UGameplayAbility> ReloadAbilityClass;

	/* ===== 枪口偏移（AimIK 用） ===== */
	UPROPERTY(EditDefaultsOnly, Category = "Equipment|Animation")
	FTransform MuzzleLocalTransform;

	/* ===== Getters ===== */
	FORCEINLINE bool IsHitscan() const { return bIsHitscan; }
	FORCEINLINE float GetHitscanRange() const { return HitscanRange; }
	FORCEINLINE FName GetMuzzleSocketName() const { return MuzzleSocketName; }
	FTransform GetMuzzleWorldTransform() const;
	FORCEINLINE TSubclassOf<ABulletBase> GetBulletClass() const { return BulletClass; }
	FORCEINLINE float GetFireRate() const { return FireRate; }
	FORCEINLINE int32 GetCurrentAmmo() const { return CurrentAmmo; }
	FORCEINLINE int32 GetMagazineCapacity() const { return MagazineCapacity; }
	FORCEINLINE int32 GetSpareMagazineCount() const { return SpareMagazineCount; }
	// A point light is spherical, so non-uniform Niagara scale uses its largest absolute axis.
	FORCEINLINE float GetMuzzleEffectSizeScale() const { return MuzzleEffectScale.GetAbsMax(); }
	FORCEINLINE UPointLightComponent* GetMuzzleFlashLight() const { return MuzzleFlashLight; }

private:
	// 可选的静态枪体叠加壳（例如 VFXPack Rifle Outline）；附着主 StaticMesh，不参与碰撞/弹道。
	UPROPERTY(VisibleAnywhere, Category = "Weapon|Visual")
	TObjectPtr<UStaticMeshComponent> StaticMeshOverlay;

	UPROPERTY(VisibleAnywhere, Category = "Weapon|VFX|Muzzle Light")
	TObjectPtr<UPointLightComponent> MuzzleFlashLight;

	void UpdateMuzzleFlashLight();
	void StopMuzzleFlashLight();

	FTimerHandle MuzzleFlashLightTimerHandle;
	float MuzzleFlashLightStartTime = 0.f;

	FGameplayAbilitySpecHandle PrimaryFireHandle;
	FGameplayAbilitySpecHandle SecondaryFireHandle;
	FGameplayAbilitySpecHandle ReloadHandle;

	// 授予技能时所用的 ASC（位于持久的 PlayerState 上）。切换角色销毁旧角色时，
	// 旧角色已被 UnPossess、PlayerState 置空，GetAbilitySystemComponent() 返回 null，
	// 此时回收技能必须靠这个缓存指针，否则技能规格泄漏累积 → 开火多颗子弹炸膛。
	TWeakObjectPtr<UAbilitySystemComponent> GrantedASC;
};
