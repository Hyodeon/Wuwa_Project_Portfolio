#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GA_Valor_ESkill.generated.h"

class UAnimMontage;
class UAbilityTask_PlayMontageAndWait;
class UAbilityTask_WaitInputRelease;
class UAbilityTask_WaitDelay;

/**
 * 용맹한 투사(Valor Fighter) E 스킬: 차지 & 릴리즈 어빌리티
 */
UCLASS()
class WUWA_PROJECT_API UGA_Valor_ESkill : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UGA_Valor_ESkill();

	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData
	) override;

	virtual void EndAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		bool bReplicateEndAbility,
		bool bWasCancelled
	) override;

protected:
	/** 스킬 몽타주 (Charge_Start, Charge_Loop, Attack_Release 포함) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Valor|Montage")
	TObjectPtr<UAnimMontage> SkillMontage;

	/** 최대 차지 유지 시간 (기본값 2.5초) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Valor|Combat")
	float MaxChargeTime = 2.5f;

	/** 몽타주 섹션 이름 상수 정의 */
	const FName Section_Start = TEXT("Charge_Start");
	const FName Section_Loop = TEXT("Charge_Loop");
	const FName Section_Release = TEXT("Attack_Release");

private:
	/** InputRelease 델리게이트용 (float 인자 수신) */
	UFUNCTION()
	void OnInputReleased(float TimeHeld);

	/** Delay 완료 및 공통 릴리즈 처리 함수 (인자 없음) */
	UFUNCTION()
	void HandleReleaseTriggered();

	UFUNCTION()
	void OnMontageCompleted();

	UFUNCTION()
	void OnMontageInterrupted();

	UFUNCTION()
	void OnMontageCancelled();

	UPROPERTY()
	TObjectPtr<UAbilityTask_PlayMontageAndWait> MontageTask;

	UPROPERTY()
	TObjectPtr<UAbilityTask_WaitInputRelease> WaitInputReleaseTask;

	UPROPERTY()
	TObjectPtr<UAbilityTask_WaitDelay> WaitDelayTask;

	/** 중복 실행 방지 플래그 */
	bool bHasReleased = false;
};