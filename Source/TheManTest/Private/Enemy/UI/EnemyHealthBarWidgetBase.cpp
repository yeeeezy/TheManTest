#include "Enemy/UI/EnemyHealthBarWidgetBase.h"

#include "Rendering/DrawElements.h"
#include "Styling/CoreStyle.h"
#include "Widgets/SLeafWidget.h"

class SEnemyHealthBarRoot final : public SLeafWidget
{
public:
	SLATE_BEGIN_ARGS(SEnemyHealthBarRoot) {}
	SLATE_END_ARGS()

	void Construct(const FArguments&) {}

	void SetHealthState(float InCurrentHealth, float InMaxHealth)
	{
		CurrentHealth = InCurrentHealth;
		MaxHealth = InMaxHealth;
		Invalidate(EInvalidateWidgetReason::Paint);
	}

	virtual FVector2D ComputeDesiredSize(float) const override
	{
		return FVector2D(180.f, 18.f);
	}

	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry,
		const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements,
		int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override
	{
		const FVector2D Size = AllottedGeometry.GetLocalSize();
		const FSlateBrush* WhiteBrush = FCoreStyle::Get().GetBrush(TEXT("WhiteBrush"));
		const float HealthFraction = MaxHealth > 0.f
			? FMath::Clamp(CurrentHealth / MaxHealth, 0.f, 1.f)
			: 0.f;

		FSlateDrawElement::MakeBox(
			OutDrawElements,
			LayerId,
			AllottedGeometry.ToPaintGeometry(),
			WhiteBrush,
			ESlateDrawEffect::None,
			FLinearColor(0.01f, 0.01f, 0.01f, 0.9f));

		const FVector2D Padding(2.f, 2.f);
		const FVector2D InnerSize(
			FMath::Max(Size.X - Padding.X * 2.f, 0.f),
			FMath::Max(Size.Y - Padding.Y * 2.f, 0.f));
		FSlateDrawElement::MakeBox(
			OutDrawElements,
			LayerId + 1,
			AllottedGeometry.ToPaintGeometry(InnerSize, FSlateLayoutTransform(Padding)),
			WhiteBrush,
			ESlateDrawEffect::None,
			FLinearColor(0.12f, 0.015f, 0.015f, 0.95f));

		if (HealthFraction > 0.f)
		{
			const FVector2D FillSize(InnerSize.X * HealthFraction, InnerSize.Y);
			FSlateDrawElement::MakeBox(
				OutDrawElements,
				LayerId + 2,
				AllottedGeometry.ToPaintGeometry(FillSize, FSlateLayoutTransform(Padding)),
				WhiteBrush,
				ESlateDrawEffect::None,
				FLinearColor(0.88f, 0.025f, 0.025f, 1.f));
		}

		return LayerId + 2;
	}

private:
	float CurrentHealth = 100.f;
	float MaxHealth = 100.f;
};

TSharedRef<SWidget> UEnemyHealthBarWidgetBase::RebuildWidget()
{
	HealthBarRoot = SNew(SEnemyHealthBarRoot);
	HealthBarRoot->SetHealthState(DisplayedCurrentHealth, DisplayedMaxHealth);
	return HealthBarRoot.ToSharedRef();
}

void UEnemyHealthBarWidgetBase::SetHealthState(float CurrentHealth, float MaxHealth)
{
	DisplayedCurrentHealth = CurrentHealth;
	DisplayedMaxHealth = MaxHealth;
	if (HealthBarRoot)
	{
		HealthBarRoot->SetHealthState(CurrentHealth, MaxHealth);
	}
}

void UEnemyHealthBarWidgetBase::ReleaseSlateResources(bool bReleaseChildren)
{
	Super::ReleaseSlateResources(bReleaseChildren);
	HealthBarRoot.Reset();
}
