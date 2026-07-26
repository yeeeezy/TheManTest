#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "TheManGameInstance.generated.h"

/**
 * UTheManGameInstance
 * 跨关卡持久容器（GameInstance 是唯一能在 OpenLevel 之间存活的对象）。
 * 携带：玩家选定的角色 ID + 当前回合数。负责"死亡→选角色→测试地图"的关卡切换流程。
 *
 * 流程：
 *   启动 → 选角色大厅 LobbyMap → 按钮调 SelectCharacterAndStart(ID) → 测试地图
 *   测试地图 GameState.BeginPlay 读 CarriedRoundNumber 接着 +1；GameMode 按 SelectedCharacterID 生成角色
 *   死亡（被打死 / 倒计时归零）→ HandlePlayerDeath(当前回合数) → 回 LobbyMap
 */
UCLASS()
class THEMANTEST_API UTheManGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	// 选角色界面按钮调用：记录所选角色，进入测试地图
	UFUNCTION(BlueprintCallable, Category = "Run")
	void SelectCharacterAndStart(FName CharacterID);

	// 死亡时调用：记录当前回合数（下回合在此基础上 +1），返回选角色地图
	UFUNCTION(BlueprintCallable, Category = "Run")
	void HandlePlayerDeath(int32 CurrentRoundNumber);

	// 游戏结束（回合时长减到下限以下）：置游戏结束标记并回大厅；大厅 UI 据此禁用选角色按钮 + 显示"游戏结束"
	UFUNCTION(BlueprintCallable, Category = "Run")
	void HandleGameOver();

	UFUNCTION(BlueprintPure, Category = "Run")
	FORCEINLINE FName GetSelectedCharacterID() const { return SelectedCharacterID; }

	UFUNCTION(BlueprintPure, Category = "Run")
	FORCEINLINE int32 GetCarriedRoundNumber() const { return CarriedRoundNumber; }

	// 是否已游戏结束（大厅 UI 读取以切换"游戏结束"显示）
	UFUNCTION(BlueprintPure, Category = "Run")
	FORCEINLINE bool IsGameOver() const { return bGameOver; }

protected:
	// 玩家选定的角色 ID（对应 DT_CharacterRoster 行名）
	UPROPERTY(BlueprintReadOnly, Category = "Run")
	FName SelectedCharacterID = NAME_None;

	// 携带的回合数：死亡写入，测试地图 GameState 读取后接着 +1（实现"敌人初始强度逐回合 +1"）
	UPROPERTY(BlueprintReadOnly, Category = "Run")
	int32 CarriedRoundNumber = 0;

	// 选角色大厅地图名（编辑器配置）
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Run|Maps")
	FName LobbyMapName = TEXT("LobbyMap");

	// 测试/游戏地图名（编辑器配置）
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Run|Maps")
	FName TestMapName = TEXT("TestMap");

	// 游戏结束标记：HandleGameOver 置位，回大厅后 UI 读取。GameInstance 随进程存活，退出 PIE 自动重置
	UPROPERTY(BlueprintReadOnly, Category = "Run")
	bool bGameOver = false;

private:
	// 防止同帧多次触发死亡切换（如倒计时归零与血量归零同时发生）
	bool bPendingTransition = false;
};
