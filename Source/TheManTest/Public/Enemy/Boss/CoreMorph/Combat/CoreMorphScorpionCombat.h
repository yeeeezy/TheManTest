#pragma once
#include "Components/ActorComponent.h"
#include "CoreMorphScorpionCombat.generated.h"
class ACoreMorphBoss;
class ACoreMorphAIController;
class UCoreMorphScorpionMovement;
class UCoreMorphScorpionLayout;
class UBehaviorTree;
UENUM(BlueprintType)
enum class ECoreMorphScorpionAction : uint8 {Idle,Approach,Face,Windup,Thrust,Recover};
UCLASS(ClassGroup=(AI),meta=(BlueprintSpawnableComponent))
class THEMANTEST_API UCoreMorphScorpionCombat : public UActorComponent
{
    GENERATED_BODY()
public:
    UCoreMorphScorpionCombat();
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type Reason) override;
    virtual void TickComponent(float Dt,ELevelTick Tick,FActorComponentTickFunction* Function) override;
    UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Combat") bool bEnabled=false;
    UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Combat") TObjectPtr<UBehaviorTree> BehaviorTree;

    UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Combat") float WakeDelay=2.5f;
    UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Combat",meta=(ClampMin="0.05")) float WindupDuration=2.f;
    UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Combat",meta=(ClampMin="0.05")) float ThrustDuration=.28f;
    UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Combat",meta=(ClampMin="0.05")) float RecoverDuration=1.1f;
    UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Combat",meta=(ClampMin="0.05")) float CooldownDuration=2.1f;
    UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Combat") float StrikeDamage=25.f;
    UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Combat") float TipRadius=65.f;
    UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Combat",meta=(ClampMin="100")) float BlastRadius=1000.f;
    float LockedBlastRadius=1000.f;
    FHitResult LockedGroundHit;
    UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category="Combat") ECoreMorphScorpionAction Action=ECoreMorphScorpionAction::Idle;
    UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category="Combat") FVector LockedTarget=FVector::ZeroVector;
    UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category="Combat") FVector TipPosition=FVector::ZeroVector;
    UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category="Combat") int32 StrikeCount=0;
    UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category="Combat") int32 HitCount=0;
    UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category="Combat") int32 BlockedStrikes=0;
    UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category="Combat") float ActionTime=0;
    UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category="Combat") TObjectPtr<AActor> Target;

    UPROPERTY(Transient) TObjectPtr<UCoreMorphScorpionMovement> Motor;
    FSimpleMulticastDelegate OnStrikeStageFinished;
    FSimpleMulticastDelegate OnStrikeInvalidated;
    DECLARE_MULTICAST_DELEGATE_OneParam(FStrikeContact,const FHitResult&);
    FStrikeContact OnStrikeContact;
    void SetPaused(bool Value);
    void ResetStrike();
    bool IsAttacking() const;
    bool IsDrivingPose() const;
    bool IsReady() const;
    bool IsPaused() const;
    bool CanStrike() const;
    bool NeedsApproach() const;
    bool StartAction(ECoreMorphScorpionAction Next);
    bool TickMovement(ECoreMorphScorpionAction Which,float Dt);
    bool ActionFinished() const;
    void AbortAction();
    void ResetCombat();
    FVector BodyPosition() const;
    FString StatusLabel() const;
    const TArray<FVector>& GetTailNodes() const {return TailNodes;}
    const TArray<float>& GetTailLengths() const {return TailLengths;}
    float GetCombatClock() const {return CombatClock;}
private:
    UPROPERTY(Transient) TObjectPtr<ACoreMorphBoss> Boss;
    TArray<int32> Bindings;
    TArray<FVector> TailRest,TailNodes;
    TArray<float> TailLengths;
    TArray<FTransform> TailFrames;
    FVector ActionStartTip=FVector::ZeroVector,WindupTip=FVector::ZeroVector;
    FVector PreviousTip=FVector::ZeroVector;
    FVector ContactTip=FVector::ZeroVector;
    float WakeTime=0,CombatClock=0,BlockedTime=0;
    bool bStrikeResolved=false,bPaused=false,bInitialized=false;
    bool bStageNotified=false;
    bool InitializeCombat();
    void AdvanceTail(float Dt);
    void ApplyPose();
    void SweepTip(const FVector& From,const FVector& To);
};
