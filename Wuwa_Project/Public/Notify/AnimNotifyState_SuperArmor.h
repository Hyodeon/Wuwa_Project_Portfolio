#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "AnimNotifyState_SuperArmor.generated.h"

/**
 * 몽타주 재생 구간 동안 액터에게 슈퍼아머(State.SuperArmor) 태그를 부여합니다.
 */
UCLASS()
class WUWA_PROJECT_API UAnimNotifyState_SuperArmor : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	UAnimNotifyState_SuperArmor();

	virtual FString GetNotifyName_Implementation() const override;

	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
};
