#include "Enemy/Boss/CoreMorph/Review/CoreMorphFlightReview.h"
#include "Enemy/Boss/CoreMorph/Combat/CoreMorphMissileCombat.h"
#include "Enemy/Boss/CoreMorph/GAS/Abilities/GA_CoreMorphMissileBarrage.h"
#include "Enemy/Boss/CoreMorph/CoreMorphBoss.h"
#include "Enemy/Boss/CoreMorph/Movement/CoreMorphFlightComponent.h"
#include "Enemy/Boss/CoreMorph/Movement/CoreMorphFlightRoute.h"
#include "Enemy/Boss/CoreMorph/Transformation/CoreMorphReassemblyComponent.h"
#include "Camera/CameraComponent.h"
#include "Enemy/Boss/CoreMorph/Combat/CoreMorphScorpionCombat.h"
#include "Enemy/Boss/CoreMorph/GAS/Abilities/GA_CoreMorphTailStrike.h"
#include "AbilitySystemComponent.h"
#include "Components/InputComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "InputCoreTypes.h"

ACoreMorphFlightReview::ACoreMorphFlightReview()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickGroup = TG_PostUpdateWork;
	GetCameraComponent()->SetFieldOfView(60.f);
	GetCameraComponent()->bConstrainAspectRatio = false;
	auto& PP = GetCameraComponent()->PostProcessSettings;
	PP.bOverride_AutoExposureMethod = true;
	PP.AutoExposureMethod = AEM_Manual;
	PP.bOverride_AutoExposureApplyPhysicalCameraExposure = true;
	PP.AutoExposureApplyPhysicalCameraExposure = false;
	PP.bOverride_AutoExposureBias = true;
	PP.AutoExposureBias = 0;
}

void ACoreMorphFlightReview::BeginPlay()
{
	Super::BeginPlay();
	if (auto* PC = GetWorld()->GetFirstPlayerController())
	{
		EnableInput(PC);
		InputComponent->BindKey(EKeys::V, IE_Pressed, this, &ThisClass::PlayFlight);
		InputComponent->BindKey(EKeys::R, IE_Pressed, this, &ThisClass::ResetFlight);
		InputComponent->BindKey(EKeys::P, IE_Pressed, this, &ThisClass::PauseFlight);
		InputComponent->BindKey(EKeys::F, IE_Pressed, this, &ThisClass::ToggleCamera);
		InputComponent->BindKey(EKeys::Q, IE_Pressed, this, &ThisClass::SlowRoll);
		InputComponent->BindKey(EKeys::E, IE_Pressed, this, &ThisClass::FastRoll);
		InputComponent->BindKey(EKeys::M, IE_Pressed, this, &ThisClass::Reassemble);
		if (!Routes.IsEmpty() || bScorpionReview)
		{
			InputComponent->BindKey(EKeys::One, IE_Pressed, this, &ThisClass::RouteOne);
			InputComponent->BindKey(EKeys::Two, IE_Pressed, this, &ThisClass::RouteTwo);
			InputComponent->BindKey(EKeys::Three, IE_Pressed, this, &ThisClass::RouteThree);
			bStartFirstRoute = !bScorpionReview;
		}
		if(bScorpionReview || bMantaReview){InputComponent->BindKey(EKeys::C,IE_Pressed,this,&ThisClass::CancelCombatStrike);InputComponent->BindKey(EKeys::T,IE_Pressed,this,&ThisClass::Strike);}
		PC->SetViewTarget(this);
	}
}

void ACoreMorphFlightReview::Tick(float Dt)
{
	Super::Tick(Dt);
	if (!IsValid(Boss)) return;
	if(bPendingReassembly && !Boss->Flight->IsFlying() && Boss->Flight->GetFlightSeconds()>=Boss->Flight->GetReleaseSeconds())
	{bPendingReassembly=false;Boss->StartReassembly();}
	// Wait for the boss's BeginPlay to initialize and grant its GA, regardless
	// of serialized actor order in the review map.
	if (bStartFirstRoute && Boss->HasActorBegunPlay())
	{
		bStartFirstRoute = false;
		SelectRoute(0);
	}
	FVector Target, Extent;
	Boss->GetActorBounds(false, Target, Extent);
	if(Boss->Reassembly->IsMorphing() || Boss->CurrentForm==ECoreMorphForm::Scorpion)
	{
		FBox VisibleBounds(ForceInit);
		for(const auto& Piece:Boss->Reassembly->GetPieces())if(Piece && Piece->IsVisible())VisibleBounds+=Piece->Bounds.GetBox();
		if(Boss->Reassembly->IsMorphing() && Boss->Reassembly->GetStreamBounds().IsValid)VisibleBounds+=Boss->Reassembly->GetStreamBounds();
		if(VisibleBounds.IsValid){Target=VisibleBounds.GetCenter();Extent=VisibleBounds.GetExtent();}
	}
	int32 Width = 16, Height = 9;
	if (auto* PC = GetWorld()->GetFirstPlayerController()) PC->GetViewportSize(Width, Height);
	const float Aspect = Height > 0 ? float(Width) / Height : 16.f / 9.f;
	const float HalfVerticalFOV = FMath::Atan(FMath::Tan(FMath::DegreesToRadians(30.f)) / FMath::Max(1.f, Aspect));
	const float Distance = FMath::Max(14000.f, float(Extent.Size()) * 1.15f / FMath::Sin(HalfVerticalFOV));
	SetActorLocation(Target + FVector(-8500, -11500, 5000).GetSafeNormal() * Distance);
	SetActorRotation((Target - GetActorLocation()).Rotation());
	if (GEngine)
	{
		if(bMantaReview)GEngine->AddOnScreenDebugMessage(uint64(GetUniqueID())+1,0.f,FColor::Yellow,TEXT("MANTA BARRAGE | V: Fly | T: Ranged GA | C: Cancel barrage | M: Morph | P: Pause | R: Reset"));
		if(bScorpionReview)GEngine->AddOnScreenDebugMessage(uint64(GetUniqueID())+1,0.f,FColor::Yellow,TEXT("SCORPION REVIEW | V: AI flight + morph | M: Morph now | 1/2/3: Move target | T: Strike GA | C: Cancel strike"));
		const FString RouteText = Routes.IsValidIndex(SelectedRoute)
			? FString::Printf(TEXT("ROUTE %d: %s | 1/2/3: Switch & Play\n"), SelectedRoute + 1, *Routes[SelectedRoute].Label) : FString();
		GEngine->AddOnScreenDebugMessage(uint64(GetUniqueID()), 0.f, FColor::White,
			RouteText + FString::Printf(TEXT("Q: Slow roll | E: Fast roll | M: Reassemble\nREVIEW | V: Play | R: Reset | P: Pause | F: Camera | Flight %.2f s | Morph %.2f s"), Boss->Flight->GetFlightSeconds(),Boss->Reassembly->GetSeconds()));
	}
}

