// Fill out your copyright notice in the Description page of Project Settings.

#include "Characters/BaseCharacter.h"

#include "Components/CapsuleComponent.h"
#include "Components/BoxComponent.h"
#include "MotionWarpingComponent.h"

#include "Perception/AIPerceptionStimuliSourceComponent.h"
#include "Perception/AISense_Sight.h"
#include "Perception/AISense_Hearing.h"

#include "AbilitySystemComponent.h"
#include "AttributeSet/ParagonAttributeSet.h"
#include "Data/PlayerAttackData.h"
#include "NiagaraFunctionLibrary.h"
#include "Items/Weapons/Weapon.h"
#include "WuwaGameplayTags.h"

#include "Kismet/GameplayStatics.h"
#include "Containers/Ticker.h"

ABaseCharacter::ABaseCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);

	AttributeSet = CreateDefaultSubobject<UParagonAttributeSet>(TEXT("AttributeSet"));

	MotionWarpingComponent = CreateDefaultSubobject<UMotionWarpingComponent>(TEXT("DynamicMotionWarpingComponent"));
	MotionWarpingComponent->Activate();

	StimuliSourceComponent = CreateDefaultSubobject<UAIPerceptionStimuliSourceComponent>(TEXT("StimuliSourceComponent"));
	if (StimuliSourceComponent)
	{
		StimuliSourceComponent->RegisterForSense(UAISense_Sight::StaticClass());
		StimuliSourceComponent->bAutoRegister = true;
		StimuliSourceComponent->RegisterWithPerceptionSystem();
	}

	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
}

void ABaseCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->InitAbilityActorInfo(this, this);
		InitializeAttributes();
		InitializeGameplayEffects();
	}
}

void ABaseCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

UAbilitySystemComponent* ABaseCharacter::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

bool ABaseCharacter::IsAlive() const
{
	return AttributeSet && AttributeSet->GetHealth() > 0.0f;
}

bool ABaseCharacter::HasMatchingGameplayTag(FGameplayTag TagToCheck) const
{
	return AbilitySystemComponent && AbilitySystemComponent->HasMatchingGameplayTag(TagToCheck);
}

void ABaseCharacter::InitializeAttributes()
{
	if (AbilitySystemComponent && DefaultAttributesEffectClass)
	{
		FGameplayEffectContextHandle ContextHandle = AbilitySystemComponent->MakeEffectContext();
		ContextHandle.AddSourceObject(this);

		FGameplayEffectSpecHandle SpecHandle = AbilitySystemComponent->MakeOutgoingSpec(DefaultAttributesEffectClass, 1.0f, ContextHandle);
		if (SpecHandle.IsValid())
		{
			AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
		}
	}
}

void ABaseCharacter::InitializeGameplayEffects()
{
}

bool ABaseCharacter::TryJustDodge(AActor* Attacker)
{
	if (HasMatchingGameplayTag(WuwaGameplayTags::State_DodgeWindow))
	{
		if (AbilitySystemComponent)
		{
			FGameplayEventData Payload;
			Payload.Instigator = Attacker;
			Payload.Target = this;

			AbilitySystemComponent->HandleGameplayEvent(WuwaGameplayTags::Event_JustDodge_Success, &Payload);
		}
		return true;
	}
	return false;
}

