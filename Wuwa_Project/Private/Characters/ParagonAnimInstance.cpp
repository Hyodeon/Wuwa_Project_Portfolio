// Fill out your copyright notice in the Description page of Project Settings.

#include "Characters/ParagonAnimInstance.h"
#include "Characters/ParagonCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "WuwaGameplayTags.h"

void UParagonAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	ParagonCharacter = Cast<AParagonCharacter>(TryGetPawnOwner());
	if (ParagonCharacter)
	{
		ParagonCharacterMovement = ParagonCharacter->GetCharacterMovement();
	}
}

void UParagonAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (!ParagonCharacterMovement)
	{
		if (ParagonCharacter)
		{
			ParagonCharacterMovement = ParagonCharacter->GetCharacterMovement();
		}
		return;
	}

	// 1. 이동 파라미터 갱신
	GroundSpeed = UKismetMathLibrary::VSizeXY(ParagonCharacterMovement->Velocity);
	IsFalling = ParagonCharacterMovement->IsFalling();

	// 2. 무기 장착 상태 유지 (무기 기본 장착)
	CharacterState = ECharacterState::ECS_EquippedOneHandedWeapon;

	// 3. GAS 태그를 기반으로 ActionState 역매핑 (ABP의 IK 제어 및 트랜지션 완벽 호환)
	if (ParagonCharacter)
	{
		if (ParagonCharacter->HasMatchingGameplayTag(WuwaGameplayTags::State_Dead))
		{
			ActionState = EActionState::EAS_Dead;
		}
		else if (ParagonCharacter->HasMatchingGameplayTag(WuwaGameplayTags::Ability_Dodge))
		{
			ActionState = EActionState::EAS_Dodge;
		}
		else if (ParagonCharacter->HasMatchingGameplayTag(WuwaGameplayTags::Ability_Attack))
		{
			ActionState = EActionState::EAS_Attacking;
		}
		else
		{
			ActionState = EActionState::EAS_Unoccupied;
		}
	}
}