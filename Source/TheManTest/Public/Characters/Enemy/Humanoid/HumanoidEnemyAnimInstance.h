#pragma once

#include "CoreMinimal.h"
#include "Characters/Animation/BaseLocomotionAnimInstance.h"
#include "HumanoidEnemyTypes.h"
#include "HumanoidEnemyAnimInstance.generated.h"

class AHumanoidEnemy;

UCLASS()
class THEMANTEST_API UHumanoidEnemyAnimInstance : public UBaseLocomotionAnimInstance
{
	GENERATED_BODY()

public:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

protected:
	// AI 状态，驱动状态机大层跳转（Patrol / Aim / SearchRush / SearchScan / Dead）
	UPROPERTY(BlueprintReadOnly, Category = "Enemy|AI")
	EHumanoidEnemyAIState AIState = EHumanoidEnemyAIState::Patrol;

	// 转身动画触发标志，Patrol 层内 Turn 状态的进入条件
	UPROPERTY(BlueprintReadOnly, Category = "Enemy|Turn")
	bool bIsTurning = false;

	// 有符号转身角度：负=左，正=右，±180=回头
	UPROPERTY(BlueprintReadOnly, Category = "Enemy|Turn")
	float TurnAngle = 0.f;

	// 转身动画索引：0=L45  1=L90  2=L135  3=TurnAround  4=R45  5=R90  6=R135
	UPROPERTY(BlueprintReadOnly, Category = "Enemy|Turn")
	int32 TurnAnimIndex = 0;

	// 死亡标志
	UPROPERTY(BlueprintReadOnly, Category = "Enemy|AI")
	bool bIsDead = false;

	// 路点等待时间足够长时触发扫视动画
	UPROPERTY(BlueprintReadOnly, Category = "Enemy|Patrol")
	bool bIsPatrolScanning = false;

	// 当前随机选中的扫视动画索引，供 Blend Poses by Int 使用（从 1 开始，避免 A-Pose）
	UPROPERTY(BlueprintReadOnly, Category = "Enemy|Patrol")
	int32 PatrolScanAnimIndex = 1;

	// 扫视动画总数，在蓝图 CDO 里设置为与 Blend Poses by Int 的输入数量一致
	UPROPERTY(EditDefaultsOnly, Category = "Enemy|Patrol")
	int32 PatrolScanAnimCount = 1;

	// 停步时虚拟减速率（cm/s²）：Speed 从 LastWalkSpeed 线性降到 0，驱动 blend space 平滑过渡
	// PathFollowing 会强制清零 CMC 速度，虚拟减速绕过此限制，让动画看起来是真实减速
	UPROPERTY(EditDefaultsOnly, Category = "Enemy|Locomotion")
	float StopDecelerationRate = 300.f;

	// 武器 StaticMesh 上左手握把 socket 的名字
	UPROPERTY(EditDefaultsOnly, Category = "IK|LeftHand")
	FName WeaponGripLeftSocket = "grip_l";

	// 左手 IK 目标位置（World Space），每帧从武器 grip_l socket 计算，传给 ABP Two-Bone IK 节点
	UPROPERTY(BlueprintReadOnly, Category = "IK|LeftHand")
	FVector LeftHandIKTarget;

	// 武器是否有效（StaticMesh 已赋值），控制 Two-Bone IK Alpha
	UPROPERTY(BlueprintReadOnly, Category = "IK|LeftHand")
	bool bHasWeapon = false;

	// --- BBBAimIK 瞄准 ---
	// AimIK 目标（组件空间），BBBAimIK 节点 AimTarget 引脚
	UPROPERTY(BlueprintReadOnly, Category = "IK|AimIK")
	FVector AimTargetComponentSpace = FVector::ZeroVector;

	// 枪口相对 hand_r 的局部变换，首帧武器有效时初始化一次
	UPROPERTY(BlueprintReadOnly, Category = "IK|AimIK")
	FTransform AimSourceLocalTransform;

	// hand_r 骨骼上的朝向辅助插槽名（+X 轴须朝向枪管方向）
	UPROPERTY(EditDefaultsOnly, Category = "IK|AimIK")
	FName AimAxisSocketName = "AimSocket";

	// 从 AimAxisSocketName 插槽计算的瞄准轴（hand_r 局部空间），连 BBBAimIK Aim Axis 引脚
	// 首帧与 AimSourceLocalTransform 同时初始化，之后固定不变
	UPROPERTY(BlueprintReadOnly, Category = "IK|AimIK")
	FVector AimAxis = FVector(1.f, 0.f, 0.f);

	// 是否有有效的瞄准目标，控制 BBBAimIK Alpha 的开关
	UPROPERTY(BlueprintReadOnly, Category = "IK|AimIK")
	bool bHasValidAimTarget = false;

	// 战斗状态标志
	UPROPERTY(BlueprintReadOnly, Category = "IK|AimIK")
	bool bIsAiming = false;

	// FInterpTo 平滑的 Alpha 值，直接连接 BBBAimIK 节点的 Alpha 引脚（float）
	UPROPERTY(BlueprintReadOnly, Category = "IK|AimIK")
	float AimAlpha = 0.f;

	// AimAlpha 插值速度（进入/退出瞄准状态的速度）
	UPROPERTY(EditDefaultsOnly, Category = "IK|AimIK")
	float AimAlphaInterpSpeed = 8.f;

private:
	bool  bPrevPatrolScanning    = false;
	bool  bWasStoppingLastFrame  = false;
	float LastWalkSpeed          = 0.f;
	float VirtualSpeed           = 0.f;
	bool  bIsVirtualDecelerating = false;
	bool  bAimSourceInitialized  = false;
	bool  bWasAimMoving          = false;	// Aim 状态下上一帧是否在移动
	bool  bIsAimStopping         = false;	// Aim 状态下检测到速度骤降

	UPROPERTY()
	TObjectPtr<AHumanoidEnemy> HumanoidOwner;
};
