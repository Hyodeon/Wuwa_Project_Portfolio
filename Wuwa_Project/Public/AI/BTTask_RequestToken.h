#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "Components/CombatManagerComponent.h"
#include "BTTask_RequestToken.generated.h"

UCLASS()
class WUWA_PROJECT_API UBTTask_RequestToken : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_RequestToken();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	UPROPERTY(EditAnywhere, Category = "Combat")
	ECombatTokenType TokenType;

	UPROPERTY(EditAnywhere, Category = "Combat")
	FBlackboardKeySelector TargetKey;
};
