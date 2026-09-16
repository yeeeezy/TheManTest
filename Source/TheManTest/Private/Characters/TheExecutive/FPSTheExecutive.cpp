#include "Characters/TheExecutive/FPSTheExecutive.h"
#include "Characters/TheExecutive/Drone/ExecutiveDroneComponent.h"
AFPSTheExecutive::AFPSTheExecutive()
{
 DroneCompanion=CreateDefaultSubobject<UExecutiveDroneComponent>(TEXT("DroneCompanion"));
}