bool ACoreMorphFlightReview::SelectRoute(int32 Index)
{
	if (!IsValid(Boss) || Boss->IsDead() || !Boss->HasActorBegunPlay()
		|| !Routes.IsValidIndex(Index) || !IsValid(Routes[Index].Route)) return false;
	Boss->ResetFlightPreview();
	Boss->Flight->FlightRoute = Routes[Index].Route;
	Boss->Flight->RouteSpeed = Routes[Index].Speed;
	SelectedRoute = Index;
	bFollow = true;
	if (auto* PC = GetWorld()->GetFirstPlayerController()) PC->SetViewTarget(this);
	return Boss->StartFlightPreview();
}

void ACoreMorphFlightReview::EndPlay(const EEndPlayReason::Type Reason)
{
	if (auto* PC = GetWorld()->GetFirstPlayerController()) DisableInput(PC);
	if (GEngine) GEngine->RemoveOnScreenDebugMessage(GetUniqueID());
	Super::EndPlay(Reason);
}

void ACoreMorphFlightReview::PlayFlight() { if(IsValid(Boss)){if(bScorpionReview){Boss->ScorpionCombat->bEnabled=true;Boss->ScorpionCombat->SetPaused(false);}else if(Boss->StartFlightPreview())bPendingReassembly=bReassemblyReview;} }
void ACoreMorphFlightReview::ResetFlight() { bPendingReassembly=false;if (IsValid(Boss)) Boss->ResetFlightPreview(); }
void ACoreMorphFlightReview::PauseFlight()
{
	if(!IsValid(Boss))return;
	if(bScorpionReview)Boss->ScorpionCombat->SetPaused(!Boss->ScorpionCombat->IsPaused());
	if(Boss->Reassembly->IsMorphing() || Boss->CurrentForm==ECoreMorphForm::Scorpion)Boss->Reassembly->SetPaused(!Boss->Reassembly->IsPaused());
	else Boss->Flight->SetPaused(!Boss->Flight->IsPaused());
}
void ACoreMorphFlightReview::Reassemble() { bPendingReassembly=false;if(IsValid(Boss)){if(bScorpionReview)Boss->ScorpionCombat->bEnabled=true;Boss->StartReassembly();} }
void ACoreMorphFlightReview::SlowRoll() { if (IsValid(Boss)) Boss->Flight->RequestRoll(false); }
void ACoreMorphFlightReview::FastRoll() { if (IsValid(Boss)) Boss->Flight->RequestRoll(true); }
void ACoreMorphFlightReview::ToggleCamera()
{
	if (auto* PC = GetWorld()->GetFirstPlayerController())
	{
		bFollow = !bFollow;
		PC->SetViewTarget(bFollow || !PC->GetPawn() ? this : static_cast<AActor*>(PC->GetPawn()));
	}
}

void ACoreMorphFlightReview::MoveCombatTarget(int32 Side)
{
 if(!IsValid(Boss) || !IsValid(Boss->ScorpionCombat->ReviewTarget))return;
 const float Angle=Side==0?0:Side==1?-65:65;
 const FVector Center=Boss->CurrentForm==ECoreMorphForm::Scorpion?Boss->GetActorLocation()-FVector(0,0,785.72):FVector(2400,0,0);
 Boss->ScorpionCombat->ReviewTarget->SetActorLocation(Center+Boss->GetActorForwardVector().RotateAngleAxis(Angle,FVector::UpVector)*6000+FVector(0,0,100));
}
void ACoreMorphFlightReview::CancelCombatStrike()
{
 if(IsValid(Boss))if(auto* S=Boss->GetAbilitySystemComponent()->FindAbilitySpecFromClass(Boss->CurrentForm==ECoreMorphForm::Manta?UGA_CoreMorphMissileBarrage::StaticClass():UGA_CoreMorphTailStrike::StaticClass()))Boss->GetAbilitySystemComponent()->CancelAbilityHandle(S->Handle);
}
void ACoreMorphFlightReview::Strike(){if(IsValid(Boss))Boss->UseRandomSkill(IsValid(Boss->ScorpionCombat->ReviewTarget)?Boss->ScorpionCombat->ReviewTarget.Get():Boss->ScorpionCombat->Target.Get(),Boss->CurrentForm==ECoreMorphForm::Manta?EEnemySkillRange::Far:EEnemySkillRange::Near);}
