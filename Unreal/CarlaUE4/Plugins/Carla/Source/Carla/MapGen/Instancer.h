// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license. For a copy,
// see <https://opensource.org/licenses/MIT>.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Instancer.generated.h"

UCLASS()
class CARLA_API AInstancer : public AActor
{
  GENERATED_BODY()

public:
  // Sets default values for this actor's properties
  AInstancer();

  UFUNCTION(CallInEditor)
  void ConvertToInstancedMeshes();


  UFUNCTION(CallInEditor)
  void ConvertToStaticMeshes();

  UFUNCTION(CallInEditor)
  void Reset();

private:

  UInstancedStaticMeshComponent* GetInstancedMeshComponent(UStaticMesh* Mesh);

  bool CheckIfMeshCompHasSameMaterials();

  UPROPERTY(VisibleAnywhere)
  TMap<FString, UInstancedStaticMeshComponent*> InstancedMeshComps;

  UPROPERTY(VisibleAnywhere)
  TSet<UStaticMeshComponent*> StaticMeshComps;

};
