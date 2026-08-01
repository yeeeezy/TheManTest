#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "EnemyBase.generated.h"

class UAbilitySystemComponent;
class UEnemyAttributeSetBase;
class UGameplayEffect;
class UGameplayAbility;

// 交战距离档：技能集内部按近/中/远分组，BT 节点选其一随机放招
UENUM(BlueprintType)
enum class EEnemySkillRange : uint8
{
	Near	UMETA(DisplayName = "Near"),
	Mid		UMETA(DisplayName = "Mid"),
	Far		UMETA(DisplayName = "Far")
};

// 一个战斗阶段的技能集，内部按交战距离分近/中/远三组
USTRUCT(BlueprintType)
struct FEnemyPhaseSkillSet
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill")
	TArray<TSubclassOf<UGameplayAbility>> NearAbilities;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill")
	TArray<TSubclassOf<UGameplayAbility>> MidAbilities;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill")
	TArray<TSubclassOf<UGameplayAbility>> FarAbilities;
};

UCLASS()
class THEMANTEST_API AEnemyBase : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	AEnemyBase();

	virtual void BeginPlay() override;

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	void OnDeath();

	// 从「当前阶段」技能集里、指定交战距离档(近/中/远)随机释放一个技能。返回是否成功放出。
	UFUNCTION(BlueprintCallable, Category = "Enemy|Combat")
	bool UseRandomSkill(AActor* Target, EEnemySkillRange Range);

	// 切换战斗阶段（1 起；默认 1，第二阶段换整组技能集）
	UFUNCTION(BlueprintCallable, Category = "Enemy|Combat")
	virtual void SetCombatPhase(int32 NewPhase);

	// 子弹命中前查询；阶段技能可让弹体继续飞行且不施加伤害。
	UFUNCTION(BlueprintPure, Category = "Enemy|Combat")
	virtual bool ShouldProjectilePassThrough() const { return false; }

	UFUNCTION(BlueprintPure, Category = "Enemy|Combat")
	FORCEINLINE int32 GetCombatPhase() const { return CurrentPhase; }

	// 当前伤害倍率（阶段1=1；阶段≥2=StrengthDamageMultiplier）。技能生成伤害时乘上它。
	UFUNCTION(BlueprintPure, Category = "Enemy|Combat")
	FORCEINLINE float GetDamageMultiplier() const { return CurrentDamageMultiplier; }

	UFUNCTION(BlueprintPure, Category = "Enemy")
	FORCEINLINE int32 GetCurrentStrength() const { return CurrentStrength; }

	UFUNCTION(BlueprintPure, Category = "Enemy")
	FORCEINLINE bool IsDead() const { return bIsDead; }

	// 玩家弹体有效命中后的统一警觉入口。基类立即水平转向攻击者；具体敌人可扩展战斗状态/Focus。
	UFUNCTION(BlueprintCallable, Category = "Enemy|Combat")
	virtual void ReactToProjectileHit(AActor* HitInstigator);

	// 施加限时移动减速。SlowPercent=0.4 表示减速40%；重复命中刷新时长且只保留最强减速。
	UFUNCTION(BlueprintCallable, Category = "Enemy|Movement")
	void ApplyMovementSlow(float SlowPercent, float Duration);

	UFUNCTION(BlueprintPure, Category = "Enemy|Movement")
	FORCEINLINE float GetActiveMovementSpeedMultiplier() const { return ActiveMovementSpeedMultiplier; }

protected:
	// 敌人状态切换必须通过此入口设置基础速度，当前减速会自动叠加且到期恢复到最新状态速度。
	void SetDesiredMaxWalkSpeed(float NewSpeed);

	// ASC 挂在敌人自身（无 PlayerState）
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
	UAbilitySystemComponent* AbilitySystemComponent;

	UPROPERTY()
	UEnemyAttributeSetBase* AttributeSet;

	// 初始化血量的 Instant GE（蓝图配置，与玩家共用 GE_CharacterBaseBase_Init）
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS")
	TSubclassOf<UGameplayEffect> InitGEClass;

	// BeginPlay 始终授予的通用技能（非阶段战斗技能；战斗技能放 PhaseSkillSets）
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS")
	TArray<TSubclassOf<UGameplayAbility>> DefaultAbilities;

	// 各阶段技能集：index 0 = 阶段1，index 1 = 阶段2……每个阶段内部再分近/中/远
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy|Combat")
	TArray<FEnemyPhaseSkillSet> PhaseSkillSets;

	// 当前战斗阶段（1 起）
	UPROPERTY(BlueprintReadOnly, Category = "Enemy|Combat")
	int32 CurrentPhase = 1;

	// 强度增强系数（每波）：回合内每过一个 StrengthIncreaseInterval（强度+1 那波），
	// 本敌人伤害倍率累加该百分比。倍率 = 1 + StrengthDamageBonusPerWave × 已增强波数。
	// 例：0.2 → 第1波 ×1.2、第2波 ×1.4、第3波 ×1.6……（与战斗阶段无关）
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy|Combat", meta = (ClampMin = "0.0"))
	float StrengthDamageBonusPerWave = 0.2f;

	// 伤害倍率上限：无论逐回合 + 回合内累加多少波，CurrentDamageMultiplier 最高钳到此值。
	// 默认 2.0 = 最多增强到初始伤害的 2 倍。蓝图 Details 面板可调。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy|Combat", meta = (ClampMin = "1.0"))
	float MaxDamageMultiplier = 2.0f;

	// 运行时当前伤害倍率：初始 1，每波 += StrengthDamageBonusPerWave，由 HandleMidRoundStrengthIncrease 维护
	UPROPERTY(BlueprintReadOnly, Category = "Enemy|Combat")
	float CurrentDamageMultiplier = 1.f;

	// 激活技能前的瞄准钩子：基类空实现；AHumanoidEnemy 重写写入 AimTargetWorld 供子弹/AimIK 用
	virtual void AimAtTarget(AActor* Target) {}

	// 把一组技能授予到 ASC（BeginPlay 用）
	void GrantAbilities(const TArray<TSubclassOf<UGameplayAbility>>& Abilities);

	// 初始属性（SetByCaller 传给 InitGE）
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS")
	float InitialMaxHealth = 100.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS")
	float InitialHealth = 100.f;

	// 每种敌人自己的基础强度，在蓝图 Details 面板配置
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy")
	int32 BaseStrength = 1;

	// 运行时强度：BeginPlay 时 = BaseStrength + RoundNumber；回合内每次广播 +1
	UPROPERTY(BlueprintReadOnly, Category = "Enemy")
	int32 CurrentStrength = 1;

private:
	bool bIsDead = false;
	bool bHasDesiredMaxWalkSpeed = false;
	float DesiredMaxWalkSpeed = 0.f;
	float ActiveMovementSpeedMultiplier = 1.f;
	FTimerHandle MovementSlowTimerHandle;

	void ClearMovementSlow();

	UFUNCTION()
	void HandleMidRoundStrengthIncrease();

	// GameState 半场广播：切换到指定战斗阶段（换技能集）
	UFUNCTION()
	void HandleCombatPhaseChanged(int32 NewPhase);
};
