// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"

#include <compiler/disable-ue4-macros.h>
#include "carla/geom/Math.h"
#include <compiler/enable-ue4-macros.h>

#include "GPULidar.h"

#include "Carla/Actor/ActorBlueprintFunctionLibrary.h"

#include "Carla/Sensor/PixelReader.h"

#include "DrawDebugHelpers.h"
#include "Engine/CollisionProfile.h"
#include "Misc/CommandLine.h"
#include "Runtime/Engine/Classes/Kismet/KismetMathLibrary.h"
#include "Carla/Sensor/SceneCaptureCamera.h"
#include "Runtime/Core/Public/GenericPlatform/GenericPlatformMath.h"
#include "StaticMeshResources.h"

FActorDefinition AGPULidar::GetSensorDefinition()
{
  return UActorBlueprintFunctionLibrary::MakeLidarDefinition(TEXT("gpu"));
}

AGPULidar::AGPULidar(const FObjectInitializer &ObjectInitializer)
  : Super(ObjectInitializer)
{
  LoadPostProcessingMaterial(
#if PLATFORM_LINUX
      TEXT("Material'/Carla/PostProcessingMaterials/DepthEffectMaterial_GLSL.DepthEffectMaterial_GLSL'")
#else
      TEXT("Material'/Carla/PostProcessingMaterials/DepthEffectMaterial.DepthEffectMaterial'")
#endif
      );
}

void AGPULidar::Set(const FActorDescription &ActorDescription)
{
  Super::Set(ActorDescription);
  Channels = UActorBlueprintFunctionLibrary::RetrieveActorAttributeToInt(
      "channels",
      ActorDescription.Variations,
      Channels);
  Range = UActorBlueprintFunctionLibrary::RetrieveActorAttributeToFloat(
      "range",
      ActorDescription.Variations,
      Range);
  PointsPerSecond = UActorBlueprintFunctionLibrary::RetrieveActorAttributeToInt(
      "points_per_second",
      ActorDescription.Variations,
      PointsPerSecond);
  RotationFrequency = UActorBlueprintFunctionLibrary::RetrieveActorAttributeToFloat(
      "rotation_frequency",
      ActorDescription.Variations,
      RotationFrequency);
  UpperFovLimit = UActorBlueprintFunctionLibrary::RetrieveActorAttributeToFloat(
      "upper_fov",
      ActorDescription.Variations,
      UpperFovLimit);
  LowerFovLimit = UActorBlueprintFunctionLibrary::RetrieveActorAttributeToFloat(
      "lower_fov",
      ActorDescription.Variations,
      LowerFovLimit);

  const float VerticalFovRad = carla::geom::Math::to_radians(
      UpperFovLimit - LowerFovLimit);

  float StaticFPS = 0.0f;
  static bool FixedFrameRate = FParse::Param(FCommandLine::Get(), TEXT("benchmark"));

  // Horizontal FOV that will be used to select wich pixels we will sended to client for
  // each game tick. That's because we are using the vertical FOV as a Lidar parameter
  float VirtualHorizontalFov = 0.0f;

  if (FixedFrameRate)
  {
    // the lidar must must have more and fixed HFov based on the VFov
    // and send the needed information...
    UE_LOG(LogCarla, Warning, TEXT(" ------ GPU Lidar is fixed frame rate ------ "));

    FParse::Value(FCommandLine::Get(), TEXT("-fps="), StaticFPS);
    UE_LOG(LogCarla, Warning, TEXT(" - FPS: %f "), StaticFPS);

    VirtualHorizontalFov = 360.0 / (StaticFPS / RotationFrequency);
    UE_LOG(LogCarla, Warning, TEXT(" - VirtualHorizontalFov: %f "), VirtualHorizontalFov);

    MaxHorizontalPoints =
        (std::tan(carla::geom::Math::to_radians(VirtualHorizontalFov) / 2.0f) /
        std::tan(VerticalFovRad / 2.0f)) * Channels;
  }
  else
  {
    UE_LOG(LogCarla, Warning, TEXT(" ---- GPU Lidar is NOT fixed frame rate ---- "));
    MaxHorizontalPoints = ImageFOV / 0.09f;
  }

  /// Horizontal FOV
  ImageFOV = carla::geom::Math::to_degrees(
      2.0f * FGenericPlatformMath::Atan(
          FGenericPlatformMath::Tan(VerticalFovRad / 2.0f) *
          (MaxHorizontalPoints / float(Channels))));

  // MaxHorizontalPoints = int(float(PointsPerSecond / RotationFrequency) / float(360.f / ImageFOV));

  UE_LOG(LogCarla, Warning, TEXT(" - Channels: %d"), Channels);
  UE_LOG(LogCarla, Warning, TEXT(" - MaxHorizontalPoints: %d"), MaxHorizontalPoints);
  UE_LOG(LogCarla, Warning, TEXT(" - Horizontal FOV: %f"), ImageFOV);
  UE_LOG(LogCarla, Warning, TEXT(" - Vertical FOV: %f"), UpperFovLimit - LowerFovLimit);

  RotateSceneCaptureComponent2D(FRotator((UpperFovLimit + LowerFovLimit) / 2.0f, 0.0f, 0.0f));
  SetImageSize(MaxHorizontalPoints, Channels);
  SetFOVAngle(ImageFOV);
}

