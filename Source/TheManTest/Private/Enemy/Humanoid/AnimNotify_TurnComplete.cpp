#include "Enemy/Humanoid/AnimNotify_TurnComplete.h"
#include "Enemy/Humanoid/HumanoidEnemy.h"
#include "Components/SkeletalMeshComponent.h"

void UAnimNotify_TurnComplete::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                      const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (!MeshComp) return;

	if (AHumanoidEnemy* Enemy = Cast<AHumanoidEnemy>(MeshComp->GetOwner()))
	{
		Enemy->OnTurnComplete();
	}
}
