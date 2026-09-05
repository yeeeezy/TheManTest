#pragma once
#include "CoreMinimal.h"
#include "Core/_Shared/Feedback/BulletTimeSubsystem.h"
#include "Physics/Experimental/ChaosEventType.h"
#include "ExplosionOutcomeSubsystem.generated.h"

class UGeometryCollectionComponent;

// Shared by all collections affected by one detonation; fragments cannot restart its feedback.
struct FExplosionOutcome
{
 FVector Origin;
 FBulletTimeSettings Settings;
 bool bResolved=false;
};

/** Short-lived observation of the physics results of explosion strain commands. */
UCLASS()
class THEMANTEST_API UExplosionOutcomeSubsystem : public UTickableWorldSubsystem
{
 GENERATED_BODY()
public:
 void Watch(UGeometryCollectionComponent* Collection, float Radius, const TSharedRef<FExplosionOutcome>& Outcome);
 virtual void Tick(float DeltaTime) override;
 virtual bool IsTickable() const override { return !IsTemplate() && Pending.Num()>0; }
 virtual TStatId GetStatId() const override { RETURN_QUICK_DECLARE_CYCLE_STAT(UExplosionOutcomeSubsystem,STATGROUP_Tickables); }
 virtual void OnWorldEndPlay(UWorld& World) override;
 virtual void Deinitialize() override;
protected:
 virtual bool DoesSupportWorldType(EWorldType::Type Type) const override { return Type==EWorldType::Game||Type==EWorldType::PIE; }
private:
 struct FPending
 {
  TWeakObjectPtr<UGeometryCollectionComponent> Collection;
  TSharedPtr<FExplosionOutcome> Outcome;
  float Radius=0;
  double Expires=0;
 };
 TArray<FPending> Pending;
 TMap<TWeakObjectPtr<UGeometryCollectionComponent>,bool> OriginalNotify;
 UFUNCTION() void OnBreak(const FChaosBreakEvent& Event);
 void Prune();
 void Clear();
};
