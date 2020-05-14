// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license. For a copy,
// see <https://opensource.org/licenses/MIT>.


#include "Instancer.h"


// Sets default values
AInstancer::AInstancer()
{
  // Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
  PrimaryActorTick.bCanEverTick = false;

  RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SceneComponent"));
  RootComponent->SetMobility(EComponentMobility::Static);

}

void AInstancer::ConvertToInstancedMeshes() {

  auto World = GetWorld();
  check(World != nullptr);

  TArray<AActor *> Actors;
  TArray<UStaticMeshComponent*> MeshComps;
  TArray<UMaterialInterface*> Materials;

  UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), Actors);

  for(AActor* Actor : Actors) {
    // Ignore me
    UE_LOG(LogCarla, Warning, TEXT("Actor %s"), *(Actor->GetName()) );
    if(Actor == this)
    {
      UE_LOG(LogCarla, Error, TEXT("Actor is this"));
      continue;
    }

    MeshComps.Reset();
    Actor->GetComponents<UStaticMeshComponent>(MeshComps, false);

    for(UStaticMeshComponent* StaticMeshComp : MeshComps)
    {
      // If it is already instanced, ignore it
      if(Cast<UInstancedStaticMeshComponent>(StaticMeshComp))
      {
        UE_LOG(LogCarla, Error, TEXT("Mesh Comp is instanced already"));
        continue;
      }

      Materials.Reset();

      UStaticMesh* StaticMesh = StaticMeshComp->GetStaticMesh();
      StaticMeshComp->GetUsedMaterials(Materials);

      if(StaticMesh)
      {
        // Look for an InstancedStaticMeshComponent
        UInstancedStaticMeshComponent* InstancedMeshComp = GetInstancedMeshComponent(StaticMesh);

        // Add new instance
        const FTransform& Transform = StaticMeshComp->GetComponentTransform();
        InstancedMeshComp->AddInstanceWorldSpace(Transform);

        // Apply materials
        for(int i = 0; i < Materials.Num(); i++)
        {
          UMaterialInterface* Material = Materials[i];
          InstancedMeshComp->SetMaterial(i, Material);
        }

        // Disable originial StaticMeshComponent
        StaticMeshComp->SetVisibility(false, false);
        // StaticMeshComp->SetStaticMesh(nullptr);
        StaticMeshComps.Add(StaticMeshComp);
      }
    }
  }
}

void AInstancer::ConvertToStaticMeshes()
{

  UE_LOG(LogCarla, Warning, TEXT("InstancedMeshComps = %d"), InstancedMeshComps.Num() );
  for(auto& It : InstancedMeshComps)
  {
    It.Value->SetVisibility(false, false);
  }

  UE_LOG(LogCarla, Warning, TEXT("StaticMeshComps = %d"), InstancedMeshComps.Num() );
  for(auto& It : StaticMeshComps)
  {
    It->SetVisibility(true, false);
  }

}

void AInstancer::Reset()
{
  ConvertToStaticMeshes();
  InstancedMeshComps.Reset();
  StaticMeshComps.Reset();
}

UInstancedStaticMeshComponent* AInstancer::GetInstancedMeshComponent(UStaticMesh* Mesh)
{
  check(Mesh);

  FString MeshName = Mesh->GetName();
  UInstancedStaticMeshComponent* MeshComp = InstancedMeshComps.FindRef(MeshName);
  if(!MeshComp) {
    MeshComp = NewObject<UInstancedStaticMeshComponent>(this);
    check(MeshComp);
    MeshComp->RegisterComponent();
    MeshComp->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
    MeshComp->SetMobility(EComponentMobility::Static);
    MeshComp->SetStaticMesh(Mesh);
    MeshComp->SetVisibility(true, false);
    InstancedMeshComps.Add(MeshName, MeshComp);

  }
  return MeshComp;

}

bool AInstancer::CheckIfMeshCompHasSameMaterials()
{

// Used materials del staticcomp

}
