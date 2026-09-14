#include "Core/TheManPlayerController.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/DataTable.h"
#include "Core/TheManCharacterTypes.h"
#include "Core/TheManGameStateBase.h"
#include "Core/TheManPlayerState.h"
#include "Characters/CharacterBase/TheManAttributeSetBase.h"
#include "Characters/CharacterBase/FPSCharacterBase/FPSCharacterBase.h"
#include "UI/Combat/CombatHUDWidgetBase.h"
#include "Weapons/_Shared/Components/EquipmentManagerComponent.h"
#include "Weapons/_Shared/EquipmentBase/EquipmentBase.h"
#include "Weapons/_Shared/Firearms/Firearm.h"
#include "Blueprint/UserWidget.h"
#include "AbilitySystemComponent.h"
#if WITH_EDITOR
#include "Enemy/Boss/CoreMorph/CoreMorphBoss.h"
#include "Enemy/Boss/CoreMorph/Movement/CoreMorphFlightComponent.h"
#include "Enemy/Boss/CoreMorph/Movement/CoreMorphFlightRoute.h"
#include "Enemy/Boss/CoreMorph/Combat/CoreMorphScorpionCombat.h"
#include "Components/SplineComponent.h"
#include "AIController.h"
#endif

void ATheManPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
		ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		Subsystem->AddMappingContext(DefaultMappingContext, 0);
	}

	// 重置为游戏输入模式：ViewportClient 跨关卡持久，大厅设的 UI Only 会带进来挡住移动输入
	SetInputMode(FInputModeGameOnly());
	bShowMouseCursor = false;

	if (IsLocalController())
	{
		CreateCombatHUD();
		BindCombatHUDToPawn(GetPawn());
	}
}

void ATheManPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	if (IsLocalController())
	{
		BindCombatHUDToPawn(InPawn);
	}
}

void ATheManPlayerController::OnUnPossess()
{
	ClearInputTest();
	UnbindCombatHUD();
	Super::OnUnPossess();
}

void ATheManPlayerController::CreateCombatHUD()
{
	if (CombatHUDWidget || !IsLocalController())
	{
		return;
	}

	TSubclassOf<UCombatHUDWidgetBase> WidgetClass = CombatHUDWidgetClass;
	if (!WidgetClass)
	{
		WidgetClass = UCombatHUDWidgetBase::StaticClass();
	}

	CombatHUDWidget = CreateWidget<UCombatHUDWidgetBase>(this, WidgetClass);
	if (CombatHUDWidget)
	{
		CombatHUDWidget->AddToPlayerScreen(0);
		CombatHUDWidget->SetVisibility(ESlateVisibility::HitTestInvisible);
		CombatHUDWidget->SetAmmoVisible(false);
		CombatHUDWidget->SetHealthVisible(false);
	}
}

void ATheManPlayerController::BindCombatHUDToPawn(APawn* InPawn)
{
	UnbindCombatHUD();
	if (!CombatHUDWidget)
	{
		CreateCombatHUD();
	}

	AFPSCharacterBase* PlayerCharacter = Cast<AFPSCharacterBase>(InPawn);
	if (!PlayerCharacter || !CombatHUDWidget)
	{
		return;
	}

	BindCombatHUDToHealth();

	BoundEquipmentManager = PlayerCharacter->GetEquipmentManager();
	if (!BoundEquipmentManager)
	{
		return;
	}

	BoundEquipmentManager->OnCurrentEquipmentChanged.AddUniqueDynamic(
		this, &ATheManPlayerController::HandleCurrentEquipmentChanged);
	BindCombatHUDToFirearm(Cast<AFirearm>(BoundEquipmentManager->GetCurrentEquipment()));
}

void ATheManPlayerController::UnbindCombatHUD()
{
	if (BoundEquipmentManager)
	{
		BoundEquipmentManager->OnCurrentEquipmentChanged.RemoveDynamic(
			this, &ATheManPlayerController::HandleCurrentEquipmentChanged);
	}
	if (BoundFirearm)
	{
		BoundFirearm->OnAmmoChanged.RemoveDynamic(this, &ATheManPlayerController::HandleAmmoChanged);
	}
	if (BoundAbilitySystem)
	{
		BoundAbilitySystem->GetGameplayAttributeValueChangeDelegate(UTheManAttributeSetBase::GetHealthAttribute())
			.Remove(HealthChangedDelegateHandle);
		BoundAbilitySystem->GetGameplayAttributeValueChangeDelegate(UTheManAttributeSetBase::GetMaxHealthAttribute())
			.Remove(MaxHealthChangedDelegateHandle);
	}
	BoundEquipmentManager = nullptr;
	BoundFirearm = nullptr;
	BoundAbilitySystem = nullptr;
	HealthChangedDelegateHandle.Reset();
	MaxHealthChangedDelegateHandle.Reset();
	if (CombatHUDWidget)
	{
		CombatHUDWidget->SetAmmoVisible(false);
		CombatHUDWidget->SetHealthVisible(false);
	}
}

