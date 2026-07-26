#pragma once

#include "CoreMinimal.h"
#include "Characters/Enemy/EnemyBase.h"
#include "HumanoidEnemyTypes.h"
#include "HumanoidEnemy.generated.h"

class APatrolPoint;
class UStaticMeshComponent;
struct FAIRequestID;
struct FPathFollowingResult;

UCLASS()
class THEMANTEST_API AHumanoidEnemy : public AEnemyBase
{
	GENERATED_BODY()

public:
	AHumanoidEnemy();

	virtual void BeginPlay() override;
	virtual void PossessedBy(AController* NewController) override;

	// --- AnimInstance 轮询接口 ---
	UFUNCTION(BlueprintPure, Category = "Enemy|AI")
	FORCEINLINE EHumanoidEnemyAIState GetAIState() const { return AIState; }

	UFUNCTION(BlueprintPure, Category = "Enemy|Turn")
	FORCEINLINE bool IsPendingTurn() const { return bPendingTurn; }

	UFUNCTION(BlueprintPure, Category = "Enemy|Turn")
	FORCEINLINE float GetPendingTurnAngle() const { return PendingTurnAngle; }

	// --- AI 调用接口 ---
	UFUNCTION(BlueprintCallable, Category = "Enemy|AI")
	void SetAIState(EHumanoidEnemyAIState NewState);

	// --- AimIK 数据（由 AIController Tick 写入，AnimInstance 读取）---
	UPROPERTY(BlueprintReadWrite, Category = "Enemy|AimIK")
	FVector AimTargetWorld = FVector::ZeroVector;

	UPROPERTY(BlueprintReadWrite, Category = "Enemy|AimIK")
	bool bIsAiming = false;

	// 由 BTTask_ResumeNearestPatrol 调用：找最近路点并重启巡逻（仅在 bNeedsPatrolResume 时有效）
	UFUNCTION(BlueprintCallable, Category = "Enemy|Patrol")
	void ResumeNearestPatrol();

	UFUNCTION(BlueprintCallable, Category = "Enemy|Turn")
	void RequestTurn(float Angle);

	UFUNCTION(BlueprintCallable, Category = "Enemy|Turn")
	void OnTurnComplete();

	virtual void Tick(float DeltaSeconds) override;

	// AnimInstance 轮询
	UFUNCTION(BlueprintPure, Category = "Enemy|Patrol")
	FORCEINLINE bool IsPatrolScanning() const { return bIsPatrolScanning; }

	UFUNCTION(BlueprintPure, Category = "Enemy|Patrol")
	FORCEINLINE bool IsStoppingAtPoint() const { return bIsStoppingAtPoint; }

	UFUNCTION(BlueprintPure, Category = "Weapon")
	FORCEINLINE UStaticMeshComponent* GetWeaponMesh() const { return WeaponMesh; }

protected:
	// 放招前把瞄准点写为目标位置，供 UGA_EnemyShoot 取子弹方向 + AimIK 用
	virtual void AimAtTarget(AActor* Target) override;

	UPROPERTY(BlueprintReadOnly, Category = "Enemy|AI")
	EHumanoidEnemyAIState AIState = EHumanoidEnemyAIState::Patrol;

	UPROPERTY(BlueprintReadOnly, Category = "Enemy|Turn")
	bool bPendingTurn = false;

	UPROPERTY(BlueprintReadOnly, Category = "Enemy|Turn")
	float PendingTurnAngle = 0.f;

	// 巡逻路点数组，在关卡实例上点选 Actor 填入
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Patrol")
	TArray<TObjectPtr<APatrolPoint>> PatrolPoints;

	// 等待时间达到此阈值才播放扫视动画（秒）
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Patrol")
	float MinScanWaitTime = 2.f;

	// 巡逻移动速度（cm/s）
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Patrol|Movement")
	float PatrolWalkSpeed = 150.f;

	// 战斗移动速度（cm/s）
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Aim|Movement")
	float CombatWalkSpeed = 300.f;

	// 偏转角超过此值才触发转身动画（度）
	UPROPERTY(EditDefaultsOnly, Category = "Patrol|Turn")
	float TurnAngleThreshold = 30.f;

	// 代码旋转速度（度/秒），需与最慢转身动画的视觉速度匹配
	UPROPERTY(EditDefaultsOnly, Category = "Patrol|Turn")
	float TurnRotationSpeed = 270.f;

	// 转身期间的移动速度（cm/s），0 = 原地转身
	UPROPERTY(EditDefaultsOnly, Category = "Patrol|Turn")
	float TurnWalkSpeed = 50.f;

	// 开始减速的距离（cm），进入此范围后线性降速到 MinApproachSpeed
	UPROPERTY(EditDefaultsOnly, Category = "Patrol|Movement")
	float SlowdownRadius = 150.f;

	// 最低接近速度（cm/s），AI 停车时角色约为此速度
	UPROPERTY(EditDefaultsOnly, Category = "Patrol|Movement")
	float MinApproachSpeed = 30.f;

	// 武器 StaticMesh，蓝图 CDO 里赋值网格资产
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	TObjectPtr<UStaticMeshComponent> WeaponMesh;

	// 武器挂载到哪个骨骼/Socket（默认 hand_r）
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	FName WeaponAttachSocket = "hand_r";

private:
	int32 CurrentPatrolIndex   = 0;
	bool  bIsPatrolScanning    = false;
	bool  bIsStoppingAtPoint   = false;
	bool  bNeedsPatrolResume   = false;
	float TargetTurnYaw = 0.f;
	FTimerHandle PatrolWaitTimer;
	FTimerHandle ScanDelayTimer;

	void MoveToNextPatrolPoint();
	void TryTurnOrMove();
	void OnPatrolWaitFinished();
	void OnPatrolMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result);
	int32 FindNearestPatrolPointIndex() const;
};
