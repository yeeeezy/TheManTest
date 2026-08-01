#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Actors/_Shared/Interfaces/Highlightable.h"
#include "HighlightComponent.generated.h"

/**
 * UHighlightComponent
 * 挂到任意 Actor 上使其可被扫描高亮。
 * StartHighlight() 开启 CustomDepth，StopHighlight() 关闭。
 * Post Process 材质（M_Highlight）读取 CustomDepth 绘制轮廓线。
 */
UCLASS(ClassGroup = "TheManTest", meta = (BlueprintSpawnableComponent))
class THEMANTEST_API UHighlightComponent : public UActorComponent, public IHighlightable
{
	GENERATED_BODY()

public:
	UHighlightComponent();

	virtual void StartHighlight_Implementation(float Duration) override;
	virtual void StopHighlight_Implementation() override;

private:
	FTimerHandle FadeTimerHandle;
	void SetCustomDepthOnMeshes(bool bEnable);
};
