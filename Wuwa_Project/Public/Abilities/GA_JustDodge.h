#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"

#include "GA_JustDodge.generated.h"

class UAbilityTask_PlayMontageAndWait;

UCLASS()
class WUWA_PROJECT_API UGA_JustDodge : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UGA_JustDodge();

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
	void OnDodgeEndEventReceived(FGameplayEventData Payload);

	UFUNCTION()
	void OnAttackInputReceived(FGameplayEventData Payload);

	UFUNCTION()
	void OnCheckComboReceived(FGameplayEventData Payload);

	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	TObjectPtr<UAnimMontage> JustDodgeMontage;

private:
	bool bAttackBuffered;

	UPROPERTY()
	TObjectPtr<UAbilityTask_PlayMontageAndWait> MontageTask;

	UPROPERTY()
	TObjectPtr<class UAbilityTask_WaitGameplayEvent> WaitEventTask;

	UPROPERTY()
	TObjectPtr<class UAbilityTask_WaitGameplayEvent> InputEventTask;

	UPROPERTY()
	TObjectPtr<class UAbilityTask_WaitGameplayEvent> CheckComboTask;
};