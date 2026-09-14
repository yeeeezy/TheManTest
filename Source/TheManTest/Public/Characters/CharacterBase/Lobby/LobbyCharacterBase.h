#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LobbyCharacterBase.generated.h"

class UAnimSequence;
class USkeletalMeshComponent;
class UStaticMeshComponent;

UENUM(BlueprintType)
enum class ELobbyCharacterPose : uint8
{
	Relaxed,
	Rifle,
	Standing
};

/** Presentation actor for character selection scenes. Assets are supplied by character subclasses. */
UCLASS(Abstract, Blueprintable)
class THEMANTEST_API ALobbyCharacterBase : public AActor
{
	GENERATED_BODY()

public:
	ALobbyCharacterBase();
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, Category="Lobby|Presentation")
	void SetDisplayPose(ELobbyCharacterPose NewPose);

	/** Select a standing idle variant and enter the unarmed standing pose. */
	UFUNCTION(BlueprintCallable, Category="Lobby|Presentation")
	void SetStandingIdleIndex(int32 NewIndex);

	/** Switch between the relaxed carry and ready-to-fire lobby poses. */
	UFUNCTION(BlueprintCallable, Category="Lobby|Presentation")
	void SetWeaponReady(bool bReady);

	UFUNCTION(BlueprintPure, Category="Lobby|Presentation")
	bool IsWeaponReady() const { return DisplayPose == ELobbyCharacterPose::Rifle; }

	UFUNCTION(BlueprintPure, Category="Lobby|Presentation")
	ELobbyCharacterPose GetDisplayPose() const { return DisplayPose; }

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Lobby|Presentation")
	TObjectPtr<USkeletalMeshComponent> DisplayMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Lobby|Presentation")
	TObjectPtr<UStaticMeshComponent> DisplayWeapon;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Lobby|Presentation")
	ELobbyCharacterPose DisplayPose = ELobbyCharacterPose::Standing;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Lobby|Animation")
	TArray<TObjectPtr<UAnimSequence>> StandingAnimations;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Lobby|Animation", meta=(ClampMin="0"))
	int32 StandingIdleIndex = 0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Lobby|Animation")
	TObjectPtr<UAnimSequence> RelaxedAnimation;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Lobby|Animation")
	TObjectPtr<UAnimSequence> RifleAnimation;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Lobby|Weapon")
	FName WeaponAttachSocket = TEXT("hand_r");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Lobby|Weapon")
	FTransform RelaxedWeaponRelativeTransform;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Lobby|Weapon")
	FTransform RifleWeaponRelativeTransform;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Lobby|Weapon")
	bool bShowWeaponWhenRelaxed = true;

private:
	void ApplyPresentation();
};
