#include "Characters/CharacterBase/FPSCharacterBase/FPSCharacterBase.h"

#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "Camera/PlayerCameraManager.h"
#include "Camera/CameraShakeBase.h"
#include "EnhancedInputComponent.h"
#include "Weapons/_Shared/Components/EquipmentManagerComponent.h"
#include "Characters/_Shared/Components/ScanEffectComponent.h"
#include "Characters/CharacterBase/Animation/CharacterBaseAnimInstance.h"
#include "Core/TheManPlayerController.h"
#include "Core/TheManPlayerState.h"
#include "AbilitySystemComponent.h"
#include "Abilities/GameplayAbility.h"
#include "GameplayEffect.h"
#include "GameplayTagContainer.h"
#include "Characters/CharacterBase/Data/TheManCharacterDataAssetBase.h"
#include "Weapons/_Shared/Firearms/Firearm.h"
#include "Core/_Shared/GAS/TheManGameplayTags.h"
#include "Engine/Engine.h"
#include "Core/TheManGameStateBase.h"
#include "Core/TheManGameInstance.h"
#include "Characters/CharacterBase/TheManAttributeSetBase.h"
#include "GameFramework/GameModeBase.h"
#include "UObject/UnrealType.h"

namespace
{
void SetAnimBool(UAnimInstance* AnimInstance, const FName PropertyName, const bool bValue)
{
	if (AnimInstance)
	{
		if (FBoolProperty* Property = FindFProperty<FBoolProperty>(AnimInstance->GetClass(), PropertyName))
		{
			Property->SetPropertyValue_InContainer(AnimInstance, bValue);
		}
	}
}

void SetAnimNumber(UAnimInstance* AnimInstance, const FName PropertyName, const double Value)
{
	if (AnimInstance)
	{
		if (FNumericProperty* Property = FindFProperty<FNumericProperty>(AnimInstance->GetClass(), PropertyName);
			Property && Property->IsFloatingPoint())
		{
			Property->SetFloatingPointPropertyValue(Property->ContainerPtrToValuePtr<void>(AnimInstance), Value);
		}
	}
}
}

