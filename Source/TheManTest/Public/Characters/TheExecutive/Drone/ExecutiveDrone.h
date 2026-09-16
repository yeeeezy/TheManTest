#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "ExecutiveDrone.generated.h"
class USphereComponent;
class USkeletalMeshComponent;
class UExecutiveDroneMovementComponent;
class UBehaviorTree;

UCLASS(Blueprintable)
class THEMANTEST_API AExecutiveDrone : public APawn
{
 GENERATED_BODY()
public:
 AExecutiveDrone();
 virtual void BeginPlay() override;
 virtual void Tick(float DeltaSeconds) override;
 virtual void EndPlay(const EEndPlayReason::Type Reason) override;
 virtual UPawnMovementComponent* GetMovementComponent() const override;
 void InitializeCompanion(AActor* InLeader, bool bPresentation);
 void UpdateFollowGoal();
 FVector GetAnchor() const;
 UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category="Drone") TObjectPtr<USphereComponent> Collision;
 UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category="Drone") TObjectPtr<USkeletalMeshComponent> DroneMesh;
 UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category="Drone") TObjectPtr<UExecutiveDroneMovementComponent> Flight;
 UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category="Drone|AI") TObjectPtr<UBehaviorTree> FollowTree;
 UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Drone|Follow") FVector FollowOffset = FVector(-130,130,100);
 UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Drone|Lobby") FVector LobbyOffset = FVector(85,15,180);
 UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Drone|Lobby") float TurnInterval = 12.f;
 UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Drone|Lobby") float TurnDuration = 5.f;
 UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Drone|Lobby") float LobbyYawOffset = 90.f;
 UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Drone|Follow") float TurnSpeed = 110.f;
 UPROPERTY(VisibleInstanceOnly,BlueprintReadOnly,Category="Drone") bool bLobbyPresentation = false;
 UPROPERTY(VisibleInstanceOnly,BlueprintReadOnly,Category="Drone") float YawRate = 0.f;
 UPROPERTY(VisibleInstanceOnly,BlueprintReadOnly,Category="Drone") int32 CompletedLobbyTurns = 0;
 UPROPERTY(VisibleInstanceOnly,BlueprintReadOnly,Category="Drone") FVector FlightGoal;
 UFUNCTION(BlueprintPure,Category="Drone") AActor* GetLeader() const { return Leader.Get(); }
private:
 UPROPERTY(Transient) TWeakObjectPtr<AActor> Leader;
 TArray<FVector> Breadcrumbs;
 float Age = 0.f;
 float LastYaw = 0.f;
 bool HasClearPath(const FVector& Target) const;
};
