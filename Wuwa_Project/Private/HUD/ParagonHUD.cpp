// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD/ParagonHUD.h"
#include "HUD/ParagonOverlay.h"

#include "Components/Widget.h"

void AParagonHUD::BeginPlay()
{
	Super::BeginPlay();

	if (ParagonOverlayClass)
	{
		ParagonOverlay = CreateWidget<UParagonOverlay>(GetWorld(), ParagonOverlayClass);
		if (ParagonOverlay)
		{
			ParagonOverlay->AddToViewport();
		}
	}
}
