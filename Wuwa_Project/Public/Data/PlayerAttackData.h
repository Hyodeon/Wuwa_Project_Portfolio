#pragma once

#include "CoreMinimal.h"
#include "BaseAttackData.h" // 공용 부모 데이터
#include "GameplayTagContainer.h"
#include "PlayerAttackData.generated.h"

/**
 * 플레이어 전용 공격 데이터 (협주 획득, 공진 감소치 등 포함)
 */
UCLASS(BlueprintType)
class WUWA_PROJECT_API UPlayerAttackData : public UBaseAttackData
{
	GENERATED_BODY()

public:
	// 공격 적중 시 감소시킬 기본 공진 수치
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat|Resonance")
	float ResonanceReduction = 15.0f;

	// 적중 시 획득할 자원 (태그와 획득량 매핑)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat|Resource")
	TMap<FGameplayTag, float> ResourceGains;
};