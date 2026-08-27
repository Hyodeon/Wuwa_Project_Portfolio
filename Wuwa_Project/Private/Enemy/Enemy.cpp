// Fill out your copyright notice in the Description page of Project Settings.

#include "Enemy/Enemy.h"

#include "Components/CapsuleComponent.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "HUD/HealthBarComponent.h"
#include "MotionWarpingComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Camera/CameraShakeBase.h"

#include "AIController.h"
#include "AI/BaseAIController.h"
#include "BrainComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Components/CombatManagerComponent.h"
#include "WuwaGameplayTags.h"

#include "AttributeSet/ParagonAttributeSet.h"
#include "AbilitySystemComponent.h"

AEnemy::AEnemy()
{
	PrimaryActorTick.bCanEverTick = true;

	GetMesh()->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetGenerateOverlapEvents(true);

	GetCapsuleComponent()->SetGenerateOverlapEvents(true);

	HealthBarWidget = CreateDefaultSubobject<UHealthBarComponent>(TEXT("HealthBarWidget"));
	HealthBarWidget->SetupAttachment(GetRootComponent());

	GetCharacterMovement()->bOrientRotationToMovement = true;
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	AIControllerClass = ABaseAIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
}

void AEnemy::BeginPlay()
{
	Super::BeginPlay();

	InitilizeEnemy();

	Tags.Add(FName("Enemy"));
}

void AEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (IsDead()) return;

	// <SWARM AI> 서서히 호전성 회복 및 받은 피해 감쇠
	if (CurrentAggression < BaseAggression)
	{
		CurrentAggression += DeltaTime * 0.1f;
	}
	if (RecentDamage > 0.0f)
	{
		RecentDamage -= DeltaTime * 15.0f;
		if (RecentDamage <= 0.0f)
		{
			RecentDamage = 0.0f;
			if (EnemyController && EnemyController->GetBlackboardComponent())
				EnemyController->GetBlackboardComponent()->SetValueAsBool(TEXT("bNeedsHelp"), false);
		}
	}

	if (EnemyController && EnemyController->GetBlackboardComponent())
	{
		EnemyController->GetBlackboardComponent()->SetValueAsFloat(TEXT("AggressionLevel"), CurrentAggression);
	}
	// </SWARM AI>

	if (EnemyController)
	{
		if (UBlackboardComponent* BB = EnemyController->GetBlackboardComponent())
		{
			bool bHitReacting = BB->GetValueAsBool(TEXT("IsHitReacting"));
			float Aggression = BB->GetValueAsFloat(TEXT("AggressionLevel"));

			// 실제 토큰 보유 여부 훔쳐보기
			FString TokenStr = TEXT("None");
			if (AActor* Target = Cast<AActor>(BB->GetValueAsObject(TEXT("TargetActor"))))
			{
				if (UCombatManagerComponent* Manager = Target->FindComponentByClass<UCombatManagerComponent>())
				{
					if (Manager->HasToken(this)) TokenStr = TEXT("Holding Token!");
				}
			}
			// 허공에 띄울 텍스트 조립
			FString DebugMsg = FString::Printf(TEXT("HitReacting: %d\nAggression: %.1f\nToken: %s"),
				bHitReacting, Aggression, *TokenStr);
			// 캐릭터 머리 위(Z축 +120)에 노란색 텍스트 렌더링
			DrawDebugString(GetWorld(), GetActorLocation() + FVector(0, 0, 120), DebugMsg, nullptr, FColor::Yellow, 0.0f, true);
		}
	}
}

float AEnemy::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	return 0.0f;
}

void AEnemy::Destroyed()
{
	Super::Destroyed();
}

void AEnemy::Attack()
{
	Super::Attack();

	// 공격 몽타주 실행
	PlayAttackMontage();
	EnemyState = EEnemyState::EES_Attacking;
}

bool AEnemy::CanAttack()
{
	return !IsDead() && EnemyState != EEnemyState::EES_Attacking && !IsHitReacting();
}

