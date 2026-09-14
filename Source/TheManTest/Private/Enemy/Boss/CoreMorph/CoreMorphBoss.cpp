#include "Enemy/Boss/CoreMorph/CoreMorphBoss.h"
#include "Enemy/Boss/CoreMorph/Combat/CoreMorphMissileCombat.h"
#include "Enemy/Boss/CoreMorph/Effects/CoreMorphMissileEffects.h"
#include "Enemy/Boss/CoreMorph/GAS/Abilities/GA_CoreMorphMissileBarrage.h"
#include "Enemy/Boss/CoreMorph/Effects/CoreMorphTailEffects.h"
#include "Enemy/Boss/CoreMorph/Movement/CoreMorphScorpionMovement.h"
#include "Enemy/Boss/CoreMorph/Combat/CoreMorphScorpionCombat.h"
#include "Enemy/Boss/CoreMorph/AI/CoreMorphAIController.h"
#include "Enemy/Boss/CoreMorph/GAS/Abilities/GA_CoreMorphTailStrike.h"
#include "Core/_Shared/GAS/TheManGameplayTags.h"
#include "Enemy/Boss/CoreMorph/Movement/CoreMorphFlightComponent.h"
#include "Enemy/Boss/CoreMorph/GAS/Abilities/GA_CoreMorphFlight.h"
#include "Enemy/Boss/CoreMorph/GAS/Effects/GE_CoreMorphManta.h"
#include "Enemy/Boss/CoreMorph/GAS/Effects/GE_CoreMorphScorpion.h"
#include "Enemy/Boss/CoreMorph/GAS/Abilities/GA_CoreMorphReassemble.h"
#include "Enemy/Boss/CoreMorph/Transformation/CoreMorphReassemblyComponent.h"
#include "AbilitySystemComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AIController.h"

ACoreMorphBoss::ACoreMorphBoss()
{
	TailEffects=CreateDefaultSubobject<UCoreMorphTailEffects>(TEXT("TailEffects"));
	Flight = CreateDefaultSubobject<UCoreMorphFlightComponent>(TEXT("CoreMorphFlight"));
	Reassembly = CreateDefaultSubobject<UCoreMorphReassemblyComponent>(TEXT("CoreMorphReassembly"));
	DefaultAbilities.Add(UGA_CoreMorphFlight::StaticClass());
	DefaultAbilities.Add(UGA_CoreMorphReassemble::StaticClass());
	ScorpionMovement=CreateDefaultSubobject<UCoreMorphScorpionMovement>(TEXT("ScorpionMovement"));
	ScorpionCombat=CreateDefaultSubobject<UCoreMorphScorpionCombat>(TEXT("ScorpionCombat"));
	MissileCombat=CreateDefaultSubobject<UCoreMorphMissileCombat>(TEXT("MissileCombat"));
	MissileEffects=CreateDefaultSubobject<UCoreMorphMissileEffects>(TEXT("MissileEffects"));
	auto& Phase=PhaseSkillSets.AddDefaulted_GetRef();Phase.NearAbilities.Add(UGA_CoreMorphTailStrike::StaticClass());Phase.FarAbilities.Add(UGA_CoreMorphMissileBarrage::StaticClass());
	AIControllerClass = ACoreMorphAIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
	bUseControllerRotationYaw = false;
	GetCharacterMovement()->DefaultLandMovementMode = MOVE_None;
	GetCharacterMovement()->GravityScale = 0;
	GetCapsuleComponent()->InitCapsuleSize(120, 160);
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetVisibility(false);
	ProjectileHitImpulse = ProjectileKillKnockbackSpeed = ProjectileKillUpwardSpeed = 0;
	// The metallic creature must never use humanoid blood/pain feedback.
	HitReactionCueTag = FGameplayTag();
	EnemyHealthBarComponent->SetRelativeLocation(FVector(0, 0, 500));
}

void ACoreMorphBoss::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	Flight->RebuildAssembly();
}

void ACoreMorphBoss::BeginPlay()
{
	Super::BeginPlay();
	GetCharacterMovement()->DisableMovement();
	GetCharacterMovement()->SetComponentTickEnabled(false);
	SetForm(ECoreMorphForm::Manta);
}

