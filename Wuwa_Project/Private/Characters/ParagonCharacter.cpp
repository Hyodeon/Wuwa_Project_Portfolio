// Fill out your copyright notice in the Description page of Project Settings.

#include "Characters/ParagonCharacter.h"

#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Engine/OverlapResult.h"

#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"

#include "Items/Weapons/Weapon.h"
#include "Enemy/Enemy.h"
#include "HUD/ParagonHUD.h"
#include "HUD/ParagonOverlay.h"
#include "AttributeSet/ParagonAttributeSet.h"
#include "Components/CombatCameraComponent.h"
#include "Components/CombatManagerComponent.h"

#include "AbilitySystemComponent.h"
#include "WuwaGameplayTags.h"

AParagonCharacter::AParagonCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	AutoPossessPlayer = EAutoReceiveInput::Player0;

	bUseControllerRotationYaw = false;
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 720.f, 0.f);

	EnableMeshCollision();

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(GetRootComponent());
	SpringArm->TargetArmLength = BaseSpringArmLength;
	SpringArm->bUsePawnControlRotation = true;

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm);
	Camera->bUsePawnControlRotation = false;

	CombatCameraComp = CreateDefaultSubobject<UCombatCameraComponent>(TEXT("CombatCameraComponent"));
	CombatManager = CreateDefaultSubobject<UCombatManagerComponent>(TEXT("CombatManager"));
}

void AParagonCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(ParagonMappingContext, 0);
		}

		if (CombatCameraComp)
		{
			CombatCameraComp->InitializeCamera(SpringArm, Camera, PlayerController);
		}
	}

	Tags.Add(FName("EngageableTarget"));

	if (DefaultWeaponClass)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.Instigator = this;

		if (AWeapon* DefaultWeapon = GetWorld()->SpawnActor<AWeapon>(DefaultWeaponClass, SpawnParams))
		{
			EquipWeapon(DefaultWeapon);
		}
	}

	InitializeParagonOverlay();
	EnableMeshCollision();
}

void AParagonCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsLockedOn && LockTarget)
	{
		if (LockTarget->ActorHasTag(FName("Dead")))
		{
			ClearLockOn();
			return;
		}

		const FVector PlayerLoc = GetActorLocation();
		const FVector TargetLoc = LockTarget->GetActorLocation();
		const FRotator LookAtRot = UKismetMathLibrary::FindLookAtRotation(PlayerLoc, TargetLoc);
		const FRotator TargetActorRot = FRotator(0.f, LookAtRot.Yaw, 0.f);

		SetActorRotation(FMath::RInterpTo(GetActorRotation(), TargetActorRot, DeltaTime, 10.f));

		if (AEnemy* EnemyTarget = Cast<AEnemy>(LockTarget))
		{
			if (ParagonOverlay && EnemyTarget->GetMesh())
			{
				const FVector TargetPos = EnemyTarget->GetMesh()->GetSocketLocation(FName("BodyCenter"));
				ParagonOverlay->UpdateLockOnMarkerPosition(TargetPos);
			}
		}
	}

	if (bIsLockedOn && !LockTarget)
	{
		ClearLockOn();
	}
}

void AParagonCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AParagonCharacter::Move);
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AParagonCharacter::Look);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &AParagonCharacter::Jump);
		EnhancedInputComponent->BindAction(EquipAction, ETriggerEvent::Triggered, this, &AParagonCharacter::FKeyPressed);
		EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Started, this, &AParagonCharacter::Attack);
		EnhancedInputComponent->BindAction(DodgeAction, ETriggerEvent::Started, this, &AParagonCharacter::Dodge);
		EnhancedInputComponent->BindAction(LockAction, ETriggerEvent::Started, this, &AParagonCharacter::ToggleLockOn);
		EnhancedInputComponent->BindAction(ESkillAction, ETriggerEvent::Started, this, &AParagonCharacter::ESkillStarted);
		EnhancedInputComponent->BindAction(ESkillAction, ETriggerEvent::Completed, this, &AParagonCharacter::ESkillReleased);
		if (UltimateAction)
		{
			EnhancedInputComponent->BindAction(UltimateAction, ETriggerEvent::Started, this, &AParagonCharacter::UltimateStarted);
		}
	}
}

