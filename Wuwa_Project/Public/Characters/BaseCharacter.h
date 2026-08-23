// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "Interfaces/HitInterface.h"
#include "GameplayTagContainer.h"
#include "BaseCharacter.generated.h"

class UAbilitySystemComponent;
class UParagonAttributeSet;
class UMotionWarpingComponent;
class UAIPerceptionStimuliSourceComponent;
class UBaseAttackData;
class UGameplayEffect;
class UAnimMontage;
class UParticleSystem;
class USoundBase;
class AWeapon;

UCLASS()
class WUWA_PROJECT_API ABaseCharacter : public ACharacter, public IAbilitySystemInterface, public IHitInterface
{
	GENERATED_BODY()

public:
	ABaseCharacter();
	virtual void Tick(float DeltaTime) override;
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	// IHitInterface 구현
	virtual void GetHit_Implementation(const FVector& ImpactPoint, AActor* Hitter) override;

	// 데미지 파이프라인 및 저스트 회피
	virtual void ApplyDamageToTarget(AActor* TargetActor, const UBaseAttackData* AttackData, const FVector& HitLocation);
	virtual bool TryJustDodge(AActor* Attacker);

	// 태그 유틸리티
	bool HasMatchingGameplayTag(FGameplayTag TagToCheck) const;

	// 노티파이(MeleeTrace, AoEHit) 연동 접근자
	FORCEINLINE FVector GetLastMeleeTraceLocation() const { return LastMeleeTraceLocation; }
	FORCEINLINE void SetLastMeleeTraceLocation(const FVector& InLoc) { LastMeleeTraceLocation = InLoc; }
	FORCEINLINE const UBaseAttackData* GetCurrentAttackData() const { return CurrentAttackData; }
	FORCEINLINE void SetCurrentAttackData(const UBaseAttackData* InData) { CurrentAttackData = InData; }

	// 타격 중복 방지
	void ClearDamagedActors();
	void AddDamagedActor(AActor* Target);
	bool HasAlreadyBeenDamaged(AActor* Target) const;

	// 콜리전 제어
	void EnableMeshCollision();
	void DisableMeshCollision();
	void DisableCapsule();

	UFUNCTION(BlueprintCallable, Category = "Combat|Weapon")
	void SetWeaponCollisionEnabled(ECollisionEnabled::Type CollisionEnabled);

	// 시간 왜곡 연출
	void TriggerGlobalDilation(float RealTimeDuration, float TimeDilation);
	void TriggerLocalDilation(float RealTimeDuration, float TimeDilation);

	// 몽타주 및 애니메이션 제어
	virtual void Attack();
	virtual bool CanAttack();

	UFUNCTION(BlueprintCallable)
	virtual void AttackEnd();
	
	UFUNCTION(BlueprintCallable)
	virtual void DodgeEnd();

	FORCEINLINE UAnimMontage* GetAttackMontage() const { return AttackMontage; }

	int32 PlayAttackMontage();
	int32 PlayAttackMontageSection(const FName& SectionName);
	int32 PlayDeathMontage();
	void PlayHitReactMontage(const FName& SectionName);
	
	UFUNCTION(BlueprintCallable)
	FName CalculateDodgeSectionAndWarp(const FVector& DodgeDirection, const bool IsLockedOn);
	
	UFUNCTION(BlueprintCallable)
	FName CalculateJustDodgeSectionAndWarp(const FVector& DodgeDirection, const bool IsLockedOn);
	
	void StopAttackMontage();
	void StopDodgeMontage();

	bool IsHitReacting() const;

	// AI 및 타겟팅 워핑 헬퍼
	UFUNCTION(BlueprintCallable, Category = "Combat|Warping")
	FVector GetTranslationWarpTarget();

	UFUNCTION(BlueprintCallable, Category = "Combat|Warping")
	FVector GetRotationWarpTarget();

	// 생존 및 사망
	virtual void Die();
	bool IsAlive() const;
	virtual void HandleDamage(float DamageAmount);

protected:
	virtual void BeginPlay() override;
	virtual void InitializeAttributes();
	virtual void InitializeGameplayEffects();

	// ================= [컴포넌트] =================
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
	TObjectPtr<UParagonAttributeSet> AttributeSet;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UMotionWarpingComponent> MotionWarpingComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Perception")
	TObjectPtr<UAIPerceptionStimuliSourceComponent> StimuliSourceComponent;

	// ================= [GAS 데이터] =================
	UPROPERTY(EditDefaultsOnly, Category = "GAS|Effects")
	TSubclassOf<UGameplayEffect> DefaultAttributesEffectClass;

	UPROPERTY(EditDefaultsOnly, Category = "GAS|Effects")
	TSubclassOf<UGameplayEffect> DamageEffectClass;

	// ================= [몽타주 및 섹션] =================
	UPROPERTY(EditDefaultsOnly, Category = "Animation|Montages")
	TObjectPtr<UAnimMontage> AttackMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Animation|Montages")
	TArray<FName> AttackMontageSections;

	UPROPERTY(EditDefaultsOnly, Category = "Animation|Montages")
	TObjectPtr<UAnimMontage> DodgeMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Animation|Montages")
	TObjectPtr<UAnimMontage> JustDodgeMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Animation|Montages")
	TObjectPtr<UAnimMontage> HitReactMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Animation|Montages")
	TObjectPtr<UAnimMontage> DeathMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Animation|Montages")
	TArray<FName> DeathMontageSections;

	// ================= [피격 / 시각 / 사운드 연출] =================
	UPROPERTY(EditDefaultsOnly, Category = "VisualFX")
	TObjectPtr<UParticleSystem> HitParticle;

	UPROPERTY(EditDefaultsOnly, Category = "SoundFX")
	TObjectPtr<USoundBase> HitSound;

	// ================= [아이템, 전투 타깃 및 궤적 데이터] =================
	UPROPERTY()
	TObjectPtr<AWeapon> EquippedWeapon;

	UPROPERTY(BlueprintReadWrite, Category = "Combat")
	TObjectPtr<AActor> CombatTarget;

	UPROPERTY(EditAnywhere, Category = "Combat")
	double WarpTargetDistance = 75.f;

	TArray<AActor*> DamagedActors;

	FVector LastMeleeTraceLocation = FVector::ZeroVector;

	UPROPERTY(Transient)
	TObjectPtr<const UBaseAttackData> CurrentAttackData;

	// 내부 연출 및 피격 헬퍼
	void PlayMontageSection(UAnimMontage* Montage, const FName& SectionName);
	int32 PlayRandomMontageSection(UAnimMontage* Montage, const TArray<FName>& SectionNames);
	void DirectionalHitReact(const FVector& ImpactPoint);
	void PlayHitSound(const FVector& ImpactPoint);
	void SpawnHitParticles(const FVector& ImpactPoint);
};