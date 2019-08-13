// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Carla/Sensor/Sensor.h"

#include "Carla/Actor/ActorDefinition.h"
#include "Carla/Sensor/LidarDescription.h"

#include <compiler/disable-ue4-macros.h>
#include <carla/sensor/s11n/ObjectLidarMeasurement.h>
#include <compiler/enable-ue4-macros.h>

#include "RayCastObjectLidar.generated.h"

/// A ray-cast based Lidar sensor.
UCLASS()
class CARLA_API ARayCastObjectLidar : public ASensor
{
  GENERATED_BODY()

  using FLidarMeasurement = carla::sensor::s11n::ObjectLidarMeasurement;

public:

  static FActorDefinition GetSensorDefinition();

  ARayCastObjectLidar(const FObjectInitializer &ObjectInitializer);

  void Set(const FActorDescription &Description) override;

  void Set(const FLidarDescription &LidarDescription);

protected:

  virtual void Tick(float DeltaTime) override;

private:

  /// Creates a Laser for each channel.
  void CreateLasers();

  /// Updates LidarMeasurement with the points read in DeltaTime.
  void ReadPoints(float DeltaTime);

  /// Shoot a laser ray-trace, return whether the laser hit something.
  bool ShootLaser(uint32 Channel, float HorizontalAngle, FVector &Point, FVector &Velocity, float &Intensity, uint32_t & ObjectId) const;

  UPROPERTY(EditAnywhere)
  FLidarDescription Description;

  TArray<float> LaserAngles;

  FLidarMeasurement LidarMeasurement;
};
