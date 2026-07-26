#include "Characters/FPSCharacterBase/FPSCharacterBase.h"

#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "Camera/PlayerCameraManager.h"
#include "EnhancedInputComponent.h"
#include "Characters/Components/EquipmentManagerComponent.h"
#include "Characters/Components/ScanEffectComponent.h"
#include "Characters/FPSCharacterBase/Animation/FPSCharacterAnimInstance.h"
#include "Core/TheManPlayerController.h"
#include "Core/TheManPlayerState.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffect.h"
#include "GameplayTagContainer.h"
#include "Characters/BaseCharacter/Data/TheManCharacterDataAssetBase.h"
#include "Equipment/Firearms/Firearm.h"
#include "GAS/TheManGameplayTags.h"
#include "Engine/Engine.h"
#include "Core/TheManGameStateBase.h"
#include "Core/TheManGameInstance.h"
#include "Characters/BaseCharacter/TheManAttributeSetBase.h"
#include "GameFramework/GameModeBase.h"

AFPSCharacterBase::AFPSCharacterBase()
{
	PrimaryActorTick.bCanEverTick = true;

	LookSensitivity = 1.0f;
	WalkSpeed       = 250.0f;   // 基础（走）：匹配 Walk_Loop 动画速度
	SprintSpeed     = 550.0f;   // 按住 Shift（跑）：匹配 Jog_Loop 动画速度
	PitchMin        = -75.0f;
	PitchMax        = 40.0f;
	SwayIntensity   = 2.0f;
	SwayInterpSpeedX = 5.33f;
	SwayInterpSpeedY = 3.33f;
	CurrentSway          = FRotator::ZeroRotator;
	LastControlRotation  = FRotator::ZeroRotator;
	BaseArmsRotation     = FRotator(0.f, -90.f, 0.f);   // 新全身骨架转正：朝 +Y → 朝 +X

	// FEAT-038 修正：身体不再物理俯仰。相机从 head 骨骼摘下挂 capsule 固定眼高，
	// 看上下交给控制器旋转（相机 bUsePawnControlRotation）+ 后续 FEAT-039 上半身 AimOffset。
	// 身体只随偏航转动，保持直立（与 BodyRoot 直立的 Shadow/Legs 一致）。
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw   = true;
	bUseControllerRotationRoll  = false;

	// FEAT-039/根运动：动画宿主合并进 ACharacter 的 GetMesh()（原独立 ArmsMesh 已删），
	// 让 CharacterMovement 能原生从 GetMesh() 提取根运动驱动胶囊体（停步/起步滑步）。
	// GetMesh() 现承担"第一人称手臂"：只渲染手臂材质段、只给自己看、不投影、始终评估姿势。
	// 骨架朝向、相对 Transform、AnimClass、材质段隐藏在角色 BP 配置。
	// FEAT-042：GetMesh() 退居"MM 宿主"——全身骨架跑 MM，驱动影子/腿（Follower），自己对玩家隐藏
	// （OwnerNoSee）、不投影。FP 手臂改由独立 ArmsViewMesh 渲染（下方），可见手臂不再来自这里。
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetOwnerNoSee(true);
	GetMesh()->bCastDynamicShadow = false;
	GetMesh()->CastShadow = false;
	// 默认 OnlyTickPoseWhenRendered：隐藏时骨骼姿势不评估，导致切角色第一帧停在参考姿势
	// （武器卡在左下角再飘到正确位置）。强制始终评估姿势，使首帧 socket 即就位。
	GetMesh()->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;

	// FEAT-038 修正：相机不再挂在会动/会俯仰的 head 骨骼上（消除 head bob + 低头时绕
	// capsule 中心画弧把视角甩向前下方）。改挂 capsule，固定在眼高，只用控制器旋转转向。
	// 眼高 Z 默认 ~77（capsule 中心 88 + 77 ≈ 世界 165 眼高），换骨架/角色可在 BP 微调。
	HeadCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("HeadCamera"));
	HeadCamera->SetupAttachment(RootComponent);
	HeadCamera->SetRelativeLocation(FVector(0.f, 0.f, 77.f));
	HeadCamera->SetRelativeRotation(FRotator::ZeroRotator);
	HeadCamera->bUsePawnControlRotation = true;

	// FEAT-042：第一人称 viewmodel 根挂在相机下。相机是 gameplay 权威，手臂作为子级天然跟随相机旋转；
	// 后续 ADS / bob / sway / movement lag 只叠到 ViewmodelRoot 或 ArmsViewMesh，不污染相机。
	ViewmodelRoot = CreateDefaultSubobject<USceneComponent>(TEXT("ViewmodelRoot"));
	ViewmodelRoot->SetupAttachment(HeadCamera);
	ViewmodelRoot->SetRelativeLocation(FVector::ZeroVector);
	ViewmodelRoot->SetRelativeRotation(FRotator::ZeroRotator);

	// FEAT-042：独立 FP 手臂 mesh。挂 ViewmodelRoot 下，跑自己的武器 ABP（持枪 pose），
	// 只给自己看、不投影、始终评估姿势。骨架/相对 Transform/AnimClass 在 BP 配。
	ArmsViewMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("ArmsViewMesh"));
	ArmsViewMesh->SetupAttachment(ViewmodelRoot);
	ArmsViewMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	ArmsViewMesh->SetOnlyOwnerSee(true);
	ArmsViewMesh->bCastDynamicShadow = false;
	ArmsViewMesh->CastShadow = false;
	ArmsViewMesh->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;


	// --- FEAT-038：第三人称全身三件套（影子 + 可见腿），与 ArmsMesh 共享同一份姿势 ---
	// BodyRoot 保持直立：绝对旋转，Tick 每帧只取 Yaw，丢弃相机俯仰 → 投影/腿不前倾。
	BodyRoot = CreateDefaultSubobject<USceneComponent>(TEXT("BodyRoot"));
	BodyRoot->SetupAttachment(RootComponent);
	BodyRoot->SetUsingAbsoluteRotation(true);
	BodyRoot->SetRelativeLocation(FVector(-30.f, 0.f, 0.f));   // 初始往后偏移（Shadow/Legs 整体后移，使影子/腿相对相机靠后）

	ShadowBodyMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("ShadowBodyMesh"));
	ShadowBodyMesh->SetupAttachment(BodyRoot);
	ShadowBodyMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	ShadowBodyMesh->SetOwnerNoSee(true);        // 对自己隐藏
	ShadowBodyMesh->CastShadow = true;
	ShadowBodyMesh->bCastHiddenShadow = true;   // 即使对玩家不可见也投影 → 完整人形影子
	ShadowBodyMesh->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;

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

