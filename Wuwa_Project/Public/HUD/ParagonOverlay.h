#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ParagonOverlay.generated.h"

class UProgressBar;
class UTextBlock;
class UImage;

UCLASS()
class WUWA_PROJECT_API UParagonOverlay : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetHealthBarPercent(float Percent);
	void SetStaminaBarPercent(float Percent);

	void SetGold(int32 Gold);
	void SetSouls(int32 Souls);

	/** 락온 마커를 켜거나 끕니다 */
	void ShowLockOnMarker(bool bShow);

	/** 타겟의 월드 좌표를 넘겨받아 락온 마커의 2D 스크린 위치를 갱신합니다 */
	void UpdateLockOnMarkerPosition(const FVector& TargetWorldLocation);

private:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> HealthProgressBar;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> StaminaProgressBar;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> GoldText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> SoulsText;

	/** WBP의 이미지 이름(LockOnMarker)과 일치해야 합니다 */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> LockOnMarker;
};