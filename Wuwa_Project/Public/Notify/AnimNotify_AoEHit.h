#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AnimNotify_AoEHit.generated.h"

class UBaseAttackData;

UCLASS()
class WUWA_PROJECT_API UAnimNotify_AoEHit : public UAnimNotify
{
	GENERATED_BODY()

public:
	UAnimNotify_AoEHit();

	virtual FString GetNotifyName_Implementation() const override;
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

protected:
	// 이 애니메이션 타격 구간에서 사용할 공격 데이터 에셋
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	TObjectPtr<UBaseAttackData> AttackData;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float AoERadius = 200.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	FName SocketName = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	bool bDrawDebug = false;
};