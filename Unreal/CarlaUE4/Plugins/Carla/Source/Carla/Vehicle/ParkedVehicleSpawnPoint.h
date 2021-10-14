// Copyright (c) 2021 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Engine/TargetPoint.h"
#include "ParkedVehicleSpawnPoint.generated.h"

/// Base class for spawner locations for walkers.
UCLASS()
class CARLA_API AParkedVehicleSpawnPoint : public ATargetPoint
{
  GENERATED_BODY()
};
