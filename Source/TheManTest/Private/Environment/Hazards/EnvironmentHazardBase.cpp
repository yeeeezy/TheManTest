#include "Environment/Hazards/EnvironmentHazardBase.h"
#include "Environment/Hazards/HazardSuppressorInterface.h"
#include "Components/SphereComponent.h"
#include "NiagaraComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Engine/EngineTypes.h"

AEnvironmentHazardBase::AEnvironmentHazardBase()
{
	// HazardZone：检测 IHazardSuppressor（RepairGun 泡泡等）进入
	// ObjectType = WorldDynamic，BulletBase 的 CollisionSphere 响应 WorldDynamic = Overlap
	// 双方均开启 GenerateOverlapEvents，满足 UE5 重叠触发条件
	HazardZone = CreateDefaultSubobject<USphereComponent>(TEXT("HazardZone"));
	RootComponent = HazardZone;
	HazardZone->SetSphereRadius(150.f);
	HazardZone->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	HazardZone->SetGenerateOverlapEvents(true);
	HazardZone->SetCollisionObjectType(ECC_WorldDynamic);
	HazardZone->SetCollisionResponseToAllChannels(ECR_Ignore);
	// ECC_GameTraceChannel1 = RepairGunBullet（项目设置中已定义）
	HazardZone->SetCollisionResponseToChannel(ECC_GameTraceChannel1, ECR_Overlap);
	HazardZone->OnComponentBeginOverlap.AddDynamic(this, &AEnvironmentHazardBase::OnHazardZoneOverlapBegin);

	HazardEffect = CreateDefaultSubobject<UNiagaraComponent>(TEXT("HazardEffect"));
	HazardEffect->SetupAttachment(RootComponent);
	HazardEffect->SetAutoActivate(true);
}

void AEnvironmentHazardBase::BeginPlay()
{
	Super::BeginPlay();
	StartDamageLoop();
}

void AEnvironmentHazardBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// 解绑所有压制者回调，避免野指针
	for (const TWeakObjectPtr<AActor>& Suppressor : ActiveSuppressors)
	{
		if (AActor* Actor = Suppressor.Get())
		{
			Actor->OnDestroyed.RemoveDynamic(this, &AEnvironmentHazardBase::OnSuppressorDestroyed);
		}
	}
	ActiveSuppressors.Empty();
	StopDamageLoop();

	Super::EndPlay(EndPlayReason);
}

void AEnvironmentHazardBase::OnHazardZoneOverlapBegin(UPrimitiveComponent* OverlappedComp,
	AActor* OtherActor, UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherActor || !OtherActor->Implements<UHazardSuppressor>()) { return; }
	Suppress(OtherActor);
}

void AEnvironmentHazardBase::OnSuppressorDestroyed(AActor* DestroyedActor)
{
	Resume(DestroyedActor);
}

void AEnvironmentHazardBase::Suppress(AActor* Suppressor)
{
	// 防止同一压制者重复注册
	for (const TWeakObjectPtr<AActor>& Existing : ActiveSuppressors)
	{
		if (Existing.Get() == Suppressor) { return; }
	}

	ActiveSuppressors.Add(Suppressor);
	Suppressor->OnDestroyed.AddDynamic(this, &AEnvironmentHazardBase::OnSuppressorDestroyed);

	if (ActiveSuppressors.Num() == 1)
	{
		// 第一个压制者到达：关闭特效和伤害
		bIsActive = false;
		HazardEffect->Deactivate();
		StopDamageLoop();
	}
}

void AEnvironmentHazardBase::Resume(AActor* Suppressor)
{
	ActiveSuppressors.RemoveAll([Suppressor](const TWeakObjectPtr<AActor>& Ptr)
	{
		return !Ptr.IsValid() || Ptr.Get() == Suppressor;
	});

	if (ActiveSuppressors.IsEmpty())
	{
		// 所有压制者已离开：重新激活
		bIsActive = true;
		HazardEffect->Activate(true);
		StartDamageLoop();
	}
}

void AEnvironmentHazardBase::StartDamageLoop()
{
	if (!GetWorldTimerManager().IsTimerActive(DamageTimerHandle))
	{
		GetWorldTimerManager().SetTimer(
			DamageTimerHandle, this,
			&AEnvironmentHazardBase::DamageTick,
			DamageInterval, true);
	}
}

void AEnvironmentHazardBase::StopDamageLoop()
{
	GetWorldTimerManager().ClearTimer(DamageTimerHandle);
}

void AEnvironmentHazardBase::DamageTick()
{
	// SphereOverlapActors 返回 TArray<AActor*>，无需 FOverlapResult
	TArray<AActor*> IgnoredActors = { this };
	TArray<AActor*> FoundActors;

	UKismetSystemLibrary::SphereOverlapActors(
		this,
		GetActorLocation(),
		HazardZone->GetScaledSphereRadius(),
		TArray<TEnumAsByte<EObjectTypeQuery>>{ UEngineTypes::ConvertToObjectType(ECC_Pawn) },
		APawn::StaticClass(),
		IgnoredActors,
		FoundActors);

	for (AActor* Target : FoundActors)
	{
		IEnvironmentHazard::Execute_InflictDamage(this, Target);
	}
}

void AEnvironmentHazardBase::InflictDamage_Implementation(AActor* Target)
{
	// 基类留空；由 AFlameHazard、APoisonGasHazard 等具体子类实现
}

bool AEnvironmentHazardBase::IsHazardActive_Implementation()
{
	return bIsActive;
}
