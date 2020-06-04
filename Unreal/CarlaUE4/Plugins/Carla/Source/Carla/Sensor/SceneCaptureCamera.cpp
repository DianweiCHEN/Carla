// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"
#include "Carla/Sensor/SceneCaptureCamera.h"

TArray<ASceneCaptureCamera*> ASceneCaptureCamera::CameraSensors = {};
int32 ASceneCaptureCamera::NumCameras = 0;

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

  UE_LOG(LogCarla, Error, TEXT("ASceneCaptureCamera::BeginPlay Adding sensor"));
  CameraSensors.Add(this);

  Pixels.Reserve(ImageWidth * ImageHeight);

  CaptureDelegate = FCoreDelegates::OnEndFrame.AddUObject(this, &ASceneCaptureCamera::Capture);
}

void ASceneCaptureCamera::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
  Super::EndPlay(EndPlayReason);

  UE_LOG(LogCarla, Error, TEXT("ASceneCaptureCamera::EndPlay Removing sensor"));
  CameraSensors.Remove(this);

  FCoreDelegates::OnEndFrame.Remove(CaptureDelegate);
}

void ASceneCaptureCamera::Capture()
{

  check(CaptureRenderTarget != nullptr);

  ASceneCaptureCamera* This = this;
  ENQUEUE_RENDER_COMMAND(ASceneCaptureCamera_SendPixelsInRenderThread)
  (
    [This, Sensors=CameraSensors](FRHICommandListImmediate& RHICmdList) mutable
    {

      NumCameras++;

      if( NumCameras < Sensors.Num() )
      {
        return;
      }

      for(ASceneCaptureCamera* Camera : Sensors)
      {
        if (Camera && Camera-> HasActorBegunPlay() && !Camera->IsPendingKill())
        {
          ASceneCaptureCamera& CameraRef = *Camera;
          auto Stream = CameraRef.GetDataStream(CameraRef);
          auto Buffer = Stream.PopBufferFromPool();

          CameraRef.CopyTexture(RHICmdList, Buffer);

          bool IsEmpty = Buffer.empty();
          auto *BufferData = Buffer.data();
          if(!IsEmpty && BufferData)
          {
            SCOPE_CYCLE_COUNTER(STAT_CaptureCameraStreamSend);
            Stream.Send(CameraRef, std::move(Buffer));
          }
        } else {
          UE_LOG(LogCarla, Error, TEXT("ASceneCaptureCamera::SendPixelsInRenderThread Invalid sensor"));
        }
      }
      NumCameras=0;
    }
  );
}

void ASceneCaptureCamera::CopyTexture(FRHICommandListImmediate& RHICmdList, carla::Buffer& Buffer)
{
  auto RenderResource = static_cast<const FTextureRenderTarget2DResource *>(CaptureRenderTarget->Resource);
  FTexture2DRHIRef Texture = RenderResource->GetRenderTargetTexture();
  if (!Texture)
  {
    UE_LOG(LogCarla, Error, TEXT("ASceneCaptureCamera::Capture: UTextureRenderTarget2D missing render target texture"));
    return;
  }

  TArray<FColor>& PixelsData = Pixels;

  FIntPoint Rect = Texture->GetSizeXY();
  {
    SCOPE_CYCLE_COUNTER(STAT_CaptureCameraReadRT);
    RHICmdList.ReadSurfaceData(
      Texture,
      FIntRect(0, 0, Rect.X, Rect.Y),
      PixelsData,
      FReadSurfaceDataFlags(RCM_UNorm, CubeFace_MAX));
  }

  {
    SCOPE_CYCLE_COUNTER(STAT_CaptureCameraBufferCopy);
    const uint32 Offset = carla::sensor::SensorRegistry::get<ASceneCaptureCamera *>::type::header_offset;
    Buffer.copy_from(Offset, PixelsData);
  }

}
