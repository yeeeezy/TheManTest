#include "Weapons/_Shared/Firearms/Firearm.h"
#include "Components/PointLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Weapons/_Shared/Firearms/FirearmAnimInstance.h"
#include "Characters/CharacterBase/FPSCharacterBase/FPSCharacterBase.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystemComponent.h"
#include "Abilities/GameplayAbility.h"
#include "Animation/AnimInstance.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "NiagaraSystem.h"
#include "TimerManager.h"
#include "UObject/ConstructorHelpers.h"

AFirearm::AFirearm()
{
	StaticMeshOverlay = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshOverlay"));
	StaticMeshOverlay->SetupAttachment(GetStaticMesh());
	StaticMeshOverlay->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	StaticMeshOverlay->SetGenerateOverlapEvents(false);
	StaticMeshOverlay->SetCastShadow(false);

	MuzzleFlashLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("MuzzleFlashLight"));
	MuzzleFlashLight->SetupAttachment(RootComponent);
	MuzzleFlashLight->SetMobility(EComponentMobility::Movable);
	MuzzleFlashLight->SetCastShadows(true);
	MuzzleFlashLight->SetIntensity(0.f);
	MuzzleFlashLight->SetVisibility(false);

	static ConstructorHelpers::FObjectFinder<UNiagaraSystem> DefaultPlayerMuzzle(
		TEXT("/Game/Weapons/RepairGun/Effects/Muzzle/Systems/NS_RepairGun_Muzzle.NS_RepairGun_Muzzle"));
	if (DefaultPlayerMuzzle.Succeeded())
	{
		MuzzleEffect = DefaultPlayerMuzzle.Object;
		MuzzleEffectScale = FVector(0.85f);
	}
}

void AFirearm::BeginPlay()
{
	Super::BeginPlay();
	MagazineCapacity = FMath::Max(1, MagazineCapacity);
	SpareMagazineCount = FMath::Max(0, SpareMagazineCount);
	CurrentAmmo = MagazineCapacity;
	OnAmmoChanged.Broadcast(CurrentAmmo, MagazineCapacity, SpareMagazineCount);
	StopMuzzleFlashLight();
}

void AFirearm::PlayMuzzleFlashLight()
{
	UWorld* World = GetWorld();
	if (!bEnableMuzzleFlashLight || !MuzzleFlashLight || !World ||
		MuzzleFlashLightIntensity <= 0.f || MuzzleFlashLightAttenuationRadius <= 0.f)
	{
		return;
	}

	const FTransform MuzzleTransform = GetMuzzleWorldTransform();
	MuzzleFlashLight->SetWorldLocationAndRotation(
		MuzzleTransform.GetLocation(), MuzzleTransform.Rotator());
	MuzzleFlashLight->SetLightColor(MuzzleFlashLightColor);
	MuzzleFlashLight->SetAttenuationRadius(MuzzleFlashLightAttenuationRadius);
	MuzzleFlashLight->SetIntensity(MuzzleFlashLightIntensity);
	MuzzleFlashLight->SetVisibility(true);

	MuzzleFlashLightStartTime = World->GetTimeSeconds();
	World->GetTimerManager().ClearTimer(MuzzleFlashLightTimerHandle);
	World->GetTimerManager().SetTimer(
		MuzzleFlashLightTimerHandle,
		this,
		&AFirearm::UpdateMuzzleFlashLight,
		1.f / 60.f,
		true);
}

void AFirearm::UpdateMuzzleFlashLight()
{
	UWorld* World = GetWorld();
	if (!World || !MuzzleFlashLight)
	{
		StopMuzzleFlashLight();
		return;
	}

	const float Duration = FMath::Max(MuzzleFlashLightDuration, 0.01f);
	const float Elapsed = World->GetTimeSeconds() - MuzzleFlashLightStartTime;
	const float FadeAlpha = 1.f - FMath::Clamp(Elapsed / Duration, 0.f, 1.f);
	MuzzleFlashLight->SetIntensity(MuzzleFlashLightIntensity * FadeAlpha * FadeAlpha);

	if (FadeAlpha <= 0.f)
	{
		StopMuzzleFlashLight();
	}
}

void AFirearm::StopMuzzleFlashLight()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(MuzzleFlashLightTimerHandle);
	}

	if (MuzzleFlashLight)
	{
		MuzzleFlashLight->SetIntensity(0.f);
		MuzzleFlashLight->SetVisibility(false);
	}
}

bool AFirearm::ConsumeRound()
{
	if (!CanFire())
	{
		return false;
	}

	--CurrentAmmo;
	OnAmmoChanged.Broadcast(CurrentAmmo, MagazineCapacity, SpareMagazineCount);
	return true;
}

bool AFirearm::ReloadMagazine()
{
	if (!CanReload())
	{
		return false;
	}

	--SpareMagazineCount;
	CurrentAmmo = MagazineCapacity;
	OnAmmoChanged.Broadcast(CurrentAmmo, MagazineCapacity, SpareMagazineCount);
	return true;
}

FTransform AFirearm::GetMuzzleWorldTransform() const
{
	if (const USkeletalMeshComponent* Mesh = GetSkeletalMesh();
		Mesh && Mesh->GetSkeletalMeshAsset() && MuzzleSocketName != NAME_None && Mesh->DoesSocketExist(MuzzleSocketName))
	{
		return Mesh->GetSocketTransform(MuzzleSocketName);
	}

	if (const UStaticMeshComponent* Mesh = GetStaticMesh();
		Mesh && Mesh->GetStaticMesh() && MuzzleSocketName != NAME_None && Mesh->DoesSocketExist(MuzzleSocketName))
	{
		return Mesh->GetSocketTransform(MuzzleSocketName);
	}

	return MuzzleLocalTransform * GetActorTransform();
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
	StopMuzzleFlashLight();

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

	if (ReloadAbilityClass && !ReloadHandle.IsValid())
	{
		ReloadHandle = ASC->GiveAbility(FGameplayAbilitySpec(ReloadAbilityClass, 1));
		UE_LOG(LogTemp, Log, TEXT("[Firearm] Reload ability granted."));
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
	if (ReloadHandle.IsValid())
	{
		ASC->ClearAbility(ReloadHandle);
		ReloadHandle = FGameplayAbilitySpecHandle();
	}

	GrantedASC = nullptr;
}
