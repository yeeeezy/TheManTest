#include "Enemy/Boss/CoreMorph/CoreMorphBoss.h"
#include "Enemy/Boss/CoreMorph/Movement/CoreMorphFlightComponent.h"
#include "Enemy/Boss/CoreMorph/GAS/Abilities/GA_CoreMorphFlight.h"
#include "Enemy/Boss/CoreMorph/GAS/Effects/GE_CoreMorphManta.h"
#include "AbilitySystemComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AIController.h"

ACoreMorphBoss::ACoreMorphBoss()
{
	Flight = CreateDefaultSubobject<UCoreMorphFlightComponent>(TEXT("CoreMorphFlight"));
	DefaultAbilities.Add(UGA_CoreMorphFlight::StaticClass());
	AIControllerClass = AAIController::StaticClass();
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
	FormEffect = AbilitySystemComponent->ApplyGameplayEffectToSelf(GetDefault<UGE_CoreMorphManta>(), 1.f,
		AbilitySystemComponent->MakeEffectContext());
}

void ACoreMorphBoss::EndPlay(const EEndPlayReason::Type Reason)
{
	AbilitySystemComponent->CancelAllAbilities();
	Flight->Shutdown();
	AbilitySystemComponent->RemoveActiveGameplayEffect(FormEffect);
	Super::EndPlay(Reason);
}

void ACoreMorphBoss::OnDeath()
{
	if (IsDead()) return;
	// No skeletal asset/PhysicsAsset is assigned. Base supplies the shared terminal lifecycle.
	Super::OnDeath();
	Flight->Shutdown();
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
	AbilitySystemComponent->CancelAbilities(nullptr, nullptr);
	Flight->ResetPreview();
	// A review replay does not reset health, combat phase, or grant another ability.
}
