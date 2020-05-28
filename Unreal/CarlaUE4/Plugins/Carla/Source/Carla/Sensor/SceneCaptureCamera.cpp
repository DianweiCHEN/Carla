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

TArray<FTexture2DRHIRef> SourceTextures;
TArray<TArray<FColor>> PixelsBuffer;
int32 NumCameras = 0;

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

  UE_LOG(LogCarla, Warning, TEXT("ASceneCaptureCamera %d %d "), SourceTextures.Num(), NumCameras);

  NumCameras++;
  PixelsBuffer.Add(TArray<FColor>());

  // CreateTextures();
  // OnEndFrame_GameThread
  FCoreDelegates::OnEndFrame.AddUObject(this, &ASceneCaptureCamera::Capture);
  // OnEndFrameRT
}

void ASceneCaptureCamera::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
  Super::EndPlay(EndPlayReason);
  NumCameras--;
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

  ASceneCaptureCamera* This = this;
  ENQUEUE_RENDER_COMMAND(MediaOutputCaptureFrameCreateTexture)(
    [This](FRHICommandListImmediate& RHICmdList)
    {
      FName FenceName = TEXT("SceneCaptureCameraReadback");
      This->GPUFence = RHICreateGPUFence(FenceName);

      FRHIResourceCreateInfo CreateInfo(*(FenceName.ToString()));
      FTexture2DRHIRef* ReadbackTextures = This->ReadbackTexture;
      uint32 Width = This->GetImageWidth();
      uint32 Height = This->GetImageHeight();
      for(int32 i = 0; i < This->MaxNumTextures; i++)
      {
        ReadbackTextures[i] = RHICreateTexture2D(
          Width,
          Height,
          PF_B8G8R8A8,
          1,
          1,
          TexCreate_CPUReadback | TexCreate_HideInVisualizeTexture,
          CreateInfo
        );
      }

      This->ResolveParams.Rect = FResolveRect(0, 0, Width, Height);
      This->ResolveParams.DestRect = FResolveRect(0, 0, Width, Height);

      This->EnqueueCopy(RHICmdList);

    }
  );
}

void ASceneCaptureCamera::EnqueueCopy(FRHICommandListImmediate& RHICmdList)
{

  const FTextureRenderTarget2DResource* RenderResource =
    static_cast<const FTextureRenderTarget2DResource *>(CaptureRenderTarget[PreviousTexture]->Resource);

  FTexture2DRHIRef SourceTexture = RenderResource->GetRenderTargetTexture();
  FTexture2DRHIRef DestinationTexture = ReadbackTexture[CurrentTexture];

  if (!SourceTexture)
  {
    UE_LOG(LogCarla, Error, TEXT("ASceneCaptureCamera::EnqueueCopy missing render target SourceTexture"));
    return;
  }

  if (!DestinationTexture)
  {
    UE_LOG(LogCarla, Error, TEXT("ASceneCaptureCamera::EnqueueCopy missing render target DestinationTexture"));
    return;
  }

  // CurrentTexture = (CurrentTexture + 1) & ~MaxNumTextures;

  GPUFence->Clear();

  if (SourceTexture)
  {
    // Transfer memory GPU -> CPU
    {
      SCOPE_CYCLE_COUNTER(STAT_CaptureCameraTransitionResource);
      RHICmdList.TransitionResource(EResourceTransitionAccess::EWritable, DestinationTexture);
    }
    {
      SCOPE_CYCLE_COUNTER(STAT_CaptureCameraCopyToResolveTarget);
      RHICmdList.CopyToResolveTarget(SourceTexture, DestinationTexture, ResolveParams);
    }
    {
      SCOPE_CYCLE_COUNTER(STAT_CaptureCameraWriteGPUFence);
      RHICmdList.WriteGPUFence(GPUFence);
    }
  }
}

static void WritePixelsToBuffer(
    //const UTextureRenderTarget2D& RenderTarget,
    const FTexture2DRHIRef Texture,
    TArray<FColor>& Pixels,
    carla::Buffer &Buffer,
    uint32 Offset,
    FRHICommandListImmediate &InRHICmdList);

