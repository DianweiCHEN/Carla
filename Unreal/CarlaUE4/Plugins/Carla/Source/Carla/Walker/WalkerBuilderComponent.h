// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Animation/MorphTarget.h"
#include "WalkerBuilderComponent.generated.h"

/** Component that contains multiple SkeletalMeshes as a single unit */
UCLASS(editinlinenew, meta = (BlueprintSpawnableComponent), ClassGroup = Rendering)
class UWalkerBuilderComponent : public USkeletalMeshComponent
{
  GENERATED_UCLASS_BODY()

  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Walker Builder")
  USkeletalMesh *WalkerHair;

  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Walker Builder")
  USkeletalMesh *WalkerHead;

  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Walker Builder")
  USkeletalMesh *WalkerTorso;

  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Walker Builder")
  USkeletalMesh *WalkerClothes;

private:

    TArray<uint32> NumVerticesPerLOD;
    virtual void OnRegister() override;
    virtual void OnUnregister() override;

    void MergeMeshes();
};