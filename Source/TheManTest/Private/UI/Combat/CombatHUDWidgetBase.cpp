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

		if (bAmmoVisible)
		{
			const FSlateFontInfo AmmoFont = FCoreStyle::GetDefaultFontStyle("Bold", 28);
			const FSlateFontInfo MagazineFont = FCoreStyle::GetDefaultFontStyle("Regular", 16);
			const FText AmmoText = FText::FromString(FString::Printf(TEXT("%d / %d"), CurrentAmmo, MagazineCapacity));
			const FText MagazineText = FText::FromString(FString::Printf(TEXT("弹夹  %d"), SpareMagazineCount));
			const TSharedRef<FSlateFontMeasure> FontMeasure = FSlateApplication::Get().GetRenderer()->GetFontMeasureService();
			const FVector2D AmmoSize = FontMeasure->Measure(AmmoText, AmmoFont);
			const FVector2D MagazineSize = FontMeasure->Measure(MagazineText, MagazineFont);
			const float Right = Size.X - 48.f;
			const float Bottom = Size.Y - 48.f;

			FSlateDrawElement::MakeText(OutDrawElements, LayerId + 1,
				AllottedGeometry.ToPaintGeometry(AmmoSize, FSlateLayoutTransform(FVector2D(Right - AmmoSize.X, Bottom - 58.f))),
				AmmoText, AmmoFont, ESlateDrawEffect::None, FLinearColor::White);
			FSlateDrawElement::MakeText(OutDrawElements, LayerId + 1,
				AllottedGeometry.ToPaintGeometry(MagazineSize, FSlateLayoutTransform(FVector2D(Right - MagazineSize.X, Bottom - 22.f))),
				MagazineText, MagazineFont, ESlateDrawEffect::None, FLinearColor(0.8f, 0.8f, 0.8f, 1.f));
		}

		return LayerId + 1;
	}

private:
	int32 CurrentAmmo = 30;
	int32 MagazineCapacity = 30;
	int32 SpareMagazineCount = 3;
	bool bAmmoVisible = false;
};

TSharedRef<SWidget> UCombatHUDWidgetBase::RebuildWidget()
{
	CombatHUDRoot = SNew(SCombatHUDRoot);
	CombatHUDRoot->SetAmmoState(
		DisplayedCurrentAmmo,
		DisplayedMagazineCapacity,
		DisplayedSpareMagazineCount);
	CombatHUDRoot->SetAmmoVisible(bDisplayedAmmoVisible);
	return CombatHUDRoot.ToSharedRef();
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