void AEnemy::AttackEnd()
{
	Super::AttackEnd();
	if (!IsDead()) EnemyState = EEnemyState::EES_Engaged;

	// 공격이 자연스럽게 끝났을 때 토큰 반납
	if (AActor* Target = Cast<AActor>(EnemyController->GetBlackboardComponent()->GetValueAsObject(FName("TargetActor"))))
	{
		if (UCombatManagerComponent* Manager = Target->FindComponentByClass<UCombatManagerComponent>())
			Manager->ReleaseToken(this);
	}
}

void AEnemy::GetHit_Implementation(const FVector& ImpactPoint, AActor* Hitter)
{
	if (HasMatchingGameplayTag(WuwaGameplayTags::State_Invulnerable) ||
		HasMatchingGameplayTag(WuwaGameplayTags::State_HyperArmor))
	{
		return;
	}

	Super::GetHit_Implementation(ImpactPoint, Hitter);

	if (!IsAlive()) return;

	// <SWARM AI> 피격 시 호전성 깎임 및 누적 데미지 증가
	RecentDamage += 25.0f;
	CurrentAggression -= 0.15f;
	if (CurrentAggression < 0.0f) CurrentAggression = 0.0f;

	if (RecentDamage > 50.0f)
	{
		if (AAIController* AICon = Cast<AAIController>(GetController()))
		{
			if (UBlackboardComponent* BB = AICon->GetBlackboardComponent())
			{
				BB->SetValueAsBool(TEXT("bNeedsHelp"), true);

				// 도움이 필요하니 토큰을 반납한다.
				if (!HasMatchingGameplayTag(WuwaGameplayTags::State_SuperArmor) || bCanBeParried)
				{
					if (AActor* Target = Cast<AActor>(EnemyController->GetBlackboardComponent()->GetValueAsObject(FName("TargetActor"))))
					{
						if (UCombatManagerComponent* Manager = Target->FindComponentByClass<UCombatManagerComponent>())
						{
							Manager->ReleaseToken(this);
						}
					}
				}
			}
		}
	}
	// </SWARM AI>

	// 맞거나 죽으면 쥐고 있던 토큰을 무조건 뱉어냄 (토큰 누수 원천 차단)
	if (!HasMatchingGameplayTag(WuwaGameplayTags::State_SuperArmor) || bCanBeParried)
	{
		if (AActor* Target = Cast<AActor>(EnemyController->GetBlackboardComponent()->GetValueAsObject(FName("TargetActor"))))
		{
			if (UCombatManagerComponent* Manager = Target->FindComponentByClass<UCombatManagerComponent>())
			{
				Manager->ReleaseToken(this);
			}
		}
	}

	// 상태 업데이트
	if (AAIController* AICon = Cast<AAIController>(GetController()))
	{
		if (!HasMatchingGameplayTag(WuwaGameplayTags::State_SuperArmor) || bCanBeParried)
		{
			AICon->StopMovement();
		}
		if (UBlackboardComponent* BB = AICon->GetBlackboardComponent())
		{
			if (!HasMatchingGameplayTag(WuwaGameplayTags::State_SuperArmor) || bCanBeParried)
			{
				BB->SetValueAsBool(FName("IsHitReacting"), true);
			}
			if (Hitter && Hitter->ActorHasTag(FName("EngageableTarget")))
			{
				BB->SetValueAsObject(FName("TargetActor"), Hitter);
			}
		}
	}

	if (bCanBeParried)
	{
		GetParried(Hitter, ImpactPoint);
		if (!HasMatchingGameplayTag(WuwaGameplayTags::State_Groggy))
		{
			PlayHitReactMontage(FName("Front"));
			if (MotionWarpingComponent)
			{
				MotionWarpingComponent->RemoveAllWarpTargets();
			}
		}
	}

	FVector PushDirection = Hitter ? Hitter->GetActorForwardVector().GetSafeNormal2D() : (GetActorLocation() - ImpactPoint).GetSafeNormal2D();
	float FinalForce = 0.f;
	CalculateAttackForce(PushDirection, Hitter, FinalForce);
	Pushed(PushDirection * FinalForce);

	ShowHealthBar();
}

