#include "Weapons/_Shared/Firearms/Bullets/BulletBase.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffect.h"
#include "Core/_Shared/GAS/TheManGameplayTags.h"
#include "Enemy/EnemyBase.h"
#include "Components/SkeletalMeshComponent.h"
#include "UObject/ConstructorHelpers.h"

ABulletBase::ABulletBase()
{
	PrimaryActorTick.bCanEverTick = false;

	static ConstructorHelpers::FClassFinder<UGameplayEffect> DefaultDamageEffect(
		TEXT("/Game/Weapons/_Shared/GAS/Effects/GE_BulletDamage"));
	if (DefaultDamageEffect.Succeeded())
	{
		HitEffectClass = DefaultDamageEffect.Class;
	}

	CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionSphere"));
	// QueryOnly：仅做查询碰撞，不参与物理求解。否则高速抛射体与 enemy 胶囊体的物理碰撞
	// 会给角色施加冲量，把 enemy 击退/飘飞（BUG-034-001）。命中检测改由下面 ProjectileMovement
	// 的 sweep 阻挡触发 OnComponentHit，不依赖物理模拟。
	CollisionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionSphere->SetCollisionObjectType(ECC_GameTraceChannel1);          // RepairGunBullet
	CollisionSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	CollisionSphere->SetCollisionResponseToChannel(ECC_WorldStatic,       ECR_Block);
	CollisionSphere->SetCollisionResponseToChannel(ECC_Pawn,              ECR_Block);
	CollisionSphere->SetCollisionResponseToChannel(ECC_GameTraceChannel1, ECR_Block); // 子弹/泡泡互相碰撞
	// Overlap 检测：允许与危险区（WorldDynamic）的触发球产生 Overlap 事件
	// AEnvironmentHazardBase::HazardZone 使用 WorldDynamic 类型，响应本 Channel 为 Overlap
	CollisionSphere->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);
	CollisionSphere->SetGenerateOverlapEvents(true);
	CollisionSphere->OnComponentHit.AddDynamic(this, &ABulletBase::OnBulletHit);
	RootComponent = CollisionSphere;

	BulletMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BulletMesh"));
	BulletMesh->SetupAttachment(CollisionSphere);

	BulletMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	BulletMesh->CastShadow = false;

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->InitialSpeed             = 5000.f;
	ProjectileMovement->MaxSpeed                 = 5000.f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce            = false;
	ProjectileMovement->ProjectileGravityScale   = 0.f;
}

void ABulletBase::InitBullet(AActor* HitInstigator, UAbilitySystemComponent* SourceASC)
{
	CachedInstigator = HitInstigator;
	CachedSourceASC  = SourceASC;

	// 忽略发射者：子弹生成在枪口（紧贴角色胶囊体/手臂），否则一出膛就撞到自己，在枪口"爆炸"。
	// IgnoreActorWhenMoving 让子弹 sweep 移动时不与这些 Actor 产生碰撞。
	if (HitInstigator && CollisionSphere)
	{
		CollisionSphere->IgnoreActorWhenMoving(HitInstigator, true);

		// 一并忽略挂在发射者身上的装备（当前武器等）
		TArray<AActor*> AttachedActors;
		HitInstigator->GetAttachedActors(AttachedActors);
		for (AActor* Attached : AttachedActors)
		{
			if (Attached)
			{
				CollisionSphere->IgnoreActorWhenMoving(Attached, true);
			}
		}
	}
}

void ABulletBase::OnBulletHit(UPrimitiveComponent* HitComponent, AActor* OtherActor,
                               UPrimitiveComponent* OtherComp, FVector NormalImpulse,
                               const FHitResult& Hit)
{
	if (!bHasProcessedHit)
	{
		ProcessHit(Hit, CachedInstigator.Get(), CachedSourceASC.Get());
	}
}

