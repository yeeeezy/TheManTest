#include "Enemy/Boss/CoreMorph/GAS/Abilities/GA_CoreMorphFlight.h"
#include "Enemy/Boss/CoreMorph/CoreMorphBoss.h"
#include "Enemy/Boss/CoreMorph/Movement/CoreMorphFlightComponent.h"
#include "Core/_Shared/GAS/TheManGameplayTags.h"

UGA_CoreMorphFlight::UGA_CoreMorphFlight()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;
	ActivationRequiredTags.AddTag(TAG_State_CoreMorph_Form_Manta);
	ActivationBlockedTags.AddTag(TAG_State_CoreMorph_Transforming);
}

bool UGA_CoreMorphFlight::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* Info,
	const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* RelevantTags) const
{
	const auto* Boss = Info ? Cast<ACoreMorphBoss>(Info->AvatarActor.Get()) : nullptr;
	return Boss && !Boss->IsDead() && Boss->CurrentForm == ECoreMorphForm::Manta && Boss->Flight && Boss->Flight->CanStartFlight()
		&& Boss->VisualLayout && Super::CanActivateAbility(Handle, Info, SourceTags, TargetTags, RelevantTags);
}

void UGA_CoreMorphFlight::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* Info,
	const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* Event)
{
	Super::ActivateAbility(Handle, Info, ActivationInfo, Event);
	auto* Boss = Cast<ACoreMorphBoss>(GetAvatarActorFromActorInfo());
	Flight = Boss ? Boss->Flight.Get() : nullptr;
	if (!Flight.IsValid() || !CommitAbility(Handle, Info, ActivationInfo))
	{ EndAbility(Handle, Info, ActivationInfo, true, true); return; }
	FinishedHandle = Flight->OnFlightFinished.AddUObject(this, &UGA_CoreMorphFlight::FinishFlight);
	if (!Flight->StartFlight()) EndAbility(Handle, Info, ActivationInfo, true, true);
}

void UGA_CoreMorphFlight::FinishFlight()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UGA_CoreMorphFlight::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* Info,
	const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicate, bool bCancelled)
{
	if (Flight.IsValid())
	{
		Flight->OnFlightFinished.Remove(FinishedHandle);
		Flight->StopFlight();
	}
	FinishedHandle.Reset();
	Flight.Reset();
	Super::EndAbility(Handle, Info, ActivationInfo, bReplicate, bCancelled);
}
