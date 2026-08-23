#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_ReleaseToken.generated.h"

UCLASS()
class WUWA_PROJECT_API UBTTask_ReleaseToken : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_ReleaseToken();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	UPROPERTY(EditAnywhere, Category = "Combat")
	FBlackboardKeySelector TargetKey;
};