void AFPSCharacterBase::EnsureViewmodelAttachment()
{
	if (!ViewmodelRoot || !ArmsViewMesh)
	{
		return;
	}

	if (ArmsViewMesh->GetAttachParent() != ViewmodelRoot)
	{
		ArmsViewMesh->AttachToComponent(ViewmodelRoot, FAttachmentTransformRules::KeepRelativeTransform);
	}
}

void AFPSCharacterBase::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	EnsureViewmodelAttachment();
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
	EnsureViewmodelAttachment();

	if (GetCharacterMovement())
	{
		GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
	}

	if (EquipmentManager)
	{
		// FEAT-042：武器挂到独立 FP 手臂 ArmsViewMesh（相机子级 viewmodel），不再挂 MM 宿主 GetMesh()。
		EquipmentManager->AttachTargetMesh = ArmsViewMesh;
		EquipmentManager->InitializeEquipment(InitialEquipmentClasses);
	}

	// FEAT-038：影子/腿共用 GetMesh() 的姿势（Leader/Follower），动画只在 GetMesh() 评估一次。
	if (ShadowBodyMesh) { ShadowBodyMesh->SetLeaderPoseComponent(GetMesh()); }
	if (LegsMesh)       { LegsMesh->SetLeaderPoseComponent(GetMesh()); }
	// FEAT-038/042：渲染分离——FP 手臂藏非手臂段（作用到 ArmsViewMesh）、腿藏躯干以上段（只改渲染不碰姿势）。
	// 物理拆好的 Arms/Legs mesh 本身已只含对应几何时，对应数组留空即可。
	HideMeshMaterialSlots(ArmsViewMesh, ArmsHiddenSections);
	HideMeshMaterialSlots(LegsMesh, LegsHiddenSections);

	// 切换角色时新角色第一帧手臂动画实例与姿势尚未就绪，此刻播放拔枪蒙太奇会导致
	// 武器起始位置错乱。先藏起手臂+当前武器，待下一帧姿势就绪后由 RevealArmsAndWeapon
	// 播放拔枪动画并恢复显示。影子/腿同样先藏，避免首帧参考姿势闪现。
	GetMesh()->SetVisibility(false, true);
	if (ArmsViewMesh)   { ArmsViewMesh->SetVisibility(false, true); }
	if (ShadowBodyMesh) { ShadowBodyMesh->SetVisibility(false, true); }
	if (LegsMesh)       { LegsMesh->SetVisibility(false, true); }
	if (EquipmentManager)
	{
		if (AEquipmentBase* Current = EquipmentManager->GetCurrentEquipment())
		{
			Current->SetActorHiddenInGame(true);
		}
	}
	GetWorldTimerManager().SetTimerForNextTick(this, &AFPSCharacterBase::RevealArmsAndWeapon);

	LastControlRotation = GetControlRotation();
}

