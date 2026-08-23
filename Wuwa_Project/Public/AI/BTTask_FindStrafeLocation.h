#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_FindStrafeLocation.generated.h"

UCLASS()
class WUWA_PROJECT_API UBTTask_FindStrafeLocation : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_FindStrafeLocation();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector TargetKey;

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector StrafeLocationKey;

	UPROPERTY(EditAnywhere, Category = "Strafe")
	float MinStrafeDistance = 500.f;

	UPROPERTY(EditAnywhere, Category = "Strafe")
	float MaxStrafeDistance = 1000.f;
};
