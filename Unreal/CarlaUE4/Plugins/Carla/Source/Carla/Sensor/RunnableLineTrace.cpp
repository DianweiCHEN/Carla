#include "Carla/Sensor/RunnableLineTrace.h"

#include "Engine/CollisionProfile.h"
#include "Runtime/Engine/Classes/Kismet/KismetMathLibrary.h"
#include <cmath>


RunnableLineTrace::RunnableLineTrace(UWorld * _World, AActor *_ParentActor, TArray<RayInfo> &_RayInfoVector, const FVector &_ActorLocation, const FRotator &_ActorRotation)
: World(_World)
, ParentActor(_ParentActor)
, RayInfoVector(_RayInfoVector)
, ActorLocation(_ActorLocation)
, ActorRotation(_ActorRotation)
{
  mStopThread = false;

  TraceParams = FCollisionQueryParams(FName(TEXT("Laser_Trace")), true, ParentActor);
  TraceParams.bTraceComplex = true;
  TraceParams.bReturnPhysicalMaterial = false;

}

RunnableLineTrace::~RunnableLineTrace()
{
}

uint32 RunnableLineTrace::Run()
{
  if(!ParentActor)
  {
    return 0;
  }

  for(int32 index = 0; index < RayInfoVector.Num(); index++)
  {
    shootLaser(RayInfoVector[index]);
  }

  return 0;
}

void RunnableLineTrace::Stop()
{
  mStopThread = true;
}

void RunnableLineTrace::Exit()
{
  mStopThread = true;
}

void RunnableLineTrace::shootLaser(RayInfo &rayInfo)
{
  FHitResult HitInfo(ForceInit);

  FRotator LaserRot (rayInfo.vAngle, rayInfo.hAngle, 0);  // float InPitch, float InYaw, float InRoll
  FRotator ResultRot = UKismetMathLibrary::ComposeRotators(
    LaserRot,
    ActorRotation
  );

  FVector EndTrace = rayInfo.range * UKismetMathLibrary::GetForwardVector(ResultRot) + ActorLocation;

  World->LineTraceSingleByChannel(
    HitInfo,
    ActorLocation,
    EndTrace,
    ECC_MAX,
    TraceParams,
    FCollisionResponseParams::DefaultResponseParam
  );

  rayInfo.Hit = HitInfo.bBlockingHit;

  if (HitInfo.bBlockingHit)
  {
    rayInfo.HitPoint = ActorLocation - HitInfo.ImpactPoint;
    rayInfo.HitPoint = UKismetMathLibrary::RotateAngleAxis(
      rayInfo.HitPoint,
      - ActorRotation.Yaw + 90,
      FVector(0, 0, 1)
    );
  }

}