void ATheManPlayerController::BindCombatHUDToHealth()
{
	ATheManPlayerState* TheManPlayerState = GetPlayerState<ATheManPlayerState>();
	BoundAbilitySystem = TheManPlayerState ? TheManPlayerState->GetAbilitySystemComponent() : nullptr;
	if (!BoundAbilitySystem || !CombatHUDWidget)
	{
		return;
	}

	HealthChangedDelegateHandle = BoundAbilitySystem
		->GetGameplayAttributeValueChangeDelegate(UTheManAttributeSetBase::GetHealthAttribute())
		.AddUObject(this, &ATheManPlayerController::HandleHealthChanged);
	MaxHealthChangedDelegateHandle = BoundAbilitySystem
		->GetGameplayAttributeValueChangeDelegate(UTheManAttributeSetBase::GetMaxHealthAttribute())
		.AddUObject(this, &ATheManPlayerController::HandleHealthChanged);
	RefreshCombatHUDHealth();
	CombatHUDWidget->SetHealthVisible(true);
}

void ATheManPlayerController::RefreshCombatHUDHealth()
{
	if (!BoundAbilitySystem || !CombatHUDWidget)
	{
		return;
	}

	CombatHUDWidget->SetHealthState(
		BoundAbilitySystem->GetNumericAttribute(UTheManAttributeSetBase::GetHealthAttribute()),
		BoundAbilitySystem->GetNumericAttribute(UTheManAttributeSetBase::GetMaxHealthAttribute()));
}

void ATheManPlayerController::HandleHealthChanged(const FOnAttributeChangeData&)
{
	RefreshCombatHUDHealth();
}

void ATheManPlayerController::BindCombatHUDToFirearm(AFirearm* Firearm)
{
	if (BoundFirearm)
	{
		BoundFirearm->OnAmmoChanged.RemoveDynamic(this, &ATheManPlayerController::HandleAmmoChanged);
	}
	BoundFirearm = Firearm;

	if (!CombatHUDWidget || !BoundFirearm)
	{
		if (CombatHUDWidget)
		{
			CombatHUDWidget->SetAmmoVisible(false);
		}
		return;
	}

	BoundFirearm->OnAmmoChanged.AddUniqueDynamic(this, &ATheManPlayerController::HandleAmmoChanged);
	HandleAmmoChanged(
		BoundFirearm->GetCurrentAmmo(),
		BoundFirearm->GetMagazineCapacity(),
		BoundFirearm->GetSpareMagazineCount());
	CombatHUDWidget->SetAmmoVisible(true);
}

void ATheManPlayerController::HandleCurrentEquipmentChanged(
	AEquipmentBase* PreviousEquipment,
	AEquipmentBase* CurrentEquipment)
{
	BindCombatHUDToFirearm(Cast<AFirearm>(CurrentEquipment));
}

void ATheManPlayerController::HandleAmmoChanged(
	int32 CurrentAmmo,
	int32 MagazineCapacity,
	int32 SpareMagazineCount)
{
	if (CombatHUDWidget)
	{
		CombatHUDWidget->SetAmmoState(CurrentAmmo, MagazineCapacity, SpareMagazineCount);
	}
}

void ATheManPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	// Controller 处理预留测试入口和元操作；角色输入仍由各 Character 自行绑定
	if (UEnhancedInputComponent* EIC = CastChecked<UEnhancedInputComponent>(InputComponent))
	{
		if (TestSwitchCharacterAction)
		{
			EIC->BindAction(TestSwitchCharacterAction, ETriggerEvent::Started,
				this, &ATheManPlayerController::HandleTestInput);
		}

		// 调试快进：仅在填了 IA 资产时绑定
		if (DebugSkipTimeAction)
		{
			EIC->BindAction(DebugSkipTimeAction, ETriggerEvent::Started,
				this, &ATheManPlayerController::HandleDebugSkipTime);
		}
	}
}

void ATheManPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	ClearInputTest();
	Super::EndPlay(EndPlayReason);
}

void ATheManPlayerController::ClearInputTest()
{
#if WITH_EDITOR
	if (AActor* AI = InputTestController.Get()) AI->Destroy();
	if (AActor* Subject = InputTestSubject.Get())
	{
		Subject->Destroy(); // Boss EndPlay cancels GA and removes Cue/components.
	}
	if (AActor* Route = InputTestRoute.Get()) Route->Destroy();
#endif
	InputTestSubject.Reset();
	InputTestRoute.Reset();
	InputTestController.Reset();
}

