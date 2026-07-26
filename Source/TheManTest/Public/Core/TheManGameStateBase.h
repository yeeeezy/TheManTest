#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "TheManGameStateBase.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnMidRoundStrengthIncrease);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCombatPhaseChanged, int32, NewPhase);

UCLASS()
class THEMANTEST_API ATheManGameStateBase : public AGameStateBase
{
	GENERATED_BODY()

public:
	ATheManGameStateBase();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	// 玩家死亡后由蓝图调用，开启下一回合
	UFUNCTION(BlueprintCallable, Category = "Round")
	void StartNewRound();

	// 死亡统一路由（被打死 / 倒计时归零都走这里）：预判下一回合时长，
	// 低于下限 → 直接游戏结束（跳过测试地图，不闪一下）；否则携回合数回选角色界面。
	UFUNCTION(BlueprintCallable, Category = "Round")
	void RoutePlayerDeath();

	// 调试用：快进 Seconds 秒（默认 150 = 2.5 分钟）。与正常流逝走同一套推进逻辑：
	// 推进强度波、检查半场二阶段；剩余时间钳到 0 不减过头，归零则触发回合结束。
	UFUNCTION(BlueprintCallable, Category = "Round|Debug")
	void DebugSkipTime(float Seconds = 150.f);

	UFUNCTION(BlueprintPure, Category = "Round")
	FORCEINLINE int32 GetRoundNumber() const { return RoundNumber; }

	UFUNCTION(BlueprintPure, Category = "Round")
	FORCEINLINE float GetTimeRemaining() const { return TimeRemaining; }

	// 本回合已发生的强度增强波数（中途生成的敌人 BeginPlay 读取以回补已累积加成）
	UFUNCTION(BlueprintPure, Category = "Round")
	FORCEINLINE int32 GetElapsedStrengthWaves() const { return ElapsedStrengthWaves; }

	// 回合内每隔 StrengthIncreaseInterval 秒广播一次，EnemyBase 绑定后 CurrentStrength++
	UPROPERTY(BlueprintAssignable, Category = "Round")
	FOnMidRoundStrengthIncrease OnMidRoundStrengthIncrease;

	// 回合进行到一半时广播一次，EnemyBase 绑定后 SetCombatPhase(NewPhase) 切换技能集
	UPROPERTY(BlueprintAssignable, Category = "Round")
	FOnCombatPhaseChanged OnCombatPhaseChanged;

protected:
	// ── 配置（蓝图默认值可调）────────────────────────────

	// 第一回合倒计时（秒）。默认 600 = 10 分钟
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Round|Config")
	float BaseCountdownDuration = 600.f;

	// 进入二阶段的时机：回合时长剩余比例降到此值时切阶段（0.5 = 半场）
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Round|Config")
	float Phase2TriggerRemainingFraction = 0.5f;

	// 每回合倒计时缩短量（秒）。默认 150 = 2.5 分钟
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Round|Config")
	float CountdownReductionPerRound = 150.f;

	// 倒计时最短下限（秒）。默认 150 = 2.5 分钟。当某回合理论时长低于此值时不再开回合，直接游戏结束
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Round|Config")
	float MinCountdownDuration = 150.f;

	// 回合内怪物强度增加间隔（秒）。默认 150 = 2.5 分钟（每过一波伤害递增 + 强度 +1）
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Round|Config")
	float StrengthIncreaseInterval = 150.f;

	// ── 运行时状态 ────────────────────────────────────

	UPROPERTY(BlueprintReadOnly, Category = "Round|State")
	int32 RoundNumber = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Round|State")
	float TimeRemaining = 0.f;

	// ── 事件（蓝图覆写）──────────────────────────────

	// 新回合开始（UI 更新、音效等）
	UFUNCTION(BlueprintNativeEvent, Category = "Round|Events")
	void OnRoundStarted(int32 NewRoundNumber, float CountdownDuration);
	virtual void OnRoundStarted_Implementation(int32 NewRoundNumber, float CountdownDuration) {}

	// 倒计时归零；蓝图负责杀死玩家，完成后调用 StartNewRound()
	UFUNCTION(BlueprintNativeEvent, Category = "Round|Events")
	void OnCountdownExpired();
	virtual void OnCountdownExpired_Implementation();

private:
	// 推进回合 DeltaSeconds 秒：倒计时 + 强度波 + 半场二阶段 + 归零结束。Tick 与 DebugSkipTime 共用。
	void AdvanceRound(float DeltaSeconds);

	// 指定回合的理论时长（未钳下限）：Base - Reduction×(Round-1)。低于 MinCountdownDuration 即视为游戏结束。
	float ComputeRoundDuration(int32 Round) const;

	float StrengthIncreaseAccumulator = 0.f;
	bool bRoundActive = false;

	// 本回合实际倒计时时长（用于算半场阈值）
	float CurrentRoundDuration = 0.f;
	// 本回合是否已切到二阶段（每回合只触发一次）
	bool bPhase2Triggered = false;
	// 本回合已发生的强度增强波数（每广播一次 OnMidRoundStrengthIncrease +1，新回合归零）
	int32 ElapsedStrengthWaves = 0;
};
