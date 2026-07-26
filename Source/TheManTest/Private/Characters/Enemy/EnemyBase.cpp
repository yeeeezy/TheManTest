#include "Characters/Enemy/EnemyBase.h"
#include "AbilitySystemComponent.h"
#include "Characters/Enemy/EnemyAttributeSetBase.h"
#include "Core/TheManGameStateBase.h"
#include "GameplayEffect.h"
#include "Abilities/GameplayAbility.h"
#include "Engine/Engine.h"

AEnemyBase::AEnemyBase()
{
	PrimaryActorTick.bCanEverTick = false;

	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AttributeSet = CreateDefaultSubobject<UEnemyAttributeSetBase>(TEXT("AttributeSet"));
}

UAbilitySystemComponent* AEnemyBase::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void AEnemyBase::BeginPlay()
{
	Super::BeginPlay();

	// ASC 初始化：敌人自身同时作为 OwnerActor 和 AvatarActor
	AbilitySystemComponent->InitAbilityActorInfo(this, this);

	// 应用初始血量 GE
	if (InitGEClass)
	{
		FGameplayEffectContextHandle Context = AbilitySystemComponent->MakeEffectContext();
		FGameplayEffectSpecHandle Spec = AbilitySystemComponent->MakeOutgoingSpec(InitGEClass, 1.f, Context);
		if (Spec.IsValid())
		{
			Spec.Data->SetSetByCallerMagnitude(
				FGameplayTag::RequestGameplayTag(FName("Data.Attribute.MaxHealth")), InitialMaxHealth);
			Spec.Data->SetSetByCallerMagnitude(
				FGameplayTag::RequestGameplayTag(FName("Data.Attribute.Health")), InitialHealth);
			AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
		}
	}

	// 授予通用技能 + 所有阶段技能集（近/中/远全部）。敌人自身即 ASC Owner。
	// 全部授予后，UseRandomSkill 按当前阶段+距离档随机激活其中之一。
	GrantAbilities(DefaultAbilities);
	for (const FEnemyPhaseSkillSet& Set : PhaseSkillSets)
	{
		GrantAbilities(Set.NearAbilities);
		GrantAbilities(Set.MidAbilities);
		GrantAbilities(Set.FarAbilities);
	}

	// 强度 + 伤害倍率初始化：回补本回合已发生的增强波数，使中途生成的敌人与开局即在场的敌人一致
	if (ATheManGameStateBase* GS = GetWorld()->GetGameState<ATheManGameStateBase>())
	{
		const int32 ElapsedWaves = GS->GetElapsedStrengthWaves();

		// 逐回合 +1：把已过的回合数（RoundNumber-1）折算成"已增强的波数"，
		// 让每个新回合的初始攻击力加成都比上一回合高一波——无论上回合是被打死还是倒计时归零。
		// 第 1 回合 RoundNumber=1 → 0 波基线（×1.0）；第 2 回合起每回合初始 +1 波。
		const int32 RoundCarriedWaves = FMath::Max(GS->GetRoundNumber() - 1, 0);
		const int32 TotalBonusWaves = RoundCarriedWaves + ElapsedWaves;

		CurrentStrength = BaseStrength + GS->GetRoundNumber() + ElapsedWaves;
		CurrentDamageMultiplier = FMath::Min(
			1.f + StrengthDamageBonusPerWave * TotalBonusWaves, MaxDamageMultiplier);

		GS->OnMidRoundStrengthIncrease.AddDynamic(this, &AEnemyBase::HandleMidRoundStrengthIncrease);
		GS->OnCombatPhaseChanged.AddDynamic(this, &AEnemyBase::HandleCombatPhaseChanged);
	}
	else
	{
		CurrentStrength = BaseStrength;
	}
}

void AEnemyBase::GrantAbilities(const TArray<TSubclassOf<UGameplayAbility>>& Abilities)
{
	if (!AbilitySystemComponent) { return; }
	for (const TSubclassOf<UGameplayAbility>& AbilityClass : Abilities)
	{
		if (AbilityClass)
		{
			AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(AbilityClass, 1));
		}
	}
}

bool AEnemyBase::UseRandomSkill(AActor* Target, EEnemySkillRange Range)
{
	if (!Target || !AbilitySystemComponent) { return false; }

	const int32 PhaseIndex = CurrentPhase - 1;
	if (!PhaseSkillSets.IsValidIndex(PhaseIndex)) { return false; }

	const FEnemyPhaseSkillSet& Set = PhaseSkillSets[PhaseIndex];
	const TArray<TSubclassOf<UGameplayAbility>>* Pool = nullptr;
	switch (Range)
	{
	case EEnemySkillRange::Near: Pool = &Set.NearAbilities; break;
	case EEnemySkillRange::Mid:  Pool = &Set.MidAbilities;  break;
	case EEnemySkillRange::Far:  Pool = &Set.FarAbilities;  break;
	}
	if (!Pool || Pool->Num() == 0) { return false; }

	// 随机挑一个技能放出
	const TSubclassOf<UGameplayAbility> Chosen = (*Pool)[FMath::RandRange(0, Pool->Num() - 1)];
	if (!Chosen) { return false; }

	AimAtTarget(Target);
	return AbilitySystemComponent->TryActivateAbilityByClass(Chosen);
}

void AEnemyBase::SetCombatPhase(int32 NewPhase)
{
	// 阶段只切技能集，不影响伤害倍率（伤害走 StrengthIncreaseInterval 每波递增）
	CurrentPhase = FMath::Max(1, NewPhase);
}

void AEnemyBase::OnDeath()
{
	if (bIsDead) return;
	bIsDead = true;

	// 敌人死亡：销毁 Actor（具体表现蓝图覆写）
	Destroy();
}

void AEnemyBase::HandleMidRoundStrengthIncrease()
{
	CurrentStrength++;

	// 每波累加强度增强系数：倍率 = 1 + StrengthDamageBonusPerWave × 波数，钳到 MaxDamageMultiplier 上限
	CurrentDamageMultiplier = FMath::Min(
		CurrentDamageMultiplier + StrengthDamageBonusPerWave, MaxDamageMultiplier);

	// 调试：确认本波伤害增强
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Yellow,
			FString::Printf(TEXT("[%s] 伤害增强一波 → 倍率 ×%.2f（强度 %d）"),
				*GetName(), CurrentDamageMultiplier, CurrentStrength));
	}
}

void AEnemyBase::HandleCombatPhaseChanged(int32 NewPhase)
{
	SetCombatPhase(NewPhase);

	// 调试：确认敌人收到阶段切换
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow,
			FString::Printf(TEXT("[%s] 切换到战斗阶段 %d"), *GetName(), CurrentPhase));
	}
}
