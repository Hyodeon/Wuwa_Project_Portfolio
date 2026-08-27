// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Characters/BaseCharacter.h"
#include "Characters/CharacterTypes.h"
#include "Enemy.generated.h"

class UHealthBarComponent;
class UNiagaraComponent;
class AAIController;

UCLASS()
class WUWA_PROJECT_API AEnemy : public ABaseCharacter
{
	GENERATED_BODY()

public:
	AEnemy();
	
	/* <AActor> */
	virtual void Tick(float DeltaTime) override;
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;
	virtual void Destroyed() override;
	/* </AActor> */

	/* <IHitInterface> */
	virtual void GetHit_Implementation(const FVector& ImpactPoint, AActor* Hitter) override;
	/* </IHitInterface> */

	// <COMBAT & AI>
	virtual void Attack() override;
	virtual bool CanAttack() override;
	virtual void AttackEnd() override;

	void SetInterest(AActor* Target);
	void LoseInterest();

	UFUNCTION(BlueprintCallable, Category = "Combat|MotionWarping")
	void UpdateCombatWarpTarget(FName WarpTargetName = FName("CombatTarget"));

	UFUNCTION(BlueprintCallable, Category = "Combat|Parry")
	void GetParried(AActor* Parrier, const FVector& ImpactPoint);

	UPROPERTY(BlueprintReadWrite, Category = "Combat|Parry")
	bool bCanBeParried = false;

	// <SWARM AI>
	UFUNCTION(BlueprintCallable, Category = "AI|Swarm")
	void OnAllyDied();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Swarm")
	float BaseAggression = 1.0f;

	UPROPERTY(BlueprintReadWrite, Category = "AI|Swarm")
	float CurrentAggression = 1.0f;

	UPROPERTY(BlueprintReadWrite, Category = "AI|Swarm")
	float RecentDamage = 0.0f;
	// </SWARM AI>
	
	// <GROGGY>
	UFUNCTION(BlueprintCallable, Category = "Combat|Groggy")
	void TriggerGroggy();

	UFUNCTION(BlueprintCallable, Category = "Combat|Groggy")
	void RecoverFromGroggy();
	// </GROGGY>
	// </COMBAT>


protected:
	/* <AActor> */
	virtual void BeginPlay() override;
	/* </AActor> */

	/* <ABaseCharacter> */
	virtual void Die() override;
	virtual void InitializeAttributes() override;
	/* </ABaseCharacter> */

	void HideHealthBar();
	void ShowHealthBar();

	void Pushed(const FVector& PushVector);
	FVector GetClampedAttackDestination(AActor* Target) const;
	void TriggerGlobalHitStop(float RealTimeDuration = 0.12f, float TimeDilation = 0.05f);
	void ResetGlobalHitStop();

	UPROPERTY(BlueprintReadOnly)
	EEnemyState EnemyState = EEnemyState::EES_Patrolling;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI")
	TObjectPtr<class UBehaviorTree> BehaviorTree;

private:
	/* AI Behaviour */
	void InitilizeEnemy();
	void CalculateAttackForce(FVector& PushDirection, AActor* Hitter, float& FinalForce);

	UPROPERTY(VisibleAnywhere)
	UHealthBarComponent* HealthBarWidget;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Parry")
	TObjectPtr<class UNiagaraSystem> ParryImpactFX;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Parry")
	TObjectPtr<USoundBase> ParrySound;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Parry")
	TSubclassOf<UCameraShakeBase> ParryShakeClass;

	UPROPERTY(EditAnywhere, Category = "Combat")
	float MaxWarpDashDistance = 350.f;

	UPROPERTY(EditAnywhere, Category = "Combat")
	float DeathLifeSpan = 5.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Groggy")
	TObjectPtr<UAnimMontage> GroggyMontage;

	FTimerHandle GroggyTimerHandle;

	UPROPERTY()
	TObjectPtr<AAIController> EnemyController;

public:
	FORCEINLINE UBehaviorTree* GetBehaviorTree() const { return BehaviorTree; }
	FORCEINLINE bool IsDead() const { return EnemyState == EEnemyState::EES_Dead; }
	FORCEINLINE bool IsEngaged() const { return EnemyState == EEnemyState::EES_Engaged; }
};