#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "InputActionValue.h"
#include "FPSCharacterBase.generated.h"

class UCameraComponent;
class USceneComponent;
class UEquipmentManagerComponent;
class UScanEffectComponent;
class UTheManCharacterDataAssetBase;
class UGameplayEffect;
class UGameplayAbility;
class UAbilitySystemComponent;
class AEquipmentBase;
class UCameraShakeBase;
class UMaterialInstanceDynamic;

UCLASS()
class THEMANTEST_API AFPSCharacterBase : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	AFPSCharacterBase();

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	virtual void Tick(float DeltaTime) override;

	// 鐢?Controller 鐨?InputComponent 鎴?Character 鐨?SetupPlayerInputComponent 椹卞姩
	void Move(const FInputActionValue& Value);
	void StopMove(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void SwitchEquipment(const FInputActionValue& Value);
	void PrimaryFire();
	void SecondaryFire();
	void Reload();

	// 鍐插埡锛氭寜浣忔妸 MaxWalkSpeed 鎻愬埌 SprintSpeed锛屾澗寮€鍥?WalkSpeed
	void StartSprint();
	void StopSprint();

	// 閫氱敤浜や簰鍏ュ彛锛氬悜 ASC 鍙戦€?Input.Character.Interact锛屽悇瑙掕壊鎺堜簣涓嶅悓鎶€鑳界洃鍚 Tag
	void ActivateInteract();

	void AddRecoil(float Pitch, float Yaw, float Damping);

	void OnDeath();

	FORCEINLINE UCameraComponent*           GetHeadCamera()       const { return HeadCamera; }
	FORCEINLINE USceneComponent*            GetViewmodelRoot()    const { return ViewmodelRoot; }
#if WITH_DEV_AUTOMATION_TESTS
	void SetSprintingForTesting(bool bValue) { bIsSprinting = bValue; }
#endif
	// FEAT-042锛欶P 鎵嬭噦鏀瑰洖鐙珛 mesh锛圓rmsViewMesh锛夛紝涓嶅綊 MM 绠?鈫?鑳借浆缁勪欢璺熻瑙掞紙MM 瀹夸富杞笉鍔紝
	FORCEINLINE USkeletalMeshComponent*     GetArmsMesh()         const { return ArmsViewMesh; }
	FORCEINLINE USceneComponent*            GetBodyRoot()         const { return BodyRoot; }
	FORCEINLINE USkeletalMeshComponent*     GetShadowBodyMesh()   const { return ShadowBodyMesh; }
	FORCEINLINE USkeletalMeshComponent*     GetShadowUpperBodyMesh() const { return ShadowUpperBodyMesh; }
	FORCEINLINE USkeletalMeshComponent*     GetLegsMesh()         const { return LegsMesh; }
	FORCEINLINE UEquipmentManagerComponent* GetEquipmentManager() const { return EquipmentManager; }
	FORCEINLINE UScanEffectComponent*       GetScanEffect()       const { return ScanEffect; }

	FORCEINLINE bool IsSprinting() const { return bIsSprinting; }

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float WalkSpeed;

	// 鍐插埡閫熷害锛堟寜浣?Sprint 閿椂鐨?MaxWalkSpeed锛夈€傚簲涓?blendspace 鐨勮窇姝ラ噰鏍烽€熷害鍖归厤銆?	U_PROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float SprintSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	float LookSensitivity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	float PitchMin;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	float PitchMax;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera|Movement")
	TSubclassOf<UCameraShakeBase> WalkingCameraShake;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera|Movement")
	TSubclassOf<UCameraShakeBase> RunningCameraShake;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Viewmodel|Lag", meta = (ClampMin = "0.0", Units = "cm"))
	float ViewmodelLookLagHorizontalCm = 1.8f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Viewmodel|Lag", meta = (ClampMin = "0.0", Units = "cm"))
	float ViewmodelLookLagVerticalCm = 1.2f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Viewmodel|Lag", meta = (ClampMin = "0.0"))
	float ViewmodelLookLagHorizontalImpulse = 7.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Viewmodel|Lag", meta = (ClampMin = "0.0"))
	float ViewmodelLookLagVerticalImpulse = 5.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Viewmodel|Lag", meta = (ClampMin = "0.0"))
	float ViewmodelLookLagSpringStiffness = 85.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Viewmodel|Lag", meta = (ClampMin = "0.0"))
	float ViewmodelLookLagSpringDamping = 12.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Viewmodel|Lag", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float ViewmodelLookLagDeadZone = 0.01f;

#if WITH_DEV_AUTOMATION_TESTS
public:
	void SetViewmodelLookInputForTesting(const FVector2D& Value) { PendingViewmodelLookInput = Value; }
protected:
#endif

	// Keep the camera-authored viewmodel framing without moving the first-person
	// skeleton off the authoritative body's world-space centre line.  The camera
	// carries the inverse lateral offset, so ArmsViewMesh may remain forward/vertical
	// offset while its component origin stays on CharacterMesh0's left/right axis.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera|Framing")
	FVector HeadCameraRelativeLocation = FVector(0.f, -18.852108f, 77.f);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Viewmodel|Sprint", meta = (ClampMin = "-45.0", ClampMax = "0.0", Units = "deg"))
	float SprintViewmodelPitchDegrees = -6.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Viewmodel|Movement", meta = (ClampMin = "0.1"))
	float ViewmodelBodySwayInterpSpeed = 6.f;

protected:
	virtual void BeginPlay() override;
	virtual void PossessedBy(AController* NewController) override;

	// 姣忎釜瑙掕壊鍦ㄨ Possess 鏃跺紩鎿庤嚜鍔ㄨ皟鐢紝鐢ㄤ簬缁戝畾鑷繁闇€瑕佸搷搴旂殑杈撳叆
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	UCameraComponent* HeadCamera;

	// 绗竴浜虹О viewmodel 鏍癸細鎸傚湪 HeadCamera 涓嬶紝浣滀负鎵嬭噦/姝﹀櫒鐨勭嫭绔嬬浉瀵瑰亸绉诲眰銆?	// 鐩告満淇濇寔 gameplay 鏉冨▉锛泇iewmodel 缁ф壙鐩告満鏃嬭浆锛屽苟鍙湪鏈眰鍙犲姞 ADS / bob / sway / lag銆?	U_PROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ArmsAiming")
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ArmsAiming")
	USceneComponent* ViewmodelRoot;

	// FEAT-042锛氱嫭绔嬬涓€浜虹О鎵嬭噦 mesh銆傛寕 ViewmodelRoot 涓嬶紝鑷繁鐨勬鍣?ABP锛堟寔鏋?pose锛夛紝OnlyOwnerSee銆?	// 涓嶅綊 MM 绠★紱姝﹀櫒鎸傚畠銆丟etArmsMesh() 鎸囧畠銆?	U_PROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ArmsAiming")
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ArmsAiming")
	USkeletalMeshComponent* ArmsViewMesh;

	// FEAT-038锛氳韩浣撴牴銆傜粷瀵规棆杞?+ Tick 姣忓抚鍙彇 Yaw 鈫?韬綋姘歌繙鐩寸珛锛堟姇褰?鑵夸笉闅忕浉鏈轰刊浠板墠鍊撅級銆?	U_PROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mesh")
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mesh")
	USceneComponent* BodyRoot;

	// FEAT-038锛氭姇褰辩敤鍏ㄨ韩 mesh銆傚鑷繁闅愯棌 + 鎶曢殣钘忛槾褰?鈫?鐜╁鐪嬩笉鍒般€佷絾鍦颁笂鏈夊畬鏁翠汉褰㈠奖瀛愩€?	// Follower锛歋etLeaderPoseComponent(ArmsMesh)锛屼笌鎵嬭噦鍏变韩鍚屼竴浠藉Э鍔裤€?	U_PROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mesh")
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mesh")
	USkeletalMeshComponent* ShadowBodyMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mesh")
	USkeletalMeshComponent* ShadowUpperBodyMesh;

	// FEAT-038锛氬彲瑙佷笅鍗婅韩 mesh銆傚彧缁欒嚜宸辩湅銆佷笉鎶曞奖锛堟姇褰变氦缁?ShadowBodyMesh锛夆啋 浣庡ご鐪嬪埌鑷繁鐨勮吙銆?	// Follower锛氫笌鎵嬭噦鍏变韩濮垮娍锛涢潬鏉愯川娈甸殣钘忚函骞蹭互涓娿€?	U_PROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mesh")
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mesh")
	USkeletalMeshComponent* LegsMesh;

	// FEAT-038锛氭覆鏌撳垎绂伙紙鍙敼娓叉煋涓嶇楠ㄩ锛孡eader/Follower 鍏变韩濮垮娍瀹夊叏锛夈€?	// ArmsMesh 闅愯棌闈炴墜鑷傜殑鏉愯川妲斤紱LegsMesh 闅愯棌闈炶吙锛堣函骞?鎵嬭噦锛夌殑鏉愯川妲姐€傜储寮曟寜韬綋 mesh 瀹為檯鏉愯川妲藉～銆?	U_PROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mesh")
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mesh")
	TArray<int32> ArmsHiddenSections;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mesh")
	TArray<int32> LegsHiddenSections;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Equipment")
	UEquipmentManagerComponent* EquipmentManager;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Scan")
	UScanEffectComponent* ScanEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Equipment")
	TArray<TSubclassOf<AEquipmentBase>> InitialEquipmentClasses;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Data")
	UTheManCharacterDataAssetBase* CharacterData;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS")
	TSubclassOf<UGameplayEffect> InitGEClass;

	// Character-owned abilities granted after the shared ASC is initialized.
	// Concrete character Blueprints configure active, passive, or event-driven abilities here.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS|Abilities")
	TArray<TSubclassOf<UGameplayAbility>> DefaultAbilityClasses;

private:


	UPROPERTY(Transient)
	TObjectPtr<UCameraShakeBase> ActiveMovementCameraShake;

	TSubclassOf<UCameraShakeBase> ActiveMovementCameraShakeClass;

	float CurrentVFXLeanSides = 0.f;
	float CurrentVFXLookUpDown = 0.f;
	FVector2D CurrentVFXMoveInput = FVector2D::ZeroVector;
	FVector2D PendingViewmodelLookInput = FVector2D::ZeroVector;
	FVector ViewmodelAuthoredRelativeLocation = FVector::ZeroVector;
	FVector CurrentViewmodelLookLagOffset = FVector::ZeroVector;
	FVector CurrentViewmodelLookLagVelocity = FVector::ZeroVector;
	TObjectPtr<UMaterialInstanceDynamic> LegsArmProximityMaterial;
	float SprintTransitionAlpha = 0.f;
	static constexpr float VFXSprintTransitionDuration = 0.2f;

	float RecoilPitchVelocity = 0.f;
	float RecoilYawVelocity   = 0.f;
	float RecoilDamping       = 18.f;

	bool bIsDead = false;

	bool bIsSprinting = false;

};
