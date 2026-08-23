#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_SetMovementSpeed.generated.h"

UCLASS()
class WUWA_PROJECT_API UBTTask_SetMovementSpeed : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_SetMovementSpeed();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	UPROPERTY(EditAnywhere, Category = "Movement")
	float NewSpeed = 500.f;
};
