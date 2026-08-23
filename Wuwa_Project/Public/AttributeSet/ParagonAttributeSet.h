#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "ParagonAttributeSet.generated.h"

// GAS 표준 접근자 매크로
#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

UCLASS()
class WUWA_PROJECT_API UParagonAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:
	UParagonAttributeSet();

	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;

	// ================= [1. 기본 체력 & 스태미나] =================
	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Health")
	FGameplayAttributeData Health;
	ATTRIBUTE_ACCESSORS(UParagonAttributeSet, Health);

	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Health")
	FGameplayAttributeData MaxHealth;
	ATTRIBUTE_ACCESSORS(UParagonAttributeSet, MaxHealth);

	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Stamina")
	FGameplayAttributeData Stamina;
	ATTRIBUTE_ACCESSORS(UParagonAttributeSet, Stamina);

	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Stamina")
	FGameplayAttributeData MaxStamina;
	ATTRIBUTE_ACCESSORS(UParagonAttributeSet, MaxStamina);

	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Stamina")
	FGameplayAttributeData DodgeCost;
	ATTRIBUTE_ACCESSORS(UParagonAttributeSet, DodgeCost);

	// ================= [2. 공격 관련 공용 스탯] =================
	// 기본 공격력
	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Combat")
	FGameplayAttributeData AttackPower;
	ATTRIBUTE_ACCESSORS(UParagonAttributeSet, AttackPower);

	// 공진 파괴 공격력
	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Combat")
	FGameplayAttributeData ResonanceAttackPower;
	ATTRIBUTE_ACCESSORS(UParagonAttributeSet, ResonanceAttackPower);

	// ================= [3. 적 전용 공진(Resonance) 스탯] =================
	// 현재 공진 수치
	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Resonance")
	FGameplayAttributeData ResonanceValue;
	ATTRIBUTE_ACCESSORS(UParagonAttributeSet, ResonanceValue);

	// 최대 공진 수치
	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Resonance")
	FGameplayAttributeData MaxResonanceValue;
	ATTRIBUTE_ACCESSORS(UParagonAttributeSet, MaxResonanceValue);

	// 공진 칸 수 (기본 4칸)
	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Resonance")
	FGameplayAttributeData ResonanceChunks;
	ATTRIBUTE_ACCESSORS(UParagonAttributeSet, ResonanceChunks);
};