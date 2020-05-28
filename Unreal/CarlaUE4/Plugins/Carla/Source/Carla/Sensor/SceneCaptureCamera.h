// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Carla/Actor/ActorDefinition.h"
#include "Carla/Sensor/PixelReader.h"
#include "Carla/Sensor/ShaderBasedSensor.h"

#include "SceneCaptureCamera.generated.h"

/// A sensor that captures images from the scene.
UCLASS()
class CARLA_API ASceneCaptureCamera : public AShaderBasedSensor
{
  GENERATED_BODY()

public:

  static FActorDefinition GetSensorDefinition();

  ASceneCaptureCamera(const FObjectInitializer &ObjectInitializer);

  void BeginPlay() override;

  void EndPlay (const EEndPlayReason::Type EndPlayReason) override;

/*
protected:

  void Tick(float DeltaTime) override;
*/

private:

  void CreateTextures(); // TODO: change name for initialize or something similar
  void Capture();
  void EnqueueCopy(FRHICommandListImmediate& RHICmdList);

  bool IsReady() {
    return !GPUFence || GPUFence->Poll();
  }


  FTexture2DRHIRef ReadbackTexture[MaxNumTextures];
  FGPUFenceRHIRef GPUFence;
  FResolveParams ResolveParams;

  TArray<FColor> Pixels[MaxNumTextures];

};
