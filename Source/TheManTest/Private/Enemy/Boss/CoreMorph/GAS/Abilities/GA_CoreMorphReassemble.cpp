#include "Enemy/Boss/CoreMorph/GAS/Abilities/GA_CoreMorphReassemble.h"
#include "Enemy/Boss/CoreMorph/GAS/Abilities/GA_CoreMorphFlight.h"
#include "Enemy/Boss/CoreMorph/GAS/Effects/GE_CoreMorphTransforming.h"
#include "Enemy/Boss/CoreMorph/Transformation/CoreMorphReassemblyComponent.h"
#include "Enemy/Boss/CoreMorph/CoreMorphBoss.h"
#include "Core/_Shared/GAS/TheManGameplayTags.h"
#include "AbilitySystemComponent.h"

UGA_CoreMorphReassemble::UGA_CoreMorphReassemble()
{
    InstancingPolicy=EGameplayAbilityInstancingPolicy::InstancedPerActor;
    NetExecutionPolicy=EGameplayAbilityNetExecutionPolicy::ServerOnly;
    ActivationRequiredTags.AddTag(TAG_State_CoreMorph_Form_Manta);
    ActivationBlockedTags.AddTag(TAG_State_CoreMorph_Transforming);
}
bool UGA_CoreMorphReassemble::CanActivateAbility(const FGameplayAbilitySpecHandle Handle,const FGameplayAbilityActorInfo* Info,
    const FGameplayTagContainer* SourceTags,const FGameplayTagContainer* TargetTags,FGameplayTagContainer* RelevantTags) const
{
    const auto* Boss=Info?Cast<ACoreMorphBoss>(Info->AvatarActor.Get()):nullptr;
    return Boss && Boss->Reassembly->CanStart() && Super::CanActivateAbility(Handle,Info,SourceTags,TargetTags,RelevantTags);
}
void UGA_CoreMorphReassemble::ActivateAbility(const FGameplayAbilitySpecHandle Handle,const FGameplayAbilityActorInfo* Info,
    const FGameplayAbilityActivationInfo ActivationInfo,const FGameplayEventData* Event)
{
    Super::ActivateAbility(Handle,Info,ActivationInfo,Event);
    auto* Boss=Cast<ACoreMorphBoss>(GetAvatarActorFromActorInfo());Reassembly=Boss?Boss->Reassembly.Get():nullptr;
    if(!Reassembly.IsValid() || !CommitAbility(Handle,Info,ActivationInfo) || !Reassembly->Start())
    {EndAbility(Handle,Info,ActivationInfo,true,true);return;}
    auto* ASC=Boss->GetAbilitySystemComponent();
    if(auto* Flight=ASC->FindAbilitySpecFromClass(UGA_CoreMorphFlight::StaticClass()))ASC->CancelAbilityHandle(Flight->Handle);
    TransformEffect=ASC->ApplyGameplayEffectToSelf(GetDefault<UGE_CoreMorphTransforming>(),1.f,ASC->MakeEffectContext());
    FinishedHandle=Reassembly->OnReassembled.AddUObject(this,&ThisClass::FinishReassembly);
    FGameplayCueParameters Params;Params.SourceObject=Boss;Params.Location=Boss->GetActorLocation();
    ASC->AddGameplayCue(TAG_GameplayCue_CoreMorph_Reassembly,Params);
}
void UGA_CoreMorphReassemble::FinishReassembly()
{
    if(auto* Boss=Cast<ACoreMorphBoss>(GetAvatarActorFromActorInfo()))Boss->SetForm(ECoreMorphForm::Scorpion);
    EndAbility(CurrentSpecHandle,CurrentActorInfo,CurrentActivationInfo,true,false);
}
void UGA_CoreMorphReassemble::EndAbility(const FGameplayAbilitySpecHandle Handle,const FGameplayAbilityActorInfo* Info,
    const FGameplayAbilityActivationInfo ActivationInfo,bool bReplicate,bool bCancelled)
{
    if(Reassembly.IsValid())
    {
        Reassembly->OnReassembled.Remove(FinishedHandle);
        if(bCancelled)Reassembly->Cancel();
        // Successful assembly leaves only the Cue's bounded sand/dust tail running.
    }
    if(auto* ASC=GetAbilitySystemComponentFromActorInfo())ASC->RemoveActiveGameplayEffect(TransformEffect);
    TransformEffect.Invalidate();FinishedHandle.Reset();Reassembly.Reset();
    Super::EndAbility(Handle,Info,ActivationInfo,bReplicate,bCancelled);
}
