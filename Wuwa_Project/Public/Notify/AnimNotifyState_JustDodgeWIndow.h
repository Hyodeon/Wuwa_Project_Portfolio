// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "AnimNotifyState_JustDodgeWindow.generated.h"

UCLASS()
class WUWA_PROJECT_API UAnimNotifyState_JustDodgeWindow : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	UAnimNotifyState_JustDodgeWindow();

	virtual FString GetNotifyName_Implementation() const override;
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
};