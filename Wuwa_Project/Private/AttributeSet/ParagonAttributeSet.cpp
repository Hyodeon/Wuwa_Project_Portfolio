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

	// 적 공진 스탯 기본값
	InitResonanceValue(100.f);
	InitMaxResonanceValue(100.f);
	InitResonanceChunks(4.f);
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
}