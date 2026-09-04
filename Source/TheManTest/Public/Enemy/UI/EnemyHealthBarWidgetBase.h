#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "EnemyHealthBarWidgetBase.generated.h"

class SEnemyHealthBarRoot;

UCLASS()
class THEMANTEST_API UEnemyHealthBarWidgetBase : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetHealthState(float CurrentHealth, float MaxHealth);

#if WITH_DEV_AUTOMATION_TESTS
	float GetDisplayedCurrentHealthForTesting() const { return DisplayedCurrentHealth; }
	float GetDisplayedMaxHealthForTesting() const { return DisplayedMaxHealth; }
#endif

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void ReleaseSlateResources(bool bReleaseChildren) override;

private:
	TSharedPtr<SEnemyHealthBarRoot> HealthBarRoot;
	float DisplayedCurrentHealth = 100.f;
	float DisplayedMaxHealth = 100.f;
};