void ACoreMorphBoss::EndPlay(const EEndPlayReason::Type Reason)
{
	AbilitySystemComponent->CancelAllAbilities();
	ScorpionCombat->ResetCombat();
	TailEffects->Shutdown();
	MissileCombat->StopSalvo();MissileEffects->EndCue();
	Reassembly->Shutdown();
	Flight->Shutdown();
	AbilitySystemComponent->RemoveActiveGameplayEffect(FormEffect);
	Super::EndPlay(Reason);
}

void ACoreMorphBoss::OnDeath()
{
	if (IsDead()) return;
	// No skeletal asset/PhysicsAsset is assigned. Base supplies the shared terminal lifecycle.
	Super::OnDeath();
	ScorpionCombat->ResetCombat();
	TailEffects->Shutdown();
	MissileCombat->StopSalvo();MissileEffects->EndCue();
	ScorpionCombat->SetComponentTickEnabled(false);
	Reassembly->Shutdown();
	Flight->Shutdown();
	AbilitySystemComponent->RemoveActiveEffectsWithGrantedTags(FGameplayTagContainer(TAG_State_CoreMorph_TailCooldown));
	AbilitySystemComponent->RemoveActiveEffectsWithGrantedTags(FGameplayTagContainer(TAG_State_CoreMorph_MissileCooldown));
	LastThreat = nullptr;
	AbilitySystemComponent->RemoveActiveGameplayEffect(FormEffect);
}

void ACoreMorphBoss::ReactToProjectileHit(AActor* HitInstigator)
{
	if (!IsDead() && IsValid(HitInstigator) && HitInstigator != this) LastThreat = HitInstigator;
	// Receiving damage never snaps the heading or interrupts this creature's choreography.
}

bool ACoreMorphBoss::StartFlightPreview()
{
	return !IsDead() && CurrentForm == ECoreMorphForm::Manta &&
		AbilitySystemComponent->TryActivateAbilityByClass(UGA_CoreMorphFlight::StaticClass());
}

void ACoreMorphBoss::ResetFlightPreview()
{
	if (IsDead()) return;
	ScorpionCombat->bEnabled=false;
	ScorpionCombat->ResetCombat();
	TailEffects->Shutdown();
	MissileCombat->StopSalvo();MissileEffects->EndCue();
	AbilitySystemComponent->CancelAbilities(nullptr, nullptr);
	AbilitySystemComponent->RemoveActiveEffectsWithGrantedTags(FGameplayTagContainer(TAG_State_CoreMorph_TailCooldown));
	AbilitySystemComponent->RemoveActiveEffectsWithGrantedTags(FGameplayTagContainer(TAG_State_CoreMorph_MissileCooldown));
	Reassembly->ResetPreview();
	SetForm(ECoreMorphForm::Manta);
	Flight->ResetPreview();
	// A review replay does not reset health, combat phase, or grant another ability.
}

bool ACoreMorphBoss::StartReassembly()
{
	return !IsDead() && AbilitySystemComponent->TryActivateAbilityByClass(UGA_CoreMorphReassemble::StaticClass());
}
void ACoreMorphBoss::SetForm(ECoreMorphForm Form)
{
	if(IsDead())return;
	if(CurrentForm==Form && FormEffect.IsValid() && AbilitySystemComponent->GetActiveGameplayEffect(FormEffect))return;
	AbilitySystemComponent->RemoveActiveGameplayEffect(FormEffect);
	CurrentForm=Form;
	const UGameplayEffect* Effect=Form==ECoreMorphForm::Manta?static_cast<const UGameplayEffect*>(GetDefault<UGE_CoreMorphManta>()):GetDefault<UGE_CoreMorphScorpion>();
	FormEffect=AbilitySystemComponent->ApplyGameplayEffectToSelf(Effect,1.f,AbilitySystemComponent->MakeEffectContext());
}

void ACoreMorphBoss::AimAtTarget(AActor* Target){if(!IsDead() && IsValid(Target)){ScorpionCombat->Target=Target;MissileCombat->Target=Target;}}
