#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AnimNotify_TurnComplete.generated.h"

// 挂在 Turn 动画末尾，通知 AHumanoidEnemy 转身完成，清除 bPendingTurn 标志
UCLASS()
class THEMANTEST_API UAnimNotify_TurnComplete : public UAnimNotify
{
	GENERATED_BODY()

public:
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	                    const FAnimNotifyEventReference& EventReference) override;
};
