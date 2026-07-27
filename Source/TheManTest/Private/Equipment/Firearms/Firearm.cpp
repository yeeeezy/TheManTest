#include "Equipment/Firearms/Firearm.h"
#include "Equipment/Firearms/FirearmAnimInstance.h"
#include "Characters/FPSCharacterBase/FPSCharacterBase.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystemComponent.h"
#include "Abilities/GameplayAbility.h"
#include "Animation/AnimInstance.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/Engine.h"

AFirearm::AFirearm()
{
}

static void SetLinkedFirearmAimSource(USkeletalMeshComponent* Mesh, TSubclassOf<UAnimInstance> AnimLayerClass, const FTransform& AimSourceLocalTransform)
{
	if (!Mesh || !AnimLayerClass)
	{
		return;
	}

	if (UAnimInstance* AnimInst = Mesh->GetAnimInstance())
	{
		UFirearmAnimInstance* FirearmAnim = Cast<UFirearmAnimInstance>(
			AnimInst->GetLinkedAnimLayerInstanceByClass(AnimLayerClass));
		if (FirearmAnim)
		{
			FirearmAnim->SetAimSourceLocalTransform(AimSourceLocalTransform);
		}
	}
}

void AFirearm::Equip(AActor* NewOwner)
{
	Super::Equip(NewOwner);

	AFPSCharacterBase* FPSChar = Cast<AFPSCharacterBase>(NewOwner);
	if (!FPSChar) { return; }

	SetLinkedFirearmAimSource(FPSChar->GetArmsMesh(), EquipmentAnimLayerClass, MuzzleLocalTransform);
	SetLinkedFirearmAimSource(FPSChar->GetMesh(), EquipmentAnimLayerClass, MuzzleLocalTransform);

	if (GEngine && EquipmentAnimLayerClass)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green,
			TEXT("[Firearm] Weapon anim layer linked on arms/body."));
	}

	// --- FEAT-009：装备时授予开火技能（PossessedBy 已运行则立即生效） ---
	GrantAbilities(FPSChar->GetAbilitySystemComponent());
}

void AFirearm::Unequip()
{
	// 回收技能
	if (AActor* CurrentOwner = GetOwner())
	{
		if (AFPSCharacterBase* FPSChar = Cast<AFPSCharacterBase>(CurrentOwner))
		{
			RevokeAbilities(FPSChar->GetAbilitySystemComponent());

			// FP viewmodel layer is unlinked by AEquipmentBase::Unequip().
		}
	}

	Super::Unequip();
}

void AFirearm::GrantAbilities(UAbilitySystemComponent* ASC)
{
	if (!ASC) { return; }

	// 缓存授予所用的 ASC，供 Unequip 在角色销毁（PlayerState 已置空）时可靠回收
	GrantedASC = ASC;

	if (PrimaryFireAbilityClass && !PrimaryFireHandle.IsValid())
	{
		PrimaryFireHandle = ASC->GiveAbility(FGameplayAbilitySpec(PrimaryFireAbilityClass, 1));
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Cyan,
			TEXT("[Firearm] PrimaryFire ability granted."));
	}

	if (SecondaryFireAbilityClass && !SecondaryFireHandle.IsValid())
	{
		SecondaryFireHandle = ASC->GiveAbility(FGameplayAbilitySpec(SecondaryFireAbilityClass, 1));
	}
}

void AFirearm::RevokeAbilities(UAbilitySystemComponent* ASC)
{
	// 切换角色时传入的 ASC 多半为 null（旧角色已被 UnPossess、PlayerState 置空），
	// 回退到授予时缓存的 ASC，确保技能规格一定被回收，避免泄漏累积。
	if (!ASC) { ASC = GrantedASC.Get(); }
	if (!ASC) { return; }

	if (PrimaryFireHandle.IsValid())
	{
		ASC->ClearAbility(PrimaryFireHandle);
		PrimaryFireHandle = FGameplayAbilitySpecHandle();
	}

	if (SecondaryFireHandle.IsValid())
	{
		ASC->ClearAbility(SecondaryFireHandle);
		SecondaryFireHandle = FGameplayAbilitySpecHandle();
	}

	GrantedASC = nullptr;
}
