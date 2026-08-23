#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_FindPatrolLocation.generated.h"

UCLASS()
class WUWA_PROJECT_API UBTTask_FindPatrolLocation : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_FindPatrolLocation();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	/** 순찰 지점 탐색 반경 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	float PatrolRadius = 1500.f;

	/** 계산된 순찰 좌표를 저장할 블랙보드 키 (Vector 타입) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	FBlackboardKeySelector PatrolLocationKey;
};