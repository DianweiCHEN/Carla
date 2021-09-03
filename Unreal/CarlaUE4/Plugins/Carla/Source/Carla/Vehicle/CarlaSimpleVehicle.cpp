// Copyright (c) 2021 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

PRAGMA_DISABLE_DEPRECATION_WARNINGS

#include "Vehicle/CarlaSimpleVehicle.h"

FName ACarlaSimpleVehicle::VehicleMovementComponentName(TEXT("MovementComp"));
FName ACarlaSimpleVehicle::VehicleMeshComponentName(TEXT("VehicleMesh"));


ACarlaSimpleVehicle::ACarlaSimpleVehicle(const FObjectInitializer& ObjectInitializer) 
  : Super(ObjectInitializer)
{
  Mesh = CreateDefaultSubobject<USkeletalMeshComponent>(VehicleMeshComponentName);
  Mesh->SetCollisionProfileName(UCollisionProfile::Vehicle_ProfileName);
  Mesh->BodyInstance.bSimulatePhysics = true;
  Mesh->BodyInstance.bNotifyRigidBodyCollision = true;
  Mesh->BodyInstance.bUseCCD = true;
  Mesh->bBlendPhysics = true;
  Mesh->SetGenerateOverlapEvents(true);
  Mesh->SetCanEverAffectNavigation(false);
  RootComponent = Mesh;

  VehicleMovement = CreateDefaultSubobject<UWheeledVehicleMovementComponent, USimpleWheeledVehicleMovementComponent>(VehicleMovementComponentName);
  VehicleMovement->SetIsReplicated(true); // Enable replication by default
  VehicleMovement->UpdatedComponent = Mesh;
}

ACarlaSimpleVehicle::~ACarlaSimpleVehicle()
{
  // Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
  PrimaryActorTick.bCanEverTick = false;
}

// Called when the game starts or when spawned
void ACarlaSimpleVehicle::BeginPlay()
{
  Super::BeginPlay();
}

// Called every frame
void ACarlaSimpleVehicle::Tick(float DeltaTime)
{
  Super::Tick(DeltaTime);
}

void ACarlaSimpleVehicle::DisplayDebug(UCanvas* Canvas, const FDebugDisplayInfo& DebugDisplay, float& YL, float& YPos)
{
  static FName NAME_Vehicle = FName(TEXT("Vehicle"));

  Super::DisplayDebug(Canvas, DebugDisplay, YL, YPos);

  if (DebugDisplay.IsDisplayOn(NAME_Vehicle))
  {
#if WITH_PHYSX && PHYSICS_INTERFACE_PHYSX
    GetVehicleMovementComponent()->DrawDebug(Canvas, YL, YPos);
#endif // WITH_PHYSX
  }
}

class UWheeledVehicleMovementComponent* ACarlaSimpleVehicle::GetVehicleMovementComponent() const
{
  return VehicleMovement;
}


PRAGMA_ENABLE_DEPRECATION_WARNINGS

