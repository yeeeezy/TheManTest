#include "Core/TheManPlayerController.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/DataTable.h"
#include "Core/TheManCharacterTypes.h"
#include "Core/TheManGameStateBase.h"
#include "Characters/FPSCharacterBase/FPSCharacterBase.h"

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
}

void ATheManPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	// Controller 只处理元操作（角色切换）；所有角色输入由各 Character 自行绑定
	if (UEnhancedInputComponent* EIC = CastChecked<UEnhancedInputComponent>(InputComponent))
	{
		if (TestSwitchCharacterAction)
		{
			EIC->BindAction(TestSwitchCharacterAction, ETriggerEvent::Started,
				this, &ATheManPlayerController::HandleTestSwitchCharacter);
		}

		// 调试快进：仅在填了 IA 资产时绑定
		if (DebugSkipTimeAction)
		{
			EIC->BindAction(DebugSkipTimeAction, ETriggerEvent::Started,
				this, &ATheManPlayerController::HandleDebugSkipTime);
		}
	}
}

void ATheManPlayerController::HandleTestSwitchCharacter()
{
	SwitchCharacter(FName("MaintenanceWorker"));
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
