#include "Enemy/Boss/CoreMorph/Movement/CoreMorphFlightRoute.h"
#include "Components/SplineComponent.h"

ACoreMorphFlightRoute::ACoreMorphFlightRoute()
{
	PrimaryActorTick.bCanEverTick = false;
	Spline = CreateDefaultSubobject<USplineComponent>(TEXT("FlightRoute"));
	SetRootComponent(Spline);
	Spline->SetSplinePoints({FVector::ZeroVector, FVector(15000, 0, 8000), FVector(30000, 12000, 16000)}, ESplineCoordinateSpace::Local);
}
