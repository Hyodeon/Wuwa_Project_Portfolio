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
	// 블루프린트의 이름과 동일.
	UPROPERTY(meta = (BindWidget))
	UProgressBar* HealthBar;

};