void AEnemy::SetInterest(AActor* Target)
{
	CombatTarget = Target;

	GetCharacterMovement()->bOrientRotationToMovement = false;
	GetCharacterMovement()->bUseControllerDesiredRotation = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 600.0f, 0.0f);

	if (EnemyController)
	{
		EnemyController->SetFocus(Target, EAIFocusPriority::Gameplay);
	}
}

void AEnemy::LoseInterest()
{
	HideHealthBar();
	CombatTarget = nullptr;

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->bUseControllerDesiredRotation = false;

	if (EnemyController)
	{
		EnemyController->ClearFocus(EAIFocusPriority::Gameplay);
	}
}

void AEnemy::UpdateCombatWarpTarget(FName WarpTargetName)
{
	if (!MotionWarpingComponent) return;

	AActor* Target = nullptr;
	if (AAIController* AICon = Cast<AAIController>(GetController()))
	{
		if (UBlackboardComponent* BB = AICon->GetBlackboardComponent())
		{
			Target = Cast<AActor>(BB->GetValueAsObject(FName("TargetActor")));
		}
	}

	if (!Target) return;

	FVector SafeWarpLocation = GetClampedAttackDestination(Target);
	MotionWarpingComponent->AddOrUpdateWarpTargetFromLocation(
		WarpTargetName,
		SafeWarpLocation
	);
}

void AEnemy::GetParried(AActor* Parrier, const FVector& ImpactPoint)
{
	if (IsDead()) return;

	bCanBeParried = false;

	if (MotionWarpingComponent)
	{
		MotionWarpingComponent->RemoveAllWarpTargets();
	}

	if (ParryImpactFX)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), ParryImpactFX, GetActorLocation());
	}
	if (ParrySound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ParrySound, GetActorLocation());
	}

	if (ParryShakeClass)
	{
		UGameplayStatics::PlayWorldCameraShake(GetWorld(), ParryShakeClass, GetActorLocation(), 0.f, 2000.f);
	}

	TriggerGlobalHitStop(0.25f, 0.05f);
}

