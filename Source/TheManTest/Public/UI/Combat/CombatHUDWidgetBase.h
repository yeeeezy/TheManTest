#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CombatHUDWidgetBase.generated.h"

class SCombatHUDRoot;

UCLASS()
class THEMANTEST_API UCombatHUDWidgetBase : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetAmmoState(int32 CurrentAmmo, int32 MagazineCapacity, int32 SpareMagazineCount);
	void SetAmmoVisible(bool bVisible);

#if WITH_DEV_AUTOMATION_TESTS
	int32 GetDisplayedCurrentAmmoForTesting() const { return DisplayedCurrentAmmo; }
	int32 GetDisplayedMagazineCapacityForTesting() const { return DisplayedMagazineCapacity; }
	int32 GetDisplayedSpareMagazineCountForTesting() const { return DisplayedSpareMagazineCount; }
	bool IsAmmoVisibleForTesting() const { return bDisplayedAmmoVisible; }
#endif

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void ReleaseSlateResources(bool bReleaseChildren) override;

private:
	TSharedPtr<SCombatHUDRoot> CombatHUDRoot;
	int32 DisplayedCurrentAmmo = 30;
	int32 DisplayedMagazineCapacity = 30;
	int32 DisplayedSpareMagazineCount = 3;
	bool bDisplayedAmmoVisible = false;
};
