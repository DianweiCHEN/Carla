#pragma once

#include "HAL/Runnable.h"

struct RayInfo
{
  float hAngle;
  float vAngle;
  float range;
  bool Hit;
  FVector HitPoint;
};

class RunnableLineTrace : public FRunnable
{
public:

  RunnableLineTrace(UWorld * _World, AActor *_ParentActor, TArray<RayInfo> &_RayInfoVector, const FVector &_ActorLocation, const FRotator &_ActorRotation);
  virtual ~RunnableLineTrace();

  /**
   * Runs the runnable object.
   */
  uint32 Run() override;

  /**
   * Stops the runnable object.
   * This is called if a thread is requested to terminate early.
  */
  void Stop() override;

  /**
   * Exits the runnable object.
   */
  void Exit() override;

  void shootLaser(RayInfo &rayInfo);

private:
  TWeakObjectPtr<UWorld> World;
  AActor * ParentActor;
  TArray<RayInfo> &RayInfoVector;

  FCollisionQueryParams TraceParams;
  FVector ActorLocation;
  FRotator ActorRotation;
  FThreadSafeBool mStopThread;
};
