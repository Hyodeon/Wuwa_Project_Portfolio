#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "BaseAttackData.generated.h"

class UAnimMontage;
class UNiagaraSystem;
class USoundBase;
class UCameraShakeBase;


UCLASS(BlueprintType)
class WUWA_PROJECT_API UBaseAttackData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	// 재생할 몽타주 (비어있으면 기본 콤보 몽타주 사용)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
	TObjectPtr<UAnimMontage> AttackMontage;

	// 몽타주 내 재생할 섹션 이름 (예: "Attack1", "Attack2")
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
	FName MontageSectionName = NAME_None;

	// 모션 배율 (1.0 = 100%, 1.2 = 120%)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat|Damage")
	float MotionValue = 1.0f;

	// 피격 힘 / 넉백
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat|Physics")
	float AttackForce = 800.0f;

	// 타격 이펙트 및 사운드 (필요시)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat|FX")
	TObjectPtr<UNiagaraSystem> HitImpactFX;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat|FX")
	TObjectPtr<USoundBase> HitImpactSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat|FX")
	TSubclassOf<UCameraShakeBase> HitCameraShake;
};