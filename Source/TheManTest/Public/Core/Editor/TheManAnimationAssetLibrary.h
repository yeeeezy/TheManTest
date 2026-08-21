#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "TheManAnimationAssetLibrary.generated.h"

class UAnimationAsset;
class UAnimInstance;
class UAnimBlueprint;
class UAnimSequence;
class UControlRigBlueprint;
class USkeleton;
class UBlueprint;
class USkeletalMesh;

// Small editor-facing repair hook used by destination-only asset migration workflows.
UCLASS()
class THEMANTEST_API UTheManAnimationAssetLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category="TheManTest|Editor|Animation")
	static bool AssignAnimationSkeleton(UAnimationAsset* AnimationAsset, USkeleton* Skeleton);

	/** Build Walk + grounded support-leg pose -> Control Rig -> Output. */
	UFUNCTION(BlueprintCallable, Category="TheManTest|Editor|Animation")
	static UAnimBlueprint* CreateControlRigAnimBlueprint(
		const FString& PackagePath,
		const FString& AssetName,
		UAnimSequence* SourceAnimation,
		UControlRigBlueprint* ControlRigBlueprint);

	/** Insert FirstPersonPoseSource -> Copy Pose above BlendBone before the main AnimGraph output. */
	UFUNCTION(BlueprintCallable, Category="TheManTest|Editor|Animation")
	static bool ConfigureFirstPersonUpperBodyCopy(
		UAnimBlueprint* AnimBlueprint,
		FName SourcePropertyName,
		FName BlendBoneName);

	/** Move the existing first-person locomotion graph into a firearm interface template,
	 * then replace the host state-machine output with a linked layer call. */
	UFUNCTION(BlueprintCallable, Category="TheManTest|Editor|Animation")
	static bool ConfigureFirstPersonFirearmLinkedLayer(
		UAnimBlueprint* HostAnimBlueprint,
		UAnimBlueprint* FirearmTemplateAnimBlueprint,
		UAnimBlueprint* ConcreteFirearmAnimBlueprint,
		UAnimBlueprint* AnimLayerInterface,
		FName LayerName);

	UFUNCTION(BlueprintCallable, Category="TheManTest|Editor|Animation")
	static bool AddAnimationAssetOverride(
		UAnimBlueprint* AnimBlueprint,
		UAnimationAsset* Target,
		UAnimationAsset* Override);

	UFUNCTION(BlueprintCallable, Category="TheManTest|Editor|Blueprint")
	static bool SetInheritedSceneComponentRotation(
		UBlueprint* Blueprint,
		FName ComponentVariableName,
		FRotator RelativeRotation);

	UFUNCTION(BlueprintCallable, Category="TheManTest|Editor|Blueprint")
	static bool SetInheritedSceneComponentTransform(
		UBlueprint* Blueprint,
		FName ComponentVariableName,
		FVector RelativeLocation,
		FRotator RelativeRotation,
		FVector RelativeScale);

	UFUNCTION(BlueprintCallable, Category="TheManTest|Editor|Blueprint")
	static bool SetInheritedSkeletalMesh(
		UBlueprint* Blueprint,
		FName ComponentVariableName,
		USkeletalMesh* SkeletalMesh);

	UFUNCTION(BlueprintCallable, Category="TheManTest|Editor|Blueprint")
	static bool SetInheritedAnimClass(
		UBlueprint* Blueprint,
		FName ComponentVariableName,
		TSubclassOf<UAnimInstance> AnimClass);

	/** Session-only helper used to open a data-only Blueprint in the full component viewport. */
	UFUNCTION(BlueprintCallable, Category="TheManTest|Editor|Blueprint")
	static bool SetBlueprintForceFullEditor(UBlueprint* Blueprint, bool bForceFullEditor);

	UFUNCTION(BlueprintCallable, Category="TheManTest|Editor|Blueprint")
	static bool FocusBlueprintComponentViewport(UBlueprint* Blueprint);

	UFUNCTION(BlueprintCallable, Category="TheManTest|Editor|Blueprint")
	static bool SetBlueprintComponentViewportView(UBlueprint* Blueprint, const FString& ViewName);
};
