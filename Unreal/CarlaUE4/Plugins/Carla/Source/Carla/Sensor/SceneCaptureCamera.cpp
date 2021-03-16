// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"
#include "Carla/Sensor/SceneCaptureCamera.h"

#include "Runtime/RenderCore/Public/RenderingThread.h"

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
  AddPostProcessingMaterial(TEXT("Material'/Carla/PostProcessingMaterials/NewPhysLensDistorsion.NewPhysLensDistorsion'"));
}

void ASceneCaptureCamera::PostPhysTick(UWorld *World, ELevelTick TickType, float DeltaSeconds)
{
  TRACE_CPUPROFILER_EVENT_SCOPE(ASceneCaptureCamera::PostPhysTick);

  if (Image_Resize == false) {
    FPixelReader::SendPixelsInRenderThread(*this, FImageResizer());
  }
  else {
    const uint32 MinX = ImageWidthRender / 2 - ImageWidthTarget / 2;
    const uint32 MaxX = MinX + ImageWidthTarget;

    const uint32 MinY = ImageHeightRender / 2 - ImageHeightTarget / 2;
    const uint32 MaxY = MinY + ImageHeightTarget;

    FPixelReader::SendPixelsInRenderThread(*this, FImageResizer(true, MinX, MaxX, MinY, MaxY));
  }
}