void AParagonCharacter::InitializeAttributes()
{
	Super::InitializeAttributes();

	if (!AbilitySystemComponent || !AttributeSet) return;

	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AttributeSet->GetHealthAttribute())
		.AddLambda([this](const FOnAttributeChangeData& Data)
			{
				if (ParagonOverlay && AttributeSet)
				{
					const float MaxHealth = AttributeSet->GetMaxHealth();
					if (MaxHealth > 0.0f)
					{
						ParagonOverlay->SetHealthBarPercent(Data.NewValue / MaxHealth);
					}
				}
			});

	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AttributeSet->GetStaminaAttribute())
		.AddLambda([this](const FOnAttributeChangeData& Data)
			{
				if (ParagonOverlay && AttributeSet)
				{
					const float MaxStamina = AttributeSet->GetMaxStamina();
					if (MaxStamina > 0.0f)
					{
						ParagonOverlay->SetStaminaBarPercent(Data.NewValue / MaxStamina);
					}
				}
			});

	if (ParagonOverlay)
	{
		const float MaxHealth = AttributeSet->GetMaxHealth();
		const float MaxStamina = AttributeSet->GetMaxStamina();
		if (MaxHealth > 0.f) ParagonOverlay->SetHealthBarPercent(AttributeSet->GetHealth() / MaxHealth);
		if (MaxStamina > 0.f) ParagonOverlay->SetStaminaBarPercent(AttributeSet->GetStamina() / MaxStamina);
	}
}

void AParagonCharacter::InitializeGameplayEffects()
{
	Super::InitializeGameplayEffects();

	if (!AbilitySystemComponent) return;

	if (StaminaRegenEffectClass)
	{
		FGameplayEffectContextHandle ContextHandle = AbilitySystemComponent->MakeEffectContext();
		ContextHandle.AddSourceObject(this);

		FGameplayEffectSpecHandle SpecHandle = AbilitySystemComponent->MakeOutgoingSpec(StaminaRegenEffectClass, 1.0f, ContextHandle);
		if (SpecHandle.IsValid())
		{
			AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
		}
	}

	auto GiveAbilityIfValid = [this](TSubclassOf<UGameplayAbility> AbilityClass, int32 InputID)
		{
			if (AbilityClass)
			{
				AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(AbilityClass, 1, InputID, this));
			}
		};

	GiveAbilityIfValid(AttackAbilityClass, INDEX_NONE);
	GiveAbilityIfValid(DodgeAbilityClass, INDEX_NONE);
	GiveAbilityIfValid(JustDodgeAbilityClass, INDEX_NONE);
	GiveAbilityIfValid(DodgeCounterAbilityClass, INDEX_NONE);
	GiveAbilityIfValid(ESkillAbilityClass, 0);
	GiveAbilityIfValid(UltimateAbilityClass, 1);
	GiveAbilityIfValid(JumpAbilityClass, INDEX_NONE);
}

void AParagonCharacter::GetHit_Implementation(const FVector& ImpactPoint, AActor* Hitter)
{
	if (HasMatchingGameplayTag(WuwaGameplayTags::State_Invulnerable))
	{
		return;
	}

	Super::GetHit_Implementation(ImpactPoint, Hitter);
	SetWeaponCollisionEnabled(ECollisionEnabled::NoCollision);
}

// ================= [입력 처리: GAS 트리거] =================

