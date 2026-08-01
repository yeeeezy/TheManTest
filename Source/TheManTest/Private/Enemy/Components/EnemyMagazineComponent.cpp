#include "Enemy/Components/EnemyMagazineComponent.h"

UEnemyMagazineComponent::UEnemyMagazineComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UEnemyMagazineComponent::BeginPlay()
{
	Super::BeginPlay();
	CurrentAmmo = MagazineCapacity;
}

bool UEnemyMagazineComponent::ConsumeRound()
{
	if (CurrentAmmo <= 0) return false;
	--CurrentAmmo;
	OnAmmoChanged.Broadcast(CurrentAmmo, MagazineCapacity);
	return true;
}

void UEnemyMagazineComponent::Reload()
{
	CurrentAmmo = MagazineCapacity;
	OnAmmoChanged.Broadcast(CurrentAmmo, MagazineCapacity);
}
