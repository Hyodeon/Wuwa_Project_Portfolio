#include "AttributeSet/ParagonAttributeSet.h"
#include "GameplayEffectExtension.h"

UParagonAttributeSet::UParagonAttributeSet()
{
	// 기본 생존 스탯
	InitHealth(1500.f);
	InitMaxHealth(1500.f);
	InitStamina(100.f);
	InitMaxStamina(100.f);
	InitDodgeCost(10.f);

	// 전투 스탯 기본값
	InitAttackPower(50.f);
	InitResonanceAttackPower(20.f);

	// 공진 관련 스탯 초기화
	InitResonanceValue(100.f);
	InitMaxResonanceValue(100.f);
	InitResonanceChunks(4.f);

	// 투사(Valor) 스탯 초기화
	InitValor(0.f);
	InitMaxValor(100.f);
	InitUltimateGauge(0.f);
	InitMaxUltimateGauge(100.f);

	// 쌍검사(BladeDance) & 협주(Concerto) 스탯 초기화
	InitBladeDance(0.f);
	InitMaxBladeDance(100.f);
	InitConcertoGauge(0.f);
	InitMaxConcertoGauge(100.f);

	// 방어 스탯 초기화
	InitIncomingDamageReduction(0.f);
}

void UParagonAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	if (Attribute == GetHealthAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxHealth());
	}
	else if (Attribute == GetStaminaAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxStamina());
	}
	else if (Attribute == GetResonanceValueAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxResonanceValue());
	}
	else if (Attribute == GetValorAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxValor());
	}
	else if (Attribute == GetUltimateGaugeAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxUltimateGauge());
	}
	else if (Attribute == GetBladeDanceAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxBladeDance());
	}
	else if (Attribute == GetConcertoGaugeAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxConcertoGauge());
	}
	else if (Attribute == GetResonanceChunksAttribute())
	{
		NewValue = FMath::Max(0.f, NewValue);
	}
}

void UParagonAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	if (Data.EvaluatedData.Attribute == GetHealthAttribute())
	{
		SetHealth(FMath::Clamp(GetHealth(), 0.f, GetMaxHealth()));
	}
	else if (Data.EvaluatedData.Attribute == GetStaminaAttribute())
	{
		SetStamina(FMath::Clamp(GetStamina(), 0.f, GetMaxStamina()));
	}
	else if (Data.EvaluatedData.Attribute == GetResonanceValueAttribute())
	{
		SetResonanceValue(FMath::Clamp(GetResonanceValue(), 0.f, GetMaxResonanceValue()));
	}
	else if (Data.EvaluatedData.Attribute == GetValorAttribute())
	{
		SetValor(FMath::Clamp(GetValor(), 0.f, GetMaxValor()));
	}
	else if (Data.EvaluatedData.Attribute == GetUltimateGaugeAttribute())
	{
		SetUltimateGauge(FMath::Clamp(GetUltimateGauge(), 0.f, GetMaxUltimateGauge()));
	}
	else if (Data.EvaluatedData.Attribute == GetBladeDanceAttribute())
	{
		SetBladeDance(FMath::Clamp(GetBladeDance(), 0.f, GetMaxBladeDance()));
	}
	else if (Data.EvaluatedData.Attribute == GetConcertoGaugeAttribute())
	{
		SetConcertoGauge(FMath::Clamp(GetConcertoGauge(), 0.f, GetMaxConcertoGauge()));
	}
}