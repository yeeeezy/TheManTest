#include "Core/Editor/TheManLobbyAssetLibrary.h"

#if WITH_EDITOR
#include "WidgetBlueprint.h"
#include "Blueprint/WidgetTree.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/Button.h"
#include "Components/ButtonSlot.h"
#include "Components/TextBlock.h"
#include "Components/Border.h"
#include "Components/SizeBox.h"
#include "UI/Lobby/LobbyPresentationWidgetBase.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "Styling/CoreStyle.h"
#endif

bool UTheManLobbyAssetLibrary::InitializePresentationMenu(UBlueprint* Blueprint)
{
#if WITH_EDITOR
	auto* BP = Cast<UWidgetBlueprint>(Blueprint);
	if (!BP || !BP->ParentClass->IsChildOf(ULobbyPresentationWidgetBase::StaticClass())
		|| !BP->WidgetTree || BP->WidgetTree->RootWidget) return false;
	BP->Modify();
	auto* Tree = BP->WidgetTree.Get();
	auto* Canvas = Tree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("PresentationCanvas"));
	Canvas->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	Tree->RootWidget = Canvas;
	auto* Menu = Tree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("PresentationMenu"));
	Menu->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	auto* MenuSlot = Canvas->AddChildToCanvas(Menu);
	MenuSlot->SetAnchors(FAnchors(0.095f, 0.56f));
	MenuSlot->SetAlignment(FVector2D(0.f, 0.5f));
	MenuSlot->SetPosition(FVector2D::ZeroVector);
	MenuSlot->SetSize(FVector2D(330.f, 130.f));
	for (int32 Index = 0; Index < 2; ++Index)
	{
		auto* Button = Tree->ConstructWidget<UButton>(UButton::StaticClass(), Index == 0 ? TEXT("Button_Character") : TEXT("Button_Weapon"));
		FButtonStyle Style;
		FSlateBrush Normal;
		Normal.DrawAs = ESlateBrushDrawType::RoundedBox;
		Normal.TintColor = FSlateColor(FLinearColor(0.032f, 0.037f, 0.043f, 0.88f));
		Normal.OutlineSettings.CornerRadii = FVector4(0, 0, 0, 0);
		Normal.OutlineSettings.RoundingType = ESlateBrushRoundingType::FixedRadius;
		Normal.OutlineSettings.Width = 1.2f;
		Normal.OutlineSettings.Color = FSlateColor(FLinearColor(0.12f, 0.13f, 0.14f, 0.65f));
		Style.Normal = Normal;
		Style.Hovered = Normal;
		Style.Hovered.TintColor = FSlateColor(FLinearColor(0.055f, 0.06f, 0.067f, 0.93f));
		Style.Hovered.OutlineSettings.Color = FSlateColor(FLinearColor(0.55f, 0.38f, 0.10f, 1.f));
		Style.Pressed = Style.Hovered;
		Style.NormalPadding = FMargin(18.f, 0.f);
		Style.PressedPadding = Style.NormalPadding;
		Button->SetStyle(Style);
		auto* Row = Menu->AddChildToVerticalBox(Button);
		Row->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
		Row->SetPadding(FMargin(0.f, Index == 0 ? 0.f : 6.f, 0.f, 0.f));
		auto* Label = Tree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), Index == 0 ? TEXT("Text_Character") : TEXT("Text_Weapon"));
		Label->SetText(FText::FromString(Index == 0 ? TEXT("CHARACTER") : TEXT("WEAPON")));
		FSlateFontInfo Font(LoadObject<UObject>(nullptr, TEXT("/Engine/EngineFonts/Roboto.Roboto")), 18, TEXT("Regular"));
		Font.LetterSpacing = 180;
		Label->SetFont(Font);
		Label->SetColorAndOpacity(FSlateColor(FLinearColor(0.78f, 0.8f, 0.82f, 1.f)));
		Label->SetVisibility(ESlateVisibility::HitTestInvisible);
		auto* LabelSlot = CastChecked<UButtonSlot>(Button->AddChild(Label));
		LabelSlot->SetHorizontalAlignment(HAlign_Left);
		LabelSlot->SetVerticalAlignment(VAlign_Center);
	}
	FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(BP);
	FKismetEditorUtilities::CompileBlueprint(BP);
	return BP->Status != BS_Error;
