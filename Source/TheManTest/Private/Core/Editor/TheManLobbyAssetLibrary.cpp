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

