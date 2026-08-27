// Fill out your copyright notice in the Description page of Project Settings.

#include "Abilities/GA_Valor_ESkill.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitInputRelease.h"
#include "Abilities/Tasks/AbilityTask_WaitDelay.h"
#include "GameFramework/Character.h"
#include "Animation/AnimInstance.h"
#include "Characters/BaseCharacter.h"
#include "Data/PlayerAttackData.h"

UGA_Valor_ESkill::UGA_Valor_ESkill()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
}

void UGA_Valor_ESkill::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (AttackData)
	{
		if (class ABaseCharacter* Character = Cast<class ABaseCharacter>(GetAvatarActorFromActorInfo()))
		{
			Character->SetCurrentAttackData(AttackData);
		}
	}

	//if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	//{
	//	EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
	//	return;
	//}

	if (!CheckCooldown(Handle, ActorInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (!SkillMontage)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	bHasReleased = false;

	// 1. 몽타주 재생 (기본 시작점: Charge_Start)
	MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this,
		TEXT("PlaySkillMontage"),
		SkillMontage,
		1.0f,
		Section_Start
	);

	if (!MontageTask)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	MontageTask->OnCompleted.AddDynamic(this, &UGA_Valor_ESkill::OnMontageCompleted);
	MontageTask->OnInterrupted.AddDynamic(this, &UGA_Valor_ESkill::OnMontageInterrupted);
	MontageTask->OnCancelled.AddDynamic(this, &UGA_Valor_ESkill::OnMontageCancelled);
	MontageTask->ReadyForActivation();

	// 2. 입력 해제(Release) 대기 태스크
	WaitInputReleaseTask = UAbilityTask_WaitInputRelease::WaitInputRelease(this, true);
	if (WaitInputReleaseTask)
	{
		//  OnRelease는 OnInputReleased(float)에 바인딩
		WaitInputReleaseTask->OnRelease.AddDynamic(this, &UGA_Valor_ESkill::OnInputReleased);
		WaitInputReleaseTask->ReadyForActivation();
	}

	// 3. 최대 차지 시간 대기 태스크 (2.5초)
	WaitDelayTask = UAbilityTask_WaitDelay::WaitDelay(this, MaxChargeTime);
	if (WaitDelayTask)
	{
		//  OnFinish는 HandleReleaseTriggered()에 바인딩
		WaitDelayTask->OnFinish.AddDynamic(this, &UGA_Valor_ESkill::HandleReleaseTriggered);
		WaitDelayTask->ReadyForActivation();
	}
}

void UGA_Valor_ESkill::OnInputReleased(float TimeHeld)
{
	HandleReleaseTriggered();
}

void UGA_Valor_ESkill::HandleReleaseTriggered()
{
	if (bHasReleased) return;
	bHasReleased = true;

	// 기존 릴리즈 점프/연결 로직 그대로 유지
	if (IsValid(WaitInputReleaseTask))
	{
		WaitInputReleaseTask->EndTask();
		WaitInputReleaseTask = nullptr; // 핵심: 쓰레기값 방지
	}
	if (IsValid(WaitDelayTask))
	{
		WaitDelayTask->EndTask();
		WaitDelayTask = nullptr; // 핵심: 쓰레기값 방지
	}

	ACharacter* Character = Cast<ACharacter>(GetAvatarActorFromActorInfo());
	if (!Character) return;

	UAnimInstance* AnimInstance = Character->GetMesh() ? Character->GetMesh()->GetAnimInstance() : nullptr;
	if (!AnimInstance || !SkillMontage) return;

	const FName CurrentSection = AnimInstance->Montage_GetCurrentSection(SkillMontage);

	//if (CurrentSection == Section_Start)
	//{
	//	AnimInstance->Montage_SetNextSection(Section_Start, Section_Release, SkillMontage);
	//}
	//else
	//{
	//	AnimInstance->Montage_JumpToSection(Section_Release, SkillMontage);
	//}

	AnimInstance->Montage_JumpToSection(Section_Release, SkillMontage);
}

void UGA_Valor_ESkill::OnMontageCompleted()
{
	CommitAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo);

	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UGA_Valor_ESkill::OnMontageInterrupted()
{

	if (bHasReleased)
	{
		// 만약 차징 중(루프)에 끊긴 게 아니라, 
		// 베기(릴리즈)가 이미 시작된 이후에 끊겼다면 쿨타임을 돌리게 처리할 수도 있습니다.
	}

	CommitAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo);

	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

void UGA_Valor_ESkill::OnMontageCancelled()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

void UGA_Valor_ESkill::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled)
{
	if (class ABaseCharacter* Character = Cast<class ABaseCharacter>(GetAvatarActorFromActorInfo()))
	{
		Character->SetCurrentAttackData(nullptr);
	}

	bHasReleased = false;

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}