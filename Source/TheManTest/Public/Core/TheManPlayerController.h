#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "TheManPlayerController.generated.h"

class UInputMappingContext;
class UInputAction;

/**
 * ATheManPlayerController
 * 职责：按键注册表。持有所有 IA_ 资产并添加 IMC，不路由任何角色逻辑。
 * 角色在 SetupPlayerInputComponent 中通过 GetController() 取 IA 资产并自行绑定。
 * 例外：TestSwitchCharacterAction 属于 Controller 级元操作，保留在此。
 */
UCLASS()
class THEMANTEST_API ATheManPlayerController : public APlayerController
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

	int32 CurrentRosterIndex = 0;

	// ── 输入映射上下文 ──
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> DefaultMappingContext;

	// ── 通用角色输入（所有 FPS 角色共用，Character 自行绑定）──
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|Movement")
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|Movement")
	TObjectPtr<UInputAction> LookAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|Movement")
	TObjectPtr<UInputAction> JumpAction;

	// 冲刺（按住提速、松开回走速）
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|Movement")
	TObjectPtr<UInputAction> SprintAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|Equipment")
	TObjectPtr<UInputAction> SwitchEquipmentAction;

	// ── 武器输入（所有携带武器的角色共用）──
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|Weapon")
	TObjectPtr<UInputAction> PrimaryFireAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|Weapon")
	TObjectPtr<UInputAction> SecondaryFireAction;

	// ── 通用交互输入（E 键，各角色按需监听）──
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|Ability")
	TObjectPtr<UInputAction> InteractAction;

	// ── Controller 级元操作（保留在此）──
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|Meta")
	TObjectPtr<UInputAction> TestSwitchCharacterAction;

	// 调试：快进回合时间（默认每次 2.5 分钟），蓝图里填一个 IA 资产即生效
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|Debug")
	TObjectPtr<UInputAction> DebugSkipTimeAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Data")
	class UDataTable* CharacterRosterTable;

	void HandleTestSwitchCharacter();

	// 调试快进：调用 GameState->DebugSkipTime()
	void HandleDebugSkipTime();

public:
	void SwitchCharacter(FName TargetCharacterID);

	// ── Getters 供 Character::SetupPlayerInputComponent 调用 ──
	FORCEINLINE UInputAction* GetMoveAction()            const { return MoveAction; }
	FORCEINLINE UInputAction* GetLookAction()            const { return LookAction; }
	FORCEINLINE UInputAction* GetJumpAction()            const { return JumpAction; }
	FORCEINLINE UInputAction* GetSprintAction()          const { return SprintAction; }
	FORCEINLINE UInputAction* GetSwitchEquipmentAction() const { return SwitchEquipmentAction; }
	FORCEINLINE UInputAction* GetPrimaryFireAction()     const { return PrimaryFireAction; }
	FORCEINLINE UInputAction* GetSecondaryFireAction()   const { return SecondaryFireAction; }
	FORCEINLINE UInputAction* GetInteractAction()        const { return InteractAction; }
};
