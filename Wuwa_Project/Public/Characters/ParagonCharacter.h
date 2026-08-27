// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Characters/BaseCharacter.h"
#include "InputActionValue.h"
#include "Data/PlayerAttackData.h"
#include "ActiveGameplayEffectHandle.h"
#include "ParagonCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UCombatCameraComponent;
class UInputMappingContext;
class UInputAction;
class UParagonOverlay;
class UGameplayAbility;
class UGameplayEffect;
class UNiagaraSystem;
class USoundBase;
class UCombatManagerComponent;

UCLASS()
class WUWA_PROJECT_API AParagonCharacter : public ABaseCharacter
{
	GENERATED_BODY()

public:
	AParagonCharacter();
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// 피격 및 무적 처리
	virtual void GetHit_Implementation(const FVector& ImpactPoint, AActor* Hitter) override;

	// 록온 및 방향 헬퍼 (GA에서 호출)
	void GetInputMoveDirection(FVector& OutWorldDirection);
	AActor* FindBestTargetInFront();
	bool CalculateCounterAttackWarpTargets(AActor* TargetActor, FVector& OutTargetLoc, FRotator& OutTargetRot);
	void AdjustAttackRotation();

	// 회피 및 전투 캐싱 데이터 접근자 (GA_Dodge 연동용)
	FORCEINLINE FVector GetCachedDodgeDirection() const { return CachedDodgeDirection; }
	FORCEINLINE void SetCachedDodgeDirection(const FVector& InDir) { CachedDodgeDirection = InDir; }
	FORCEINLINE bool GetCachedIsLockedOn() const { return bCachedIsLockedOn; }
	FORCEINLINE void SetCachedIsLockedOn(bool bLocked) { bCachedIsLockedOn = bLocked; }
	FORCEINLINE AActor* GetLockTarget() const { return LockTarget; }
	FORCEINLINE AActor* GetLastDodgedEnemy() const { return LastDodgedEnemy.Get(); }
	FORCEINLINE void SetLastDodgedEnemy(AActor* Enemy) { LastDodgedEnemy = Enemy; }
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Combat|Targeting")
	FORCEINLINE bool IsLockedOn() const { return bIsLockedOn && LockTarget != nullptr; }
	TSubclassOf<UGameplayAbility> GetDodgeCounterAbilityClass() const { return DodgeCounterAbilityClass; }	

	// 무기 장착
	void EquipWeapon(AWeapon* Weapon);

	// 시각/사운드 이펙트
	UPROPERTY(EditDefaultsOnly, Category = "VisualFX")
	TObjectPtr<UNiagaraSystem> JustDodgeFX;

	UPROPERTY(EditDefaultsOnly, Category = "SoundFX")
	TObjectPtr<USoundBase> JustDodgeSound;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Weapon")
	TSubclassOf<AWeapon> DefaultWeaponClass;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Attack")
	TArray<TObjectPtr<UPlayerAttackData>> BasicAttackCombo;

protected:
	virtual void BeginPlay() override;
	virtual void InitializeAttributes() override;
	virtual void InitializeGameplayEffects() override;

	// ================= [컴포넌트] =================
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<USpringArmComponent> SpringArm;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<UCameraComponent> Camera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<UCombatCameraComponent> CombatCameraComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	TObjectPtr<UCombatManagerComponent> CombatManager;

	// ================= [입력 액션 및 매핑] =================
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> ParagonMappingContext;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> LookAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> JumpAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> EquipAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> AttackAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> DodgeAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> LockAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> ESkillAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> UltimateAction;

	// ================= [GAS 어빌리티 클래스 목록] =================
	UPROPERTY(EditDefaultsOnly, Category = "GAS|Abilities")
	TSubclassOf<UGameplayAbility> AttackAbilityClass;

	UPROPERTY(EditDefaultsOnly, Category = "GAS|Abilities")
	TSubclassOf<UGameplayAbility> DodgeAbilityClass;

	UPROPERTY(EditDefaultsOnly, Category = "GAS|Abilities")
	TSubclassOf<UGameplayAbility> JustDodgeAbilityClass;

	UPROPERTY(EditDefaultsOnly, Category = "GAS|Abilities")
	TSubclassOf<UGameplayAbility> DodgeCounterAbilityClass;

	UPROPERTY(EditDefaultsOnly, Category = "GAS|Abilities")
	TSubclassOf<UGameplayAbility> ESkillAbilityClass;

	UPROPERTY(EditDefaultsOnly, Category = "GAS|Abilities")
	TSubclassOf<UGameplayAbility> UltimateAbilityClass;

	UPROPERTY(EditDefaultsOnly, Category = "GAS|Abilities")
	TSubclassOf<UGameplayAbility> JumpAbilityClass;

	// 패시브 효과 (스태미나 재생 등)
	UPROPERTY(EditDefaultsOnly, Category = "GAS|Effects")
	TSubclassOf<UGameplayEffect> StaminaRegenEffectClass;

	// ================= [타겟팅 및 카메라 파라미터] =================
	UPROPERTY(EditDefaultsOnly, Category = "Combat|Targeting")
	float BaseSpringArmLength = 350.f;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Targeting")
	float LockOnSearchRadius = 1500.f;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Targeting")
	float AutoTargetRadius = 800.f;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Targeting")
	float AutoTargetAngle = 60.f;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Targeting")
	float DodgeAttackTargetOffset = 150.f;

private:
	// 입력 핸들러 (GAS 트리거 전담)
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	virtual void Jump() override;
	void FKeyPressed();
	void Attack();
	void Dodge();
	void ToggleLockOn();
	void ESkillStarted();
	void ESkillReleased();
	void UltimateStarted();

	virtual void AttackEnd() override;
	virtual void DodgeEnd() override;

	// 록온 내부 처리
	void ClearLockOn();
	AActor* FindBestLockOnTarget();

	// UI 및 오버레이
	void InitializeParagonOverlay();

	UPROPERTY()
	TObjectPtr<UParagonOverlay> ParagonOverlay;

	UPROPERTY()
	TObjectPtr<AActor> OverlappingItem;

	UPROPERTY()
	TObjectPtr<AActor> LockTarget;

	TWeakObjectPtr<AActor> LastDodgedEnemy;

	bool bIsLockedOn = false;
	bool bCachedIsLockedOn = false;
	FVector CachedDodgeDirection = FVector::ZeroVector;

public:
	// ================= [패시브 (Valor)] =================
	UPROPERTY(EditDefaultsOnly, Category = "Combat|Passives")
	TSubclassOf<UGameplayEffect> GE_Valor_DamageReduction;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Passives")
	TSubclassOf<UGameplayEffect> GE_Valor_SuperArmor;

	// Valor 수치가 변경될 때 AttributeSet이 이 함수를 호출합니다.
	void OnValorChanged(float OldValue, float NewValue);

private:
	// 현재 적용중인 GE들의 핸들 (조건 불만족 시 회수하기 위함)
	FActiveGameplayEffectHandle ActiveValorDRHandle;
	FActiveGameplayEffectHandle ActiveValorSuperArmorHandle;
};