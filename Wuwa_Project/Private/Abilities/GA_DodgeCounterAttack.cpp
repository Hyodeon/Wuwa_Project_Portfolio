#include "Abilities/GA_DodgeCounterAttack.h"
#include "Characters/ParagonCharacter.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "AbilitySystemComponent.h"
#include "WuwaGameplayTags.h"

UGA_DodgeCounterAttack::UGA_DodgeCounterAttack()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;

	AbilityTags.AddTag(WuwaGameplayTags::Ability_Dodge_Counter);
	AbilityTags.AddTag(WuwaGameplayTags::Ability_Attack);

	CancelAbilitiesWithTag.AddTag(WuwaGameplayTags::Ability_JustDodge);
	CancelAbilitiesWithTag.AddTag(WuwaGameplayTags::Ability_Dodge);

	ActivationOwnedTags.AddTag(WuwaGameplayTags::State_Invulnerable);
	ActivationOwnedTags.AddTag(WuwaGameplayTags::State_JustDodge_Active);
}

void UGA_DodgeCounterAttack::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	AParagonCharacter* Character = Cast<AParagonCharacter>(GetAvatarActorFromActorInfo());
	if (!Character)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// 1. 락온 타겟이 있으면 락온 타겟에게
	if (AActor* Target = Character->GetLockTarget())
	{
		FVector DirectionToTarget = Target->GetActorLocation() - Character->GetActorLocation();
		DirectionToTarget.Z = 0.f;
		if (!DirectionToTarget.IsNearlyZero())
		{
			Character->SetActorRotation(DirectionToTarget.ToOrientationRotator());
		}
	}
	// 2. 락온 타겟이 없으면, 마지막으로 저스트 회피를 발동시킨 적에게!
	else if (AActor* DodgedEnemy = Character->GetLastDodgedEnemy())
	{
		FVector DirectionToDodged = DodgedEnemy->GetActorLocation() - Character->GetActorLocation();
		DirectionToDodged.Z = 0.f;
		if (!DirectionToDodged.IsNearlyZero())
		{
			Character->SetActorRotation(DirectionToDodged.ToOrientationRotator());
		}
	}
	else 
	{
		// 3. 둘 다 없으면 그냥 현재 입력/바라보는 방향으로
	}

	UAnimMontage* AttackMontage = Character->GetAttackMontage();
	if (AttackMontage)
	{
		MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
			this,
			TEXT("DodgeCounterMontageTask"),
			AttackMontage,
			1.0f,
			FName("AttackDodge")
		);

		WaitEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, WuwaGameplayTags::Event_Combat_AttackEnd);

		if (MontageTask && WaitEventTask)
		{
			MontageTask->OnCompleted.AddDynamic(this, &UGA_DodgeCounterAttack::OnMontageCompleted);
			MontageTask->OnInterrupted.AddDynamic(this, &UGA_DodgeCounterAttack::OnMontageInterrupted);
			MontageTask->OnCancelled.AddDynamic(this, &UGA_DodgeCounterAttack::OnMontageCancelled);
			MontageTask->ReadyForActivation();

			WaitEventTask->EventReceived.AddDynamic(this, &UGA_DodgeCounterAttack::OnAttackEndEventReceived);
			WaitEventTask->ReadyForActivation();
			
			return;
		}
	}

	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

void UGA_DodgeCounterAttack::OnAttackEndEventReceived(FGameplayEventData Payload)
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UGA_DodgeCounterAttack::OnMontageCompleted()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UGA_DodgeCounterAttack::OnMontageInterrupted()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

void UGA_DodgeCounterAttack::OnMontageCancelled()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

void UGA_DodgeCounterAttack::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled)
{
	if (AParagonCharacter* Character = Cast<AParagonCharacter>(GetAvatarActorFromActorInfo()))
	{
		Character->SetCurrentAttackData(nullptr);
		Character->SetWeaponCollisionEnabled(ECollisionEnabled::NoCollision);
		// 공격이 끝났으니 타겟 초기화 (선택사항)
		Character->SetLastDodgedEnemy(nullptr); 
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
