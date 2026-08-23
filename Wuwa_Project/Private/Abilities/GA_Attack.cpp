#include "Abilities/GA_Attack.h"
#include "Characters/ParagonCharacter.h"
#include "Data/PlayerAttackData.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "AbilitySystemComponent.h"
#include "WuwaGameplayTags.h"

UGA_Attack::UGA_Attack()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;

	AbilityTags.AddTag(WuwaGameplayTags::Ability_Attack);

	// 저스트 닷지 중일 때는 일반 공격(GA_Attack)이 새로 발동되지 않도록 막음
	ActivationBlockedTags.AddTag(WuwaGameplayTags::State_JustDodge_Active);
}

void UGA_Attack::ActivateAbility(
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

	ComboIndex = 0;
	bInputBuffered = false;
	ActivationTime = GetWorld()->GetTimeSeconds();

	// 이벤트 리스너들을 한 번만 생성해두고 계속 이벤트를 받습니다. (OnlyTriggerOnce = false)
	InputEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, WuwaGameplayTags::Event_Combat_AttackInput, nullptr, false, false);
	if (InputEventTask)
	{
		InputEventTask->EventReceived.AddDynamic(this, &UGA_Attack::OnAttackInputReceived);
		InputEventTask->ReadyForActivation();
	}

	CheckComboTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, WuwaGameplayTags::Event_Combat_CheckCombo, nullptr, false, false);
	if (CheckComboTask)
	{
		CheckComboTask->EventReceived.AddDynamic(this, &UGA_Attack::OnCheckComboReceived);
		CheckComboTask->ReadyForActivation();
	}

	AttackEndTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, WuwaGameplayTags::Event_Combat_AttackEnd, nullptr, false, false);
	if (AttackEndTask)
	{
		AttackEndTask->EventReceived.AddDynamic(this, &UGA_Attack::OnAttackEndReceived);
		AttackEndTask->ReadyForActivation();
	}

	PlayNextCombo();
}

void UGA_Attack::PlayNextCombo()
{
	bInputBuffered = false;

	AParagonCharacter* Character = Cast<AParagonCharacter>(GetAvatarActorFromActorInfo());
	if (!Character)
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}

	if (ComboIndex >= Character->BasicAttackCombo.Num())
	{
		// 콤보 끝
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
		return;
	}

	UPlayerAttackData* AttackData = Character->BasicAttackCombo[ComboIndex];
	if (!AttackData || !AttackData->AttackMontage)
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}

	// 캐릭터의 현재 공격 데이터 업데이트 (무기가 데미지를 가할 때 사용)
	Character->SetCurrentAttackData(AttackData);

	// 공격 직전, 유저의 입력 방향 또는 적 타겟 방향으로 회전 적용
	Character->AdjustAttackRotation();

	// 몽타주 태스크를 새로 생성하여 실행
	if (MontageTask)
	{
		MontageTask->EndTask();
	}

	MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this,
		NAME_None,
		AttackData->AttackMontage,
		1.0f,
		AttackData->MontageSectionName
	);

	if (MontageTask)
	{
		MontageTask->OnCompleted.AddDynamic(this, &UGA_Attack::OnMontageCompleted);
		MontageTask->OnInterrupted.AddDynamic(this, &UGA_Attack::OnMontageInterrupted);
		MontageTask->OnCancelled.AddDynamic(this, &UGA_Attack::OnMontageCancelled);
		MontageTask->ReadyForActivation();
	}
}

void UGA_Attack::OnAttackInputReceived(FGameplayEventData Payload)
{
	// 트리거된 순간의 이벤트를 WaitGameplayEvent가 잡는 것을 방지
	if (GetWorld()->GetTimeSeconds() - ActivationTime < 0.1f) return;

	bInputBuffered = true;
}

void UGA_Attack::OnCheckComboReceived(FGameplayEventData Payload)
{
	if (bInputBuffered)
	{
		ComboIndex++;
		PlayNextCombo();
	}
}

void UGA_Attack::OnAttackEndReceived(FGameplayEventData Payload)
{
	// 콤보가 종료되고 평타를 마칩니다.
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UGA_Attack::OnMontageCompleted()
{
	// 몽타주가 끝날 때까지 다른 타수 입력이 없었거나 AttackEnd 노티파이가 없었다면 종료
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UGA_Attack::OnMontageInterrupted()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

void UGA_Attack::OnMontageCancelled()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

void UGA_Attack::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled)
{
	// 콤보가 종료되면 무기의 충돌 판정 방지 등을 위해 데이터 제거
	if (AParagonCharacter* Character = Cast<AParagonCharacter>(GetAvatarActorFromActorInfo()))
	{
		Character->SetCurrentAttackData(nullptr);
		Character->SetWeaponCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
