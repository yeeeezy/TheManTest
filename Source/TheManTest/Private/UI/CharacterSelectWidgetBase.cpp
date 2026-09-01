#include "UI/CharacterSelectWidgetBase.h"
#include "Core/TheManGameInstance.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Engine/DataTable.h"

void UCharacterSelectWidgetBase::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	// 绑定点击事件（AddUnique 防热重载/重复初始化重复绑定）
	if (Button_Character1)
	{
		Button_Character1->OnClicked.AddUniqueDynamic(this, &UCharacterSelectWidgetBase::OnButton1Clicked);
	}
	if (Button_Character2)
	{
		Button_Character2->OnClicked.AddUniqueDynamic(this, &UCharacterSelectWidgetBase::OnButton2Clicked);
	}
	if (Button_Character3)
	{
		Button_Character3->OnClicked.AddUniqueDynamic(this, &UCharacterSelectWidgetBase::OnButton3Clicked);
	}

	// 自动填充按钮名字（放了 Text 控件才生效）
	ApplyButtonLabel(Text_Character1, Character1ID);
	ApplyButtonLabel(Text_Character2, Character2ID);
	ApplyButtonLabel(Text_Character3, Character3ID);

	// 游戏结束状态：游戏结束时禁用选角色按钮 + 显示"游戏结束"
	bool bGameOver = false;
	if (const UTheManGameInstance* GI = GetGameInstance<UTheManGameInstance>())
	{
		bGameOver = GI->IsGameOver();
	}
	ApplyGameOverState(bGameOver);
}

void UCharacterSelectWidgetBase::OnButton1Clicked() { SelectCharacter(Character1ID); }
void UCharacterSelectWidgetBase::OnButton2Clicked() { SelectCharacter(Character2ID); }
void UCharacterSelectWidgetBase::OnButton3Clicked() { SelectCharacter(Character3ID); }

void UCharacterSelectWidgetBase::SelectCharacter(FName CharacterID)
{
	if (UTheManGameInstance* GI = GetGameInstance<UTheManGameInstance>())
	{
		GI->SelectCharacterAndStart(CharacterID);
	}
}

bool UCharacterSelectWidgetBase::GetCharacterInfo(FName CharacterID, FCharacterType& OutInfo) const
{
	if (CharacterRosterTable && !CharacterID.IsNone())
	{
		if (const FCharacterType* Row = CharacterRosterTable->FindRow<FCharacterType>(CharacterID, TEXT("GetCharacterInfo")))
		{
			OutInfo = *Row;
			return true;
		}
	}
	return false;
}

void UCharacterSelectWidgetBase::ApplyButtonLabel(UTextBlock* Label, FName CharacterID) const
{
	if (!Label) { return; }

	FCharacterType Info;
	if (GetCharacterInfo(CharacterID, Info))
	{
		Label->SetText(Info.CharacterDisplayName);
	}
}

void UCharacterSelectWidgetBase::ApplyGameOverState(bool bGameOver)
{
	// 游戏结束 → 禁用三个选角色按钮（无法再开新局）
	if (Button_Character1) { Button_Character1->SetIsEnabled(!bGameOver); }
	if (Button_Character2) { Button_Character2->SetIsEnabled(!bGameOver); }
	if (Button_Character3) { Button_Character3->SetIsEnabled(!bGameOver); }

	// 游戏结束 → 显示"游戏结束"文本（放了 Text_GameOver 控件才生效）
	if (Text_GameOver)
	{
		if (bGameOver)
		{
			const FText Msg = GameOverText.IsEmpty() ? FText::FromString(TEXT("GAME OVER")) : GameOverText;
			Text_GameOver->SetText(Msg);
			Text_GameOver->SetVisibility(ESlateVisibility::Visible);
		}
		else
		{
			Text_GameOver->SetVisibility(ESlateVisibility::Collapsed);
		}
	}

	// 蓝图自定义表现（背景变暗、重开按钮等）
	OnGameOverState(bGameOver);
}
