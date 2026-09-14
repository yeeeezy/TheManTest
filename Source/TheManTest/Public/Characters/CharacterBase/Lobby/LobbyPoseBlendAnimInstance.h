#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimSingleNodeInstance.h"
#include "LobbyPoseBlendAnimInstance.generated.h"

/** Plays authored lobby sequences, blending from the last displayed pose without IK. */
UCLASS(Transient)
class THEMANTEST_API ULobbyPoseBlendAnimInstance : public UAnimSingleNodeInstance
{
	GENERATED_BODY()
public:
	bool PlayPose(UAnimationAsset* Animation, bool bBlend);
	void SetPoseBlendAlpha(float Alpha);
protected:
	virtual FAnimInstanceProxy* CreateAnimInstanceProxy() override;
};