AFPSCharacterBase::AFPSCharacterBase()
{
	PrimaryActorTick.bCanEverTick = true;

	LookSensitivity = 1.0f;
	WalkSpeed       = 550.0f;   // VFXPack FirstPersonCharacter.Speed_Walking
	SprintSpeed     = 750.0f;   // VFXPack FirstPersonCharacter.Speed_Sprinting
	PitchMin        = -75.0f;
	PitchMax        = 40.0f;
	// VFXPack FirstPersonCharacter.SK_ArmMesh exact relative rotation.
	BaseArmsRotation = FRotator(-3.f, -90.f, -1.f);

	// FEAT-038 修正：身体不再物理俯仰。相机从 head 骨骼摘下挂 capsule 固定眼高，
	// 看上下交给控制器旋转（相机 bUsePawnControlRotation）+ 后续 FEAT-039 上半身 AimOffset。
	// 身体只随偏航转动，保持直立（与 BodyRoot 直立的 Shadow/Legs 一致）。
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw   = true;
	bUseControllerRotationRoll  = false;
	// CharacterBase meshes are authored facing local +Y. Rotate that visual axis
	// onto ACharacter's +X arrow; the first-person arms keep their own camera-space
	// transform and must not determine the full-body component basis.
	GetMesh()->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));

	// FEAT-039/根运动：动画宿主合并进 ACharacter 的 GetMesh()（原独立 ArmsMesh 已删），
	// 让 CharacterMovement 能原生从 GetMesh() 提取根运动驱动胶囊体（停步/起步滑步）。
	// GetMesh() 现承担"第一人称手臂"：只渲染手臂材质段、只给自己看、不投影、始终评估姿势。
	// 骨架朝向、相对 Transform、AnimClass、材质段隐藏在角色 BP 配置。
	// FEAT-042：GetMesh() 退居"MM 宿主"——全身骨架跑 MM，驱动影子/腿（Follower），自己对玩家隐藏
	// （OwnerNoSee）、不投影。FP 手臂改由独立 ArmsViewMesh 渲染（下方），可见手臂不再来自这里。
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetOwnerNoSee(true);
	GetMesh()->bCastDynamicShadow = false;
	GetMesh()->CastShadow = true;
	GetMesh()->bCastHiddenShadow = true;
	// 默认 OnlyTickPoseWhenRendered：隐藏时骨骼姿势不评估，导致切角色第一帧停在参考姿势
	// （武器卡在左下角再飘到正确位置）。强制始终评估姿势，使首帧 socket 即就位。
	GetMesh()->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;

	// FEAT-038 修正：相机不再挂在会动/会俯仰的 head 骨骼上（消除 head bob + 低头时绕
	// capsule 中心画弧把视角甩向前下方）。改挂 capsule，固定在眼高，只用控制器旋转转向。
	// 眼高 Z 默认 ~77（capsule 中心 88 + 77 ≈ 世界 165 眼高），换骨架/角色可在 BP 微调。
	HeadCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("HeadCamera"));
	HeadCamera->SetupAttachment(RootComponent);
	HeadCamera->SetRelativeLocation(HeadCameraRelativeLocation);
	HeadCamera->SetRelativeRotation(FRotator::ZeroRotator);
	HeadCamera->SetFieldOfView(77.0f);
	HeadCamera->bUsePawnControlRotation = true;

	// FEAT-042：第一人称 viewmodel 根挂在相机下。相机是 gameplay 权威，手臂作为子级天然跟随相机旋转；
	// 后续 ADS / bob / sway / movement lag 只叠到 ViewmodelRoot 或 ArmsViewMesh，不污染相机。
	ViewmodelRoot = CreateDefaultSubobject<USceneComponent>(TEXT("ViewmodelRoot"));
	ViewmodelRoot->SetupAttachment(HeadCamera);
	ViewmodelRoot->SetRelativeLocation(FVector::ZeroVector);
	ViewmodelRoot->SetRelativeRotation(ViewmodelOffsetRotation);

	// FEAT-042：独立 FP 手臂 mesh。挂 ViewmodelRoot 下，跑自己的武器 ABP（持枪 pose），
	// 只给自己看、不投影、始终评估姿势。骨架/相对 Transform/AnimClass 在 BP 配。
	ArmsViewMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("ArmsViewMesh"));
	ArmsViewMesh->SetupAttachment(ViewmodelRoot);
	ArmsViewMesh->SetRelativeLocation(ViewmodelOffsetLocation);
	ArmsViewMesh->SetRelativeRotation(BaseArmsRotation);
	ArmsViewMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	ArmsViewMesh->SetOnlyOwnerSee(true);
	ArmsViewMesh->bCastDynamicShadow = false;
	ArmsViewMesh->CastShadow = false;
	ArmsViewMesh->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;
	// CharacterMesh0 consumes the completed ArmsViewMesh local-bone pose in its own
	// AnimGraph. Force the viewmodel to evaluate first so the shadow never trails it.
	GetMesh()->AddTickPrerequisiteComponent(ArmsViewMesh);


	// --- FEAT-038：第三人称全身三件套（影子 + 可见腿），与 ArmsMesh 共享同一份姿势 ---
	// BodyRoot follows the capsule hierarchy. Its authored Blueprint transform is the
	// runtime baseline; C++ must not replace it during construction or Tick.
	BodyRoot = CreateDefaultSubobject<USceneComponent>(TEXT("BodyRoot"));
	BodyRoot->SetupAttachment(RootComponent);
	// LegsMesh and the authoritative shadow caster CharacterMesh0 must share the
	// same capsule-space origin.  First-person framing belongs to the camera/viewmodel,
	// never to the body/legs root.
	BodyRoot->SetRelativeLocation(FVector::ZeroVector);

	ShadowBodyMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("ShadowBodyMesh"));
	ShadowBodyMesh->SetupAttachment(BodyRoot);
	ShadowBodyMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	ShadowBodyMesh->SetOwnerNoSee(true);        // 对自己隐藏
	ShadowBodyMesh->SetHiddenInGame(true);
	ShadowBodyMesh->CastShadow = true;
	ShadowBodyMesh->bCastHiddenShadow = true;   // 即使对玩家不可见也投影 → 完整人形影子
	ShadowBodyMesh->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;

	ShadowUpperBodyMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("ShadowUpperBodyMesh"));
	ShadowUpperBodyMesh->SetupAttachment(BodyRoot);
	ShadowUpperBodyMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	ShadowUpperBodyMesh->SetOwnerNoSee(true);
	ShadowUpperBodyMesh->SetHiddenInGame(true);
	ShadowUpperBodyMesh->CastShadow = false;
	ShadowUpperBodyMesh->bCastHiddenShadow = false;
	ShadowUpperBodyMesh->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;

	LegsMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("LegsMesh"));
	LegsMesh->SetupAttachment(BodyRoot);
	LegsMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	LegsMesh->SetOnlyOwnerSee(true);            // 只给自己看（低头看到自己的腿）
	LegsMesh->CastShadow = false;               // 投影交给 ShadowBodyMesh，避免重叠
	LegsMesh->bCastDynamicShadow = false;
	LegsMesh->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;

	EquipmentManager = CreateDefaultSubobject<UEquipmentManagerComponent>(TEXT("EquipmentManager"));
	ScanEffect       = CreateDefaultSubobject<UScanEffectComponent>(TEXT("ScanEffect"));
}

