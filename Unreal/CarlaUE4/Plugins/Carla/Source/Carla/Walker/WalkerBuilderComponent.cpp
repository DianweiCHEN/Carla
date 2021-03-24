// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "WalkerBuilderComponent.h"
#include "SkeletalMeshMerge.h"
#include "SkeletalMeshModel.h"

UWalkerBuilderComponent::UWalkerBuilderComponent(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{

    NumVerticesPerLOD.Add(0);
}

void UWalkerBuilderComponent::MergeMeshes()
{
  if(GetOwner() == nullptr) return;

  TArray<USkeletalMesh*> MeshArray;
  if(WalkerHair != nullptr) MeshArray.Add(WalkerHair);
  if(WalkerHead != nullptr) MeshArray.Add(WalkerHead);
  if(WalkerTorso != nullptr) MeshArray.Add(WalkerTorso);
  if(WalkerClothes != nullptr) MeshArray.Add(WalkerClothes);

  if(MeshArray.Num() > 0)
  {
    USkeletalMesh *FinalMesh = NewObject<USkeletalMesh>(this, USkeletalMesh::StaticClass(), FName("MergedMesh"), RF_Transient);
    TArray<FSkelMeshMergeSectionMapping> SectionMappings;
    FSkeletalMeshMerge Merger(FinalMesh, MeshArray, SectionMappings, 0);
    //Merger.GenerateLODModel<int>(0);

    const bool MergeStatus = Merger.DoMerge();
    check(MergeStatus == true);

    SetSkeletalMesh(FinalMesh);

    for(int i = 0; i < MeshArray.Num(); ++i)
    {
      for(int j = 0; j < MeshArray[i]->MorphTargets.Num(); ++j)
      {
        if(MeshArray[i]->MorphTargets[j]->HasDataForLOD(0) == true)
        {
          int32 NumDeltas = 0;
          MeshArray[i]->MorphTargets[j]->GetMorphTargetDelta(0, NumDeltas);
          NumVerticesPerLOD[0] += NumDeltas;
          UE_LOG(LogTemp, Log, TEXT("This morph target has this many deltas : %d"), NumDeltas);
        }
      }
    }

    for(int i = 0; i < MeshArray.Num(); ++i)
    {
      for(int j = 0; j < MeshArray[i]->MorphTargets.Num(); ++j)
      {
        if(MeshArray[i]->MorphTargets[j]->HasDataForLOD(0) == true)
        {
          UMorphTarget* FinalMeshMorphTarget = NewObject<UMorphTarget>(this, UMorphTarget::StaticClass());
          TArray<FMorphTargetDelta> NewMorphTargetDeltas;
          int32 NumDeltas = 0;
          FMorphTargetDelta *MorphTargetDeltaArray = MeshArray[i]->MorphTargets[j]->GetMorphTargetDelta(0, NumDeltas);
          for(int z = 0; z < NumDeltas; ++z)
          {
            FMorphTargetDelta NewTargetDelta(MorphTargetDeltaArray[z]);
            NewTargetDelta.SourceIdx += NumVerticesPerLOD[0];
            NewMorphTargetDeltas.Add(NewTargetDelta);
          }
          #if WITH_EDITOR
          FinalMeshMorphTarget->PopulateDeltas(NewMorphTargetDeltas, 0, MeshArray[i]->GetImportedModel()->LODModels[0].Sections);
          #endif
          FinalMeshMorphTarget->BaseSkelMesh = FinalMesh;
          FinalMesh->MorphTargets.Add(FinalMeshMorphTarget);
          FinalMesh->MorphTargetIndexMap.Add(TPair<FName, int32>(FName("NewMorphTarget"), 0));
          //UE_LOG(LogTemp, Log, TEXT("The source idx of the new morph target is : %d | The old one was : %d"), NewMorphTarget.SourceIdx, MorphTargetInfo->SourceIdx);
        }
      }
    }

    NumVerticesPerLOD[0] = 0;
  }
}

void UWalkerBuilderComponent::OnRegister()
{
  MergeMeshes();
  Super::OnRegister();
}

void UWalkerBuilderComponent::OnUnregister()
{
  Super::OnUnregister();
}