void ABaseCharacter::ApplyDamageToTarget(AActor* TargetActor, const UBaseAttackData* AttackData, const FVector& HitLocation)
{
	if (!TargetActor || !AttackData || !DamageEffectClass || !AbilitySystemComponent) return;

	if (ABaseCharacter* TargetBase = Cast<ABaseCharacter>(TargetActor))
	{
		if (TargetBase->TryJustDodge(this))
		{
			return;
		}
	}

	UAbilitySystemComponent* TargetASC = TargetActor->FindComponentByClass<UAbilitySystemComponent>();
	if (!TargetASC) return;

	FGameplayEffectContextHandle ContextHandle = AbilitySystemComponent->MakeEffectContext();
	ContextHandle.AddSourceObject(this);
	ContextHandle.AddHitResult(FHitResult(TargetActor, nullptr, HitLocation, FVector::UpVector));

	FGameplayEffectSpecHandle SpecHandle = AbilitySystemComponent->MakeOutgoingSpec(DamageEffectClass, 1.0f, ContextHandle);
	if (!SpecHandle.IsValid()) return;

	SpecHandle.Data->SetSetByCallerMagnitude(WuwaGameplayTags::Data_Damage_MotionValue, AttackData->MotionValue);

	float ResonanceReduction = 10.0f;
	if (const UPlayerAttackData* PlayerData = Cast<UPlayerAttackData>(AttackData))
	{
		ResonanceReduction = PlayerData->ResonanceReduction;
	}
	SpecHandle.Data->SetSetByCallerMagnitude(WuwaGameplayTags::Data_Damage_ResonanceValue, ResonanceReduction);

	AbilitySystemComponent->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), TargetASC);

	// 플레이어인 경우(공격 데이터가 PlayerAttackData인 경우), 자신에게 자원 획득 이펙트 적용
	if (const UPlayerAttackData* PlayerData = Cast<UPlayerAttackData>(AttackData))
	{
		if (ResourceGainEffectClass)
		{
			FGameplayEffectSpecHandle ResourceSpecHandle = AbilitySystemComponent->MakeOutgoingSpec(ResourceGainEffectClass, 1.0f, ContextHandle);
			if (ResourceSpecHandle.IsValid())
			{
				for (const auto& Pair : PlayerData->ResourceGains)
				{
					ResourceSpecHandle.Data->SetSetByCallerMagnitude(Pair.Key, Pair.Value);
				}
				AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*ResourceSpecHandle.Data.Get());
			}
		}
	}

	if (AttackData->HitImpactFX)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), AttackData->HitImpactFX, HitLocation);
	}
	if (AttackData->HitImpactSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, AttackData->HitImpactSound, HitLocation);
	}
	if (AttackData->HitCameraShake)
	{
		if (APlayerController* PC = Cast<APlayerController>(GetController()))
		{
			PC->ClientStartCameraShake(AttackData->HitCameraShake);
		}
	}
}

void ABaseCharacter::GetHit_Implementation(const FVector& ImpactPoint, AActor* Hitter)
{
	if (HasMatchingGameplayTag(WuwaGameplayTags::State_Invulnerable) || HasMatchingGameplayTag(WuwaGameplayTags::State_HyperArmor))
	{
		return;
	}

	if (IsAlive())
	{
		if (!HasMatchingGameplayTag(WuwaGameplayTags::State_SuperArmor) && !HasMatchingGameplayTag(WuwaGameplayTags::State_Groggy))
		{
			if (MotionWarpingComponent)
			{
				MotionWarpingComponent->RemoveAllWarpTargets();
			}
			DirectionalHitReact(Hitter ? Hitter->GetActorLocation() : ImpactPoint);
		}
	}
	else
	{
		Die();
	}

	PlayHitSound(ImpactPoint);
	SpawnHitParticles(ImpactPoint);
}

void ABaseCharacter::DirectionalHitReact(const FVector& ImpactPoint)
{
	const FVector Forward = GetActorForwardVector();
	const FVector ToHit = (ImpactPoint - GetActorLocation()).GetSafeNormal2D();

	const double CosTheta = FVector::DotProduct(Forward, ToHit);
	double Theta = FMath::RadiansToDegrees(FMath::Acos(CosTheta));

	const FVector CrossProduct = FVector::CrossProduct(Forward, ToHit);
	if (CrossProduct.Z < 0)
	{
		Theta *= -1.f;
	}

	FName SectionName = FName("FromFront");
	if (Theta >= -90.f && Theta < 90.f)
	{
		SectionName = FName("FromFront");
	}
	else
	{
		SectionName = FName("FromBack");
	}

	PlayHitReactMontage(SectionName);
}

void ABaseCharacter::PlayHitReactMontage(const FName& SectionName)
{
	if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
	{
		AnimInstance->Montage_Play(HitReactMontage);
		AnimInstance->Montage_JumpToSection(SectionName, HitReactMontage);
	}
}

bool ABaseCharacter::IsHitReacting() const
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	return AnimInstance && AnimInstance->Montage_IsPlaying(HitReactMontage);
}

void ABaseCharacter::Die()
{
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->AddLooseGameplayTag(WuwaGameplayTags::State_Dead);
		AbilitySystemComponent->CancelAllAbilities();
	}

	Tags.Add(FName("Dead"));
	PlayDeathMontage();
	DisableMeshCollision();
}

int32 ABaseCharacter::PlayDeathMontage()
{
	return PlayRandomMontageSection(DeathMontage, DeathMontageSections);
}

void ABaseCharacter::PlayHitSound(const FVector& ImpactPoint)
{
	if (HitSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, HitSound, ImpactPoint);
	}
}

void ABaseCharacter::SpawnHitParticles(const FVector& ImpactPoint)
{
	if (HitParticle)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), HitParticle, ImpactPoint);
	}
}

void ABaseCharacter::Attack()
{
	if (CombatTarget && CombatTarget->ActorHasTag(FName("Dead")))
	{
		CombatTarget = nullptr;
	}
}

