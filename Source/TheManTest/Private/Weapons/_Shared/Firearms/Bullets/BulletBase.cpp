#include "Weapons/_Shared/Firearms/Bullets/BulletBase.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffect.h"
#include "Core/_Shared/GAS/TheManGameplayTags.h"
#include "Enemy/EnemyBase.h"

ABulletBase::ABulletBase()
{
	PrimaryActorTick.bCanEverTick = false;

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
	if (AEnemyBase* Enemy = Cast<AEnemyBase>(HitResult.GetActor()))
	{
		if (Enemy->ShouldProjectilePassThrough())
		{
			if (CollisionSphere) CollisionSphere->IgnoreActorWhenMoving(Enemy, true);
			return;
		}
		// 只把玩家/非敌方发射者视为威胁；避免敌人友军火力互相改写战斗目标。
		if (HitInstigator && !HitInstigator->IsA<AEnemyBase>())
		{
			Enemy->ReactToProjectileHit(HitInstigator);
		}
	}
	bHasProcessedHit = true;

	// 命中目标若带 ASC 则施加伤害 GE；打墙/地等无 ASC 目标跳过此步，但子弹仍会按下方逻辑销毁。
	if (HitEffectClass && SourceASC)
	{
		if (AActor* HitActor = HitResult.GetActor())
		{
			if (IAbilitySystemInterface* TargetInterface = Cast<IAbilitySystemInterface>(HitActor))
			{
				if (UAbilitySystemComponent* TargetASC = TargetInterface->GetAbilitySystemComponent())
				{
					FGameplayEffectContextHandle Context = SourceASC->MakeEffectContext();
					Context.AddInstigator(HitInstigator, nullptr);

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
