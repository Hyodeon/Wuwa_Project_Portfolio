// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GA_Attack.generated.h"

class UAbilityTask_PlayMontageAndWait;
class UAbilityTask_WaitGameplayEvent;

UCLASS()
class WUWA_PROJECT_API UGA_Attack : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UGA_Attack();

	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

	virtual void EndAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		bool bReplicateEndAbility,
		bool bWasCancelled) override;

protected:
	UFUNCTION()
	void OnMontageCompleted();

	UFUNCTION()
	void OnMontageInterrupted();

	UFUNCTION()
	void OnMontageCancelled();

	UFUNCTION()
	void OnAttackInputReceived(FGameplayEventData Payload);

	UFUNCTION()
	void OnCheckComboReceived(FGameplayEventData Payload);

	UFUNCTION()
	void OnAttackEndReceived(FGameplayEventData Payload);

	void PlayNextCombo();

private:
	int32 ComboIndex;
	bool bInputBuffered;
	float ActivationTime;

	UPROPERTY()
	TObjectPtr<UAbilityTask_PlayMontageAndWait> MontageTask;

	UPROPERTY()
	TObjectPtr<UAbilityTask_WaitGameplayEvent> InputEventTask;

	UPROPERTY()
	TObjectPtr<UAbilityTask_WaitGameplayEvent> CheckComboTask;

	UPROPERTY()
	TObjectPtr<UAbilityTask_WaitGameplayEvent> AttackEndTask;
};