bool ABaseCharacter::CanAttack()
{
	return true;
}

void ABaseCharacter::AttackEnd()
{
}

void ABaseCharacter::DodgeEnd()
{
}

void ABaseCharacter::HandleDamage(float DamageAmount)
{
}

int32 ABaseCharacter::PlayAttackMontage()
{
	return PlayRandomMontageSection(AttackMontage, AttackMontageSections);
}

int32 ABaseCharacter::PlayAttackMontageSection(const FName& SectionName)
{
	PlayMontageSection(AttackMontage, SectionName);
	return 0;
}

void ABaseCharacter::StopAttackMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && AttackMontage)
	{
		AnimInstance->Montage_Stop(0.25f, AttackMontage);
	}
}

void ABaseCharacter::StopDodgeMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && DodgeMontage)
	{
		AnimInstance->Montage_Stop(0.3f, DodgeMontage);
	}
}

FName ABaseCharacter::CalculateDodgeSectionAndWarp(const FVector& DodgeDirection, const bool IsLockedOn)
{
	const float DodgeDistance = 350.f;
	const bool bHasInput = !DodgeDirection.IsNearlyZero();
	const FVector MoveDir = bHasInput ? DodgeDirection : -GetActorForwardVector();

	const FVector ActorLoc = GetActorLocation();
	const FVector LocalDir = GetActorTransform().InverseTransformVectorNoScale(MoveDir);
	const float LocalAngle = FMath::RadiansToDegrees(FMath::Atan2(LocalDir.Y, LocalDir.X));

	FName TargetSection = FName("Back");
	FVector TargetLocation = ActorLoc + (MoveDir * DodgeDistance);
	FRotator TargetRotation = GetActorRotation();

	if (!IsLockedOn && bHasInput)
	{
		TargetSection = FName("Front");
		TargetRotation = MoveDir.ToOrientationRotator();
		SetActorRotation(TargetRotation);
	}
	else
	{
		if (LocalAngle >= -45.0f && LocalAngle <= 45.0f)
		{
			TargetSection = FName("Front");
		}
		else if (LocalAngle > 45.0f && LocalAngle <= 135.0f)
		{
			TargetSection = FName("Right");
		}
		else if (LocalAngle < -45.0f && LocalAngle >= -135.0f)
		{
			TargetSection = FName("Left");
		}
		else
		{
			TargetSection = FName("Back");
		}
	}

	if (MotionWarpingComponent)
	{
		// 록온 중이고 앞으로 향할 때 적을 통과하지 않도록 거리 클램프
		if (IsLockedOn && CombatTarget && TargetSection == FName("Front"))
		{
			FVector ToTarget = CombatTarget->GetActorLocation() - ActorLoc;
			ToTarget.Z = 0.f;
			float DistToTarget = ToTarget.Size();
			float MinStandDistance = 120.f;
			
			if (DistToTarget < DodgeDistance + MinStandDistance)
			{
				float ClampedDist = FMath::Max(0.f, DistToTarget - MinStandDistance);
				TargetLocation = ActorLoc + (MoveDir * ClampedDist);
			}
		}

		MotionWarpingComponent->AddOrUpdateWarpTargetFromLocationAndRotation(
			FName("DodgeTarget"),
			TargetLocation,
			TargetRotation
		);
	}

	return TargetSection;
}

FName ABaseCharacter::CalculateJustDodgeSectionAndWarp(const FVector& DodgeDirection, const bool IsLockedOn)
{
	const float DodgeDistance = 175.f;
	const bool bHasInput = !DodgeDirection.IsNearlyZero();
	const FVector MoveDir = bHasInput ? DodgeDirection : -GetActorForwardVector();

	const FVector ActorLoc = GetActorLocation();
	const FVector LocalDir = GetActorTransform().InverseTransformVectorNoScale(MoveDir);
	const float LocalAngle = FMath::RadiansToDegrees(FMath::Atan2(LocalDir.Y, LocalDir.X));

	FName TargetSection = FName("Back");
	FVector TargetLocation = ActorLoc + (MoveDir * DodgeDistance);
	FRotator TargetRotation = GetActorRotation();

	if (!IsLockedOn && bHasInput)
	{
		TargetSection = FName("Front");
		TargetRotation = MoveDir.ToOrientationRotator();
		SetActorRotation(TargetRotation);
	}
	else
	{
		if (LocalAngle >= -45.0f && LocalAngle <= 45.0f)
		{
			TargetSection = FName("Front");
		}
		else if (LocalAngle > 45.0f && LocalAngle <= 135.0f)
		{
			TargetSection = FName("Right");
		}
		else if (LocalAngle < -45.0f && LocalAngle >= -135.0f)
		{
			TargetSection = FName("Left");
		}
		else
		{
			TargetSection = FName("Back");
		}
	}

	if (MotionWarpingComponent)
	{
		// 록온 중이고 앞으로 향할 때 적을 통과하지 않도록 거리 클램프
		if (IsLockedOn && CombatTarget && TargetSection == FName("Front"))
		{
			FVector ToTarget = CombatTarget->GetActorLocation() - ActorLoc;
			ToTarget.Z = 0.f;
			float DistToTarget = ToTarget.Size();
			float MinStandDistance = 120.f;
			
			if (DistToTarget < DodgeDistance + MinStandDistance)
			{
				float ClampedDist = FMath::Max(0.f, DistToTarget - MinStandDistance);
				TargetLocation = ActorLoc + (MoveDir * ClampedDist);
			}
		}

		MotionWarpingComponent->AddOrUpdateWarpTargetFromLocationAndRotation(
			FName("DodgeTarget"),
			TargetLocation,
			TargetRotation
		);
	}

	return TargetSection;
}