void AParagonCharacter::Move(const FInputActionValue& Value)
{
	if (HasMatchingGameplayTag(WuwaGameplayTags::State_Dead)) return;
	
	// 반격기 도중에는 움직일 수 없음
	if (HasMatchingGameplayTag(WuwaGameplayTags::State_JustDodge_Active)) return;

	const FVector2D MoveVector = Value.Get<FVector2D>();

	FVector ForwardDirection;
	FVector RightDirection;

	if (bIsLockedOn && LockTarget)
	{
		ForwardDirection = GetActorForwardVector();
		RightDirection = GetActorRightVector();
	}
	else
	{
		const FRotator Rotation = Controller ? Controller->GetControlRotation() : FRotator::ZeroRotator;
		const FRotator YawRotation(0.f, Rotation.Yaw, 0.f);

		ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
	}

	AddMovementInput(ForwardDirection, MoveVector.Y);
	AddMovementInput(RightDirection, MoveVector.X);
}

void AParagonCharacter::Look(const FInputActionValue& Value)
{
	if (bIsLockedOn) return;

	const FVector2D LookVector = Value.Get<FVector2D>();
	AddControllerYawInput(LookVector.X);
	AddControllerPitchInput(LookVector.Y);
}

void AParagonCharacter::Jump()
{
	if (HasMatchingGameplayTag(WuwaGameplayTags::State_Dead) || HasMatchingGameplayTag(WuwaGameplayTags::State_JustDodge_Active))
	{
		return;
	}

	if (JumpAbilityClass && AbilitySystemComponent)
	{
		AbilitySystemComponent->TryActivateAbilityByClass(JumpAbilityClass);
	}
	else
	{
		Super::Jump();
	}
}

void AParagonCharacter::Attack()
{
	Super::Attack();
	
	if (AbilitySystemComponent)
	{
		FGameplayEventData Payload;
		Payload.Instigator = this;
		Payload.Target = this;
		
		AbilitySystemComponent->HandleGameplayEvent(WuwaGameplayTags::Event_Combat_AttackInput, &Payload);
	}
}

void AParagonCharacter::Dodge()
{
	if (AbilitySystemComponent && DodgeAbilityClass)
	{
		AbilitySystemComponent->TryActivateAbilityByClass(DodgeAbilityClass);

		UE_LOG(LogTemp, Warning, TEXT("Dodge ability activated."));
	}
}

void AParagonCharacter::ESkillStarted()
{
	if (AbilitySystemComponent)
	{
		// 클래스 강제 호출(TryActivate)을 지우고, 입력 신호만 전달합니다.
		AbilitySystemComponent->AbilityLocalInputPressed(0);
	}
}

void AParagonCharacter::ESkillReleased()
{
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->AbilityLocalInputReleased(0);
	}
}

void AParagonCharacter::AttackEnd()
{
	if (AbilitySystemComponent)
	{
		FGameplayEventData Payload;
		Payload.Instigator = this;
		Payload.Target = this;
		AbilitySystemComponent->HandleGameplayEvent(WuwaGameplayTags::Event_Combat_AttackEnd, &Payload);
	}
}

void AParagonCharacter::DodgeEnd()
{
	if (AbilitySystemComponent)
	{
		FGameplayEventData Payload;
		Payload.Instigator = this;
		Payload.Target = this;
		AbilitySystemComponent->HandleGameplayEvent(WuwaGameplayTags::Event_Combat_DodgeEnd, &Payload);
	}
}

void AParagonCharacter::UltimateStarted()
{
	if (AbilitySystemComponent && UltimateAbilityClass)
	{
		AbilitySystemComponent->AbilityLocalInputPressed(1);
		AbilitySystemComponent->TryActivateAbilityByClass(UltimateAbilityClass);
	}
}

void AParagonCharacter::FKeyPressed()
{
	if (AWeapon* OverlappingWeapon = Cast<AWeapon>(OverlappingItem)) 
	{
		EquipWeapon(OverlappingWeapon);
	}
}

void AParagonCharacter::EquipWeapon(AWeapon* Weapon)
{
	if (!Weapon) return;
	Weapon->Equip(GetMesh(), FName("RightHandSocket"), this, this);
	OverlappingItem = nullptr;
	EquippedWeapon = Weapon;
}

