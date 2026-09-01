#include "Weapons/_Shared/GAS/Abilities/GA_Reload.h"

#include "Characters/CharacterBase/FPSCharacterBase/FPSCharacterBase.h"
#include "Core/_Shared/GAS/TheManGameplayTags.h"
#include "Weapons/_Shared/Components/EquipmentManagerComponent.h"
#include "Weapons/_Shared/Firearms/Firearm.h"

UGA_Reload::UGA_Reload()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalOnly;

	FAbilityTriggerData TriggerData;
	TriggerData.TriggerTag = TAG_Input_Weapon_Reload;
	TriggerData.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
	AbilityTriggers.Add(TriggerData);
}

bool UGA_Reload::CanActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayTagContainer* SourceTags,
	const FGameplayTagContainer* TargetTags,
	FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	const AFPSCharacterBase* Character = ActorInfo
		? Cast<AFPSCharacterBase>(ActorInfo->AvatarActor.Get())
		: nullptr;
	const AFirearm* Firearm = Character && Character->GetEquipmentManager()
		? Cast<AFirearm>(Character->GetEquipmentManager()->GetCurrentEquipment())
		: nullptr;
	return Firearm && Firearm->CanReload();
}

void UGA_Reload::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	AFPSCharacterBase* Character = Cast<AFPSCharacterBase>(GetAvatarActorFromActorInfo());
	AFirearm* Firearm = Character && Character->GetEquipmentManager()
		? Cast<AFirearm>(Character->GetEquipmentManager()->GetCurrentEquipment())
		: nullptr;

	const bool bReloaded = Firearm && Firearm->ReloadMagazine();
	EndAbility(Handle, ActorInfo, ActivationInfo, true, !bReloaded);
}
