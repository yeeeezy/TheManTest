#pragma once

#include "CoreMinimal.h"
#include "Characters/FPSCharacterBase/FPSCharacterBase.h"
#include "FPSInfiltrator.generated.h"

/**
 * AFPSInfiltrator
 * 潜行者 FPS 版。
 * E 键由基类统一发送 Input.Character.Interact；扫描技能和其他角色默认技能
 * 统一通过 AFPSCharacterBase::DefaultAbilityClasses 配置与授予。
 */
UCLASS()
class THEMANTEST_API AFPSInfiltrator : public AFPSCharacterBase
{
	GENERATED_BODY()

public:
	AFPSInfiltrator();
};
