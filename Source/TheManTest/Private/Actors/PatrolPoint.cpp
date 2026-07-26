#include "Actors/PatrolPoint.h"
#include "Components/SphereComponent.h"

APatrolPoint::APatrolPoint()
{
	PrimaryActorTick.bCanEverTick = false;

	Sphere = CreateDefaultSubobject<USphereComponent>(TEXT("Sphere"));
	Sphere->SetSphereRadius(30.f);
	Sphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SetRootComponent(Sphere);
}
