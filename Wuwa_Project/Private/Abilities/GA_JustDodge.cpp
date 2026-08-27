#include "Abilities/GA_JustDodge.h"
#include "Characters/ParagonCharacter.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "AbilitySystemComponent.h"
#include "WuwaGameplayTags.h"
#include "NiagaraFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"

UGA_JustDodge::UGA_JustDodge()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;

	AbilityTags.AddTag(WuwaGameplayTags::Ability_JustDodge);

	// 저스트 회피 발동 시 일반 회피와 공격 등 캔슬
	CancelAbilitiesWithTag.AddTag(WuwaGameplayTags::Ability_Dodge);
	CancelAbilitiesWithTag.AddTag(WuwaGameplayTags::Ability_Attack);

	// 발동 중 무적 및 저스트 닷지 상태 부여
	ActivationOwnedTags.AddTag(WuwaGameplayTags::State_Invulnerable);
	ActivationOwnedTags.AddTag(WuwaGameplayTags::State_JustDodge_Active);
}

void UGA_JustDodge::ActivateAbility(
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

	// 닷지를 유발한 적을 캐싱 (반격기 등에서 타겟팅에 사용)
	if (TriggerEventData && TriggerEventData->Instigator)
	{
		const AActor* ConstInstigator = TriggerEventData->Instigator;
		Character->SetLastDodgedEnemy(const_cast<AActor*>(ConstInstigator));
	}

	// 기존 회피에서 캐싱한 방향과 록온 상태 가져오기
	FVector CachedDirection = Character->GetCachedDodgeDirection();
	bool bIsLockedOn = Character->GetCachedIsLockedOn();

	// 록온 상태가 아닐 때 방향키 입력이 있었다면 해당 방향으로 회전
	if (!bIsLockedOn && !CachedDirection.IsNearlyZero())
	{
		Character->SetActorRotation(CachedDirection.ToOrientationRotator());
	}

	// 1. 시각적/청각적 이펙트 및 글로벌 슬로우(Time Dilation) 적용
	if (Character->JustDodgeFX)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, Character->JustDodgeFX, Character->GetActorLocation(), Character->GetActorRotation());
	}

	if (Character->JustDodgeSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, Character->JustDodgeSound, Character->GetActorLocation());
	}

	// 저스트 닷지 시 1.0초간 0.1배속 (원하시는 수치로 조정 가능)
	Character->TriggerGlobalDilation(1.0f, 0.1f);

	// 2. 몽타주 섹션 산출 및 모션 워핑 타겟 업데이트
	FName TargetSection = Character->CalculateJustDodgeSectionAndWarp(CachedDirection, bIsLockedOn);

	if (JustDodgeMontage)
	{
		MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
			this,
			TEXT("JustDodgeMontageTask"),
			JustDodgeMontage,
			1.0f,
			TargetSection
		);

		WaitEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, WuwaGameplayTags::Event_Combat_DodgeEnd);

		if (MontageTask && WaitEventTask)
		{
			MontageTask->OnCompleted.AddDynamic(this, &UGA_JustDodge::OnMontageCompleted);
			MontageTask->OnInterrupted.AddDynamic(this, &UGA_JustDodge::OnMontageInterrupted);
			MontageTask->OnCancelled.AddDynamic(this, &UGA_JustDodge::OnMontageCancelled);
			MontageTask->ReadyForActivation();

			WaitEventTask->EventReceived.AddDynamic(this, &UGA_JustDodge::OnDodgeEndEventReceived);
			WaitEventTask->ReadyForActivation();

			bAttackBuffered = false;
			bComboWindowOpen = false;

			InputEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, WuwaGameplayTags::Event_Combat_AttackInput, nullptr, false, false);
			if (InputEventTask)
			{
				InputEventTask->EventReceived.AddDynamic(this, &UGA_JustDodge::OnAttackInputReceived);
				InputEventTask->ReadyForActivation();
			}

			CheckComboTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, WuwaGameplayTags::Event_Combat_CheckCombo, nullptr, false, false);
			if (CheckComboTask)
			{
				CheckComboTask->EventReceived.AddDynamic(this, &UGA_JustDodge::OnCheckComboReceived);
				CheckComboTask->ReadyForActivation();
			}
			
			return;
		}
	}

	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

void UGA_JustDodge::OnDodgeEndEventReceived(FGameplayEventData Payload)
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UGA_JustDodge::TriggerCounterAttack()
{
	AParagonCharacter* Character = Cast<AParagonCharacter>(GetAvatarActorFromActorInfo());
	if (Character && Character->GetDodgeCounterAbilityClass())
	{
		// 수동으로 반격기 호출 후 현재 닷지(자신)는 종료
		Character->GetAbilitySystemComponent()->TryActivateAbilityByClass(Character->GetDodgeCounterAbilityClass());
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
	}
}

void UGA_JustDodge::OnAttackInputReceived(FGameplayEventData Payload)
{
	bAttackBuffered = true;
	
	// 콤보 윈도우가 이미 열려있다면 즉시 반격기 발동
	if (bComboWindowOpen)
	{
		TriggerCounterAttack();
	}
}

void UGA_JustDodge::OnCheckComboReceived(FGameplayEventData Payload)
{
	// 콤보 윈도우 개방
	bComboWindowOpen = true;

	// 이미 입력이 버퍼링 되어 있다면 즉시 반격기 발동
	if (bAttackBuffered)
	{
		TriggerCounterAttack();
	}
}

void UGA_JustDodge::OnMontageCompleted()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UGA_JustDodge::OnMontageInterrupted()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

void UGA_JustDodge::OnMontageCancelled()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

void UGA_JustDodge::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