UAbilitySystemComponent* AFPSCharacterBase::GetAbilitySystemComponent() const
{
	if (ATheManPlayerState* PS = GetPlayerState<ATheManPlayerState>())
	{
		return PS->GetAbilitySystemComponent();
	}
	return nullptr;
}

// FEAT-038：隐藏指定材质槽对应的 section（只影响渲染，不改骨骼姿势 → Leader/Follower 共享姿势安全）。
// 首版按"材质槽 index == LOD0 section index"的常见情形处理；若身体 mesh 的 section/材质映射不同，
// 需按真实布局调整（如改为遍历 RenderSections 按 MaterialIndex 匹配）。索引无效时引擎内部 no-op。
static void HideMeshMaterialSlots(USkeletalMeshComponent* Mesh, const TArray<int32>& Slots)
{
	if (!Mesh) { return; }
	// UE5.7：用 ShowMaterialSection(..., bShow=false, ...) 隐藏；无独立 HideMaterialSection。
	for (int32 Slot : Slots)
	{
		Mesh->ShowMaterialSection(Slot, Slot, /*bShow=*/false, /*LODIndex=*/0);
	}
}

void AFPSCharacterBase::BeginPlay()
{
	Super::BeginPlay();
	if (GetCharacterMovement())
	{
		GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
	}
	// Apply static first-person framing once after Blueprint defaults have loaded.
	// Runtime Tick only adds dynamic motion such as the sprint pivot rotation.
	if (ViewmodelRoot)
	{
		ViewmodelRoot->SetRelativeLocation(FVector::ZeroVector);
		ViewmodelRoot->SetRelativeRotation(ViewmodelOffsetRotation);
	}
	if (ArmsViewMesh)
	{
		ArmsViewMesh->SetRelativeLocation(ViewmodelOffsetLocation);
		ArmsViewMesh->SetRelativeRotation(BaseArmsRotation);
	}

	// 角色蓝图可能保存旧的可见性动画 Tick 配置并覆盖构造函数默认值。
	// GetMesh() 是影子/腿的 Leader，即使对本地 Owner 不可见也必须刷新骨骼变换。
	GetMesh()->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;

	if (EquipmentManager)
	{
		// FEAT-042：武器挂到独立 FP 手臂 ArmsViewMesh（相机子级 viewmodel），不再挂 MM 宿主 GetMesh()。
		EquipmentManager->AttachTargetMesh = ArmsViewMesh;
		EquipmentManager->InitializeEquipment(InitialEquipmentClasses);

		// InitialEquipment links the weapon animation layer during BeginPlay. Evaluate the
		// completed graph before the first rendered frame so the viewmodel never exposes
		// the body-only entry pose and then appears to dip into the rifle idle pose.
		auto PrimeInitialWeaponPose = [](USkeletalMeshComponent* MeshComponent)
		{
			if (MeshComponent && MeshComponent->GetAnimInstance())
			{
				MeshComponent->TickAnimation(0.f, false);
				MeshComponent->RefreshBoneTransforms();
			}
		};
		PrimeInitialWeaponPose(GetMesh());
		PrimeInitialWeaponPose(ArmsViewMesh);
	}

	// FEAT-038：影子/腿共用 GetMesh() 的姿势（Leader/Follower），动画只在 GetMesh() 评估一次。
	// CharacterMesh0 already owns the authoritative, fully animated body pose. Let that
	// hidden mesh cast the shadow directly; a duplicate follower can preserve stale BP
	// transforms and is the source of the 90-degree/duplicate upper-body silhouette.
	if (ShadowBodyMesh)
	{
		ShadowBodyMesh->SetSkeletalMesh(nullptr);
		ShadowBodyMesh->SetLeaderPoseComponent(nullptr);
		ShadowBodyMesh->CastShadow = false;
		ShadowBodyMesh->bCastHiddenShadow = false;
	}
	if (ShadowUpperBodyMesh)
	{
		ShadowUpperBodyMesh->SetSkeletalMesh(nullptr);
		ShadowUpperBodyMesh->SetLeaderPoseComponent(nullptr);
	}
	if (LegsMesh)       { LegsMesh->SetLeaderPoseComponent(GetMesh()); }
	// FEAT-038/042：渲染分离——FP 手臂藏非手臂段（作用到 ArmsViewMesh）、腿藏躯干以上段（只改渲染不碰姿势）。
	// 物理拆好的 Arms/Legs mesh 本身已只含对应几何时，对应数组留空即可。
	HideMeshMaterialSlots(ArmsViewMesh, ArmsHiddenSections);
	HideMeshMaterialSlots(LegsMesh, LegsHiddenSections);

	// 角色与装备保持首帧可见；只把拔枪 Montage 延迟到下一帧，等待 AnimInstance
	// 完成初始化。不要在这里隐藏整套 Mesh，否则进入地图时会出现一帧“角色未加载”的空白。
	GetWorldTimerManager().SetTimerForNextTick(this, &AFPSCharacterBase::PlayInitialEquipEffect);

}