// ================= [타겟팅 및 록온] =================

void AParagonCharacter::ToggleLockOn()
{
	if (bIsLockedOn)
	{
		ClearLockOn();
	}
	else
	{
		LockTarget = FindBestLockOnTarget();
		if (LockTarget)
		{
			bIsLockedOn = true;
			if (ParagonOverlay) ParagonOverlay->ShowLockOnMarker(true);
			GetCharacterMovement()->bOrientRotationToMovement = false;

			if (CombatCameraComp)
			{
				CombatCameraComp->SetLockOnTarget(LockTarget);
			}
		}
	}
}

void AParagonCharacter::ClearLockOn()
{
	if (!bIsLockedOn) return;

	bIsLockedOn = false;
	if (ParagonOverlay) ParagonOverlay->ShowLockOnMarker(false);
	LockTarget = nullptr;

	GetCharacterMovement()->bOrientRotationToMovement = true;

	if (CombatCameraComp)
	{
		CombatCameraComp->SetLockOnTarget(nullptr);
	}
}

AActor* AParagonCharacter::FindBestLockOnTarget()
{
	TArray<FOverlapResult> OverlapResults;
	const FVector StartLoc = GetActorLocation();

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);

	const bool bHit = GetWorld()->OverlapMultiByChannel(
		OverlapResults,
		StartLoc,
		FQuat::Identity,
		ECC_Pawn,
		FCollisionShape::MakeSphere(LockOnSearchRadius),
		QueryParams
	);

	AActor* BestTarget = nullptr;
	float MaxDotProduct = -1.f;

	const FVector CameraLoc = Camera->GetComponentLocation();
	const FVector CameraForward = Camera->GetForwardVector();

	for (const FOverlapResult& Result : OverlapResults)
	{
		AActor* OverlappedActor = Result.GetActor();
		if (!OverlappedActor) continue;

		if (OverlappedActor->ActorHasTag(FName("Enemy")) && !OverlappedActor->ActorHasTag(FName("Dead")))
		{
			const FVector TargetLoc = OverlappedActor->GetActorLocation();
			const FVector DirToTarget = (TargetLoc - CameraLoc).GetSafeNormal();
			const float Dot = FVector::DotProduct(CameraForward, DirToTarget);

			if (Dot > 0.5f && Dot > MaxDotProduct)
			{
				FHitResult LineTraceHit;
				GetWorld()->LineTraceSingleByChannel(
					LineTraceHit,
					CameraLoc,
					TargetLoc,
					ECC_Visibility,
					QueryParams
				);

				if (!LineTraceHit.bBlockingHit || LineTraceHit.GetActor() == OverlappedActor)
				{
					MaxDotProduct = Dot;
					BestTarget = OverlappedActor;
				}
			}
		}
	}

	return BestTarget;
}

AActor* AParagonCharacter::FindBestTargetInFront()
{
	TArray<AActor*> OverlappingActors;
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_WorldDynamic));

	TArray<AActor*> IgnoreActors;
	IgnoreActors.Add(this);

	UKismetSystemLibrary::SphereOverlapActors(
		GetWorld(),
		GetActorLocation(),
		AutoTargetRadius,
		ObjectTypes,
		AActor::StaticClass(),
		IgnoreActors,
		OverlappingActors
	);

	AActor* ClosestTarget = nullptr;
	float MinDistanceSq = FLT_MAX;
	const FVector ForwardVec = GetActorForwardVector();

	for (AActor* Actor : OverlappingActors)
	{
		if (!Actor || !Actor->ActorHasTag(FName("Enemy")) || Actor->ActorHasTag(FName("Dead"))) continue;

		const FVector ToTarget = (Actor->GetActorLocation() - GetActorLocation()).GetSafeNormal2D();
		const float DotProduct = FVector::DotProduct(ForwardVec, ToTarget);
		const float AngleInDegrees = FMath::RadiansToDegrees(FMath::Acos(DotProduct));

		if (AngleInDegrees <= AutoTargetAngle)
		{
			const float DistSq = FVector::DistSquared(GetActorLocation(), Actor->GetActorLocation());
			if (DistSq < MinDistanceSq)
			{
				MinDistanceSq = DistSq;
				ClosestTarget = Actor;
			}
		}
	}

	return ClosestTarget;
}

