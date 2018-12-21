// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/geom/Math.h"

#include "Carla.h"
#include "GPULidar.h"

#include "Carla/Actor/ActorBlueprintFunctionLibrary.h"

#include "Carla/Sensor/PixelReader.h"

#include "DrawDebugHelpers.h"
#include "Engine/CollisionProfile.h"
#include "Runtime/Engine/Classes/Kismet/KismetMathLibrary.h"
//#include "Runtime/Engine/Classes/Components/SceneCaptureComponent2D.h"
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

  MaxHorizontalPoints = 600;
      //int(float(PointsPerSecond / RotationFrequency) / float(360.0f / ImageFOV));

  const float VerticalFovRad = carla::geom::Math::to_radians(
    UpperFovLimit - LowerFovLimit);

  /// Horizontal FOV
  ImageFOV = carla::geom::Math::to_degrees(2.0f * FGenericPlatformMath::Atan(
    FGenericPlatformMath::Tan(VerticalFovRad / 2.0f) *
    (MaxHorizontalPoints / float(Channels))));

  SetImageSize(MaxHorizontalPoints, Channels); // fix this with the correct MaxHorizontalPoints
  SetFOVAngle(ImageFOV); // fix this with the correct FOV

  if(!ArePostProcessingEffectsEnabled())
  {
    EnablePostProcessingEffects();
  }
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

  /// @todo some points left, must use the remainder of the division
  CurrentHorizontalPoints = carla::geom::Math::clamp<uint32>(
    0, MaxHorizontalPoints, (DeltaTime * PointsPerSecond) / Channels);

  Rotate(DeltaTime);
  FPixelReader::SendPixelsInRenderThread(*this);

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

void AGPULidar::RenderDebugLidar()
{
  // camera debug delimiters
  static const uint8 rays = 6;

  // location and rotation
  FVector loc = GetActorLocation();
  FRotator rot = GetActorRotation();

  FVector rays_rot[rays] = {
    rot.RotateVector(FRotator( UpperFovLimit,  5.f, 0.f).Vector()),
    rot.RotateVector(FRotator( UpperFovLimit, -5.f, 0.f).Vector()),
    rot.RotateVector(FRotator(-LowerFovLimit,  5.f, 0.f).Vector()),
    rot.RotateVector(FRotator(-LowerFovLimit, -5.f, 0.f).Vector()),
    rot.RotateVector(FRotator(           0.f,  5.f, 0.f).Vector()),
    rot.RotateVector(FRotator(           0.f, -5.f, 0.f).Vector())
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
