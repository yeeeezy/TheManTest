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
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/Image.h"
#include "UI/Lobby/LobbyPresentationWidgetBase.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "Styling/CoreStyle.h"
#endif

bool UTheManLobbyAssetLibrary::AddCharacterDetails(UBlueprint* Blueprint)
{
#if WITH_EDITOR
	auto* BP = Cast<UWidgetBlueprint>(Blueprint);
	if (!BP || !BP->WidgetTree) return false;
	auto* Tree = BP->WidgetTree.Get();
	auto* Canvas = Cast<UCanvasPanel>(Tree->RootWidget);
	auto* OriginalPanel = Cast<UVerticalBox>(Tree->FindWidget(TEXT("WeaponDetailsPanel")));
	auto* ExistingButton = Cast<UButton>(Tree->FindWidget(TEXT("Button_Character")));
	if (!Canvas || !OriginalPanel || !ExistingButton) return false;
	if (Tree->FindWidget(TEXT("CharacterDetailsPanel"))) return true;
	BP->Modify(); Tree->Modify(); Canvas->Modify();
	auto* Panel = Tree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("CharacterDetailsPanel"));
	Panel->SetVisibility(ESlateVisibility::Collapsed);
	auto* Slot = Canvas->AddChildToCanvas(Panel);
	auto* SourceSlot = CastChecked<UCanvasPanelSlot>(OriginalPanel->Slot);
	Slot->SetLayout(SourceSlot->GetLayout()); Slot->SetAutoSize(true);
	auto AddText = [&](const TCHAR* Name, const TCHAR* SourceName, const TCHAR* Text, float Bottom)
	{
		auto* Label = Tree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), Name);
		auto* Source = CastChecked<UTextBlock>(Tree->FindWidget(SourceName));
		Label->SetText(FText::FromString(Text)); Label->SetFont(Source->GetFont());
		Label->SetColorAndOpacity(Source->GetColorAndOpacity());
		Label->SetWrapTextAt(440); Label->SetMinDesiredWidth(440);
		Label->SetLineHeightPercentage(1.45f);
		Label->SetVisibility(ESlateVisibility::HitTestInvisible);
		Panel->AddChildToVerticalBox(Label)->SetPadding(FMargin(0,0,0,Bottom));
	};
	AddText(TEXT("Text_CharacterCategory"), TEXT("Text_WeaponCategory"), TEXT("CHARACTER"), 22);
	const TCHAR* Keys[] = {TEXT("MaintenanceWorker"), TEXT("Executive")};
	for (int32 Index = 0; Index < 2; ++Index)
	{
		const FString Key(Keys[Index]);
		auto* Size = Tree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), FName(TEXT("CharacterSize_") + Key));
		Size->SetWidthOverride(440); Size->SetHeightOverride(54);
		Panel->AddChildToVerticalBox(Size)->SetPadding(FMargin(0,0,0,Index == 0 ? 10 : 30));
		auto* Button = Tree->ConstructWidget<UButton>(UButton::StaticClass(), FName(TEXT("Button_") + Key));
		Button->SetStyle(ExistingButton->GetStyle()); Size->SetContent(Button);
		auto* Label = Tree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), FName(TEXT("Text_") + Key + TEXT("Choice")));
		Label->SetFont(FSlateFontInfo(LoadObject<UObject>(nullptr, TEXT("/Engine/EngineFonts/Roboto.Roboto")), 16, TEXT("Regular")));
		Label->SetColorAndOpacity(FSlateColor(FLinearColor(.78f,.8f,.82f,1)));
		Label->SetVisibility(ESlateVisibility::HitTestInvisible);
		auto* LabelSlot = CastChecked<UButtonSlot>(Button->AddChild(Label));
		LabelSlot->SetHorizontalAlignment(HAlign_Left); LabelSlot->SetVerticalAlignment(VAlign_Center);
	}
	AddText(TEXT("Text_CharacterName"), TEXT("Text_WeaponName"), TEXT(""), 22);
	auto* RuleSize = Tree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("CharacterRuleSize"));
	RuleSize->SetWidthOverride(72); RuleSize->SetHeightOverride(1);
	auto* RuleSlot = Panel->AddChildToVerticalBox(RuleSize); RuleSlot->SetHorizontalAlignment(HAlign_Left); RuleSlot->SetPadding(FMargin(0,0,0,22));
	auto* Rule = Tree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("CharacterRule"));
	Rule->SetPadding(FMargin(0)); Rule->SetBrushColor(FLinearColor(.28f,.30f,.31f,.75f)); RuleSize->SetContent(Rule);
	AddText(TEXT("Text_CharacterDescription"), TEXT("Text_WeaponDescription"), TEXT(""), 0);
	FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(BP); FKismetEditorUtilities::CompileBlueprint(BP);
	return BP->Status != BS_Error;
