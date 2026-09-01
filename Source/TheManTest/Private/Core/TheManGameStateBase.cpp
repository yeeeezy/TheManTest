#include "Core/TheManGameStateBase.h"
#include "Core/TheManGameInstance.h"
#include "Engine/Engine.h"

// 把秒数格式化为「分:秒」显示（内部仍以秒计时）。例：125.4s → "2:05.4"
static FString FormatTimeMMSS(float TotalSeconds)
{
	const float Clamped = FMath::Max(TotalSeconds, 0.f);
	const int32 Minutes = FMath::FloorToInt(Clamped / 60.f);
	const float Seconds = Clamped - Minutes * 60.f;
	return FString::Printf(TEXT("%d:%04.1f"), Minutes, Seconds);
}

ATheManGameStateBase::ATheManGameStateBase()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ATheManGameStateBase::BeginPlay()
{
	Super::BeginPlay();

	// 从 GameInstance 读携带的回合数衔接（实现敌人初始强度逐回合 +1）；
	// StartNewRound 内 RoundNumber++，故首局 0→1，死亡重选后接着上回合 +1。
	if (UTheManGameInstance* GI = GetGameInstance<UTheManGameInstance>())
	{
		RoundNumber = GI->GetCarriedRoundNumber();
	}

	StartNewRound();
}

void ATheManGameStateBase::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!bRoundActive) return;

	AdvanceRound(DeltaSeconds);

	// 调试：屏幕固定位置显示剩余时间（key=1 原地刷新，不滚屏）
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(1, 0.f, FColor::Cyan,
			FString::Printf(TEXT("[Round %d | Phase %d] Time Remaining: %s"),
				RoundNumber, bPhase2Triggered ? 2 : 1, *FormatTimeMMSS(TimeRemaining)));
	}
}

void ATheManGameStateBase::AdvanceRound(float DeltaSeconds)
{
	if (!bRoundActive || DeltaSeconds <= 0.f) return;

	// 倒计时：钳到 0，不减过头
	TimeRemaining = FMath::Max(TimeRemaining - DeltaSeconds, 0.f);

	// 回合内强度递增：while 处理大跨度（调试快进可能一次跨多波）；间隔 ≤0 时跳过防死循环
	StrengthIncreaseAccumulator += DeltaSeconds;
	while (StrengthIncreaseInterval > 0.f && StrengthIncreaseAccumulator >= StrengthIncreaseInterval)
	{
		StrengthIncreaseAccumulator -= StrengthIncreaseInterval;
		ElapsedStrengthWaves++;
		OnMidRoundStrengthIncrease.Broadcast();
	}

	// 半场切二阶段（每回合一次）
	if (!bPhase2Triggered &&
		TimeRemaining <= CurrentRoundDuration * Phase2TriggerRemainingFraction)
	{
		bPhase2Triggered = true;
		OnCombatPhaseChanged.Broadcast(2);

		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow,
				FString::Printf(TEXT("[Round %d] Phase 2 started - enemies switched ability sets."), RoundNumber));
		}
		UE_LOG(LogTemp, Warning,
			TEXT("[TheManGameState] 回合 %d 半场，广播 OnCombatPhaseChanged(2)"), RoundNumber);
	}

	// 归零 → 回合结束（自然到点 / 调试减过头，同一路径，只触发一次）
	if (TimeRemaining <= 0.f)
	{
		bRoundActive = false;
		OnCountdownExpired();
	}
}

void ATheManGameStateBase::DebugSkipTime(float Seconds)
{
	if (!bRoundActive)
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Silver,
				TEXT("[Debug] No active round. Time skip ignored."));
		}
		return;
	}

	const float Skip = (Seconds > 0.f) ? Seconds : 150.f;
	AdvanceRound(Skip);   // 减过头、强度波、二阶段、回合结束都由 AdvanceRound 统一处理

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 4.0f, FColor::Green,
			FString::Printf(TEXT("[Debug] Skipped %.0fs | Remaining: %s | Strength waves: %d"),
				Skip, *FormatTimeMMSS(TimeRemaining), ElapsedStrengthWaves));
	}
}

void ATheManGameStateBase::StartNewRound()
{
	RoundNumber++;

	// 本回合理论时长：基数逐回合缩短。低于下限 → 不再开回合，游戏结束（回大厅显示"游戏结束"）。
	// 正常流程下游戏结束已在死亡时预判（RoutePlayerDeath，不会加载到此回合）；这里是防御性兜底。
	const float RawDuration = ComputeRoundDuration(RoundNumber);
	if (RawDuration < MinCountdownDuration)
	{
		bRoundActive = false;
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red,
				FString::Printf(TEXT("[Round %d] Duration is below the %.0fs minimum - Game Over."), RoundNumber, MinCountdownDuration));
		}
		if (UTheManGameInstance* GI = GetGameInstance<UTheManGameInstance>())
		{
			GI->HandleGameOver();
		}
		return;
	}

	const float Duration = RawDuration;

	TimeRemaining = Duration;
	CurrentRoundDuration = Duration;
	StrengthIncreaseAccumulator = 0.f;
	bPhase2Triggered = false;
	ElapsedStrengthWaves = 0;
	bRoundActive = true;

	OnRoundStarted(RoundNumber, Duration);
}

float ATheManGameStateBase::ComputeRoundDuration(int32 Round) const
{
	return BaseCountdownDuration - CountdownReductionPerRound * (Round - 1);
}

void ATheManGameStateBase::RoutePlayerDeath()
{
	// 预判下一回合（当前回合数 +1，与 StartNewRound 里的 RoundNumber++ 对应）：
	// 其理论时长低于下限 → 直接游戏结束，跳过测试地图（不再先加载再瞬切回大厅）。
	const int32 NextRound = RoundNumber + 1;
	if (UTheManGameInstance* GI = GetGameInstance<UTheManGameInstance>())
	{
		if (ComputeRoundDuration(NextRound) < MinCountdownDuration)
		{
			GI->HandleGameOver();
		}
		else
		{
			GI->HandlePlayerDeath(RoundNumber);
		}
	}
}

void ATheManGameStateBase::OnCountdownExpired_Implementation()
{
	// 倒计时归零 = 玩家死亡：与被打死同一路径，统一走 RoutePlayerDeath（含游戏结束预判）
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red,
			FString::Printf(TEXT("[Round %d] Time expired - player defeated."), RoundNumber));
	}

	RoutePlayerDeath();
}
