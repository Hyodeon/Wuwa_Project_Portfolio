
#include "GAS/DamageExecutionCalculation.h"
#include "AttributeSet/ParagonAttributeSet.h"
#include "WuwaGameplayTags.h"

// 스탯 캡처 구조체 정의
struct FDamageStatics
{
	// Source (공격자) 스탯 캡처 선언
	DECLARE_ATTRIBUTE_CAPTUREDEF(AttackPower);
	DECLARE_ATTRIBUTE_CAPTUREDEF(ResonanceAttackPower);

	FDamageStatics()
	{
		// 공격자의 AttackPower 캡처 (스냅샷: false - 적용 시점 값 사용)
		DEFINE_ATTRIBUTE_CAPTUREDEF(UParagonAttributeSet, AttackPower, Source, false);
		// 공격자의 ResonanceAttackPower 캡처
		DEFINE_ATTRIBUTE_CAPTUREDEF(UParagonAttributeSet, ResonanceAttackPower, Source, false);
	}
};

static const FDamageStatics& DamageStatics()
{
	static FDamageStatics DStatics;
	return DStatics;
}

UDamageExecutionCalculation::UDamageExecutionCalculation()
{
	// 캡처 목록 등록
	RelevantAttributesToCapture.Add(DamageStatics().AttackPowerDef);
	RelevantAttributesToCapture.Add(DamageStatics().ResonanceAttackPowerDef);
}

void UDamageExecutionCalculation::Execute_Implementation(
	const FGameplayEffectCustomExecutionParameters& ExecutionParams,
	FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
	const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();

	// 1. 공격자(Source)와 피격자(Target)의 태그 컨테이너 추출
	const FGameplayTagContainer* SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	const FGameplayTagContainer* TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();

	FAggregatorEvaluateParameters EvaluationParameters;
	EvaluationParameters.SourceTags = SourceTags;
	EvaluationParameters.TargetTags = TargetTags;

	// =========================================================================
	// [1] 무적(Invulnerable) 체크: 대상이 무적이면 데미지 0 처리 후 즉시 종료
	// =========================================================================
	if (TargetTags && TargetTags->HasTagExact(WuwaGameplayTags::State_Invulnerable))
	{
		return;
	}

	// =========================================================================
	// [2] 공격자 스탯 및 SetByCaller 모션 배율 추출
	// =========================================================================
	float SourceAttackPower = 0.0f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().AttackPowerDef, EvaluationParameters, SourceAttackPower);
	SourceAttackPower = FMath::Max(0.0f, SourceAttackPower);

	float SourceResonanceAttackPower = 0.0f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().ResonanceAttackPowerDef, EvaluationParameters, SourceResonanceAttackPower);
	SourceResonanceAttackPower = FMath::Max(0.0f, SourceResonanceAttackPower);

	// SetByCaller에서 전달된 모션 배율 (기본값: 1.0f)
	float MotionValue = Spec.GetSetByCallerMagnitude(WuwaGameplayTags::Data_Damage_MotionValue, false, 1.0f);
	// SetByCaller에서 전달된 공진 감소치 (기본값: 10.0f)
	float ResonanceReduction = Spec.GetSetByCallerMagnitude(WuwaGameplayTags::Data_Damage_ResonanceValue, false, 10.0f);

	// =========================================================================
	// [3] 체력 데미지 계산 (그로기 시 1.5배 피해 증폭)
	// =========================================================================
	float FinalDamage = SourceAttackPower * MotionValue;

	if (TargetTags && TargetTags->HasTagExact(WuwaGameplayTags::State_Groggy))
	{
		const float GroggyDamageMultiplier = 1.5f;
		FinalDamage *= GroggyDamageMultiplier;
	}

	// =========================================================================
	// [4] 공진(Resonance) 파괴량 계산
	// =========================================================================
	// 공격자의 공진 공격력 계수와 모션 고유 공진 감소치를 결합
	float FinalResonanceDamage = (SourceResonanceAttackPower * 0.1f) + ResonanceReduction;

	// =========================================================================
	// [5] 피격자 AttributeSet에 적용 (Health 감소, ResonanceValue 감소)
	// =========================================================================
	if (FinalDamage > 0.0f)
	{
		// Health는 음수를 더하여(Add) 감소시킴
		OutExecutionOutput.AddOutputModifier(
			FGameplayModifierEvaluatedData(
				UParagonAttributeSet::GetHealthAttribute(),
				EGameplayModOp::Additive,
				-FinalDamage
			)
		);
	}

	if (FinalResonanceDamage > 0.0f)
	{
		// ResonanceValue 감소
		OutExecutionOutput.AddOutputModifier(
			FGameplayModifierEvaluatedData(
				UParagonAttributeSet::GetResonanceValueAttribute(),
				EGameplayModOp::Additive,
				-FinalResonanceDamage
			)
		);
	}
}