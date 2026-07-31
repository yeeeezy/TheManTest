#include "Characters/Enemy/Humanoid/HumanoidAIController.h"
#include "Characters/Enemy/Humanoid/HumanoidEnemy.h"
#include "Characters/Enemy/Humanoid/HumanoidEnemyTypes.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Kismet/GameplayStatics.h"

const FName AHumanoidAIController::BB_TargetActor              = "TargetActor";
const FName AHumanoidAIController::BB_LastKnownPlayerLocation  = "LastKnownPlayerLocation";

AHumanoidAIController::AHumanoidAIController()
{
	UAISenseConfig_Sight* SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
	SightConfig->SightRadius                              = 1500.f;
	SightConfig->LoseSightRadius                          = 1800.f;
	SightConfig->PeripheralVisionAngleDegrees             = 60.f;
	SightConfig->DetectionByAffiliation.bDetectEnemies    = true;
	SightConfig->DetectionByAffiliation.bDetectNeutrals   = true;
	SightConfig->DetectionByAffiliation.bDetectFriendlies = false;

	PerceptionComp = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("PerceptionComp"));
	PerceptionComp->ConfigureSense(*SightConfig);
	PerceptionComp->SetDominantSense(SightConfig->GetSenseImplementation());
	SetPerceptionComponent(*PerceptionComp);
}

void AHumanoidAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	if (BehaviorTree)
	{
		RunBehaviorTree(BehaviorTree);
	}

	PerceptionComp->OnTargetPerceptionUpdated.AddDynamic(
		this, &AHumanoidAIController::OnTargetPerceptionUpdated);
}

void AHumanoidAIController::OnUnPossess()
{
	PerceptionComp->OnTargetPerceptionUpdated.RemoveDynamic(
		this, &AHumanoidAIController::OnTargetPerceptionUpdated);
	Super::OnUnPossess();
}

void AHumanoidAIController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	AHumanoidEnemy* Enemy = Cast<AHumanoidEnemy>(GetPawn());
	if (!Enemy) return;

	// 战斗中每帧把瞄准点更新为玩家位置：供 AimIK（FEAT-031）与子弹方向使用
	if (Enemy->GetAIState() == EHumanoidEnemyAIState::Aim)
	{
		if (APawn* Player = UGameplayStatics::GetPlayerPawn(this, 0))
		{
			Enemy->AimTargetWorld = Player->GetActorLocation();
			Enemy->bIsAiming      = true;
		}
	}
	else
	{
		Enemy->bIsAiming = false;
	}
}

void AHumanoidAIController::OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
	// 只关心玩家
	if (Actor != UGameplayStatics::GetPlayerPawn(this, 0)) return;

	AHumanoidEnemy* Enemy = Cast<AHumanoidEnemy>(GetPawn());
	if (!Enemy) return;

	UBlackboardComponent* BB = GetBlackboardComponent();

	if (Stimulus.WasSuccessfullySensed())
	{
		// 发现玩家：写黑板目标 + Focus 锁定朝向 + 切战斗（SetAIState(Aim) 内已停巡逻、转 Focus 朝向）
		if (BB) { BB->SetValueAsObject(BB_TargetActor, Actor); }
		SetFocus(Actor);
		Enemy->SetAIState(EHumanoidEnemyAIState::Aim);
	}
	else
	{
		// 丢失玩家：记录最后位置、清目标，然后启动公共 SearchRush → SearchScan → Patrol 流程。
		if (BB)
		{
			BB->SetValueAsVector(BB_LastKnownPlayerLocation, Stimulus.StimulusLocation);
			BB->ClearValue(BB_TargetActor);
		}
		ClearFocus(EAIFocusPriority::Gameplay);
		Enemy->StartLostTargetSearch(Stimulus.StimulusLocation);
	}
}
