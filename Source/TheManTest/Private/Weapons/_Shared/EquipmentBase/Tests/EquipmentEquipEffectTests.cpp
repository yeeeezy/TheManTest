#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR
#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "Tests/AutomationEditorCommon.h"
#include "Weapons/_Shared/EquipmentBase/EquipmentBase.h"
#include "Weapons/_Shared/EquipmentBase/Effects/EquipmentEquipEffectComponent.h"
#include "Weapons/_Shared/Components/EquipmentManagerComponent.h"
#include "Characters/CharacterBase/FPSCharacterBase/FPSCharacterBase.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Animation/AnimInstance.h"
#include "Editor.h"
#include "Engine/StaticMesh.h"
#include "ImageUtils.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "UObject/UnrealType.h"

class FEquipmentRevealCheck final : public IAutomationLatentCommand
{
public:
    explicit FEquipmentRevealCheck(FAutomationTestBase* InTest) : Test(InTest) {}
    bool Update() override
    {
        UWorld* World = GEditor ? GEditor->PlayWorld : nullptr;
        AFPSCharacterBase* Player = World && World->GetFirstPlayerController()
            ? Cast<AFPSCharacterBase>(World->GetFirstPlayerController()->GetPawn()) : nullptr;
        if (!Player || !Player->GetEquipmentManager()) { return false; }
        AEquipmentBase* Equipment = Player->GetEquipmentManager()->GetCurrentEquipment();
        if (!Equipment) { return false; }
        if (!bStarted)
        {
            UMeshComponent* Surface = Equipment->GetSkeletalMesh()->GetSkeletalMeshAsset()
                ? static_cast<UMeshComponent*>(Equipment->GetSkeletalMesh()) : Equipment->GetStaticMesh();
            Originals.Reset();
            Equipment->FindComponentByClass<UEquipmentEquipEffectComponent>()->Stop();
            for (auto* Material : Surface->GetMaterials()) { Originals.Add(Material); }
            Equipment->PlayEquipEffect();
            bStarted = true;
            return false;
        }
        const float Elapsed = Equipment->GetEquipEffectElapsedForTesting();
        if (!bCaptured && Elapsed >= 0.09f && Equipment->IsEquipEffectPlaying())
        {
            UMeshComponent* Mesh = Equipment->GetSkeletalMesh()->GetSkeletalMeshAsset()
                ? static_cast<UMeshComponent*>(Equipment->GetSkeletalMesh()) : Equipment->GetStaticMesh();
            Test->TestTrue(TEXT("Visible equipment has material slots"), Mesh->GetNumMaterials() > 0);
            for (int32 I=0; I<Mesh->GetNumMaterials(); ++I)
            {
                auto* MID = Cast<UMaterialInstanceDynamic>(Mesh->GetMaterial(I));
                Test->TestNotNull(TEXT("Every visible material receives the shared effect"), MID);
                Test->TestTrue(TEXT("Reveal MID retains the exact original surface parent"),
                    MID && Originals.IsValidIndex(I) && MID->Parent == Originals[I]);
                float Value = 0.f;
                Test->TestTrue(TEXT("Shared dissolve parameter is active on visible surface"),
                    MID && MID->GetScalarParameterValue(TEXT("Amount (S)"), Value)
                    && FMath::IsNearlyEqual(Value, Equipment->GetEquipEffectValueForTesting(), 0.01f));
            }
            if (World->GetGameViewport() && World->GetGameViewport()->Viewport)
            {
                FViewport* Viewport = World->GetGameViewport()->Viewport;
                TArray<FColor> Pixels;
                const FIntPoint Size = Viewport->GetSizeXY();
                if (Viewport->ReadPixels(Pixels) && Pixels.Num()==Size.X*Size.Y)
                {
                    TArray64<uint8> Png;
                    FImageUtils::PNGCompressImageArray(Size.X,Size.Y,Pixels,Png);
                    const FString Filename=FPaths::Combine(FPaths::ProjectSavedDir(),
                        FString::Printf(TEXT("Screenshots/WindowsEditor/TMT_EquipReveal_%d.png"), Index));
                    Test->TestTrue(TEXT("Equipment reveal screenshot saved"),FFileHelper::SaveArrayToFile(Png,*Filename));
                }
            }
            bCaptured = true;
        }
        if (Elapsed < UEquipmentEquipEffectComponent::Duration) { return false; }
        Test->TestTrue(TEXT("Reveal sampled while active"), bCaptured);
        Test->TestFalse(TEXT("Reveal component stops ticking after completion"),
            Equipment->FindComponentByClass<UEquipmentEquipEffectComponent>()->IsComponentTickEnabled());
        if (World->GetGameViewport() && World->GetGameViewport()->Viewport)
        {
            FViewport* Viewport=World->GetGameViewport()->Viewport;
            TArray<FColor> Pixels; const FIntPoint Size=Viewport->GetSizeXY();
            if (Viewport->ReadPixels(Pixels) && Pixels.Num()==Size.X*Size.Y)
            {
                TArray64<uint8> Png; FImageUtils::PNGCompressImageArray(Size.X,Size.Y,Pixels,Png);
                Test->TestTrue(TEXT("Finished weapon surface captured"),FFileHelper::SaveArrayToFile(Png,
                    *FPaths::Combine(FPaths::ProjectSavedDir(),FString::Printf(TEXT("Screenshots/WindowsEditor/TMT_WeaponSurface_%d.png"),Index))));
            }
        }
        if (++Index < 3)
        {
            Player->GetEquipmentManager()->SwitchEquipment(1);
            bCaptured = false;
            bStarted = false;
            return false;
        }

        // An unsupported custom surface must never be replaced with a grey fallback.
        AEquipmentBase* Generic = World->SpawnActor<AEquipmentBase>();
        UStaticMeshComponent* Cube = Generic->GetStaticMesh();
        Cube->SetStaticMesh(LoadObject<UStaticMesh>(nullptr,TEXT("/Engine/BasicShapes/Cube.Cube")));
        UMaterialInterface* Original = Cube->GetMaterial(0);
        Generic->PlayEquipEffect();
        Test->TestFalse(TEXT("Unsupported material does not start a false reveal"), Generic->IsEquipEffectPlaying());
        Test->TestEqual(TEXT("Unprepared surface keeps its exact material"), Cube->GetMaterial(0),Original);
        Generic->Unequip();
        Test->TestEqual(TEXT("Interrupted reveal restores original material"),Cube->GetMaterial(0),Original);
        Test->TestFalse(TEXT("Unequip cancels the reveal"),Generic->IsEquipEffectPlaying());
        Original=LoadObject<UMaterialInterface>(nullptr,TEXT("/Game/Weapons/ElectricGun/Materials/MI_ElectricGun.MI_ElectricGun"));
        Cube->SetMaterial(0,Original);
        Generic->PlayEquipEffect();
        Test->TestTrue(TEXT("Non-firearm equipment with a compatible surface inherits shared VFX"),Generic->IsEquipEffectPlaying());
        Generic->Unequip();
        Test->TestEqual(TEXT("Compatible surface is restored exactly on cancellation"),Cube->GetMaterial(0),Original);
        Generic->Destroy();

        // Per-equipment animation is independent from the shared dissolve.
        FBoolProperty* AnimationFlag=FindFProperty<FBoolProperty>(AEquipmentBase::StaticClass(),TEXT("bPlayEquipAnimation"));
        Test->TestNotNull(TEXT("Equipment exposes optional equip animation"),AnimationFlag);
        if (AnimationFlag)
        {
            AnimationFlag->SetPropertyValue_InContainer(Equipment,true);
            Equipment->PlayEquipEffect();
            Test->TestTrue(TEXT("Optional animation does not disable shared VFX"),Equipment->IsEquipEffectPlaying());
            Test->TestTrue(TEXT("Equipment plays its own selected montage"),
                Player->GetArmsMesh()->GetAnimInstance()->Montage_IsPlaying(Equipment->GetEquipMontage()));
        }
        return true;
    }
private:
    FAutomationTestBase* Test;
    int32 Index=0;
    bool bStarted=false;
    bool bCaptured=false;
    TArray<UMaterialInterface*> Originals;
};

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSharedEquipmentRevealTest,
    "TheManTest.Equipment.SharedEquipReveal",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FSharedEquipmentRevealTest::RunTest(const FString& Parameters)
{
    AutomationOpenMap(TEXT("/Game/Maps/VFXTest/VFXTestMap"));
    ADD_LATENT_AUTOMATION_COMMAND(FStartPIECommand(false));
    ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(1.f));
    ADD_LATENT_AUTOMATION_COMMAND(FEquipmentRevealCheck(this));
    ADD_LATENT_AUTOMATION_COMMAND(FEndPlayMapCommand());
    return true;
}
#endif