#else
	return false;
#endif
}

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

bool UTheManLobbyAssetLibrary::RefineWeaponDetails(UBlueprint* Blueprint, UTexture2D* CornerTexture)
{
#if WITH_EDITOR
	auto* BP = Cast<UWidgetBlueprint>(Blueprint);
	if (!BP || !BP->WidgetTree) return false;
	auto* Tree = BP->WidgetTree.Get();
	auto* Canvas = Cast<UCanvasPanel>(Tree->RootWidget);
	auto* Panel = Cast<UVerticalBox>(Tree->FindWidget(TEXT("WeaponDetailsPanel")));
	auto* Back = Cast<UButton>(Tree->FindWidget(TEXT("Button_Back")));
	auto* Category = Cast<UTextBlock>(Tree->FindWidget(TEXT("Text_WeaponCategory")));
	auto* Name = Cast<UTextBlock>(Tree->FindWidget(TEXT("Text_WeaponName")));
	auto* Description = Cast<UTextBlock>(Tree->FindWidget(TEXT("Text_WeaponDescription")));
	if (!Canvas || !Panel || !Back || !Category || !Name || !Description) return false;
	auto SetImageSize = [](UImage* Image, FVector2D Size)
	{
		FSlateBrush Brush = Image->GetBrush(); Brush.SetImageSize(Size); Image->SetBrush(Brush);
	};
	if (Tree->FindWidget(TEXT("WeaponChoices")))
	{
		BP->Modify();
		for (const TCHAR* Key : {TEXT("RepairGun"), TEXT("ExplosionGun"), TEXT("ElectricGun")})
		{
			if (auto* Icon = Cast<UImage>(Tree->FindWidget(FName(FString(TEXT("Image_")) + Key)))) SetImageSize(Icon, FVector2D(100,50));
			if (auto* Corner = Cast<UImage>(Tree->FindWidget(FName(FString(TEXT("Corner_")) + Key)))) SetImageSize(Corner, FVector2D(18,18));
		}
		FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(BP); FKismetEditorUtilities::CompileBlueprint(BP);
		return BP->Status != BS_Error;
	}
	BP->Modify(); Tree->Modify(); Canvas->Modify(); Panel->Modify();
	UWidget* ObsoleteWidgets[] = {Tree->FindWidget(TEXT("WeaponInfoCard")), Tree->FindWidget(TEXT("WeaponInfoContent")), Tree->FindWidget(TEXT("BackButtonSize"))};
	UWidget* RetainedWidgets[] = {Category, Name, Description, Back};
	for (UWidget* Widget : RetainedWidgets) { Widget->Modify(); Widget->RemoveFromParent(); }
	Panel->ClearChildren();
	for (UWidget* Widget : ObsoleteWidgets)
		if (Widget) Widget->Rename(nullptr, GetTransientPackage(), REN_DontCreateRedirectors);
	auto* PanelSlot = CastChecked<UCanvasPanelSlot>(Panel->Slot);
	PanelSlot->SetAnchors(FAnchors(0.095f, 0.28f)); PanelSlot->SetAlignment(FVector2D::ZeroVector); PanelSlot->SetPosition(FVector2D::ZeroVector);
	auto Font = [](int32 Size, int32 Spacing) { FSlateFontInfo Info(LoadObject<UObject>(nullptr, TEXT("/Engine/EngineFonts/Roboto.Roboto")), Size, TEXT("Regular")); Info.LetterSpacing = Spacing; return Info; };
	Category->SetFont(Font(16, 500)); Category->SetMinDesiredWidth(0.f);
	Panel->AddChildToVerticalBox(Category)->SetPadding(FMargin(0,0,0,22));
	auto* Choices = Tree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("WeaponChoices"));
	Panel->AddChildToVerticalBox(Choices)->SetPadding(FMargin(0,0,0,30));
	const TCHAR* Keys[] = {TEXT("RepairGun"), TEXT("ExplosionGun"), TEXT("ElectricGun")};
	for (int32 Index = 0; Index < 3; ++Index)
	{
		const FString Key(Keys[Index]);
		auto* Size = Tree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), FName(TEXT("Size_") + Key));
		Size->SetWidthOverride(112); Size->SetHeightOverride(112);
		Choices->AddChildToHorizontalBox(Size)->SetPadding(FMargin(0,0,Index < 2 ? 16 : 0,0));
		auto* Button = Tree->ConstructWidget<UButton>(UButton::StaticClass(), FName(TEXT("Button_") + Key));
		Size->SetContent(Button);
		FButtonStyle Style;
		Style.Normal.DrawAs = ESlateBrushDrawType::RoundedBox;
		Style.Normal.TintColor = FSlateColor(FLinearColor(0.01f,0.012f,0.016f,0.15f));
		Style.Normal.OutlineSettings.RoundingType = ESlateBrushRoundingType::FixedRadius;
		Style.Normal.OutlineSettings.CornerRadii = FVector4(0,0,0,0);
		Style.Normal.OutlineSettings.Width = 1.f;
		Style.Normal.OutlineSettings.Color = FSlateColor(FLinearColor(0.2f,0.22f,0.24f,0.65f));
		Style.Hovered = Style.Normal; Style.Hovered.OutlineSettings.Color = FSlateColor(FLinearColor(0.65f,0.4f,0.09f,1));
		Style.Pressed = Style.Normal; Style.Disabled = Style.Normal;
		Style.NormalPadding = FMargin(0); Style.PressedPadding = FMargin(0); Button->SetStyle(Style);
		auto* Overlay = Tree->ConstructWidget<UOverlay>(UOverlay::StaticClass(), FName(TEXT("Overlay_") + Key));
		Overlay->SetVisibility(ESlateVisibility::HitTestInvisible);
		auto* ButtonSlot = CastChecked<UButtonSlot>(Button->AddChild(Overlay)); ButtonSlot->SetPadding(FMargin(0)); ButtonSlot->SetHorizontalAlignment(HAlign_Fill); ButtonSlot->SetVerticalAlignment(VAlign_Fill);
		auto* Icon = Tree->ConstructWidget<UImage>(UImage::StaticClass(), FName(TEXT("Image_") + Key));
		SetImageSize(Icon, FVector2D(100,50));
		auto* IconSlot = Overlay->AddChildToOverlay(Icon); IconSlot->SetHorizontalAlignment(HAlign_Center); IconSlot->SetVerticalAlignment(VAlign_Center);
		auto* Corner = Tree->ConstructWidget<UImage>(UImage::StaticClass(), FName(TEXT("Corner_") + Key));
		Corner->SetBrushFromTexture(CornerTexture); SetImageSize(Corner, FVector2D(18,18)); Corner->SetVisibility(ESlateVisibility::Collapsed);
		auto* CornerSlot = Overlay->AddChildToOverlay(Corner); CornerSlot->SetHorizontalAlignment(HAlign_Right); CornerSlot->SetVerticalAlignment(VAlign_Top);
	}
	Name->SetFont(Font(27,180)); Name->SetWrapTextAt(440); Name->SetMinDesiredWidth(440);
	Panel->AddChildToVerticalBox(Name)->SetPadding(FMargin(0,0,0,22));
	auto* LineSize = Tree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("WeaponRuleSize"));
	LineSize->SetWidthOverride(72); LineSize->SetHeightOverride(1);
	auto* RuleSlot = Panel->AddChildToVerticalBox(LineSize); RuleSlot->SetHorizontalAlignment(HAlign_Left); RuleSlot->SetPadding(FMargin(0,0,0,22));
	auto* Rule = Tree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("WeaponRule")); Rule->SetPadding(FMargin(0)); Rule->SetBrushColor(FLinearColor(0.28f,0.30f,0.31f,0.75f)); LineSize->SetContent(Rule);
	Description->SetFont(Font(15,100)); Description->SetWrapTextAt(440); Description->SetMinDesiredWidth(440); Description->SetLineHeightPercentage(1.45f);
	Panel->AddChildToVerticalBox(Description);
	auto* BackSlot = Canvas->AddChildToCanvas(Back); BackSlot->SetAnchors(FAnchors(0.095f,0.92f)); BackSlot->SetAlignment(FVector2D(0,1)); BackSlot->SetPosition(FVector2D::ZeroVector); BackSlot->SetSize(FVector2D(180,48));
	FButtonStyle BackStyle = Back->GetStyle(); BackStyle.Normal.DrawAs = ESlateBrushDrawType::NoDrawType; BackStyle.Normal.TintColor = FSlateColor(FLinearColor::Transparent); BackStyle.Hovered = BackStyle.Normal; BackStyle.Hovered.DrawAs = ESlateBrushDrawType::Box; BackStyle.Hovered.TintColor = FSlateColor(FLinearColor(0.55f,0.38f,0.10f,0.10f)); BackStyle.Pressed = BackStyle.Normal; BackStyle.NormalPadding = FMargin(0); BackStyle.PressedPadding = FMargin(0); Back->SetStyle(BackStyle);
	if (auto* Label = Cast<UTextBlock>(Tree->FindWidget(TEXT("Text_Back")))) { Label->SetText(FText::FromString(TEXT("\u2039  BACK"))); Label->SetFont(Font(16,350)); }
	Back->SetVisibility(ESlateVisibility::Collapsed);
	FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(BP); FKismetEditorUtilities::CompileBlueprint(BP);
	return BP->Status != BS_Error;
