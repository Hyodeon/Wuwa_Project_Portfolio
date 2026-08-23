#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CombatCameraComponent.generated.h"

class USpringArmComponent;
class UCameraComponent;
class APlayerController;
class ACameraActor;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class WUWA_PROJECT_API UCombatCameraComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCombatCameraComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/** 카메라 컴포넌트 필수 참조 초기화 */
	UFUNCTION(BlueprintCallable, Category = "Combat|Camera")
	void InitializeCamera(USpringArmComponent* InSpringArm, UCameraComponent* InCamera, APlayerController* InPC);

	/** 록온 타겟 설정 및 해제 */
	UFUNCTION(BlueprintCallable, Category = "Combat|Camera")
	void SetLockOnTarget(AActor* NewTarget);

	/** 시네마틱 액션 카메라 뷰 실행 */
	UFUNCTION(BlueprintCallable, Category = "Combat|Camera")
	void PlayActionCameraMove(const FTransform& TargetTransform, float Duration, float ReturnBlendTime = 0.5f);

protected:
	virtual void BeginPlay() override;

private:
	/* ---------------------------------------------------------------------- */
	/*                                내부 참조                               */
	/* ---------------------------------------------------------------------- */

	UPROPERTY()
	TObjectPtr<USpringArmComponent> SpringArmComp;

	UPROPERTY()
	TObjectPtr<UCameraComponent> CameraComp;

	UPROPERTY()
	TObjectPtr<APlayerController> PlayerController;

	UPROPERTY()
	TObjectPtr<AActor> CurrentLockTarget;

	UPROPERTY()
	TObjectPtr<ACameraActor> TempActionCamera;

	/* ---------------------------------------------------------------------- */
	/*                            카메라 랙 및 록온 옵션                      */
	/* ---------------------------------------------------------------------- */

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Camera|Lag")
	float DefaultCameraLocationLagSpeed = 16.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Camera|LockOn")
	float LockOnRotationInterpSpeed = 10.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Camera|LockOn")
	float LockOnFocusInterpSpeed = 15.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Camera|LockOn")
	float BaseSpringArmLength = 400.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Camera|LockOn")
	float MinLockOnArmLength = 450.0f;

	/* ---------------------------------------------------------------------- */
	/*                         다이내믹 바이어스 상태 변수                    */
	/* ---------------------------------------------------------------------- */

	float PreviousIdealYaw = 0.0f;
	float TargetLockOnBias = 0.0f;
	float CurrentLockOnBias = 0.0f;

	/* ---------------------------------------------------------------------- */
	/*                              시네마틱 타이머                           */
	/* ---------------------------------------------------------------------- */

	FTimerHandle ActionCamDurationTimer;
	FTimerHandle ActionCamCleanupTimer;

	void OnActionCameraDurationEnd(float ReturnBlendTime);
	void OnActionCameraBlendComplete();
};