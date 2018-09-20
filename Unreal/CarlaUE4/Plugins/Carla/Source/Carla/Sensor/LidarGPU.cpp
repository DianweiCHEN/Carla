// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"
#include "LidarGPU.h"

#include "DrawDebugHelpers.h"
#include "Engine/CollisionProfile.h"
#include "Runtime/Engine/Classes/Kismet/KismetMathLibrary.h"
//#include "Runtime/Engine/Classes/Components/SceneCaptureComponent2D.h"
#include "../Sensor/SceneCaptureCamera.h"
#include "StaticMeshResources.h"

class Dummy : public ISensorDataSink {
public:
  
  virtual void Write(const FSensorDataView &) {}
};

ALidarGPU::ALidarGPU(const FObjectInitializer &ObjectInitializer)
  : Super(ObjectInitializer)
{
  PrimaryActorTick.bCanEverTick = true;
  SetSensorDataSink(MakeShared<Dummy>());

  RootComponent = ObjectInitializer.CreateDefaultSubobject<USceneComponent>(this, TEXT("SceneComponent"));
}

void ALidarGPU::Set(const ULidarDescription &LidarDescription)
{
  Super::Set(LidarDescription);
  Description = &LidarDescription;
  LidarMeasurement = FLidarMeasurement(GetId(), Description->Channels);
  CreateLasers();
}

void ALidarGPU::Tick(float DeltaTime)
{
  Super::Tick(DeltaTime);

  ReadPoints(DeltaTime);
  WriteSensorData(LidarMeasurement.GetView());
  Rotate(DeltaTime);
  RenderDebugLidar();
}

void ALidarGPU::CreateLasers()
{
}

void ALidarGPU::Rotate(float DeltaTime)
{
  // this will be read from some LidarGPUDescription struct in the future
  static const float ChannelCount = 32u;
  static const float Range = 5000.0f;
  static const uint32 PointsPerSecond = 56000u;
  static const float RotationFrequency = 0.5f;
  static const float UFLim = 10.0f;
  static const float LFLim = 30.0f;;
  static const bool ShowDebugPoints = false;

  const uint32 PointsToScanWithOneLaser =
    FMath::RoundHalfFromZero(
      PointsPerSecond * DeltaTime / float(ChannelCount));

  if (PointsToScanWithOneLaser <= 0)
  {
    UE_LOG(LogCarla, Warning, 
      TEXT("%s: no points requested this frame, try increasing the number of points per second."), *GetName());
    return;
  }

  const float AngleDistanceOfTick = RotationFrequency * 360.0f * DeltaTime;
  const float AngleDistanceOfLaserMeasure = AngleDistanceOfTick / PointsToScanWithOneLaser;

  AddActorLocalRotation(FRotator(0.f, AngleDistanceOfTick, 0.f));

  UE_LOG(LogTemp, Warning, TEXT("AngleDistanceOfTick: %f"), AngleDistanceOfTick);
}

void ALidarGPU::ReadPoints(float DeltaTime)
{
}

bool ALidarGPU::ShootLaser(uint32 Channel, float HorizontalAngle, FVector & Point) const
{
  return false;
}

void ALidarGPU::RenderDebugLidar()
{
  // this will be read from some LidarGPUDescription struct in the future
  const float Channels = 32u;
  const float Range = 5000.0f;
  const uint32 PointsPerSecond = 56000u;
  const float RotationFrequency = 10.0f;
  const float UFLim = 10.0f;
  const float LFLim = 30.0f;;
  const bool ShowDebugPoints = false;

  // camera debug delimiters
  static const uint8 rays = 6;

  // location and rotation
  FVector loc = GetActorLocation();
  FRotator rot = GetActorRotation();

  FVector rays_rot[rays] = {
    rot.RotateVector(FRotator( UFLim,  5.f, 0.f).Vector()),
    rot.RotateVector(FRotator( UFLim, -5.f, 0.f).Vector()),
    rot.RotateVector(FRotator(-LFLim,  5.f, 0.f).Vector()),
    rot.RotateVector(FRotator(-LFLim, -5.f, 0.f).Vector()),
    rot.RotateVector(FRotator( 0.f,    5.f, 0.f).Vector()),
    rot.RotateVector(FRotator( 0.f,   -5.f, 0.f).Vector())
  };

  // draw camera
  DrawDebugCamera(GetWorld(), loc, rot, 80.f, 1.0, FColor::Orange);

  // draw fov
  for (int i = 0; i < rays; ++i)
    DrawDebugLine(
      GetWorld(), loc, loc + (rays_rot[i] * Range), 
      (i<4)?FColor::Red: FColor::Purple, false, -1.f, 0.f, (i<4)?2.f:1.f);
  
  //UE_LOG(LogTemp, Warning, TEXT("GetActorLocation(): %s"), *(this->GetActorLocation().ToString()));
}