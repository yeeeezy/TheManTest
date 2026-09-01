#include "UI/Combat/CombatHUDWidgetBase.h"

#include "Fonts/FontMeasure.h"
#include "Framework/Application/SlateApplication.h"
#include "Rendering/DrawElements.h"
#include "Styling/CoreStyle.h"
#include "Widgets/SLeafWidget.h"

class SCombatHUDRoot final : public SLeafWidget
{
public:
	SLATE_BEGIN_ARGS(SCombatHUDRoot) {}
	SLATE_END_ARGS()

	void Construct(const FArguments&) {}

	void SetAmmoState(int32 InCurrentAmmo, int32 InMagazineCapacity, int32 InSpareMagazineCount)
	{
		CurrentAmmo = InCurrentAmmo;
		MagazineCapacity = InMagazineCapacity;
		SpareMagazineCount = InSpareMagazineCount;
		Invalidate(EInvalidateWidgetReason::Paint);
	}

	void SetAmmoVisible(bool bInVisible)
	{
		bAmmoVisible = bInVisible;
		Invalidate(EInvalidateWidgetReason::Paint);
	}

	void SetHealthState(float InCurrentHealth, float InMaxHealth)
	{
		CurrentHealth = InCurrentHealth;
		MaxHealth = InMaxHealth;
		Invalidate(EInvalidateWidgetReason::Paint);
	}

	void SetHealthVisible(bool bInVisible)
	{
		bHealthVisible = bInVisible;
		Invalidate(EInvalidateWidgetReason::Paint);
	}

	virtual FVector2D ComputeDesiredSize(float) const override
	{
		return FVector2D(1920.f, 1080.f);
	}

	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry,
		const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements,
		int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override
	{
		const FVector2D Size = AllottedGeometry.GetLocalSize();
		const FVector2D Center = Size * 0.5f;
		constexpr float Radius = 57.6f;
		constexpr int32 Segments = 96;

		TArray<FVector2D> CirclePoints;
		CirclePoints.Reserve(Segments + 1);
		for (int32 Index = 0; Index <= Segments; ++Index)
		{
			const float Angle = 2.f * UE_PI * static_cast<float>(Index) / static_cast<float>(Segments);
			CirclePoints.Add(Center + FVector2D(FMath::Cos(Angle), FMath::Sin(Angle)) * Radius);
		}

		FSlateDrawElement::MakeLines(
			OutDrawElements,
			LayerId,
			AllottedGeometry.ToPaintGeometry(),
			CirclePoints,
			ESlateDrawEffect::None,
			FLinearColor(1.f, 1.f, 1.f, 0.85f),
			true,
			2.5f);

		const TSharedRef<FSlateFontMeasure> FontMeasure = FSlateApplication::Get().GetRenderer()->GetFontMeasureService();
		const float HudBaselineY = Size.Y - 70.f;

		if (bHealthVisible)
		{
			const FSlateFontInfo HealthIconFont = FCoreStyle::GetDefaultFontStyle("Bold", 30);
			const FSlateFontInfo HealthFont = FCoreStyle::GetDefaultFontStyle("Bold", 48);
			const FText HealthIconText = FText::FromString(TEXT("+"));
			const FText HealthText = FText::AsNumber(FMath::RoundToInt(FMath::Clamp(CurrentHealth, 0.f, MaxHealth)));
			const FVector2D HealthIconSize = FontMeasure->Measure(HealthIconText, HealthIconFont);
			const FVector2D HealthSize = FontMeasure->Measure(HealthText, HealthFont);
			const float HealthX = Size.X * 0.30f;

			FSlateDrawElement::MakeText(OutDrawElements, LayerId + 1,
				AllottedGeometry.ToPaintGeometry(HealthIconSize, FSlateLayoutTransform(FVector2D(HealthX, HudBaselineY - HealthIconSize.Y))),
				HealthIconText, HealthIconFont, ESlateDrawEffect::None, FLinearColor(0.86f, 0.9f, 0.9f, 0.9f));
			FSlateDrawElement::MakeText(OutDrawElements, LayerId + 1,
				AllottedGeometry.ToPaintGeometry(HealthSize, FSlateLayoutTransform(FVector2D(HealthX + HealthIconSize.X + 12.f, HudBaselineY - HealthSize.Y))),
				HealthText, HealthFont, ESlateDrawEffect::None, FLinearColor::White);
		}

		if (bAmmoVisible)
		{
			const FSlateFontInfo AmmoFont = FCoreStyle::GetDefaultFontStyle("Bold", 48);
			const FSlateFontInfo MagazineFont = FCoreStyle::GetDefaultFontStyle("Regular", 24);
			const FText AmmoText = FText::AsNumber(CurrentAmmo);
			const FText MagazineText = FText::AsNumber(SpareMagazineCount);
			const FVector2D AmmoSize = FontMeasure->Measure(AmmoText, AmmoFont);
			const FVector2D MagazineSize = FontMeasure->Measure(MagazineText, MagazineFont);
			const float AmmoX = Size.X * 0.64f;

			FSlateDrawElement::MakeText(OutDrawElements, LayerId + 1,
				AllottedGeometry.ToPaintGeometry(AmmoSize, FSlateLayoutTransform(FVector2D(AmmoX, HudBaselineY - AmmoSize.Y))),
				AmmoText, AmmoFont, ESlateDrawEffect::None, FLinearColor::White);
			FSlateDrawElement::MakeText(OutDrawElements, LayerId + 1,
				AllottedGeometry.ToPaintGeometry(MagazineSize, FSlateLayoutTransform(FVector2D(AmmoX + AmmoSize.X + 18.f, HudBaselineY - MagazineSize.Y))),
				MagazineText, MagazineFont, ESlateDrawEffect::None, FLinearColor(0.72f, 0.76f, 0.78f, 1.f));
		}

		return LayerId + 1;
	}

private:
	int32 CurrentAmmo = 30;
	int32 MagazineCapacity = 30;
	int32 SpareMagazineCount = 3;
	bool bAmmoVisible = false;
	float CurrentHealth = 100.f;
	float MaxHealth = 100.f;
	bool bHealthVisible = false;
};

