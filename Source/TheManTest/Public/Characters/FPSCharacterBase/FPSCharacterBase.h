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

UCLASS()
class THEMANTEST_API AFPSCharacterBase : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	AFPSCharacterBase();

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	virtual void Tick(float DeltaTime) override;
	virtual void OnConstruction(const FTransform& Transform) override;

	// 鐢?Controller 鐨?InputComponent 鎴?Character 鐨?SetupPlayerInputComponent 椹卞姩
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void SwitchEquipment(const FInputActionValue& Value);
	void PrimaryFire();
	void SecondaryFire();

	// 鍐插埡锛氭寜浣忔妸 MaxWalkSpeed 鎻愬埌 SprintSpeed锛屾澗寮€鍥?WalkSpeed
	void StartSprint();
	void StopSprint();

	// 閫氱敤浜や簰鍏ュ彛锛氬悜 ASC 鍙戦€?Input.Character.Interact锛屽悇瑙掕壊鎺堜簣涓嶅悓鎶€鑳界洃鍚 Tag
	void ActivateInteract();

	void AddRecoil(float Pitch, float Yaw, float Damping);

	void OnDeath();

	FORCEINLINE UCameraComponent*           GetHeadCamera()       const { return HeadCamera; }
	FORCEINLINE USceneComponent*            GetViewmodelRoot()    const { return ViewmodelRoot; }
	// FEAT-042锛欶P 鎵嬭噦鏀瑰洖鐙珛 mesh锛圓rmsViewMesh锛夛紝涓嶅綊 MM 绠?鈫?鑳借浆缁勪欢璺熻瑙掞紙MM 瀹夸富杞笉鍔紝
	FORCEINLINE USkeletalMeshComponent*     GetArmsMesh()         const { return ArmsViewMesh; }
	FORCEINLINE USceneComponent*            GetBodyRoot()         const { return BodyRoot; }
	FORCEINLINE USkeletalMeshComponent*     GetShadowBodyMesh()   const { return ShadowBodyMesh; }
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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WeaponSway")
	float SwayIntensity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WeaponSway")
	float SwayInterpSpeedX;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WeaponSway")
	float SwayInterpSpeedY;

	// ArmsMesh 鍩虹鐩稿鏃嬭浆銆傛柊鍏ㄨ韩楠ㄦ灦鍙傝€冨Э鍔挎湞 +Y锛岄渶 Yaw -90掳 杞鏈?+X銆?	// 姝﹀櫒鎽囨憜姣忓抚鍙犲姞鍦ㄥ畠涔嬩笂锛堣€岄潪瑕嗙洊锛夛紝鍚﹀垯韬綋浼氳鐢╁洖 identity 姝悜渚ч潰銆?	U_PROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh")
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh")
	FRotator BaseArmsRotation;

	// 鈹€鈹€ 绗竴浜虹О viewmodel 璋冭瘯鍙傛暟锛堜繚鐣欑粰鍚庣画鎭㈠鎵嬭噦婊炲悗/鎯€э級鈹€鈹€
	// 褰撳墠缁撴瀯涓?HeadCamera -> ViewmodelRoot -> ArmsViewMesh锛屾墜鑷傚ぉ鐒剁户鎵跨浉鏈烘棆杞€?	// 杩欎簺鍙傛暟鏆備笉椹卞姩 pitch锛涘悗缁嫢瑕佸仛鎵嬭噦鐩稿鐩告満鐨勫欢杩?鎯€э紝鍙鐢ㄦ垨鏇挎崲涓?viewmodel offset 鍙傛暟銆?	U_PROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ArmsAiming")
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ArmsAiming")
	bool bArmsPitchFollow = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ArmsAiming", meta = (ClampMin = "-1.0", ClampMax = "1.0"))
	float ArmsPitchFollowAmount = 0.7f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ArmsAiming")
	float ArmsPitchInterpSpeed = 12.f;

	// 第一人称最终构图只作用于相机子级 ViewmodelRoot，不改变 gameplay 相机或骨架基础校正。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Viewmodel|Framing")
	FVector ViewmodelOffsetLocation = FVector(-25.f, 2.f, -6.f);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Viewmodel|Framing")
	FRotator ViewmodelOffsetRotation = FRotator(0.f, -10.f, 0.f);

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
	void EnsureViewmodelAttachment();
	void ApplyViewmodelFraming();

	void PlayInitialEquipMontage();

	FRotator CurrentSway;
	FRotator LastControlRotation;

	float CurrentArmsPitch = 0.f;

	float RecoilPitchVelocity = 0.f;
	float RecoilYawVelocity   = 0.f;
	float RecoilDamping       = 18.f;

	bool bIsDead = false;

	bool bIsSprinting = false;

};
