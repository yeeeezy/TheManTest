#include "Characters/CharacterBase/TheManAttributeSetBase.h"
#include "GameplayEffectExtension.h"
#include "Characters/CharacterBase/FPSCharacterBase/FPSCharacterBase.h"
#include "Enemy/EnemyBase.h"
#include "Engine/Engine.h"

UTheManAttributeSetBase::UTheManAttributeSetBase()
{
	InitHealth(100.0f);
	InitMaxHealth(100.0f);
}

void UTheManAttributeSetBase::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	if (Data.EvaluatedData.Attribute == GetHealthAttribute())
	{
		SetHealth(FMath::Clamp(GetHealth(), 0.f, GetMaxHealth()));

		UAbilitySystemComponent* OwningASC = GetOwningAbilitySystemComponent();
		AActor* Avatar = OwningASC ? OwningASC->GetAvatarActor() : nullptr;

		// 本次血量变化量（伤害为负，取负得扣血量）
		const float DamageTaken = -Data.EvaluatedData.Magnitude;

		// 调试：敌人血量变化时屏幕输出剩余血量
		if (AEnemyBase* DamagedEnemy = Cast<AEnemyBase>(Avatar))
		{
			if (GEngine)
			{
				GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Orange,
					FString::Printf(TEXT("[%s] Health: %.0f / %.0f"),
						*DamagedEnemy->GetName(), GetHealth(), GetMaxHealth()));
			}
		}
		// 调试：玩家角色扣血时屏幕输出扣血量 + 剩余血量
		else if (AFPSCharacterBase* DamagedCharacter = Cast<AFPSCharacterBase>(Avatar))
		{
			if (GEngine && DamageTaken > 0.f)
			{
				GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red,
					FString::Printf(TEXT("[%s] Damage: -%.0f | Health: %.0f / %.0f"),
						*DamagedCharacter->GetName(), DamageTaken, GetHealth(), GetMaxHealth()));
			}
		}

		if (GetHealth() <= 0.f)
		{
			if (AFPSCharacterBase* Character = Cast<AFPSCharacterBase>(Avatar))
			{
				Character->OnDeath();
			}
			else if (AEnemyBase* Enemy = Cast<AEnemyBase>(Avatar))
			{
				Enemy->OnDeath();
			}
		}
	}
}