TSharedRef<SWidget> UCombatHUDWidgetBase::RebuildWidget()
{
	CombatHUDRoot = SNew(SCombatHUDRoot);
	CombatHUDRoot->SetAmmoState(
		DisplayedCurrentAmmo,
		DisplayedMagazineCapacity,
		DisplayedSpareMagazineCount);
	CombatHUDRoot->SetAmmoVisible(bDisplayedAmmoVisible);
	CombatHUDRoot->SetHealthState(DisplayedCurrentHealth, DisplayedMaxHealth);
	CombatHUDRoot->SetHealthVisible(bDisplayedHealthVisible);
	return CombatHUDRoot.ToSharedRef();
}

void UCombatHUDWidgetBase::SetHealthState(float CurrentHealth, float MaxHealth)
{
	DisplayedCurrentHealth = CurrentHealth;
	DisplayedMaxHealth = MaxHealth;
	if (CombatHUDRoot)
	{
		CombatHUDRoot->SetHealthState(CurrentHealth, MaxHealth);
	}
}

void UCombatHUDWidgetBase::SetHealthVisible(bool bVisible)
{
	bDisplayedHealthVisible = bVisible;
	if (CombatHUDRoot)
	{
		CombatHUDRoot->SetHealthVisible(bVisible);
	}
}

void UCombatHUDWidgetBase::ReleaseSlateResources(bool bReleaseChildren)
{
	Super::ReleaseSlateResources(bReleaseChildren);
	CombatHUDRoot.Reset();
}

void UCombatHUDWidgetBase::SetAmmoState(int32 CurrentAmmo, int32 MagazineCapacity, int32 SpareMagazineCount)
{
	DisplayedCurrentAmmo = CurrentAmmo;
	DisplayedMagazineCapacity = MagazineCapacity;
	DisplayedSpareMagazineCount = SpareMagazineCount;
	if (CombatHUDRoot)
	{
		CombatHUDRoot->SetAmmoState(CurrentAmmo, MagazineCapacity, SpareMagazineCount);
	}
}

void UCombatHUDWidgetBase::SetAmmoVisible(bool bVisible)
{
	bDisplayedAmmoVisible = bVisible;
	if (CombatHUDRoot)
	{
		CombatHUDRoot->SetAmmoVisible(bVisible);
	}
}