void ABulletBase::ProcessHit_Implementation(
	const FHitResult& HitResult,
	AActor* HitInstigator,
	UAbilitySystemComponent* SourceASC)
{
	if (bHasProcessedHit) { return; }
	const FVector ShotDirection = GetVelocity().GetSafeNormal(UE_SMALL_NUMBER, GetActorForwardVector());
	FName ImpulseBone = NAME_None;
	FVector ImpulseLocalPoint = FVector::ZeroVector;
	TWeakObjectPtr<AEnemyBase> ImpulseTarget;
	if (AEnemyBase* Enemy = Cast<AEnemyBase>(HitResult.GetActor()))
	{
		if (Enemy->ShouldProjectilePassThrough())
		{
			if (CollisionSphere) CollisionSphere->IgnoreActorWhenMoving(Enemy, true);
			return;
		}
		// Resolve against actual bodies before the damage/alert callbacks can change the pose.
		if (USkeletalMeshComponent* Mesh = Enemy->GetMesh(); Mesh && Mesh->GetPhysicsAsset())
		{
			FHitResult BodyHit = HitResult;
			bool bFound = HitResult.GetComponent() == Mesh && !HitResult.BoneName.IsNone();
			if (!bFound)
			{
				const FVector PathPoint = HitResult.TraceStart.Equals(HitResult.TraceEnd)
					? GetActorLocation() : FVector(HitResult.Location);
				const float Span = FMath::Max(100.f, Mesh->Bounds.SphereRadius * 2.f);
				FCollisionQueryParams Query(SCENE_QUERY_STAT(ProjectileBodyImpulse), false);
				bFound = Mesh->LineTraceComponent(BodyHit, PathPoint-ShotDirection*Span, PathPoint+ShotDirection*Span, Query);
				if (!bFound)
				{
					FVector Point, Normal; float Distance = 0.f; FName Bone;
					bFound = Mesh->K2_GetClosestPointOnPhysicsAsset(HitResult.ImpactPoint, Point, Normal, Bone, Distance);
					if (bFound) { BodyHit.ImpactPoint = Point; BodyHit.BoneName = Bone; }
				}
			}
			if (bFound && !BodyHit.BoneName.IsNone())
			{
				ImpulseTarget = Enemy;
				ImpulseBone = BodyHit.BoneName;
				ImpulseLocalPoint = Mesh->GetSocketTransform(ImpulseBone).InverseTransformPosition(BodyHit.ImpactPoint);
			}
		}
		// 只把玩家/非敌方发射者视为威胁；避免敌人友军火力互相改写战斗目标。
		if (HitInstigator && !HitInstigator->IsA<AEnemyBase>())
		{
			Enemy->ReactToProjectileHit(HitInstigator);
		}
	}
	bHasProcessedHit = true;

	if (SourceASC && ImpactCueTag.IsValid())
	{
		FGameplayCueParameters CueParameters;
		CueParameters.Location = HitResult.ImpactPoint;
		CueParameters.Normal = HitResult.ImpactNormal;
		CueParameters.Instigator = HitInstigator;
		CueParameters.EffectCauser = this;
		CueParameters.PhysicalMaterial = HitResult.PhysMaterial.Get();
		FGameplayEffectContextHandle CueContext = SourceASC->MakeEffectContext();
		CueContext.AddHitResult(HitResult, true);
		CueParameters.EffectContext = CueContext;
		// Projectile impacts can occur after the LocalOnly firing ability has ended. At that point
		// ExecuteGameplayCue has neither authority nor a live prediction key, so its pending RPC
		// path silently drops the cue. This project is single-player and impact feedback is local,
		// therefore invoke the Executed event immediately on the source ASC.
		SourceASC->InvokeGameplayCueEvent(
			ImpactCueTag, EGameplayCueEvent::Executed, CueParameters);
	}

	// 命中目标若带 ASC 则施加伤害 GE；打墙/地等无 ASC 目标跳过此步，但子弹仍会按下方逻辑销毁。
	const AEnemyBase* HitEnemy = Cast<AEnemyBase>(HitResult.GetActor());
	const bool bHitCorpse = HitEnemy && HitEnemy->IsDead();
	if (HitEffectClass && SourceASC && !bHitCorpse)
	{
		if (AActor* HitActor = HitResult.GetActor())
		{
			if (IAbilitySystemInterface* TargetInterface = Cast<IAbilitySystemInterface>(HitActor))
			{
				if (UAbilitySystemComponent* TargetASC = TargetInterface->GetAbilitySystemComponent())
				{
					FGameplayEffectContextHandle Context = SourceASC->MakeEffectContext();
					Context.AddInstigator(HitInstigator, nullptr);
					Context.AddHitResult(HitResult, true);

					FGameplayEffectSpecHandle Spec = SourceASC->MakeOutgoingSpec(HitEffectClass, 1.f, Context);
					if (Spec.IsValid())
					{
						// 把子弹的 Damage 以负值传入 GE 的 SetByCaller(Data.Damage)：
						// GE 的 Health 修改器为 Add 型，读取该值即对目标扣血。GE 不使用此 Tag 时本调用无副作用。
						Spec.Data->SetSetByCallerMagnitude(TAG_Data_Damage, -Damage);
						SourceASC->ApplyGameplayEffectSpecToTarget(*Spec.Data.Get(), TargetASC);
					}
				}
			}
		}
	}

	// Damage can enable ragdoll synchronously: the lethal shot and later corpse hits use this same path.
	if (AEnemyBase* Enemy = Cast<AEnemyBase>(HitResult.GetActor()); IsValid(Enemy) && !bHitCorpse && Enemy->IsDead())
	{
		if (USkeletalMeshComponent* Mesh = Enemy->GetMesh(); Mesh && Mesh->IsSimulatingPhysics())
		{
			const FVector LaunchVelocity = ShotDirection * FMath::Max(0.f, Enemy->ProjectileKillKnockbackSpeed)
				+ FVector::UpVector * FMath::Max(0.f, Enemy->ProjectileKillUpwardSpeed);
			Mesh->SetAllPhysicsLinearVelocity(LaunchVelocity, true);
		}
	}
	if (AEnemyBase* Enemy = ImpulseTarget.Get(); Enemy && Enemy->ProjectileHitImpulse > 0.f)
	{
		USkeletalMeshComponent* Mesh = Enemy->GetMesh();
		if (Mesh && Mesh->IsSimulatingPhysics(ImpulseBone))
		{
			const FVector Point = Mesh->GetSocketTransform(ImpulseBone).TransformPosition(ImpulseLocalPoint);
			Mesh->AddImpulseAtLocation(ShotDirection * Enemy->ProjectileHitImpulse, Point, ImpulseBone);
		}
	}

	// Zero-damage and corpse hits have no Health callback; still play flesh/decal feedback once.
	if (Damage == 0.f || bHitCorpse)
	{
		if (AEnemyBase* Enemy = Cast<AEnemyBase>(HitResult.GetActor()); IsValid(Enemy) && !Enemy->IsActorBeingDestroyed())
		{
			if (UAbilitySystemComponent* ContextASC = SourceASC ? SourceASC : Enemy->GetAbilitySystemComponent())
			{
				FGameplayEffectContextHandle Context = ContextASC->MakeEffectContext();
				Context.AddInstigator(HitInstigator, this);
				Context.AddHitResult(HitResult, true);
				Enemy->ExecuteHitReactionCue(Context, 0.f, true);
			}
		}
	}

	// 命中即停下并销毁（普通伤害弹）。自管生命周期的子类置 bDestroyOnHit=false 以保留自身逻辑。
	if (bDestroyOnHit)
	{
		if (ProjectileMovement)
		{
			ProjectileMovement->StopMovementImmediately();
		}
		Destroy();
	}
}
