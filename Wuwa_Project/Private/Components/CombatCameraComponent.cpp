#include "Components/CombatCameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Camera/CameraActor.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/KismetMathLibrary.h"

UCombatCameraComponent::UCombatCameraComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UCombatCameraComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UCombatCameraComponent::InitializeCamera(USpringArmComponent* InSpringArm, UCameraComponent* InCamera, APlayerController* InPC)
{
	SpringArmComp = InSpringArm;
	CameraComp = InCamera;
	PlayerController = InPC;

	if (SpringArmComp)
	{
		BaseSpringArmLength = SpringArmComp->TargetArmLength;

		SpringArmComp->bUsePawnControlRotation = true;
		SpringArmComp->bInheritPitch = true;
		SpringArmComp->bInheritYaw = true;
		SpringArmComp->bInheritRoll = false;

		SpringArmComp->bEnableCameraLag = true;
		SpringArmComp->CameraLagSpeed = DefaultCameraLocationLagSpeed;
		SpringArmComp->bEnableCameraRotationLag = false;
	}
}

void UCombatCameraComponent::SetLockOnTarget(AActor* NewTarget)
{
	CurrentLockTarget = NewTarget;

	if (CurrentLockTarget && PlayerController && SpringArmComp)
	{
		SpringArmComp->bUsePawnControlRotation = true;
		SpringArmComp->SetRelativeRotation(FRotator::ZeroRotator);
		SpringArmComp->TargetOffset = FVector::ZeroVector;
		SpringArmComp->SocketOffset.Y = 0.0f;

		if (CameraComp)
		{
			CameraComp->SetRelativeRotation(FRotator::ZeroRotator);
		}
	}
	else
	{
		if (SpringArmComp)
		{
			SpringArmComp->TargetArmLength = BaseSpringArmLength;
			SpringArmComp->SocketOffset = FVector::ZeroVector;
			SpringArmComp->TargetOffset = FVector::ZeroVector;
			SpringArmComp->SetRelativeRotation(FRotator::ZeroRotator);
		}
		if (CameraComp)
		{
			CameraComp->SetRelativeRotation(FRotator::ZeroRotator);
		}
	}
}

void UCombatCameraComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!PlayerController || !SpringArmComp || !CurrentLockTarget || !IsValid(CurrentLockTarget))
	{
		return;
	}

	AActor* OwnerActor = GetOwner();
	if (!OwnerActor) return;

	const FVector PlayerLoc = OwnerActor->GetActorLocation();
	const FVector TargetLoc = CurrentLockTarget->GetActorLocation();

	// 1. [타겟 조준 기본 각도 산출]
	FRotator LookAtRot = UKismetMathLibrary::FindLookAtRotation(PlayerLoc, TargetLoc);
	float IdealYaw = LookAtRot.Yaw;

	// 2. [거리 비례 Dynamic Pitch 연산]
	float Dist2D = FVector::Dist2D(PlayerLoc, TargetLoc);
	float ClampedDist = FMath::Clamp(Dist2D, 150.0f, 1000.0f);
	float DistAlpha = (ClampedDist - 150.0f) / 850.0f; // 0.0(초근접) ~ 1.0(원거리)

	// 초근접 시 -27도 하향 앵글 유지
	float TargetPitch = FMath::Lerp(-27.0f, -12.0f, DistAlpha);

	// 3. [컨트롤러 부드러운 회전 보간]
	FRotator DesiredControlRot = FRotator(TargetPitch, IdealYaw, 0.0f);
	FRotator CurrentControlRot = PlayerController->GetControlRotation();
	FRotator NewControlRot = FMath::RInterpTo(CurrentControlRot, DesiredControlRot, DeltaTime, LockOnRotationInterpSpeed);
	PlayerController->SetControlRotation(NewControlRot);

	// 4. [근접 시 카메라 높이(Z) 및 거리 보정]
	float TargetArmZOffset = FMath::Lerp(50.0f, 20.0f, DistAlpha);
	SpringArmComp->SocketOffset.Z = FMath::FInterpTo(SpringArmComp->SocketOffset.Z, TargetArmZOffset, DeltaTime, 5.0f);
	SpringArmComp->SocketOffset.Y = 0.0f; // 수평 오프셋 고정

	float TargetArmLength = FMath::Lerp(400.0f, 650.0f, DistAlpha);
	SpringArmComp->TargetArmLength = FMath::FInterpTo(SpringArmComp->TargetArmLength, TargetArmLength, DeltaTime, 4.0f);
}

void UCombatCameraComponent::PlayActionCameraMove(const FTransform& TargetTransform, float Duration, float ReturnBlendTime)
{
	if (!PlayerController || !GetWorld()) return;

	GetWorld()->GetTimerManager().ClearTimer(ActionCamDurationTimer);
	GetWorld()->GetTimerManager().ClearTimer(ActionCamCleanupTimer);

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = GetOwner();
	SpawnParams.Instigator = Cast<APawn>(GetOwner());
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	if (!TempActionCamera || !IsValid(TempActionCamera))
	{
		TempActionCamera = GetWorld()->SpawnActor<ACameraActor>(ACameraActor::StaticClass(), TargetTransform, SpawnParams);
	}
	else
	{
		TempActionCamera->SetActorTransform(TargetTransform);
	}

	if (!TempActionCamera) return;

	PlayerController->SetViewTargetWithBlend(TempActionCamera, 0.0f);

	FTimerDelegate ReturnDelegate;
	ReturnDelegate.BindUObject(this, &UCombatCameraComponent::OnActionCameraDurationEnd, ReturnBlendTime);
	GetWorld()->GetTimerManager().SetTimer(ActionCamDurationTimer, ReturnDelegate, Duration, false);
}

void UCombatCameraComponent::OnActionCameraDurationEnd(float ReturnBlendTime)
{
	AActor* OwnerActor = GetOwner();
	if (!PlayerController || !OwnerActor || !GetWorld()) return;

	PlayerController->SetViewTargetWithBlend(OwnerActor, ReturnBlendTime, EViewTargetBlendFunction::VTBlend_Cubic);

	GetWorld()->GetTimerManager().SetTimer(
		ActionCamCleanupTimer,
		this,
		&UCombatCameraComponent::OnActionCameraBlendComplete,
		ReturnBlendTime,
		false
	);
}

void UCombatCameraComponent::OnActionCameraBlendComplete()
{
	if (TempActionCamera && IsValid(TempActionCamera))
	{
		TempActionCamera->Destroy();
		TempActionCamera = nullptr;
	}
}