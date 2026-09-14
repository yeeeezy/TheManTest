#pragma once
#include "Components/ActorComponent.h"
#include "Enemy/Boss/CoreMorph/Data/CoreMorphVisualLayout.h"
#include "Enemy/Boss/CoreMorph/Movement/CoreMorphFlightPath.h"
#include "CoreMorphReassemblyComponent.generated.h"

class UInstancedStaticMeshComponent;
class UPointLightComponent;
class UStaticMeshComponent;
class ACoreMorphBoss;
DECLARE_MULTICAST_DELEGATE(FCoreMorphReassembled);

// One boss owns both assemblies. The GA owns the transaction and the Cue owns
// the transient visual pools; source stream/build mathematics are retained.
UCLASS(ClassGroup=(CoreMorph))
class THEMANTEST_API UCoreMorphReassemblyComponent : public UActorComponent
{
    GENERATED_BODY()
public:
    UCoreMorphReassemblyComponent();
    UPROPERTY(EditAnywhere, Category="CoreMorph") TObjectPtr<UCoreMorphVisualLayout> Layout;
    UPROPERTY(EditInstanceOnly, Category="CoreMorph") TObjectPtr<AActor> LandingReference;
    bool CanStart() const;
    bool Start();
    void Cancel();
    void ResetPreview();
    void Shutdown();
    void BeginCue();
    void EndCue();
    void SetPaused(bool Value) { bPaused = Value; }
    bool IsPaused() const { return bPaused; }
    bool IsMorphing() const { return bMorphing; }
    bool HasCue() const { return Fragments != nullptr; }
    float GetSeconds() const { return Elapsed - 13.4f; }
    FVector GetFocus() const;
    FBox GetStreamBounds() const {return StreamBounds.IsValid?StreamBounds.TransformBy(FixedTransform):FBox(ForceInit);}
    const TArray<TObjectPtr<UStaticMeshComponent>>& GetPieces() const { return Components; }
    const TArray<float>& GetReveals() const { return Reveals; }
    FCoreMorphReassembled OnReassembled;
    virtual void TickComponent(float Dt, ELevelTick Tick, FActorComponentTickFunction* Function) override;
    virtual void EndPlay(const EEndPlayReason::Type Reason) override;
private:
    ACoreMorphBoss* Boss() const;
    UPROPERTY(Transient) TArray<TObjectPtr<UStaticMeshComponent>> Components;
    UPROPERTY(Transient) TObjectPtr<UInstancedStaticMeshComponent> Fragments;
    UPROPERTY(Transient) TObjectPtr<UInstancedStaticMeshComponent> Sparks;
    UPROPERTY(Transient) TObjectPtr<UInstancedStaticMeshComponent> ImpactDebris;
    UPROPERTY(Transient) TObjectPtr<UInstancedStaticMeshComponent> ImpactGlow;
    UPROPERTY(Transient) TObjectPtr<UInstancedStaticMeshComponent> ImpactDust;
    UPROPERTY(Transient) TObjectPtr<UStaticMeshComponent> ImpactFlash;
    UPROPERTY(Transient) TObjectPtr<UPointLightComponent> ImpactLight;
    UPROPERTY(Transient) TObjectPtr<USceneComponent> EffectRoot;
    TArray<FTransform> Live, Captured;
    TArray<float> Reveals;
    TArray<int32> FormIndices[2];
    FTransform FixedTransform, CapturedRoot;
    FVector ReleaseVelocity = FVector::ZeroVector;
    bool bMorphing=false, bPaused=false;
    int32 CurrentForm=0;
    float Progress=0, IdleTime=0, Elapsed=13.4f;
    // These are source construction timing parameters, independent of combat phase.
    float Duration=8.5f, AscentExtension=8.f, AirflowDuration=0, CrashDuration=.65f, FeedDuration=1.15f, ConstructionExtension=.3f;
    float GetMorphDuration() const { return 18.6f; }
    float GetFormScale(int32 Form) const { return Form==0?1.5f:1.f; }
    FVector GetFormOffset(int32 Form) const { return Form==0?FVector(-16000,0,1600):FVector(2400,0,0); }
    FTransform SourcePose(int32 Index,float Time) const;
    float Schedule(float Choreography) const;
    float ImpactTime() const {return Schedule(.70f)-FeedDuration/GetMorphDuration();}
    float ParticleReleaseTime() const {return 13.4f/GetMorphDuration();}
    float PeelStart(const FCoreMorphVisualPiece& Piece) const;
    float BuildStart(const FCoreMorphVisualPiece& Piece) const;
    float BuildSpan(const FCoreMorphVisualPiece& Piece) const;
    struct FFlowParticle {FVector A,D,Impact,Axis,LaneStart,LaneEnd,ReleaseTangent,StreamStart,StreamEnd; TArray<FVector> BuildPath; float Birth=0,Land=1,Arrival=1,Speed=1; int32 Strand=0;};
    struct FBuildGuide {FVector Axis=FVector::ForwardVector; float Min=0,Span=1; TArray<FVector> Path;};
    TArray<FFlowParticle> FlowParticles;
    TArray<FBuildGuide> BuildGuides;
    FVector GroundPoint=FVector(2400,0,-785.72), FlowFocus=FVector::ZeroVector;
    FBox StreamBounds=FBox(ForceInit);
    TArray<float> SandGroundHeights;
    void PrepareBuildGuides();
    void ResolveGround();
    FVector ProbeGround(FVector Position) const;
    int32 StreamFor(const FCoreMorphVisualPiece& Piece) const;
    void PrepareStreams();
    FVector FlowPoint(const FFlowParticle& Particle,float Travel,int32 Seed) const;
    FVector CoilOffset(const FFlowParticle& Particle,float Travel) const;
    FVector GroundFlowPoint(const FFlowParticle& Particle,float Travel) const;
    FVector ParticlePoint(const FFlowParticle& Particle,float Time,int32 Seed) const;
    void UpdateImpact();
    void UpdatePose();
};
