// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"
#include "Carla/Sensor/SceneCaptureCamera.h"

FActorDefinition ASceneCaptureCamera::GetSensorDefinition()
{
  constexpr bool bEnableModifyingPostProcessEffects = true;
  return UActorBlueprintFunctionLibrary::MakeCameraDefinition(
      TEXT("rgb"),
      bEnableModifyingPostProcessEffects);
}

ASceneCaptureCamera::ASceneCaptureCamera(const FObjectInitializer &ObjectInitializer)
  : Super(ObjectInitializer)
{
  PrimaryActorTick.bCanEverTick = false;

  AddPostProcessingMaterial(
      TEXT("Material'/Carla/PostProcessingMaterials/PhysicLensDistortion.PhysicLensDistortion'"));
}

void ASceneCaptureCamera::BeginPlay()
{
  Super::BeginPlay();

  CreateTextures();
  // OnEndFrame_GameThread
  FCoreDelegates::OnEndFrame.AddUObject(this, &ASceneCaptureCamera::Capture);
}
/*
void ASceneCaptureCamera::Tick(float DeltaTime)
{
  Super::Tick(DeltaTime);

  FPixelReader::SendPixelsInRenderThread(*this);
}
 */

void ASceneCaptureCamera::CreateTextures()
{

  if (!ReadbackTexture)
  {
    ASceneCaptureCamera* This = this;
    ENQUEUE_RENDER_COMMAND(MediaOutputCaptureFrameCreateTexture)(
      [This](FRHICommandListImmediate& RHICmdList)
      {
        FRHIResourceCreateInfo CreateInfo;
        This->ReadbackTexture = RHICreateTexture2D(
          This->GetImageWidth(),
          This->GetImageHeight(),
          PF_B8G8R8A8,
          1,
          1,
          TexCreate_CPUReadback,
          CreateInfo
        );
      }
    );
  }
}

void ASceneCaptureCamera::Capture()
{
  FPixelReader::SendPixelsInRenderThread(*this, ReadbackTexture);
}