#else
	return false;
#endif
}


bool UTheManLobbyAssetLibrary::RefineCharacterNavigation(UBlueprint* Blueprint)
{
#if WITH_EDITOR
 auto* BP = Cast<UWidgetBlueprint>(Blueprint);
 if (!BP || !BP->WidgetTree) return false;
 auto* Tree = BP->WidgetTree.Get();
 auto* Panel = Cast<UVerticalBox>(Tree->FindWidget(TEXT("CharacterDetailsPanel")));
 auto* Weapon = Cast<UVerticalBox>(Tree->FindWidget(TEXT("WeaponDetailsPanel")));
 auto* Name = Cast<UTextBlock>(Tree->FindWidget(TEXT("Text_CharacterName")));
 auto* Description = Cast<UTextBlock>(Tree->FindWidget(TEXT("Text_CharacterDescription")));
 auto* Category = Cast<UTextBlock>(Tree->FindWidget(TEXT("Text_CharacterCategory")));
 auto* RuleSize = Cast<USizeBox>(Tree->FindWidget(TEXT("CharacterRuleSize")));
 if (!Panel || !Weapon || !Name || !Description || !Category || !RuleSize) return false;
 if (Tree->FindWidget(TEXT("Button_StartGame")))
 {
  BP->Modify(); Name->Modify(); Description->Modify();
  Name->SetLineHeightPercentage(1.f);
  auto DescriptionFont=Description->GetFont(); DescriptionFont.LetterSpacing=20; Description->SetFont(DescriptionFont);
  if (auto* Rule=Cast<UBorder>(RuleSize->GetContent())) { Rule->Modify(); Rule->SetBrushColor(FLinearColor(.65f,.40f,.09f,1)); }
  if (auto* Arrow=Tree->FindWidget(TEXT("Text_NextCharacterArrow"))) { Arrow->Modify(); Arrow->SetRenderTranslation(FVector2D(0,-8)); }
  FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(BP); FKismetEditorUtilities::CompileBlueprint(BP);
  return BP->Status != BS_Error;
 }
 BP->Modify(); Tree->Modify(); Panel->Modify(); Weapon->Modify();
 const FLinearColor Gold(.65f,.40f,.09f,1);
 const FLinearColor White(.78f,.80f,.82f,1);
 auto Font = [](int32 Size, int32 Spacing) { FSlateFontInfo F(LoadObject<UObject>(nullptr,TEXT("/Engine/EngineFonts/Roboto.Roboto")),Size,TEXT("Regular")); F.LetterSpacing=Spacing; return F; };
 auto Text = [&](const TCHAR* Key,const TCHAR* Copy,int32 Size,int32 Spacing,FLinearColor Color)
 {
  auto* T=Tree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(),Key);
  T->SetText(FText::FromString(Copy)); T->SetFont(Font(Size,Spacing)); T->SetColorAndOpacity(FSlateColor(Color)); T->SetVisibility(ESlateVisibility::HitTestInvisible); return T;
 };
 auto Button = [&](const TCHAR* Key)
 {
  auto* B=Tree->ConstructWidget<UButton>(UButton::StaticClass(),Key);
  FButtonStyle S; S.Normal.DrawAs=ESlateBrushDrawType::NoDrawType; S.Hovered=S.Normal; S.Pressed=S.Normal; S.Disabled=S.Normal;
  S.NormalPadding=FMargin(0); S.PressedPadding=FMargin(0); B->SetStyle(S); return B;
 };
 auto ButtonText = [&](UButton* B,UTextBlock* T)
 {
  auto* S=CastChecked<UButtonSlot>(B->AddChild(T)); S->SetPadding(FMargin(0)); S->SetHorizontalAlignment(HAlign_Left); S->SetVerticalAlignment(VAlign_Center);
 };
 for (const TCHAR* Key : {TEXT("CharacterSize_MaintenanceWorker"),TEXT("CharacterSize_Executive")})
  if (auto* W=Tree->FindWidget(Key)) { W->Modify(); W->SetVisibility(ESlateVisibility::Collapsed); }
 Panel->ClearChildren();
 Category->SetText(FText::FromString(TEXT("SELECT YOUR CHARACTER"))); Category->SetFont(Font(16,350)); Category->SetColorAndOpacity(FSlateColor(Gold)); Category->SetMinDesiredWidth(0); Category->SetWrapTextAt(0);
 Panel->AddChildToVerticalBox(Category)->SetPadding(FMargin(0,0,0,24));
 auto* NameRow=Tree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(),TEXT("CharacterNameRow"));
 Panel->AddChildToVerticalBox(NameRow)->SetPadding(FMargin(0,0,0,10));
 Name->SetFont(Font(36,230)); Name->SetLineHeightPercentage(1.f); Name->SetColorAndOpacity(FSlateColor(White)); Name->SetMinDesiredWidth(0); Name->SetWrapTextAt(0); Name->SetAutoWrapText(false);
 NameRow->AddChildToHorizontalBox(Name)->SetVerticalAlignment(VAlign_Center);
 auto* Next=Button(TEXT("Button_NextCharacter")); ButtonText(Next,Text(TEXT("Text_NextCharacterArrow"),TEXT("\u203a"),38,0,Gold));
 Next->GetContent()->SetRenderTranslation(FVector2D(0,-8));
 auto* NextSlot=NameRow->AddChildToHorizontalBox(Next); NextSlot->SetPadding(FMargin(22,0,0,0)); NextSlot->SetVerticalAlignment(VAlign_Center);
 auto* Hint=Tree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(),TEXT("CharacterNextHint"));
 Panel->AddChildToVerticalBox(Hint)->SetPadding(FMargin(0,0,0,38));
 auto* Key=Tree->ConstructWidget<UBorder>(UBorder::StaticClass(),TEXT("TabKeycap"));
 FSlateBrush KeyBrush; KeyBrush.DrawAs=ESlateBrushDrawType::RoundedBox; KeyBrush.TintColor=FSlateColor(FLinearColor(.025f,.025f,.025f,.7f)); KeyBrush.OutlineSettings.RoundingType=ESlateBrushRoundingType::FixedRadius; KeyBrush.OutlineSettings.CornerRadii=FVector4(3,3,3,3); KeyBrush.OutlineSettings.Width=1; KeyBrush.OutlineSettings.Color=FSlateColor(FLinearColor(.22f,.22f,.22f,1));
 Key->SetBrush(KeyBrush); Key->SetPadding(FMargin(10,4)); Key->SetContent(Text(TEXT("Text_TabHint"),TEXT("TAB"),12,200,FLinearColor(.45f,.45f,.45f,1)));
 Hint->AddChildToHorizontalBox(Key)->SetVerticalAlignment(VAlign_Center);
 auto* HintSlot=Hint->AddChildToHorizontalBox(Text(TEXT("Text_NextCharacterHint"),TEXT("NEXT CHARACTER"),12,200,FLinearColor(.32f,.34f,.36f,1))); HintSlot->SetPadding(FMargin(14,0,0,0)); HintSlot->SetVerticalAlignment(VAlign_Center);
 RuleSize->SetWidthOverride(48); RuleSize->SetHeightOverride(1);
 if (auto* Rule=Cast<UBorder>(RuleSize->GetContent())) Rule->SetBrushColor(Gold);
 auto* RuleSlot=Panel->AddChildToVerticalBox(RuleSize); RuleSlot->SetHorizontalAlignment(HAlign_Left); RuleSlot->SetPadding(FMargin(0,0,0,18));
 Description->SetFont(Font(15,20)); Description->SetWrapTextAt(440); Description->SetMinDesiredWidth(440); Description->SetLineHeightPercentage(1.45f);
 Panel->AddChildToVerticalBox(Description)->SetPadding(FMargin(0,0,0,32));
 auto* View=Button(TEXT("Button_ViewWeapons")); ButtonText(View,Text(TEXT("Text_ViewWeapons"),TEXT("VIEW WEAPONS  \u203a"),14,200,FLinearColor(.65f,.68f,.70f,1)));
 Panel->AddChildToVerticalBox(View)->SetHorizontalAlignment(HAlign_Left);
 // Preserve the existing weapon hierarchy, typography, thumbnails and spacing.
 auto* Start=Button(TEXT("Button_StartGame")); ButtonText(Start,Text(TEXT("Text_StartGame"),TEXT("START GAME  \u203a"),16,250,Gold));
 auto* StartSlot=Weapon->AddChildToVerticalBox(Start); StartSlot->SetPadding(FMargin(0,32,0,0)); StartSlot->SetHorizontalAlignment(HAlign_Left);
 if (auto* T=Cast<UTextBlock>(Tree->FindWidget(TEXT("Text_WeaponCategory")))) T->SetColorAndOpacity(FSlateColor(Gold));
 if (auto* R=Cast<UBorder>(Tree->FindWidget(TEXT("WeaponRule")))) R->SetBrushColor(Gold);
 if (auto* T=Cast<UTextBlock>(Tree->FindWidget(TEXT("Text_Character")))) T->SetText(FText::FromString(TEXT("START")));
 if (auto* T=Cast<UTextBlock>(Tree->FindWidget(TEXT("Text_Weapon")))) T->SetText(FText::FromString(TEXT("SETTINGS")));
 if (auto* B=Cast<UButton>(Tree->FindWidget(TEXT("Button_Weapon")))) { B->SetIsEnabled(false); B->SetToolTipText(FText::FromString(TEXT("Settings are not available yet."))); }
 FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(BP); FKismetEditorUtilities::CompileBlueprint(BP);
 return BP->Status != BS_Error;
#else
 return false;
#endif
}
