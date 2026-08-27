// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "HealthBar.generated.h"

class UProgressBar;

/**
 * 
 */
UCLASS()
class WUWA_PROJECT_API UHealthBar : public UUserWidget
{
	GENERATED_BODY()
	
public:
	// 에디터의 막대 이름과 일치해야 함.
	UPROPERTY(meta = (BindWidget))
	UProgressBar* HealthBar;

	UPROPERTY(meta = (BindWidget))
	UProgressBar* ResonanceBar;
};