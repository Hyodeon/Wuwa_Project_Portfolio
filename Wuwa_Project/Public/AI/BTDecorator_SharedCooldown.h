#pragma once
#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "BTDecorator_SharedCooldown.generated.h"

UCLASS()
class WUWA_PROJECT_API UBTDecorator_SharedCooldown : public UBTDecorator
{
	GENERATED_BODY()
public:
	UBTDecorator_SharedCooldown();
protected:
	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;
	virtual void OnNodeActivation(FBehaviorTreeSearchData& SearchData) override;

	UPROPERTY(EditAnywhere, Category = "Cooldown")
	float CooldownTime = 2.5f;

	UPROPERTY(EditAnywhere, Category = "Cooldown")
	FBlackboardKeySelector LastAttackTimeKey;
};