void AParagonCharacter::AdjustAttackRotation()
{
	if (bIsLockedOn && LockTarget)
	{
		return;
	}

	FVector InputWorldDirection = FVector::ZeroVector;
	GetInputMoveDirection(InputWorldDirection);

	if (!InputWorldDirection.IsNearlyZero())
	{
		FRotator TargetRot = InputWorldDirection.Rotation();
		TargetRot.Pitch = 0.f;
		TargetRot.Roll = 0.f;
		SetActorRotation(TargetRot);
	}
	else
	{
		if (AActor* BestTarget = FindBestTargetInFront())
		{
			const FVector DirToTarget = (BestTarget->GetActorLocation() - GetActorLocation()).GetSafeNormal2D();
			const FRotator TargetRot = DirToTarget.Rotation();
			SetActorRotation(FRotator(0.f, TargetRot.Yaw, 0.f));
		}
	}
}

void AParagonCharacter::GetInputMoveDirection(FVector& OutWorldDirection)
{
	FVector2D MoveInput = FVector2D::ZeroVector;

	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedPlayerInput* EnhancedPlayerInput = Cast<UEnhancedPlayerInput>(PC->PlayerInput))
		{
			if (MoveAction)
			{
				MoveInput = EnhancedPlayerInput->GetActionValue(MoveAction).Get<FVector2D>();
			}
		}
	}

	if (!MoveInput.IsNearlyZero())
	{
		FVector ForwardVector;
		FVector RightVector;

		if (bIsLockedOn && LockTarget)
		{
			ForwardVector = GetActorForwardVector();
			RightVector = GetActorRightVector();
		}
		else if (Controller)
		{
			const FRotator ControlRotation = Controller->GetControlRotation();
			const FRotator YawRotation(0.f, ControlRotation.Yaw, 0.f);

			ForwardVector = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
			RightVector = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		}
		else
		{
			ForwardVector = GetActorForwardVector();
			RightVector = GetActorRightVector();
		}

		OutWorldDirection = (ForwardVector * MoveInput.Y + RightVector * MoveInput.X).GetSafeNormal();
	}
	else
	{
		OutWorldDirection = FVector::ZeroVector;
	}
}

bool AParagonCharacter::CalculateCounterAttackWarpTargets(AActor* TargetActor, FVector& OutTargetLoc, FRotator& OutTargetRot)
{
	if (!TargetActor) return false;

	const FVector PlayerLoc = GetActorLocation();
	const FVector TargetLoc = TargetActor->GetActorLocation();

	FVector DirToTarget = (TargetLoc - PlayerLoc);
	DirToTarget.Z = 0.f;
	const float Distance2D = DirToTarget.Size();
	DirToTarget.Normalize();

	OutTargetRot = DirToTarget.Rotation();

	if (Distance2D > DodgeAttackTargetOffset)
	{
		OutTargetLoc = TargetLoc - (DirToTarget * DodgeAttackTargetOffset);
	}
	else
	{
		OutTargetLoc = PlayerLoc;
	}

	OutTargetLoc.Z = TargetLoc.Z;
	return true;
}

void AParagonCharacter::InitializeParagonOverlay()
{
	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if (AParagonHUD* ParagonHUD = Cast<AParagonHUD>(PlayerController->GetHUD()))
		{
			ParagonOverlay = ParagonHUD->GetParagonOverlay();
			if (ParagonOverlay)
			{
				ParagonOverlay->SetHealthBarPercent(1.f);
				ParagonOverlay->SetStaminaBarPercent(1.f);
				ParagonOverlay->SetGold(0);
				ParagonOverlay->SetSouls(0);
			}
		}
	}
}