// Fill out your copyright notice in the Description page of Project Settings.

#include "Weapons/_Shared/Firearms/FirearmAnimInstance.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Controller.h"
#include "Components/SkeletalMeshComponent.h"

void UFirearmAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	USkeletalMeshComponent* Mesh = GetOwningComponent();
	if (!Mesh) return;

	APawn* PawnOwner = Cast<APawn>(GetOwningActor());
	if (!PawnOwner) return;

	AController* Controller = PawnOwner->GetController();
	if (!Controller) return;

	FVector ViewLocation;
	FRotator ViewRotation;
	Controller->GetPlayerViewPoint(ViewLocation, ViewRotation);

	const FVector TraceEnd = ViewLocation + ViewRotation.Vector() * 10000.0f;

	FHitResult HitResult;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(PawnOwner);

	const bool bHit = GetWorld()->LineTraceSingleByChannel(
		HitResult,
		ViewLocation,
		TraceEnd,
		ECC_Visibility,
		Params
	);

	const FVector TargetWorld = bHit ? HitResult.ImpactPoint : TraceEnd;
	AimTargetComponentSpace = Mesh->GetComponentTransform().InverseTransformPosition(TargetWorld);
	bHasValidAimTarget = true;

	// TODO: 接入输入动作后替换为实际瞄准状态
	bIsAiming = true;
}
