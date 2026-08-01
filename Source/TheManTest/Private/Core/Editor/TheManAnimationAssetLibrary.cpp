#include "Core/Editor/TheManAnimationAssetLibrary.h"
#include "Animation/AnimationAsset.h"
#include "Animation/Skeleton.h"

bool UTheManAnimationAssetLibrary::AssignAnimationSkeleton(UAnimationAsset* AnimationAsset, USkeleton* Skeleton)
{
#if WITH_EDITOR
	if (!AnimationAsset || !Skeleton) return false;
	AnimationAsset->Modify();
	AnimationAsset->SetSkeleton(Skeleton);
	AnimationAsset->ValidateSkeleton();
	AnimationAsset->MarkPackageDirty();
	return AnimationAsset->GetSkeleton() == Skeleton;
#else
	return false;
#endif
}