void ASceneCaptureCamera::Capture()
{

  check(CaptureRenderTarget != nullptr);
  // FPixelReader::SendPixelsInRenderThread(*this, ReadbackTexture);

  ASceneCaptureCamera* This = this;
  TArray<FTexture2DRHIRef>& Textures = SourceTextures;
  TArray<TArray<FColor>>& PixelsBuf = PixelsBuffer;
  ENQUEUE_RENDER_COMMAND(ASceneCaptureCamera_SendPixelsInRenderThread)
  (
    [This, &Textures, &PixelsBuf, Stream=GetDataStream(*this)](FRHICommandListImmediate& RHICmdList) mutable
    {

      /// @todo Can we make sure the sensor is not going to be destroyed?
      if (!This->IsPendingKill())
      {

        UTextureRenderTarget2D* CaptureRenderTarget = This->CaptureRenderTarget[This->CurrentTexture];
        auto RenderResource = static_cast<const FTextureRenderTarget2DResource *>(CaptureRenderTarget->Resource);
        FTexture2DRHIRef SourceTexture = RenderResource->GetRenderTargetTexture();
        if (!SourceTexture)
        {
          UE_LOG(LogCarla, Error, TEXT("ASceneCaptureCamera::Capture: UTextureRenderTarget2D missing render target texture"));
          return;
        }
        Textures.Add(SourceTexture);

        if( Textures.Num() < NumCameras )
        {
          return;
        }

        auto Buffer = Stream.PopBufferFromPool();
        uint32 Offset = carla::sensor::SensorRegistry::get<ASceneCaptureCamera *>::type::header_offset;
        for(int32 i = 0; i < Textures.Num(); i++)
        {
          //TArray<FColor>& PixelsData = This->Pixels[This->CurrentTexture];
          TArray<FColor>& PixelsData = PixelsBuf[i];
          FTexture2DRHIRef Texture = Textures[i];
          /*
          WritePixelsToBuffer(
            Textures[i],
            PixelsData,
            Buffer,
            Offset,
            RHICmdList); */

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
            Buffer.copy_from(Offset, PixelsData);
            // Buffer.copy_from(Offset, (unsigned char*)OutData, Width*Height*4);
          }
        }
        {
          SCOPE_CYCLE_COUNTER(STAT_CaptureCameraStreamSend);
          Stream.Send(*This, std::move(Buffer));
        }
        Textures.Reset();
      }
    }
  );
}
static void WritePixelsToBuffer(
    //const UTextureRenderTarget2D& RenderTarget,
    const FTexture2DRHIRef Texture,
    TArray<FColor>& Pixels,
    carla::Buffer& Buffer,
    uint32 Offset,
    FRHICommandListImmediate& InRHICmdList)
{
  //check(IsInRenderingThread());
// FFrameGrabber g;
// MediaCapture m;
  //RHICmdList.EnqueueLambda([&](FRHICommandListImmediate &InRHICmdList)
  //{
/*
    auto RenderResource = static_cast<const FTextureRenderTarget2DResource *>(RenderTarget.Resource);
    FTexture2DRHIRef Texture = RenderResource->GetRenderTargetTexture();
    if (!Texture)
    {
      UE_LOG(LogCarla, Error, TEXT("FPixelReader: UTextureRenderTarget2D missing render target texture"));
      return;
    }
     */
    FIntPoint Rect = Texture->GetSizeXY();
/*

    FPooledRenderTargetDesc OutputDesc = FPooledRenderTargetDesc::Create2DDesc(
      Rect,
      PF_B8G8R8A8, // PF_R8G8B8A8
      FClearValueBinding::None,
      TexCreate_None,
      TexCreate_RenderTargetable,
      false);
    TRefCountPtr<IPooledRenderTarget> ResampleTexturePooledRenderTarget;
    GRenderTargetPool.FindFreeElement(InRHICmdList, OutputDesc, ResampleTexturePooledRenderTarget, TEXT("SceneCaptureSensor"));
    const FSceneRenderTargetItem& DestRenderTarget = ResampleTexturePooledRenderTarget->GetRenderTargetItem();

    FResolveParams ResolveParams;
    ResolveParams.Rect = FResolveRect(0, 0, Rect.X, Rect.Y);
    ResolveParams.DestRect = FResolveRect(0, 0, Rect.X, Rect.Y);
*/

    // NS: Extra copy here, don't know how to avoid it.
    //TArray<FColor> Pixels;
    //void* OutData = nullptr;
    //int32 Width = 0, Height = 0;

    {
      SCOPE_CYCLE_COUNTER(STAT_CaptureCameraReadRT);


      InRHICmdList.ReadSurfaceData(
        Texture,
        FIntRect(0, 0, Rect.X, Rect.Y),
        Pixels,
        FReadSurfaceDataFlags(RCM_UNorm, CubeFace_MAX));


    // Asynchronously copy target from GPU to GPU
    //void* Pixels = nullptr;

      //InRHICmdList.TransitionResource(EResourceTransitionAccess::EWritable, ReadbackTexture);
      //InRHICmdList.CopyToResolveTarget(Texture, ReadbackTexture, ResolveParams);
      //InRHICmdList.CopyToResolveTarget(Texture, DestRenderTarget.TargetableTexture, ResolveParams);
      //check(ReadbackTexture.IsValid());
      // Asynchronously copy duplicate target from GPU to System Memory
      //InRHICmdList.CopyToResolveTarget(DestRenderTarget.TargetableTexture, ReadbackTexture, FResolveParams());



      //void* OutData = nullptr; //(void*)Pixels.GetData();
    }
    {
      //SCOPE_CYCLE_COUNTER(STAT_CaptureCameraWaitOnRenderThreadTaskFence);
      // InRHICmdList.WaitOnRenderThreadTaskFence(GraphEventRef);
      // InRHICmdList.WriteGPUFence(Fence);
    }
    {
      // SCOPE_CYCLE_COUNTER(STAT_CaptureCameraMapStagingSurface);
      // InRHICmdList.MapStagingSurface(ReadbackTexture, OutData, Width, Height);
    }


    {
      SCOPE_CYCLE_COUNTER(STAT_CaptureCameraBufferCopy);
      Buffer.copy_from(Offset, Pixels);
      // Buffer.copy_from(Offset, (unsigned char*)OutData, Width*Height*4);
    }

    {
      // SCOPE_CYCLE_COUNTER(STAT_CaptureCameraUnmapStagingSurface);
      // InRHICmdList.UnmapStagingSurface(ReadbackTexture);
    }

  //});
  //RHICmdList.RHIThreadFence(true);
}