void ATheManPlayerController::HandleTestInput()
{
#if WITH_EDITOR
	// IA_Test is the user's reserved test slot. Replace this scenario for future work;
	// never add a second mapping or save its transient objects into TestMap.
	if (!IsLocalController() || !GetWorld() || GetWorld()->WorldType != EWorldType::PIE) return;
	if (InputTestSubject.IsValid() || InputTestRoute.IsValid())
	{
		ClearInputTest();
		ClientMessage(TEXT("Manta missile test cleared."));
		return;
	}
	APawn* PlayerPawn = GetPawn();
	if (!IsValid(PlayerPawn)) return;
	UClass* BossClass = LoadClass<ACoreMorphBoss>(nullptr,
		TEXT("/Game/Enemy/Boss/CoreMorph/Blueprint/BP_CoreMorphBoss.BP_CoreMorphBoss_C"));
	if (!BossClass) return;

	FVector Center = PlayerPawn->GetActorLocation();
	FHitResult Ground;
	FCollisionQueryParams Query(SCENE_QUERY_STAT(ReservedInputTestGround), false, PlayerPawn);
	if (!GetWorld()->LineTraceSingleByObjectType(Ground, Center + FVector(0, 0, 500),
		Center - FVector(0, 0, 15000), FCollisionObjectQueryParams(ECC_WorldStatic), Query))
	{
		ClientMessage(TEXT("Manta missile test needs ground below the player."));
		return;
	}
	Center.Z = Ground.ImpactPoint.Z + 6500;
	const FVector Forward = FRotator(0, GetControlRotation().Yaw, 0).Vector();
	const FVector Side = FVector::CrossProduct(FVector::UpVector, Forward);
	const FVector Start = Center + Forward * 14000;
	FActorSpawnParameters Spawn;
	Spawn.Owner = this;
	Spawn.ObjectFlags |= RF_Transient;
	Spawn.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	auto* Route = GetWorld()->SpawnActor<ACoreMorphFlightRoute>(
		ACoreMorphFlightRoute::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, Spawn);
	InputTestRoute = Route;
	if (!Route) return;
	Route->Spline->ClearSplinePoints(false);
	for (int32 I = 0; I < 8; ++I)
	{
		const float Angle = I * UE_TWO_PI / 8;
		Route->Spline->AddSplinePoint(Center + (Forward * FMath::Cos(Angle) + Side * FMath::Sin(Angle)) * 14000,
			ESplineCoordinateSpace::World, false);
	}
	Route->Spline->SetClosedLoop(true, true);
	auto* Boss = GetWorld()->SpawnActor<ACoreMorphBoss>(BossClass, Start, Side.Rotation(), Spawn);
	InputTestSubject = Boss;
	InputTestController = Boss ? Boss->GetController() : nullptr;
	if (!Boss) { ClearInputTest(); return; }
	Boss->Flight->FlightRoute = Route;
	Boss->Flight->RouteSpeed = 6000;
	Boss->ScorpionCombat->bEnabled = true; // Saved main BT owns flight and ranged GA decisions.
	Boss->LastThreat = PlayerPawn;
	SetControlRotation((Start - PlayerPawn->GetPawnViewLocation()).Rotation());
	ClientMessage(TEXT("Manta missile test started. Press 1 again to clear."));
#endif
}

void ATheManPlayerController::HandleDebugSkipTime()
{
	if (ATheManGameStateBase* GS = GetWorld()->GetGameState<ATheManGameStateBase>())
	{
		GS->DebugSkipTime();   // 默认 150s = 2.5 分钟
	}
}

void ATheManPlayerController::SwitchCharacter(FName TargetCharacterID)
{
	if (!CharacterRosterTable) { return; }

	FCharacterType* CharRow = CharacterRosterTable->FindRow<FCharacterType>(
		TargetCharacterID, TEXT("SwitchCharacter"));
	if (!CharRow || !CharRow->CharacterClass) { return; }

	APawn* CurrentPawn = GetPawn();
	FVector  NewLocation = CurrentPawn ? CurrentPawn->GetActorLocation() : FVector::ZeroVector;
	FRotator NewRotation = CurrentPawn ? CurrentPawn->GetActorRotation() : FRotator::ZeroRotator;

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AFPSCharacterBase* NewCharacter = GetWorld()->SpawnActor<AFPSCharacterBase>(
		CharRow->CharacterClass, NewLocation, NewRotation, SpawnParams);

	if (NewCharacter)
	{
		Possess(NewCharacter);
		if (CurrentPawn) { CurrentPawn->Destroy(); }
	}
}
