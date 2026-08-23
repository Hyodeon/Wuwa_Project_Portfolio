#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectExecutionCalculation.h"
#include "DamageExecutionCalculation.generated.h"

/**
 * 모션 배율, 그로기 증폭, 무적 판정 및 공진 감소를 계산하는 단일 데미지 연산 클래스
 */
UCLASS()
class WUWA_PROJECT_API UDamageExecutionCalculation : public UGameplayEffectExecutionCalculation
{
	GENERATED_BODY()

public:
	UDamageExecutionCalculation();

	virtual void Execute_Implementation(
		const FGameplayEffectCustomExecutionParameters& ExecutionParams,
		FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const override;
};