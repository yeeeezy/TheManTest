#include "Characters/Infiltrator/FPSInfiltrator.h"
#include "AbilitySystemComponent.h"
#include "Abilities/GameplayAbility.h"

AFPSInfiltrator::AFPSInfiltrator()
{
}

void AFPSInfiltrator::PossessedBy(AController* NewController)
{
	// 基类负责：ASC 初始化、属性 GE 应用、武器技能补授
	Super::PossessedBy(NewController);

	// 自授角色专属默认技能（与武器无关）
	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	if (!ASC) { return; }

	for (const TSubclassOf<UGameplayAbility>& AbilityClass : DefaultAbilityClasses)
	{
		if (AbilityClass)
		{
			ASC->GiveAbility(FGameplayAbilitySpec(AbilityClass, 1));
		}
	}
}