void AEnemy::Die()
{
	Super::Die();

	EnemyState = EEnemyState::EES_Dead;
	Tags.Remove(FName("Enemy"));

	if (AActor* Target = Cast<AActor>(EnemyController->GetBlackboardComponent()->GetValueAsObject(FName("TargetActor"))))
	{
		if (UCombatManagerComponent* Manager = Target->FindComponentByClass<UCombatManagerComponent>())
		{
			Manager->ReleaseToken(this); // 맞거나 죽으면 무조건 토큰 반납
		}
	}

	// <SWARM AI> 동료 사망 시 남은 적들의 호전성 증가 (Enrage)
	TArray<AActor*> FoundEnemies;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AEnemy::StaticClass(), FoundEnemies);
	for (AActor* Actor : FoundEnemies)
	{
		AEnemy* Ally = Cast<AEnemy>(Actor);
		if (Ally && Ally != this && !Ally->IsDead())
		{
			if (FVector::Dist(GetActorLocation(), Ally->GetActorLocation()) < 2500.f)
			{
				Ally->OnAllyDied();
			}
		}
	}
	// </SWARM AI>

	if (AAIController* AICon = Cast<AAIController>(GetController()))
	{
		if (UBrainComponent* BrainComp = AICon->GetBrainComponent())
		{
			BrainComp->StopLogic(TEXT("Enemy Died"));
		}
		AICon->StopMovement();
	}

	LoseInterest();

	HideHealthBar();
	DisableCapsule();
	DisableMeshCollision();
	SetLifeSpan(DeathLifeSpan);
	GetCharacterMovement()->bOrientRotationToMovement = false;
	GetCharacterMovement()->StopMovementImmediately();

	SetWeaponCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AEnemy::InitializeAttributes()
{
	Super::InitializeAttributes();

	if (!AbilitySystemComponent || !AttributeSet) return;

	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AttributeSet->GetHealthAttribute())
		.AddLambda([this](const FOnAttributeChangeData& Data)
			{
				if (HealthBarWidget && AttributeSet)
				{
					const float MaxHealth = AttributeSet->GetMaxHealth();
					if (MaxHealth > 0.0f)
					{
						HealthBarWidget->SetHealthPercent(Data.NewValue / MaxHealth);
						ShowHealthBar();

						if (Data.NewValue <= 0.0f && !IsDead())
						{
							Die();
						}
					}
				}
			});

	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AttributeSet->GetResonanceValueAttribute())
		.AddLambda([this](const FOnAttributeChangeData& Data)
			{
				if (HealthBarWidget && AttributeSet)
				{
					const float MaxResonance = AttributeSet->GetMaxResonanceValue();
					if (MaxResonance > 0.0f)
					{
						HealthBarWidget->SetResonancePercent(Data.NewValue / MaxResonance);
						
						UE_LOG(LogTemp, Warning, TEXT("ResonanceValue: %f / %f"), Data.NewValue, MaxResonance);

						if (Data.NewValue <= 0.0f && !HasMatchingGameplayTag(WuwaGameplayTags::State_Groggy) && IsAlive())
						{
							TriggerGroggy();
						}
					}
				}
			});

	if (HealthBarWidget && AttributeSet)
	{
		const float MaxHealth = AttributeSet->GetMaxHealth();
		if (MaxHealth > 0.0f)
		{
			HealthBarWidget->SetHealthPercent(AttributeSet->GetHealth() / MaxHealth);
			const float MaxResonance = AttributeSet->GetMaxResonanceValue();
			if (MaxResonance > 0.0f) { HealthBarWidget->SetResonancePercent(AttributeSet->GetResonanceValue() / MaxResonance); }
		}
	}
}

void AEnemy::Pushed(const FVector& LaunchVelocity)
{
	if (UCharacterMovementComponent* MonsterMovement = GetCharacterMovement())
	{
		MonsterMovement->Launch(LaunchVelocity);
	}
}

void AEnemy::HideHealthBar()
{
	if (HealthBarWidget)
	{
		HealthBarWidget->SetVisibility(false);
	}
}

void AEnemy::ShowHealthBar()
{
	if (HealthBarWidget)
	{
		HealthBarWidget->SetVisibility(true);
	}
}

FVector AEnemy::GetClampedAttackDestination(AActor* Target) const
{
	if (!Target) return GetActorLocation();

	FVector MyLocation = GetActorLocation();
	FVector TargetLocation = Target->GetActorLocation();

	FVector ToTarget = (TargetLocation - MyLocation);
	ToTarget.Z = 0.f;

	float ActualDistance = ToTarget.Size();
	FVector DirToTarget = ToTarget.GetSafeNormal();

	const float StopOffsetDistance = 150.0f;
	float DesiredMoveDistance = FMath::Max(0.0f, ActualDistance - StopOffsetDistance);
	float FinalMoveDistance = FMath::Min(DesiredMoveDistance, MaxWarpDashDistance);

	FVector FinalDestination = MyLocation + (DirToTarget * FinalMoveDistance);
	FinalDestination.Z = TargetLocation.Z;

	return FinalDestination;
}

void AEnemy::TriggerGlobalHitStop(float RealTimeDuration, float TimeDilation)
{
	TriggerGlobalDilation(RealTimeDuration, TimeDilation);
}

void AEnemy::ResetGlobalHitStop()
{
	UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 1.0f);
}

void AEnemy::InitilizeEnemy()
{
	EnemyController = Cast<AAIController>(GetController());
	HideHealthBar();
}

