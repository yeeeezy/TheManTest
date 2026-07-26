#include "Environment/Flame/FlameHazard.h"

AFlameHazard::AFlameHazard()
{
	// 蓝图中配置 HazardEffect 资产和 HazardZone 半径
}

void AFlameHazard::InflictDamage_Implementation(AActor* Target)
{
	// TODO：应用 GE_FireDamage 到 Target 的 ASC
	// 示例（占位）：
	// IAbilitySystemInterface* TargetASI = Cast<IAbilitySystemInterface>(Target);
	// if (!TargetASI) { return; }
	// UAbilitySystemComponent* TargetASC = TargetASI->GetAbilitySystemComponent();
	// FGameplayEffectContextHandle Context = SourceASC->MakeEffectContext();
	// SourceASC->ApplyGameplayEffectToTarget(FireDamageEffectClass, TargetASC, 1.f, Context);
}
