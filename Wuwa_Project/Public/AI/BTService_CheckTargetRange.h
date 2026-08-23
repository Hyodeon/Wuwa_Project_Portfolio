#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BTService_CheckTargetRange.generated.h"

UCLASS()
class WUWA_PROJECT_API UBTService_CheckTargetRange : public UBTService
{
	GENERATED_BODY()

public:
	UBTService_CheckTargetRange();

protected:
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	FBlackboardKeySelector TargetKey;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	FBlackboardKeySelector InAttackRangeKey;

	// [추가된 부분]
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	FBlackboardKeySelector CanSlamKey;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	FBlackboardKeySelector TargetDistanceKey;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	float AttackRange = 250.f; // 인파이팅 진입 사거리

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	float SlamRange = 600.f;   // 슬램 공격 가능 최대 사거리

	UPROPERTY(EditAnywhere, Category = "AI")
	float MaxLoseInterestDistance = 3500.0f;
};