#pragma once

#include "CoreMinimal.h"
#include "Characters/FPSCharacterBase/FPSCharacterBase.h"
#include "FPSInfiltrator.generated.h"

class UGameplayAbility;

/**
 * AFPSInfiltrator
 * 潜行者 FPS 版。
 * E 键由基类统一发送 Input.Character.Interact，Infiltrator 在 PossessedBy 时
 * 授予 GA_InfiltratorScan，后者监听该 Tag 执行扫描逻辑。
 */
UCLASS()
class THEMANTEST_API AFPSInfiltrator : public AFPSCharacterBase
{
	GENERATED_BODY()

public:
	AFPSInfiltrator();

protected:
	virtual void PossessedBy(AController* NewController) override;

	// 角色专属默认技能，在蓝图 Details 中配置（不依赖任何武器）
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS|Abilities")
	TArray<TSubclassOf<UGameplayAbility>> DefaultAbilityClasses;
};
