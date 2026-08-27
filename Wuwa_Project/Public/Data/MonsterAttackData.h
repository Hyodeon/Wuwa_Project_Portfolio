#pragma once

#include "CoreMinimal.h"
#include "Data/BaseAttackData.h" // 경로에 맞게 조정
#include "MonsterAttackData.generated.h"

/**
 * 몬스터 전용 공격 데이터 (패링 가능 여부 등)
 */
UCLASS(BlueprintType)
class WUWA_PROJECT_API UMonsterAttackData : public UBaseAttackData
{
	GENERATED_BODY()

public:
	// 플레이어가 이 공격을 패링할 수 있는지 여부
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat|Parry")
	bool bIsParryable = false;

	// 피격당한 플레이어의 스태미나 추가 감소량 (가드 브레이크용 등)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat|Damage")
	float StaminaDrain = 0.0f;

	// 해당 공격 진행 중 슈퍼아머(SuperArmor)를 가지는지 여부
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat|State")
	bool bHasSuperArmor = false;
};