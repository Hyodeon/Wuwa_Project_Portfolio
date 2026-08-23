#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AnimNotify_CheckCombo.generated.h"

/**
 * 평타 콤보 연속 입력을 확인하기 위해 호출되는 노티파이
 * 호출 시점 이전까지 AttackInput 이벤트가 발생했다면, 캐릭터는 다음 타수로 넘어갑니다.
 */
UCLASS()
class WUWA_PROJECT_API UAnimNotify_CheckCombo : public UAnimNotify
{
	GENERATED_BODY()

public:
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
};
