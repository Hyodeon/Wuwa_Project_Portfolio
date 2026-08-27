#include "HUD/ParagonOverlay.h"
#include "Components/ProgressBar.h"
#include "Components/RadialSlider.h"	
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/CanvasPanelSlot.h"
#include "Blueprint/WidgetLayoutLibrary.h"


void UParagonOverlay::SetHealthBarPercent(float Percent)
{
	if (HealthProgressBar) HealthProgressBar->SetPercent(Percent);
}

void UParagonOverlay::SetStaminaBarPercent(float Percent)
{
	if (StaminaProgressBar) StaminaProgressBar->SetPercent(Percent);
}

void UParagonOverlay::SetValorBarPercent(float Percent)
{
	if (ValorProgressBar) ValorProgressBar->SetPercent(Percent);
}

void UParagonOverlay::SetUltimateBarPercent(float Percent)
{
	if (UltimateProgressBar) UltimateProgressBar->SetValue(Percent);
}

void UParagonOverlay::SetGold(int32 Gold)
{
	if (GoldText) GoldText->SetText(FText::AsNumber(Gold));
}

void UParagonOverlay::SetSouls(int32 Souls)
{
	if (SoulsText) SoulsText->SetText(FText::AsNumber(Souls));
}

void UParagonOverlay::ShowLockOnMarker(bool bShow)
{
	if (LockOnMarker)
	{
		LockOnMarker->SetVisibility(bShow ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	}
}

void UParagonOverlay::UpdateLockOnMarkerPosition(const FVector& TargetWorldLocation)
{
	if (!LockOnMarker || LockOnMarker->GetVisibility() == ESlateVisibility::Collapsed) return;

	APlayerController* PC = GetOwningPlayer();
	if (!PC) return;

	FVector2D ScreenPosition;
	// 3D 월드 좌표 -> 뷰포트 2D 위젯 좌표로 변환
	bool bProjected = UWidgetLayoutLibrary::ProjectWorldLocationToWidgetPosition(PC, TargetWorldLocation, ScreenPosition, false);

	if (bProjected)
	{
		if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(LockOnMarker->Slot))
		{
			// 마커의 중심 정렬
			CanvasSlot->SetAlignment(FVector2D(0.5f, 0.5f));
			CanvasSlot->SetPosition(ScreenPosition);
		}
	}
}