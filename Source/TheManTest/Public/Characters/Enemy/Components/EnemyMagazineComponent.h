#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "EnemyMagazineComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FEnemyAmmoChanged, int32, CurrentAmmo, int32, Capacity);

UCLASS(ClassGroup=(Enemy), meta=(BlueprintSpawnableComponent))
class THEMANTEST_API UEnemyMagazineComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UEnemyMagazineComponent();
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, Category="Enemy|Weapon")
	bool ConsumeRound();

	UFUNCTION(BlueprintCallable, Category="Enemy|Weapon")
	void Reload();

	UFUNCTION(BlueprintPure, Category="Enemy|Weapon")
	bool IsEmpty() const { return CurrentAmmo <= 0; }

	UFUNCTION(BlueprintPure, Category="Enemy|Weapon")
	int32 GetCurrentAmmo() const { return CurrentAmmo; }

	UFUNCTION(BlueprintPure, Category="Enemy|Weapon")
	int32 GetCapacity() const { return MagazineCapacity; }

	UPROPERTY(BlueprintAssignable, Category="Enemy|Weapon")
	FEnemyAmmoChanged OnAmmoChanged;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Enemy|Weapon", meta=(ClampMin="1"))
	int32 MagazineCapacity = 20;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Enemy|Weapon")
	int32 CurrentAmmo = 20;
};
