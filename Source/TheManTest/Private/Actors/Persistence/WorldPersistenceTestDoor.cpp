#include "Actors/Persistence/WorldPersistenceTestDoor.h"
#include "Core/Persistence/PersistentStateComponent.h"
#include "Components/SceneComponent.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Pawn.h"
#include "UObject/ConstructorHelpers.h"

#if WITH_DEV_AUTOMATION_TESTS
TSet<FGuid> AWorldPersistenceTestDoor::BeginPlayIdsForTests;
#endif

AWorldPersistenceTestDoor::AWorldPersistenceTestDoor()
{
	PrimaryActorTick.bCanEverTick = false;

	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	DoorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DoorMesh"));
	DoorMesh->SetupAttachment(Root);
	DoorMesh->SetRelativeScale3D(FVector(0.15f, 1.f, 1.5f));
	DoorMesh->SetCollisionProfileName(TEXT("BlockAll"));

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (CubeMesh.Succeeded())
	{
		DoorMesh->SetStaticMesh(CubeMesh.Object);
	}

	PersistentState = CreateDefaultSubobject<UPersistentStateComponent>(TEXT("PersistentState"));

	OpenTrigger = CreateDefaultSubobject<UBoxComponent>(TEXT("OpenTrigger"));
	OpenTrigger->SetupAttachment(Root);
	OpenTrigger->SetBoxExtent(FVector(160.f, 160.f, 180.f));
	OpenTrigger->SetCollisionProfileName(TEXT("Trigger"));
	OpenTrigger->OnComponentBeginOverlap.AddDynamic(
		this, &AWorldPersistenceTestDoor::HandleTriggerBeginOverlap);
}

void AWorldPersistenceTestDoor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	ApplyDoorVisual();
}

void AWorldPersistenceTestDoor::BeginPlay()
{
#if WITH_DEV_AUTOMATION_TESTS
	BeginPlayIdsForTests.Add(PersistentState->PersistentId);
#endif
	Super::BeginPlay();
	bWasOpenAtBeginPlay = bIsOpen;
	TransformAtBeginPlay = GetActorTransform();
}

#if WITH_DEV_AUTOMATION_TESTS
void AWorldPersistenceTestDoor::ResetBeginPlayHistoryForTests()
{
	BeginPlayIdsForTests.Reset();
}

bool AWorldPersistenceTestDoor::DidPersistentIdBeginPlayForTests(const FGuid& PersistentId)
{
	return BeginPlayIdsForTests.Contains(PersistentId);
}
#endif

void AWorldPersistenceTestDoor::ToggleDoor()
{
	SetDoorOpen(!bIsOpen);
}

void AWorldPersistenceTestDoor::SetDoorOpen(bool bOpen)
{
	bIsOpen = bOpen;
	ApplyDoorVisual();
}

void AWorldPersistenceTestDoor::ApplyDoorVisual()
{
	if (DoorMesh)
	{
		DoorMesh->SetRelativeRotation(FRotator(0.f, bIsOpen ? OpenYaw : 0.f, 0.f));
	}
}

void AWorldPersistenceTestDoor::HandleTriggerBeginOverlap(
	UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent,
	int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor && OtherActor->IsA<APawn>())
	{
		SetDoorOpen(true);
	}
}

void AWorldPersistenceTestDoor::CapturePersistentState_Implementation(FInstancedStruct& OutPersistentState) const
{
	FWorldPersistenceTestDoorState State;
	State.bIsOpen = bIsOpen;
	OutPersistentState = FInstancedStruct::Make(State);
}

void AWorldPersistenceTestDoor::ApplyPersistentState_Implementation(const FInstancedStruct& PersistentStateData)
{
	if (const FWorldPersistenceTestDoorState* State =
		PersistentStateData.GetPtr<FWorldPersistenceTestDoorState>())
	{
		SetDoorOpen(State->bIsOpen);
	}
}
