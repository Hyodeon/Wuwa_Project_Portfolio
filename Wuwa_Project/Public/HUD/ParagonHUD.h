// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "ParagonHUD.generated.h"

class UParagonOverlay;

/**
 * 
 */
UCLASS()
class WUWA_PROJECT_API AParagonHUD : public AHUD
{
	GENERATED_BODY()
protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY(EditDefaultsOnly, Category = Paragon)
	TSubclassOf<UParagonOverlay> ParagonOverlayClass;

	UPROPERTY()
	UParagonOverlay* ParagonOverlay;

public:
	FORCEINLINE UParagonOverlay* GetParagonOverlay() const { return ParagonOverlay; }
};
