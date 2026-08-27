// Fill out your copyright notice in the Description page of Project Settings.

#include "Abilities/GA_Dodge.h"
#include "Characters/ParagonCharacter.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "AbilitySystemComponent.h"
#include "WuwaGameplayTags.h"

UGA_Dodge::UGA_Dodge()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;

	// 1. 어빌리티 태그 식별
	AbilityTags.AddTag(WuwaGameplayTags::Ability_Dodge);

	// 2. 발동 시 공격 태그를 가진 어빌리티 자동 캔슬
	CancelAbilitiesWithTag.AddTag(WuwaGameplayTags::Ability_Attack);

	// 3. 발동 중 적용될 상태 태그
	ActivationOwnedTags.AddTag(WuwaGameplayTags::State_Invulnerable);

	ActivationBlockedTags.AddTag(WuwaGameplayTags::State_JustDodge_Active);
}

void UGA_Dodge::ActivateAbility(
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

	// 스태미나 소모(Cost) 적용
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	
	UE_LOG(LogTemp, Log, TEXT("UGA_Dodge::ActivateAbility() - Stamina Cost Applied"));

	// 1. 회피 방향 및 록온 상태 계산
	FVector WorldDirection = FVector::ZeroVector;
	Character->GetInputMoveDirection(WorldDirection);

	const bool bIsLockedOn = (Character->GetLockTarget() != nullptr);

	// 저스트 회피 발생 시 사용할 수 있도록 캐릭터에 캐싱
	Character->SetCachedDodgeDirection(WorldDirection);
	Character->SetCachedIsLockedOn(bIsLockedOn);

	// 2. 타겟 방향 회전 (비 록온 상태일 때만)
	if (!bIsLockedOn && !WorldDirection.IsNearlyZero())
	{
		Character->SetActorRotation(WorldDirection.ToOrientationRotator());
	}

	// 3. 몽타주 섹션 계산 및 Motion Warping 타겟 업데이트
	FName TargetSection = Character->CalculateDodgeSectionAndWarp(WorldDirection, bIsLockedOn);

	// 4. 몽타주 재생 및 종료(Cancel) 이벤트 대기
	if (DodgeMontage)
	{
		UE_LOG(LogTemp, Log, TEXT("UGA_Dodge::ActivateAbility() - Playing Dodge Montage"));

		// 애니메이션 재생 태스크
		MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
			this,
			TEXT("DodgeMontageTask"),
			DodgeMontage,
			1.0f,
			TargetSection
		);

		// 애니메이션 도중 캔슬 가능 시점 이벤트를 대기하는 태스크
		WaitEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, WuwaGameplayTags::Event_Combat_DodgeEnd);

		if (MontageTask && WaitEventTask)
		{
			MontageTask->OnCompleted.AddDynamic(this, &UGA_Dodge::OnMontageCompleted);
			MontageTask->OnInterrupted.AddDynamic(this, &UGA_Dodge::OnMontageInterrupted);
			MontageTask->OnCancelled.AddDynamic(this, &UGA_Dodge::OnMontageCancelled);
			MontageTask->ReadyForActivation();

			WaitEventTask->EventReceived.AddDynamic(this, &UGA_Dodge::OnDodgeEndEventReceived);
			WaitEventTask->ReadyForActivation();
			
			return;
		}
	}

	// 몽타주가 없거나 태스크 생성 실패 시 즉시 종료
	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

void UGA_Dodge::OnDodgeEndEventReceived(FGameplayEventData Payload)
{
	// ABP의 DodgeEnd 노티파이가 밟혀서 이벤트가 도착하면 어빌리티 종료
	// 이렇게 하면 애니메이션 후딜레이 중에 새로운 입력이 가능해집니다.
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UGA_Dodge::OnMontageCompleted()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UGA_Dodge::OnMontageInterrupted()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

void UGA_Dodge::OnMontageCancelled()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

void UGA_Dodge::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled)
{
	// 잔여 저스트 회피 윈도우 태그 정리 방어
	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
	{
		ASC->RemoveLooseGameplayTag(WuwaGameplayTags::State_DodgeWindow);
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}