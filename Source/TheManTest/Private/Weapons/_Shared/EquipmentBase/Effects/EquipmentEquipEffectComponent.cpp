#include "Weapons/_Shared/EquipmentBase/Effects/EquipmentEquipEffectComponent.h"
#include "Components/MeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"

namespace
{
    const FName AmountParameter(TEXT("Amount (S)"));
    float EvaluateReveal(float Time)
    {
        const float A = FMath::Clamp(Time / UEquipmentEquipEffectComponent::Duration, 0.f, 1.f);
        // Original 0.5-second reveal: Hermite 1 -> 0, starting tangent -5.434987.
        return 2.f*A*A*A - 3.f*A*A + 1.f
            + (A*A*A - 2.f*A*A + A) * UEquipmentEquipEffectComponent::Duration * -5.434987f;
    }
}

UEquipmentEquipEffectComponent::UEquipmentEquipEffectComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = false;
}

void UEquipmentEquipEffectComponent::Play()
{
    Stop();
    if (!GetOwner()) { return; }
    TInlineComponentArray<UMeshComponent*> Meshes(GetOwner());
    for (UMeshComponent* Mesh : Meshes)
    {
        if (const auto* Static = Cast<UStaticMeshComponent>(Mesh); Static && !Static->GetStaticMesh()) { continue; }
        if (const auto* Skeletal = Cast<USkeletalMeshComponent>(Mesh); Skeletal && !Skeletal->GetSkeletalMeshAsset()) { continue; }
        for (int32 Slot = 0; Slot < Mesh->GetNumMaterials(); ++Slot)
        {
            UMaterialInterface* Original = Mesh->GetMaterial(Slot);
            if (!Original) { continue; }
            float ExistingAmount = 0.f;
            // Never replace an unsupported surface with a generic grey shell.
            // Equipment materials opt into the shared dissolve function at authoring time.
            if (!Original->GetScalarParameterValue(AmountParameter, ExistingAmount)) { continue; }
            UMaterialInstanceDynamic* Dynamic = UMaterialInstanceDynamic::Create(Original, this);
            Dynamic->SetScalarParameterValue(AmountParameter, 1.f);
            Mesh->SetMaterial(Slot, Dynamic);
            FEquipmentEffectMaterial& Entry = Materials.AddDefaulted_GetRef();
            Entry.Mesh = Mesh;
            Entry.Slot = Slot;
            Entry.Original = Original;
            Entry.Dynamic = Dynamic;
        }
    }
    Elapsed = 0.f;
    Amount = 1.f;
    bPlaying = !Materials.IsEmpty();
    SetComponentTickEnabled(bPlaying);
}

void UEquipmentEquipEffectComponent::Stop()
{
    for (const FEquipmentEffectMaterial& Entry : Materials)
    {
        if (UMeshComponent* Mesh = Entry.Mesh.Get(); Mesh && Mesh->GetMaterial(Entry.Slot) == Entry.Dynamic)
        {
            Mesh->SetMaterial(Entry.Slot, Entry.Original);
        }
    }
    Materials.Reset();
    Amount = 0.f;
    bPlaying = false;
    SetComponentTickEnabled(false);
}

void UEquipmentEquipEffectComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    if (!bPlaying) { return; }
    Elapsed = FMath::Min(Elapsed + DeltaTime, Duration);
    Amount = EvaluateReveal(Elapsed);
    for (const FEquipmentEffectMaterial& Entry : Materials)
    {
        if (Entry.Dynamic) { Entry.Dynamic->SetScalarParameterValue(AmountParameter, Amount); }
    }
    if (Elapsed >= Duration) { Stop(); }
}

void UEquipmentEquipEffectComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Stop();
    Super::EndPlay(EndPlayReason);
}