uint32_t AGPULidar::GetMaxHorizontalPoints() const
{
  return MaxHorizontalPoints;
}

uint32_t AGPULidar::GetChannels() const
{
  return Channels;
}

uint32_t AGPULidar::GetCurrentHorizontalPoints() const
{
  return CurrentHorizontalPoints;
}

float AGPULidar::GetFov() const
{
  return ImageFOV;
}

float AGPULidar::GetHorizontalAngle() const
{
  return CurrentHorizontalAngle;
}

void AGPULidar::Tick(float DeltaTime)
{
  Super::Tick(DeltaTime);

  FPixelReader::SendPixelsInRenderThread(
      *this,
      CurrentHorizontalPoints,
      CurrentHorizontalAngle);

  /// @todo some points left, must use the remainder of the division
  CurrentHorizontalPoints = carla::geom::Math::clamp<uint32>(
      0, MaxHorizontalPoints, (DeltaTime * PointsPerSecond) / Channels);

  Rotate(DeltaTime);

  if(ShowDebugPoints)
  {
    RenderDebugLidar();
  }
}

void AGPULidar::Rotate(float DeltaTime)
{
  const uint32 PointsToScanWithOneLaser =
      FMath::RoundHalfFromZero(
      PointsPerSecond * DeltaTime / float(Channels));

  if (PointsToScanWithOneLaser <= 0u)
  {
    UE_LOG(LogCarla,
        Warning,
        TEXT("%s: no points requested this frame, try increasing the number of points per second."),
        *GetName());
    return;
  }

  const float AngleDistanceOfTick = RotationFrequency * 360.f * DeltaTime;

  CurrentHorizontalAngle += AngleDistanceOfTick;

  if (CurrentHorizontalAngle > 180.f)
  {
    CurrentHorizontalAngle -= 360.f;
  }
  else if (CurrentHorizontalAngle < -180.f)
  {
    CurrentHorizontalAngle += 360.f;
  }

  AddActorLocalRotation(FRotator(0.f, AngleDistanceOfTick, 0.f));
}

void AGPULidar::RenderDebugLidar()
{
  // camera debug delimiters
  static const uint8 rays = 6;

  // location and rotation
  FVector loc = GetActorLocation();
  FRotator rot = GetActorRotation();
  float Fov = ImageFOV / 2.0f;
  float CameraPitch = (UpperFovLimit + LowerFovLimit) / 2.0f;

  FVector rays_rot[rays] = {
    rot.RotateVector(FRotator(UpperFovLimit,  Fov/2.0f, 0.f).Vector()),
    rot.RotateVector(FRotator(UpperFovLimit, -Fov/2.0f, 0.f).Vector()),
    rot.RotateVector(FRotator(LowerFovLimit,  Fov/2.0f, 0.f).Vector()),
    rot.RotateVector(FRotator(LowerFovLimit, -Fov/2.0f, 0.f).Vector()),
    rot.RotateVector(FRotator(  CameraPitch,  Fov/2.0f, 0.f).Vector()),
    rot.RotateVector(FRotator(  CameraPitch, -Fov/2.0f, 0.f).Vector())
  };

  // draw camera
  DrawDebugCamera(GetWorld(), loc, rot, 80.f, 1.0, FColor::Orange);

  // draw fov
  for (int i = 0; i < rays; ++i)
  {
    DrawDebugLine(
        GetWorld(), loc, loc + (rays_rot[i] * Range),
        (i < 4) ? FColor::Red : FColor::Purple, false, -1.f, 0.f, (i < 4) ? 2.f : 1.f);
  }

  DrawDebugLine(
        GetWorld(), loc, loc + (rot.RotateVector(GetSceneCaptureComponent2DRotator().Vector()) * Range),
        FColor::Cyan, false, -1.f, 0.f, 2.f);
}
