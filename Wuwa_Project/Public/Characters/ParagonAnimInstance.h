// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Characters/CharacterTypes.h"
#include "ParagonAnimInstance.generated.h"

class AParagonCharacter;
class UCharacterMovementComponent;

UCLASS()
class WUWA_PROJECT_API UParagonAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

protected:
	UPROPERTY(BlueprintReadOnly, Category = "Character")
	TObjectPtr<AParagonCharacter> ParagonCharacter;

	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	TObjectPtr<UCharacterMovementComponent> ParagonCharacterMovement;

	// 기존 ABP 노드와 100% 일치하는 변수명 유지
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	float GroundSpeed = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	bool IsFalling = false;

	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	ECharacterState CharacterState = ECharacterState::ECS_EquippedOneHandedWeapon;

	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	EActionState ActionState = EActionState::EAS_Unoccupied;
};