#include "Characters/MaintenanceWorker/FPSMaintenanceWorker.h"
#include "GameFramework/CharacterMovementComponent.h"

void AFPSMaintenanceWorker::BeginPlay()
{
	// VFXPack FirstPersonCharacter 精确移动默认值。角色 BP 曾序列化 100/300，必须在
	// Super::BeginPlay 写入 MaxWalkSpeed 前覆盖，保证原 BlendSpace 看到 550/750。
	WalkSpeed = 550.f;
	SprintSpeed = 750.f;
	Super::BeginPlay();

	GetCharacterMovement()->MaxAcceleration = 2000.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 750.f;
}