void AFPSCharacterBase::RevealArmsAndWeapon()
{
	// 此时手臂动画实例与姿势已就绪：先播放当前武器拔枪动画（从第 0 帧干净开始），
	// 再恢复手臂与武器显示，消除切角色时武器起始位置错乱。
	if (EquipmentManager)
	{
		if (AEquipmentBase* Current = EquipmentManager->GetCurrentEquipment())
		{
			Current->PlayEquipMontage();
			Current->SetActorHiddenInGame(false);
		}
	}
	GetMesh()->SetVisibility(true, true);
	// FEAT-038/042：姿势就绪后一并恢复 FP 手臂/影子/腿显示。
	if (ArmsViewMesh)   { ArmsViewMesh->SetVisibility(true, true); }
	if (ShadowBodyMesh) { ShadowBodyMesh->SetVisibility(true, true); }
	if (LegsMesh)       { LegsMesh->SetVisibility(true, true); }
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
	bIsSprinting = true;
	if (GetCharacterMovement())
	{
		GetCharacterMovement()->MaxWalkSpeed = SprintSpeed;
	}
}

void AFPSCharacterBase::StopSprint()
{
	bIsSprinting = false;
	if (GetCharacterMovement())
	{
		GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
	}
}

void AFPSCharacterBase::Move(const FInputActionValue& Value)
{
	if (!Controller) { return; }
	const FVector2D V = Value.Get<FVector2D>();
	if (V.IsNearlyZero()) { return; }

	const FRotator YawRotation(0, Controller->GetControlRotation().Yaw, 0);
	AddMovementInput(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X), V.Y);
	AddMovementInput(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y), V.X);
}

void AFPSCharacterBase::Look(const FInputActionValue& Value)
{
	if (!Controller) { return; }
	const FVector2D V = Value.Get<FVector2D>();
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

	// 身体保持直立并直接跟随 Pawn yaw；相机俯仰不传给身体组件。
	if (BodyRoot)
	{
		BodyRoot->SetWorldRotation(FRotator(0.f, GetActorRotation().Yaw, 0.f));
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

	// 武器摇摆 / viewmodel 相对偏移——只作用到相机子级 viewmodel。
	// 不再碰 GetMesh()：它是 MM 宿主，朝向归 MM（OffsetRootBone）管，外部转无效。
	// 不再手动按 pitch 转父级：ArmsViewMesh 挂在 HeadCamera 下，已天然继承相机旋转，支点就是相机原点。
	if (ArmsViewMesh && ViewmodelRoot)
	{
		const FRotator CurrentControlRot = GetControlRotation();
		const FRotator RotDelta = (CurrentControlRot - LastControlRotation).GetNormalized();
		LastControlRotation = CurrentControlRot;

		// Sway disabled for now while tuning the independent FP arms.
		// const FRotator SwayTarget(
		// 	-RotDelta.Pitch * SwayIntensity,
		// 	-RotDelta.Yaw   * SwayIntensity,
		// 	-RotDelta.Yaw   * SwayIntensity
		// );
		// CurrentSway.Pitch = FMath::FInterpTo(CurrentSway.Pitch, SwayTarget.Pitch, DeltaTime, SwayInterpSpeedY);
		// CurrentSway.Yaw   = FMath::FInterpTo(CurrentSway.Yaw,   SwayTarget.Yaw,   DeltaTime, SwayInterpSpeedX);
		// CurrentSway.Roll  = FMath::FInterpTo(CurrentSway.Roll,  SwayTarget.Roll,  DeltaTime, SwayInterpSpeedX);
		CurrentSway = FRotator::ZeroRotator;

		// 手臂基础朝向（BaseArmsRotation 含骨架 Yaw -90 转正）+ 摇摆，作用在 ArmsViewMesh 上。
		ArmsViewMesh->SetRelativeRotation(BaseArmsRotation + CurrentSway);
		CurrentArmsPitch = 0.f;
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