void AFPSCharacterBase::PlayInitialEquipEffect()
{
	// Keep the established next-tick equip hook, but reveal the weapon with the
	// source VFXPack-style material dissolve instead of moving the player's arms.
	if (EquipmentManager)
	{
		if (AEquipmentBase* Current = EquipmentManager->GetCurrentEquipment())
		{
			Current->PlayEquipEffect();
			Current->SetActorHiddenInGame(false);
		}
	}
}

void AFPSCharacterBase::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	if (APlayerController* PC = Cast<APlayerController>(NewController))
	{
		if (PC->PlayerCameraManager)
		{
			PC->PlayerCameraManager->ViewPitchMin = PitchMin;
			PC->PlayerCameraManager->ViewPitchMax = PitchMax;
		}
	}

	ATheManPlayerState* PS = GetPlayerState<ATheManPlayerState>();
	if (!PS) { return; }

	UAbilitySystemComponent* ASC = PS->GetAbilitySystemComponent();
	if (!ASC) { return; }

	ASC->InitAbilityActorInfo(PS, this);

	// Every concrete player character owns the same configurable default-ability list.
	// Individual Blueprints decide which active, passive, or event-driven abilities to grant.
	for (const TSubclassOf<UGameplayAbility>& AbilityClass : DefaultAbilityClasses)
	{
		if (AbilityClass)
		{
			ASC->GiveAbility(FGameplayAbilitySpec(AbilityClass, 1));
		}
	}

	if (CharacterData && InitGEClass)
	{
		FGameplayEffectContextHandle ContextHandle = ASC->MakeEffectContext();
		ContextHandle.AddInstigator(this, this);
		FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(InitGEClass, 1.0f, ContextHandle);

		if (SpecHandle.IsValid())
		{
			SpecHandle.Data->SetSetByCallerMagnitude(
				FGameplayTag::RequestGameplayTag(FName("Data.Attribute.MaxHealth")),
				CharacterData->InitialMaxHealth);
			SpecHandle.Data->SetSetByCallerMagnitude(
				FGameplayTag::RequestGameplayTag(FName("Data.Attribute.Health")),
				CharacterData->InitialHealth);
			ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());

			if (GEngine)
			{
				GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green,
					FString::Printf(TEXT("[%s] GAS 初始化完成 MaxHP:%.0f HP:%.0f"),
						*GetName(), CharacterData->InitialMaxHealth, CharacterData->InitialHealth));
			}
		}
	}

	// BeginPlay 时 ASC 尚未就绪，PossessedBy 后补授当前装备的开火技能
	if (EquipmentManager)
	{
		if (AFirearm* Firearm = Cast<AFirearm>(EquipmentManager->GetCurrentEquipment()))
		{
			Firearm->GrantAbilities(ASC);
		}
	}
}

void AFPSCharacterBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	UEnhancedInputComponent* EIC = CastChecked<UEnhancedInputComponent>(PlayerInputComponent);
	ATheManPlayerController* PC  = Cast<ATheManPlayerController>(GetController());
	if (!PC) { return; }

	// 移动 / 视角
	if (PC->GetMoveAction())
	{
		EIC->BindAction(PC->GetMoveAction(), ETriggerEvent::Triggered, this, &AFPSCharacterBase::Move);
		EIC->BindAction(PC->GetMoveAction(), ETriggerEvent::Completed, this, &AFPSCharacterBase::StopMove);
		EIC->BindAction(PC->GetMoveAction(), ETriggerEvent::Canceled, this, &AFPSCharacterBase::StopMove);
	}
	if (PC->GetLookAction())
		EIC->BindAction(PC->GetLookAction(), ETriggerEvent::Triggered, this, &AFPSCharacterBase::Look);

	// 跳跃（ACharacter 原生方法，无需包装）
	if (PC->GetJumpAction())
	{
		EIC->BindAction(PC->GetJumpAction(), ETriggerEvent::Started,   this, &ACharacter::Jump);
		EIC->BindAction(PC->GetJumpAction(), ETriggerEvent::Completed, this, &ACharacter::StopJumping);
	}

	// 装备切换
	if (PC->GetSwitchEquipmentAction())
		EIC->BindAction(PC->GetSwitchEquipmentAction(), ETriggerEvent::Triggered, this, &AFPSCharacterBase::SwitchEquipment);

	// 武器开火
	if (PC->GetPrimaryFireAction())
		EIC->BindAction(PC->GetPrimaryFireAction(),   ETriggerEvent::Started, this, &AFPSCharacterBase::PrimaryFire);
	if (PC->GetSecondaryFireAction())
		EIC->BindAction(PC->GetSecondaryFireAction(), ETriggerEvent::Started, this, &AFPSCharacterBase::SecondaryFire);

	// 通用交互（E 键）：发送 Input.Character.Interact，由各角色已授予的技能监听处理
	if (PC->GetInteractAction())
		EIC->BindAction(PC->GetInteractAction(), ETriggerEvent::Started, this, &AFPSCharacterBase::ActivateInteract);

	// 冲刺（按住提速、松开回走速）
	if (PC->GetSprintAction())
	{
		EIC->BindAction(PC->GetSprintAction(), ETriggerEvent::Started,   this, &AFPSCharacterBase::StartSprint);
		EIC->BindAction(PC->GetSprintAction(), ETriggerEvent::Completed, this, &AFPSCharacterBase::StopSprint);
	}
}

