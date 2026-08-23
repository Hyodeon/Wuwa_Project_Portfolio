#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_WaitHitReact.generated.h"

UCLASS()
class WUWA_PROJECT_API UBTTask_WaitHitReact : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_WaitHitReact();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

	/** 피격 상태 해제 시 꺼줄 블랙보드 Bool 키 (기본: IsHit) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	FBlackboardKeySelector HitStateKey;
};