#else
	return false;
#endif
}

bool UTheManLobbyAssetLibrary::AddWeaponDetails(UBlueprint* Blueprint)
{
#if WITH_EDITOR
	auto* BP = Cast<UWidgetBlueprint>(Blueprint);
	if (!BP || !BP->WidgetTree) return false;
	auto* Tree = BP->WidgetTree.Get();
	auto* Canvas = Cast<UCanvasPanel>(Tree->RootWidget);
	auto* Existing = Cast<UButton>(Tree->FindWidget(TEXT("Button_Weapon")));
	if (!Canvas || !Existing || Tree->FindWidget(TEXT("WeaponDetailsPanel"))) return false;
	BP->Modify();
	Tree->Modify();
	Canvas->Modify();
	auto* Panel = Tree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("WeaponDetailsPanel"));
	Panel->SetVisibility(ESlateVisibility::Collapsed);
	auto* Slot = Canvas->AddChildToCanvas(Panel);
	Slot->SetAnchors(FAnchors(0.095f, 0.56f));
	Slot->SetAlignment(FVector2D(0.f, 0.5f));
	Slot->SetAutoSize(true);
	auto* Card = Tree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("WeaponInfoCard"));
	Card->SetBrushColor(FLinearColor(0.022f, 0.027f, 0.033f, 0.90f));
	Card->SetPadding(FMargin(24.f));
	Panel->AddChildToVerticalBox(Card);
	auto* Content = Tree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("WeaponInfoContent"));
	Card->SetContent(Content);
	auto AddText = [&](const TCHAR* Name, const TCHAR* Text, int32 Size, FLinearColor Color, float Bottom)
	{
		auto* Label = Tree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), Name);
		Label->SetText(FText::FromString(Text));
		FSlateFontInfo Font(LoadObject<UObject>(nullptr, TEXT("/Engine/EngineFonts/Roboto.Roboto")), Size, TEXT("Regular"));
		Font.LetterSpacing = Size == 13 ? 180 : 20;
		Label->SetFont(Font);
		Label->SetColorAndOpacity(FSlateColor(Color));
		Label->SetWrapTextAt(330.f);
		Label->SetMinDesiredWidth(330.f);
		Label->SetVisibility(ESlateVisibility::HitTestInvisible);
		Content->AddChildToVerticalBox(Label)->SetPadding(FMargin(0.f, 0.f, 0.f, Bottom));
		return Label;
	};
	AddText(TEXT("Text_WeaponCategory"), TEXT("WEAPON"), 13, FLinearColor(0.55f, 0.38f, 0.10f, 1.f), 16.f);
	AddText(TEXT("Text_WeaponName"), TEXT("WEAPON NAME"), 27, FLinearColor(0.85f, 0.87f, 0.89f, 1.f), 18.f);
	AddText(TEXT("Text_WeaponDescription"), TEXT("Weapon description."), 16, FLinearColor(0.60f, 0.64f, 0.68f, 1.f), 0.f)->SetLineHeightPercentage(1.3f);
	auto* BackSize = Tree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("BackButtonSize"));
	BackSize->SetHeightOverride(62.f);
	Panel->AddChildToVerticalBox(BackSize)->SetPadding(FMargin(0.f, 10.f, 0.f, 0.f));
	auto* Back = Tree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("Button_Back"));
	Back->SetStyle(Existing->GetStyle());
	BackSize->SetContent(Back);
	auto* Label = Tree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("Text_Back"));
	Label->SetText(NSLOCTEXT("Lobby", "Back", "BACK"));
	auto* OriginalLabel = Cast<UTextBlock>(Tree->FindWidget(TEXT("Text_Weapon")));
	if (OriginalLabel) { Label->SetFont(OriginalLabel->GetFont()); Label->SetColorAndOpacity(OriginalLabel->GetColorAndOpacity()); }
	Label->SetVisibility(ESlateVisibility::HitTestInvisible);
	auto* BackSlot = CastChecked<UButtonSlot>(Back->AddChild(Label));
	BackSlot->SetHorizontalAlignment(HAlign_Left);
	BackSlot->SetVerticalAlignment(VAlign_Center);
	FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(BP);
	FKismetEditorUtilities::CompileBlueprint(BP);
	return BP->Status != BS_Error;
#else
	return false;
#endif
}