void AFPSCharacterBase::StartSprint()
{
	// VFXPack rejects sprint while airborne. The source blueprint drives its
	// timelines from sprint intent, not from the velocity reached afterwards.
	if (GetCharacterMovement() && GetCharacterMovement()->IsFalling())
	{
		return;
	}
	bIsSprinting = true;
}

void AFPSCharacterBase::StopSprint()
{
	bIsSprinting = false;
}

void AFPSCharacterBase::Move(const FInputActionValue& Value)
{
	if (!Controller) { return; }
	const FVector2D V = Value.Get<FVector2D>();
	CurrentVFXMoveInput = V;
	if (V.IsNearlyZero()) { return; }

	const FRotator YawRotation(0, Controller->GetControlRotation().Yaw, 0);
	AddMovementInput(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X), V.Y);
	AddMovementInput(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y), V.X);
}

void AFPSCharacterBase::StopMove(const FInputActionValue&)
{
	CurrentVFXMoveInput = FVector2D::ZeroVector;
}

void AFPSCharacterBase::Look(const FInputActionValue& Value)
{
	if (!Controller) { return; }
	const FVector2D V = Value.Get<FVector2D>();
	CurrentVFXLookInputX = V.X;
	CurrentVFXLookInputY = V.Y;
	AddControllerYawInput(V.X   * LookSensitivity);
	AddControllerPitchInput(V.Y * LookSensitivity);
}

void AFPSCharacterBase::SwitchEquipment(const FInputActionValue& Value)
{
	const float Dir = Value.Get<float>();
	if (FMath::IsNearlyZero(Dir) || !EquipmentManager)
	{
		return;
	}

	// 只取方向符号，不依赖滚轮数值大小：任何非零滚动都精确切一格
	const int32 Step = (Dir > 0.f) ? 1 : -1;
	EquipmentManager->SwitchEquipment(Step);
}

void AFPSCharacterBase::PrimaryFire()
{
	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
	{
		FGameplayEventData Payload;
		Payload.Instigator = this;
		ASC->HandleGameplayEvent(TAG_Input_Weapon_PrimaryFire, &Payload);
	}
}

void AFPSCharacterBase::SecondaryFire()
{
	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
	{
		FGameplayEventData Payload;
		Payload.Instigator = this;
		ASC->HandleGameplayEvent(TAG_Input_Weapon_SecondaryFire, &Payload);
	}
}

void AFPSCharacterBase::ActivateInteract()
{
	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
	{
		FGameplayEventData Payload;
		Payload.Instigator = this;
		ASC->HandleGameplayEvent(TAG_Input_Character_Interact, &Payload);
	}
}

void AFPSCharacterBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	const FVector AnimationVelocity = GetVelocity();
	const double CharacterSpeed = AnimationVelocity.Size();
	const float GroundSpeed = AnimationVelocity.Size2D();
	// VFXPack uses a 0.2 second Timeline that plays on Sprint pressed and reverses
	// from its current position on release. One alpha owns speed and BodyRotator.
	const float SprintTimelineStep = DeltaTime / VFXSprintTransitionDuration;
	SprintTransitionAlpha = FMath::Clamp(
		SprintTransitionAlpha + (bIsSprinting ? SprintTimelineStep : -SprintTimelineStep),
		0.f,
		1.f);
	if (GetCharacterMovement())
	{
		GetCharacterMovement()->MaxWalkSpeed = FMath::Lerp(WalkSpeed, SprintSpeed, SprintTransitionAlpha);
	}

	// 后坐力：角速度积分驱动，FInterpTo 衰减
	if (FMath::Abs(RecoilPitchVelocity) > KINDA_SMALL_NUMBER ||
		FMath::Abs(RecoilYawVelocity)   > KINDA_SMALL_NUMBER)
	{
		if (APlayerController* PC = Cast<APlayerController>(GetController()))
		{
			FRotator Rot = PC->GetControlRotation();
			Rot.Pitch += RecoilPitchVelocity * DeltaTime;
			Rot.Yaw   += RecoilYawVelocity   * DeltaTime;
			PC->SetControlRotation(Rot);
		}
		RecoilPitchVelocity = FMath::FInterpTo(RecoilPitchVelocity, 0.f, DeltaTime, RecoilDamping);
		RecoilYawVelocity   = FMath::FInterpTo(RecoilYawVelocity,   0.f, DeltaTime, RecoilDamping);
	}

	// VFXPack 的 Walk_Run_1D 只有速度轴。左右移动时的姿态偏移来自原角色
	// Body_Sway：原蓝图同时使用移动轴和本帧鼠标轴，再以 Walk=2 / Sprint=8 插值。
	if (ArmsViewMesh && ViewmodelRoot)
	{
		// ViewmodelRoot is the original VFXPack BodyRotator equivalent. Only its
		// dynamic sprint rotation changes at runtime; static framing was applied once.
		ViewmodelRoot->SetRelativeRotation(
			ViewmodelOffsetRotation + FRotator(SprintViewmodelPitchDegrees * SprintTransitionAlpha, 0.f, 0.f));

		// VFXPack FirstPerson_AnimBP 原 Event Blueprint Update Animation 的等价逻辑。
		// 直接写原变量，保留其原 StateMachine / BlendSpace 资产，不另造动画状态机。
		// Raw movement input drives only the source AnimBP's Modify Bone chain.
		// The input stays in [-1, 1]; the walk/sprint values below control interpolation speed,
		// independently of the authored output offsets applied later.
		const float SideInput = FMath::Clamp(CurrentVFXMoveInput.X, -1.f, 1.f);
		const float ForwardSwayTarget = FMath::Clamp(-CurrentVFXMoveInput.Y, -1.f, 1.f);
		const float BodySwayInterpSpeed = bIsSprinting ? 8.f : ViewmodelBodySwayInterpSpeed;
		CurrentVFXLeanSides = FMath::FInterpTo(CurrentVFXLeanSides, SideInput, DeltaTime, BodySwayInterpSpeed);
		CurrentVFXLookUpDown = FMath::FInterpTo(CurrentVFXLookUpDown, ForwardSwayTarget, DeltaTime, BodySwayInterpSpeed);

		// The source AnimBP authored its component-space spine correction with the
		// arm component at Yaw -15 degrees. Our finalized animations keep root at
		// identity and the arm component supplies Yaw -90 degrees instead. Rotate
		// the authored Roll/Pitch vector through that 75-degree basis difference so
		// the visible correction stays aligned with the source camera space.
		constexpr float SourceToCurrentBasisDegrees = 75.f;
		const float BasisRadians = FMath::DegreesToRadians(SourceToCurrentBasisDegrees);
		const float BasisCos = FMath::Cos(BasisRadians);
		const float BasisSin = FMath::Sin(BasisRadians);
		// The source AnimBP EventGraph multiplies its interpolated [-1, 1] inputs
		// by the authored Lean/Look offsets before the Modify Bone nodes evaluate.
		constexpr float SourceLeanSidesOffset = 8.f;
		constexpr float SourceLookUpOffset = 2.f;
		const float SourceLeanRoll = CurrentVFXLeanSides * SourceLeanSidesOffset;
		const float SourceLookPitch = CurrentVFXLookUpDown * SourceLookUpOffset;
		const float RemappedLeanRoll = SourceLeanRoll * BasisCos - SourceLookPitch * BasisSin;
		const float RemappedLookPitch = SourceLeanRoll * BasisSin + SourceLookPitch * BasisCos;

		auto UpdateVFXPackAnimInstance = [this, CharacterSpeed, RemappedLeanRoll, RemappedLookPitch](UAnimInstance* AnimInstance)
		{
			if (UCharacterBaseAnimInstance* CharacterAnimInstance = Cast<UCharacterBaseAnimInstance>(AnimInstance))
			{
				CharacterAnimInstance->UpdateCharacterAnimationState(
					static_cast<float>(CharacterSpeed),
					GetCharacterMovement()->IsFalling(),
					RemappedLeanRoll,
					RemappedLookPitch);
			}
			else
			{
				// Compatibility path for the restored original VFXPack AnimBP. Keep
				// this until its complete graph has been migrated into the template.
				SetAnimBool(AnimInstance, TEXT("Is_Moving"), CharacterSpeed > 0.0);
				SetAnimBool(AnimInstance, TEXT("Is_InAir"), GetCharacterMovement()->IsFalling());
				SetAnimNumber(AnimInstance, TEXT("Character_Speed"), CharacterSpeed);
				SetAnimNumber(AnimInstance, TEXT("Lean_Sides_Amount"), RemappedLeanRoll);
				SetAnimNumber(AnimInstance, TEXT("Look_Up_Amount"), RemappedLookPitch);
			}
		};

		// Arms owns the first-person graph and weapon layer. CharacterMesh0 runs its
		// independent body locomotion, then copies only the completed upper-body pose
		// from ArmsViewMesh in the Body AnimGraph.
		UpdateVFXPackAnimInstance(ArmsViewMesh->GetAnimInstance());

		CurrentArmsPitch = 0.f;
		// Mouse axis values in the source are frame deltas. Consume this frame's value
		// so a stopped mouse cannot leave a persistent lean target.
		CurrentVFXLookInputX = 0.f;
		CurrentVFXLookInputY = 0.f;
	}

	// VFXPack 原蓝图：CharacterMovement.Velocity.Size() > 0 判定移动；走路 Shake Scale=0.5，
	// 冲刺 Shake Scale=1.0；状态退出时 StopCameraShake(bImmediately=true)。
	if (APlayerController* PC = Cast<APlayerController>(GetController()); PC && PC->PlayerCameraManager)
	{
		TSubclassOf<UCameraShakeBase> DesiredShake;
		float DesiredScale = 1.f;
		const bool bIsMoving = GetCharacterMovement()->Velocity.Size() > 0.f;
		if (bIsMoving && GetCharacterMovement()->IsMovingOnGround())
		{
			DesiredShake = bIsSprinting ? RunningCameraShake : WalkingCameraShake;
			DesiredScale = bIsSprinting ? 1.f : 0.5f;
		}

		if (DesiredShake)
		{
			if (ActiveMovementCameraShake && DesiredShake != ActiveMovementCameraShakeClass)
			{
				PC->PlayerCameraManager->StopCameraShake(ActiveMovementCameraShake, true);
			}
			ActiveMovementCameraShakeClass = DesiredShake;
			// 原蓝图在移动检查链中每帧调用 StartCameraShake；资产启用 Single Instance，
			// 因而返回/刷新同一实例，而不是不断堆叠新 Shake。
			ActiveMovementCameraShake = PC->PlayerCameraManager->StartCameraShake(DesiredShake, DesiredScale);
		}
		else if (ActiveMovementCameraShake)
		{
			PC->PlayerCameraManager->StopCameraShake(ActiveMovementCameraShake, true);
			ActiveMovementCameraShake = nullptr;
			ActiveMovementCameraShakeClass = nullptr;
		}
	}
}

void AFPSCharacterBase::OnDeath()
{
	if (bIsDead) return;
	bIsDead = true;

	// 死亡：交给 GameState 统一路由（含游戏结束预判 → 直接跳过测试地图）。
	// 不在此原地复活/回血/传送——这些由"选角色 → 重新加载测试地图"自然完成。
	if (ATheManGameStateBase* GS = GetWorld()->GetGameState<ATheManGameStateBase>())
	{
		GS->RoutePlayerDeath();
	}
}

void AFPSCharacterBase::AddRecoil(float Pitch, float Yaw, float Damping)
{
	RecoilPitchVelocity += Pitch;
	RecoilYawVelocity   += Yaw;
	RecoilDamping        = Damping;
}
