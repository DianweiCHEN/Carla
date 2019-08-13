// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"
#include "Carla/Sensor/RayCastLidar.h"

#include "Carla/Actor/ActorBlueprintFunctionLibrary.h"

#include "DrawDebugHelpers.h"
#include "Engine/CollisionProfile.h"
#include "Runtime/Engine/Classes/Kismet/KismetMathLibrary.h"

#include <chrono>

FActorDefinition ARayCastLidar::GetSensorDefinition()
{
  return UActorBlueprintFunctionLibrary::MakeLidarDefinition(TEXT("ray_cast"));
}

ARayCastLidar::ARayCastLidar(const FObjectInitializer& ObjectInitializer)
  : Super(ObjectInitializer)
{
  PrimaryActorTick.bCanEverTick = true;
}

void ARayCastLidar::Set(const FActorDescription &ActorDescription)
{
  Super::Set(ActorDescription);
  FLidarDescription LidarDescription;
  UActorBlueprintFunctionLibrary::SetLidar(ActorDescription, LidarDescription);
  Set(LidarDescription);
}

void ARayCastLidar::Set(const FLidarDescription &LidarDescription)
{
  Description = LidarDescription;
  LidarMeasurement = FLidarMeasurement(Description.Channels);
  CreateLasers();
}

void ARayCastLidar::CreateLasers()
{
  const auto NumberOfLasers = Description.Channels;
  check(NumberOfLasers > 0u);
  const float DeltaAngle = NumberOfLasers == 1u ? 0.f :
    (Description.UpperFovLimit - Description.LowerFovLimit) /
    static_cast<float>(NumberOfLasers - 1);
  LaserAngles.Empty(NumberOfLasers);
  for(auto i = 0u; i < NumberOfLasers; ++i)
  {
    const float VerticalAngle =
      Description.UpperFovLimit - static_cast<float>(i) * DeltaAngle;
    LaserAngles.Emplace(VerticalAngle);
  }
}

void ARayCastLidar::Tick(const float DeltaTime)
{
  Super::Tick(DeltaTime);

  ReadPoints(DeltaTime);

  auto DataStream = GetDataStream(*this);
  DataStream.Send(*this, LidarMeasurement, DataStream.PopBufferFromPool());
}

void ARayCastLidar::ReadPoints(const float DeltaTime)
{
  const uint32 ChannelCount = Description.Channels;
  const uint32 PointsToScanWithOneLaser =
    FMath::RoundHalfFromZero(
        Description.PointsPerSecond * DeltaTime / float(ChannelCount));

  if (PointsToScanWithOneLaser <= 0)
  {
    UE_LOG(
        LogCarla,
        Warning,
        TEXT("%s: no points requested this frame, try increasing the number of points per second."),
        *GetName());
    return;
  }

  check(ChannelCount == LaserAngles.Num());
  const float CurrentHorizontalAngle = LidarMeasurement.GetHorizontalAngle();
  const float AngleDistanceOfTick = Description.RotationFrequency * 360.0f * DeltaTime;
  const float AngleDistanceOfLaserMeasure = AngleDistanceOfTick / PointsToScanWithOneLaser;

  LidarMeasurement.Reset(ChannelCount * PointsToScanWithOneLaser);

  std::chrono::high_resolution_clock::time_point tStart = std::chrono::high_resolution_clock::now();

  const uint32 ThreadCount = 4u;
  const uint32 PointsPerThread = (ChannelCount * PointsToScanWithOneLaser) / ThreadCount;

  // setup data
  TArray<TArray<RayInfo>> RayInfoVector;
  TArray<FRunnableThread*> ThreadVector;
  const float Range = 5000.f;
  RayInfoVector.SetNum(ThreadCount);

  for (auto ThreadId = 0u; ThreadId < ThreadCount; ++ThreadId)
  {
    RayInfoVector[ThreadId].SetNumUninitialized(PointsPerThread);

    for (auto i = 0u; i < PointsPerThread; ++i)
    {
      FVector Point;

      const uint32 index = ThreadId * PointsPerThread + i;
      const uint32 Channel = index / PointsToScanWithOneLaser;
      const uint32 K = index % PointsToScanWithOneLaser;

      const float Angle = CurrentHorizontalAngle + AngleDistanceOfLaserMeasure * static_cast<float>(K);

      RayInfoVector[ThreadId][i] = RayInfo{Angle, LaserAngles[Channel], Range, false, FVector(0, 0, 0)};
    }
  }

  // create runnables and threads

  for (auto ThreadId = 0u; ThreadId < ThreadCount; ++ThreadId)
  {
    RunnableLineTrace* Runnable = new RunnableLineTrace(GetWorld(), this, RayInfoVector[ThreadId], GetActorLocation(), GetActorRotation());
    ThreadVector.Push(FRunnableThread::Create(Runnable, TEXT("LineTraceThread")));
  }

  std::chrono::high_resolution_clock::time_point tLaunch = std::chrono::high_resolution_clock::now();


  // wait for data
  for (auto ThreadId = 0u; ThreadId < ThreadCount; ++ThreadId)
  {
    ThreadVector[ThreadId]->WaitForCompletion();
  }
  ThreadVector.Empty();

  std::chrono::high_resolution_clock::time_point tWrite = std::chrono::high_resolution_clock::now();

  for (auto ThreadId = 0u; ThreadId < ThreadCount; ++ThreadId)
  {
    for (auto i = 0u; i < PointsPerThread; ++i)
    {
      if (RayInfoVector[ThreadId][i].Hit)
      {
        // thread to channel conversion
        const uint32 index = ThreadId * PointsPerThread + i;
        const uint32 Channel = index / PointsToScanWithOneLaser;
        const uint32 K = index % PointsToScanWithOneLaser;

        LidarMeasurement.WritePoint(Channel, RayInfoVector[ThreadId][i].HitPoint);
      }
    }
  }
  RayInfoVector.Empty();

  std::chrono::high_resolution_clock::time_point tEnd = std::chrono::high_resolution_clock::now();
  auto spawnTime = std::chrono::duration_cast<std::chrono::milliseconds>(tLaunch - tStart).count();
  auto writeTime = std::chrono::duration_cast<std::chrono::milliseconds>(tEnd - tWrite).count();
  auto time = std::chrono::duration_cast<std::chrono::milliseconds>(tEnd - tStart).count();
  UE_LOG(
      LogCarla,
      Warning,
      TEXT("%s: Thread spawning time = %d ms"),
      *GetName(),
      spawnTime);

  UE_LOG(
      LogCarla,
      Warning,
      TEXT("%s: Raycast execution parallel = %d ms"),
      *GetName(),
      time);

  UE_LOG(
      LogCarla,
      Warning,
      TEXT("%s: Raycast execution write = %d ms"),
      *GetName(),
      writeTime);

  const float HorizontalAngle = std::fmod(CurrentHorizontalAngle + AngleDistanceOfTick, 360.0f);
  LidarMeasurement.SetHorizontalAngle(HorizontalAngle);
}