void AEnemy::CalculateAttackForce(FVector& PushDirection, AActor* Hitter, float& FinalForce)
{
	FRotator LookAtHitterRot = (-PushDirection).Rotation();
	SetActorRotation(FRotator(0.0f, LookAtHitterRot.Yaw, 0.0f));

	float BaseForce = 1200.f;
	float MinForce = 300.f;
	float MaxDistance = 450.f;

	float Distance = Hitter ? FVector::Distance(GetActorLocation(), Hitter->GetActorLocation()) : 0.f;
	float DistanceAlpha = FMath::Clamp(1.0f - (Distance / MaxDistance), 0.0f, 1.0f);
	FinalForce = FMath::Lerp(MinForce, BaseForce, DistanceAlpha);
}

void AEnemy::OnAllyDied()
{
	if (IsDead()) return;
	
	// Enrage: Boost aggression when allies die
	BaseAggression += 0.3f;
	if (BaseAggression > 1.5f) BaseAggression = 1.5f;
	
	CurrentAggression += 0.5f;
	if (CurrentAggression > BaseAggression) CurrentAggression = BaseAggression;
	
	if (EnemyController && EnemyController->GetBlackboardComponent())
	{
		EnemyController->GetBlackboardComponent()->SetValueAsFloat(TEXT("AggressionLevel"), CurrentAggression);
	}
}

void AEnemy::TriggerGroggy()
{
	if (!IsAlive() || HasMatchingGameplayTag(WuwaGameplayTags::State_Groggy)) return;

	// 태그 추가
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->AddLooseGameplayTag(WuwaGameplayTags::State_Groggy);
	}

	// 블랙보드 업데이트 (BT 정지)
	if (AAIController* AICon = Cast<AAIController>(GetController()))
	{
		AICon->StopMovement();
		if (UBlackboardComponent* BB = AICon->GetBlackboardComponent())
		{
			BB->SetValueAsBool(TEXT("Groggy"), true);
		}
	}

	// 타겟팅 해제 및 토큰 반납 (Swarm AI 대응)
	if (EnemyController)
	{
		if (UBlackboardComponent* BB = EnemyController->GetBlackboardComponent())
		{
			if (AActor* Target = Cast<AActor>(BB->GetValueAsObject(FName("TargetActor"))))
			{
				if (UCombatManagerComponent* Manager = Target->FindComponentByClass<UCombatManagerComponent>())
				{
					Manager->ReleaseToken(this);
				}
			}
		}
	}

	// 몽타주 재생
	if (GroggyMontage)
	{
		UE_LOG(LogTemp, Warning, TEXT("Playing Groggy Montage!"));
		
		// 혹시 재생 중인 몽타주가 있다면 강제로 끊습니다.
		if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
		{
			AnimInstance->Montage_Stop(0.1f);
		}

		PlayAnimMontage(GroggyMontage);
	}

	// 20초 후 자동 회복
	GetWorldTimerManager().SetTimer(GroggyTimerHandle, this, &AEnemy::RecoverFromGroggy, 6.f, false);
}

void AEnemy::RecoverFromGroggy()
{
	if (!IsAlive()) return;

	// 타이머 취소
	GetWorldTimerManager().ClearTimer(GroggyTimerHandle);

	// 태그 제거
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->RemoveLooseGameplayTag(WuwaGameplayTags::State_Groggy);
	}

	// 공진 수치 원상 복구
	if (AttributeSet)
	{
		float MaxResonance = AttributeSet->GetMaxResonanceValue();
		AttributeSet->SetResonanceValue(MaxResonance);
		
		if (HealthBarWidget && MaxResonance > 0.0f)
		{
			HealthBarWidget->SetResonancePercent(1.0f);
		}
	}

	// 몽타주 정지
	if (GroggyMontage)
	{
		StopAnimMontage(GroggyMontage);
	}

	// 블랙보드 업데이트 (BT 재개)
	if (AAIController* AICon = Cast<AAIController>(GetController()))
	{
		if (UBlackboardComponent* BB = AICon->GetBlackboardComponent())
		{
			BB->SetValueAsBool(TEXT("Groggy"), false);
		}
	}
}

