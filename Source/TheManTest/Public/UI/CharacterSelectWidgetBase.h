#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Core/TheManCharacterTypes.h"
#include "CharacterSelectWidgetBase.generated.h"

class UDataTable;
class UButton;
class UTextBlock;

/**
 * UCharacterSelectWidgetBase
 * 选角色 UI（WBP_CharacterSelect）的 C++ 基类。逻辑全在基类，蓝图只负责摆控件 + 美术。
 *
 * 工作方式：
 *  - 用 BindWidget 绑定三个按钮（UMG 里控件必须叫 Button_Character1/2/3）。
 *  - NativeOnInitialized 自动给三个按钮挂 OnClicked → 各自调 SelectCharacter(对应 ID)。
 *  - 若放了同名 Text 控件（可选），自动从花名册填角色显示名。
 *
 * 你在 WBP_CharacterSelect 里只需：
 *  1) 父类设为本类；填 CharacterRosterTable=DT_CharacterRoster；
 *  2) 放三个 Button，命名 Button_Character1 / Button_Character2 / Button_Character3；
 *  3) （可选）各放一个 Text，命名 Text_Character1 / 2 / 3，自动显示角色名；
 *  4) （可选）三个 ID 默认已是 Infiltrator/MaintenanceWorker/TheExecutive，需要可在默认值里改。
 */
UCLASS(Abstract)
class THEMANTEST_API UCharacterSelectWidgetBase : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeOnInitialized() override;

	// 选定角色 → 写入 GameInstance → 进测试地图。按钮点击内部调用，也可蓝图自定义按钮调用。
	UFUNCTION(BlueprintCallable, Category = "CharacterSelect")
	void SelectCharacter(FName CharacterID);

	// 据 ID 取花名册行（显示名/头像/描述/角色类），供蓝图自定义显示（如头像）。返回是否找到。
	UFUNCTION(BlueprintCallable, Category = "CharacterSelect")
	bool GetCharacterInfo(FName CharacterID, FCharacterType& OutInfo) const;

protected:
	// ── 三个角色按钮（UMG 必须有同名控件，否则编译报缺失绑定）──
	UPROPERTY(meta = (BindWidget))
	UButton* Button_Character1 = nullptr;

	UPROPERTY(meta = (BindWidget))
	UButton* Button_Character2 = nullptr;

	UPROPERTY(meta = (BindWidget))
	UButton* Button_Character3 = nullptr;

	// ── 可选：游戏结束提示文本（放了就在游戏结束时自动显示 GameOverText，否则隐藏）──
	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* Text_GameOver = nullptr;

	// 游戏结束时显示的文字，留空则用默认"游戏结束"
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CharacterSelect")
	FText GameOverText;

	// 游戏结束状态应用后回调，蓝图可做额外表现（背景变暗、动画、重开按钮等）。bGameOver=true 即结束。
	UFUNCTION(BlueprintImplementableEvent, Category = "CharacterSelect")
	void OnGameOverState(bool bGameOver);

	// ── 可选：按钮上的名字文本（放了就自动填花名册 DisplayName）──
	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* Text_Character1 = nullptr;

	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* Text_Character2 = nullptr;

	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* Text_Character3 = nullptr;

	// 各按钮对应的角色 ID（= DT_CharacterRoster 行名），默认即三个角色，可在默认值改
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CharacterSelect")
	FName Character1ID = TEXT("Infiltrator");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CharacterSelect")
	FName Character2ID = TEXT("MaintenanceWorker");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CharacterSelect")
	FName Character3ID = TEXT("TheExecutive");

	// 角色花名册（DT_CharacterRoster），供读取显示信息
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CharacterSelect")
	UDataTable* CharacterRosterTable = nullptr;

private:
	// 三个按钮各自的点击回调（OnClicked 无参，故一按钮一函数）
	UFUNCTION() void OnButton1Clicked();
	UFUNCTION() void OnButton2Clicked();
	UFUNCTION() void OnButton3Clicked();

	// 若 TextBlock 有效，从花名册把该 ID 的 DisplayName 写进去
	void ApplyButtonLabel(UTextBlock* Label, FName CharacterID) const;

	// 据游戏结束状态：禁用/启用三个选角色按钮 + 显示/隐藏游戏结束文本 + 调蓝图 OnGameOverState
	void ApplyGameOverState(bool bGameOver);
};
