
#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_Attack.generated.h"



UCLASS()
class WUWA_PROJECT_API UBTTask_Attack : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_Attack();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

	// 에디터에서 선택할 공격 몽타주의 섹션 이름 (Slam 등)
	UPROPERTY(EditAnywhere, Category = "Combat")
	FName AttackSectionName = NAME_None;
};