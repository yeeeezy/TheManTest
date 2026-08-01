#include "Enemy/Humanoid/HumanoidEnemyAnimInstance.h"
#include "Enemy/Humanoid/HumanoidEnemy.h"
#include "Components/StaticMeshComponent.h"

void UHumanoidEnemyAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
	HumanoidOwner = Cast<AHumanoidEnemy>(TryGetPawnOwner());
}

void UHumanoidEnemyAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds); // 更新 Speed / Direction / bIsFalling / AimPitch

	if (!HumanoidOwner) return;

	AIState    = HumanoidOwner->GetAIState();
	bIsTurning = HumanoidOwner->IsPendingTurn();
	TurnAngle  = HumanoidOwner->GetPendingTurnAngle();
	bIsDead    = HumanoidOwner->IsDead();

	// 转身动画索引（仅转身期间更新，避免 bIsTurning=false 时索引跳变）
	if (bIsTurning)
	{
		const float Abs   = FMath::Abs(TurnAngle);
		const bool  bLeft = TurnAngle < 0.f;

		if      (Abs >= 157.5f) TurnAnimIndex = 3;
		else if (Abs >= 112.5f) TurnAnimIndex = bLeft ? 2 : 6;
		else if (Abs >=  67.5f) TurnAnimIndex = bLeft ? 1 : 5;
		else                    TurnAnimIndex = bLeft ? 0 : 4;
	}

	bIsPatrolScanning = HumanoidOwner->IsPatrolScanning();

	// Index 从 1 开始：Blend Poses by Int 以 0 作为 blend source，
	// Index=0 时会用 Reference Pose 混入，偶发 A-Pose
	if (bIsPatrolScanning && !bPrevPatrolScanning && PatrolScanAnimCount > 0)
	{
		if (PatrolScanAnimCount == 1)
		{
			PatrolScanAnimIndex = 1;
		}
		else
		{
			int32 NextIndex = PatrolScanAnimIndex;
			while (NextIndex == PatrolScanAnimIndex)
				NextIndex = FMath::RandRange(1, PatrolScanAnimCount);
			PatrolScanAnimIndex = NextIndex;
		}
	}
	bPrevPatrolScanning = bIsPatrolScanning;

	const bool  bCurrentStopping = HumanoidOwner->IsStoppingAtPoint();
	const float RealSpeed        = Speed; // Super 写入的真实 CMC 速度，虚拟减速覆盖前保存

	// Aim 状态：检测 BT MoveTo 完成后速度骤降，生成与巡逻停车等效的触发标志
	if (AIState == EHumanoidEnemyAIState::Aim)
	{
		if (RealSpeed > 10.f)                        { bWasAimMoving = true;  bIsAimStopping = false; }
		if (bWasAimMoving && RealSpeed < 10.f)       { bIsAimStopping = true; bWasAimMoving  = false; }
	}
	else
	{
		bWasAimMoving  = false;
		bIsAimStopping = false;
	}

	const bool bAnyStopping = bCurrentStopping || bIsAimStopping;

	// 正常行走期间缓存真实速度，供虚拟减速使用
	if (!bAnyStopping && RealSpeed > 10.f)
	{
		LastWalkSpeed = RealSpeed;
	}

	// 虚拟减速：PathFollowing 停车时会强制将 CMC 速度归零，
	// 用 StopDecelerationRate 线性衰减 Speed 让 blend space 看到平滑减速过程
	if (bAnyStopping && !bWasStoppingLastFrame)
	{
		VirtualSpeed           = LastWalkSpeed;
		bIsVirtualDecelerating = true;
	}
	if (bIsVirtualDecelerating)
	{
		VirtualSpeed = FMath::Max(VirtualSpeed - StopDecelerationRate * DeltaSeconds, 0.f);
		Speed        = VirtualSpeed;
		if (VirtualSpeed < 1.f)
		{
			bIsVirtualDecelerating = false;
		}
	}
	if (!bAnyStopping)
	{
		bIsVirtualDecelerating = false;
		VirtualSpeed           = 0.f;
	}

	bWasStoppingLastFrame = bAnyStopping;

	// --- 左手武器 IK Target ---
	// 每帧取武器 grip_l socket 的世界坐标，ABP Two-Bone IK 节点以 WorldSpace 接收
	bHasWeapon = false;
	if (UStaticMeshComponent* Weapon = HumanoidOwner->GetWeaponMesh())
	{
		const bool bWeaponMeshValid = (Weapon->GetStaticMesh() != nullptr);
		bHasWeapon = bWeaponMeshValid && Weapon->DoesSocketExist(WeaponGripLeftSocket);
		if (bHasWeapon)
		{
			const FVector GripWorld = Weapon->GetSocketLocation(WeaponGripLeftSocket);
			LeftHandIKTarget = HumanoidOwner->GetMesh()->GetComponentTransform().InverseTransformPosition(GripWorld);
		}

		// 首帧从武器 Muzzle socket 计算稳定绑定变换，之后固定不再更新。
		if (bWeaponMeshValid && !bAimSourceInitialized && Weapon->DoesSocketExist(FName("Muzzle")))
		{
			const FTransform HandWorldTr   = HumanoidOwner->GetMesh()->GetSocketTransform(FName("hand_r"), RTS_World);
			const FTransform MuzzleWorldTr = Weapon->GetSocketTransform(FName("Muzzle"), RTS_World);
			AimSourceLocalTransform = MuzzleWorldTr.GetRelativeTransform(HandWorldTr);

			if (HumanoidOwner->GetMesh()->DoesSocketExist(AimAxisSocketName))
			{
				const FTransform SocketWorldTr = HumanoidOwner->GetMesh()->GetSocketTransform(AimAxisSocketName, RTS_World);
				const FTransform SocketLocalTr = SocketWorldTr.GetRelativeTransform(HandWorldTr);
				AimAxis = SocketLocalTr.GetRotation().GetForwardVector();
			}
			bAimSourceInitialized = true;
		}
	}

	// --- BBBAimIK 瞄准 ---
	bIsAiming          = HumanoidOwner->bIsAiming;
	// bHasValidAimTarget 须等 AimSourceLocalTransform 初始化完再开启，否则 solver 用 Identity 偏移
	bHasValidAimTarget = bIsAiming && bAimSourceInitialized;
	if (bHasValidAimTarget)
	{
		const FTransform& CompTr = HumanoidOwner->GetMesh()->GetComponentTransform();
		AimTargetComponentSpace  = CompTr.InverseTransformPosition(HumanoidOwner->AimTargetWorld);
	}
	// float Alpha：FInterpTo 平滑过渡，直接连 BBBAimIK 节点 Alpha 引脚
	const float TargetAlpha = bHasValidAimTarget ? 1.f : 0.f;
	AimAlpha = FMath::FInterpTo(AimAlpha, TargetAlpha, DeltaSeconds, AimAlphaInterpSpeed);
}