FVector ABaseCharacter::GetTranslationWarpTarget()
{
	if (CombatTarget == nullptr) return FVector();

	const FVector CombatTargetLocation = CombatTarget->GetActorLocation();
	const FVector Location = GetActorLocation();

	FVector TargetToMe = (Location - CombatTargetLocation).GetSafeNormal();
	TargetToMe *= WarpTargetDistance;

	return CombatTargetLocation + TargetToMe;
}

FVector ABaseCharacter::GetRotationWarpTarget()
{
	if (CombatTarget)
	{
		return CombatTarget->GetActorLocation();
	}
	return FVector();
}

void ABaseCharacter::PlayMontageSection(UAnimMontage* Montage, const FName& SectionName)
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && Montage)
	{
		AnimInstance->Montage_Play(Montage);
		AnimInstance->Montage_JumpToSection(SectionName, Montage);
	}
}

int32 ABaseCharacter::PlayRandomMontageSection(UAnimMontage* Montage, const TArray<FName>& SectionNames)
{
	if (SectionNames.Num() <= 0) return -1;

	const int32 Selection = FMath::RandRange(0, SectionNames.Num() - 1);
	PlayMontageSection(Montage, SectionNames[Selection]);
	return Selection;
}

void ABaseCharacter::ClearDamagedActors()
{
	DamagedActors.Empty();
}

void ABaseCharacter::AddDamagedActor(AActor* Target)
{
	if (Target)
	{
		DamagedActors.AddUnique(Target);
	}
}

bool ABaseCharacter::HasAlreadyBeenDamaged(AActor* Target) const
{
	if (!Target) return true;
	return DamagedActors.Contains(Target);
}

void ABaseCharacter::SetWeaponCollisionEnabled(ECollisionEnabled::Type CollisionEnabled)
{
	if (EquippedWeapon && EquippedWeapon->GetWeaponBox())
	{
		EquippedWeapon->GetWeaponBox()->SetCollisionEnabled(CollisionEnabled);
		EquippedWeapon->IgnoreActors.Empty();
	}
}

void ABaseCharacter::EnableMeshCollision()
{
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GetMesh()->SetCollisionObjectType(ECC_WorldDynamic);
	GetMesh()->SetCollisionResponseToAllChannels(ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	GetMesh()->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);
	GetMesh()->SetGenerateOverlapEvents(true);
}

void ABaseCharacter::DisableMeshCollision()
{
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ABaseCharacter::DisableCapsule()
{
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ABaseCharacter::TriggerGlobalDilation(float RealTimeDuration, float TimeDilation)
{
	UWorld* World = GetWorld();
	if (!World) return;

	UGameplayStatics::SetGlobalTimeDilation(World, TimeDilation);

	FTSTicker::GetCoreTicker().AddTicker(
		FTickerDelegate::CreateWeakLambda(this, [World](float DeltaTime)
			{
				if (World)
				{
					UGameplayStatics::SetGlobalTimeDilation(World, 1.0f);
				}
				return false;
			}),
		RealTimeDuration
	);
}

void ABaseCharacter::TriggerLocalDilation(float RealTimeDuration, float TimeDilation)
{
	CustomTimeDilation = TimeDilation;

	FTSTicker::GetCoreTicker().AddTicker(
		FTickerDelegate::CreateWeakLambda(this, [this](float DeltaTime)
			{
				this->CustomTimeDilation = 1.0f;
				return false;
			}),
		RealTimeDuration
	);
}