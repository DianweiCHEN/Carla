// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Sensor/Sensor.h"

#include "Sensor/LidarMeasurement.h"
#include "Settings/LidarDescription.h"

#include "LidarGPU.generated.h"

/// A GPU depth based Lidar sensor.
UCLASS()
class CARLA_API ALidarGPU : public ASensor
{
  GENERATED_BODY()
	
public:	
	
  ALidarGPU(const FObjectInitializer &ObjectInitializer);

  void Set(const ULidarDescription &LidarDescription);

protected:

  virtual void Tick(float DeltaTime) override;

private:	
  
  /// Creates a Laser for each channel.
  void CreateLasers();

  void CreateSceneCapture2D();

  void Rotate(float DeltaTime);

  /// Updates LidarMeasurement with the points read in DeltaTime.
  void ReadPoints(float DeltaTime);

  /// Shoot a laser ray-trace, return whether the laser hit something.
  bool ShootLaser(uint32 Channel, float HorizontalAngle, FVector &Point) const;

  /// Debug Lidar visualization
  void RenderDebugLidar();

  UPROPERTY(Category = "Lidar", VisibleAnywhere)
  const ULidarDescription *Description = nullptr;

  TArray<float> LaserAngles;

  FLidarMeasurement LidarMeasurement;

};
