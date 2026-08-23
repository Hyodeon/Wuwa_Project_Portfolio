#pragma once

#include "CoreMinimal.h"
#include "BaseAttackData.h" // 경로에 맞게 조정
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

	// 공격 적중 시 획득할 협주(Concerto) 게이지 양
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat|Concerto")
	float ConcertoGain = 10.0f;

	// 칼날 춤(BladeDance) 또는 용맹(Valor) 등 특수 자원 획득량
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat|Resource")
	float SpecialResourceGain = 10.0f;
};