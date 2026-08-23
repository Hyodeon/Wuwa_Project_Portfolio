// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "AnimNotifyState_MeleeTrace.generated.h"

class UBaseAttackData;

UCLASS(meta = (DisplayName = "Melee Trace"))
class WUWA_PROJECT_API UAnimNotifyState_MeleeTrace : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	UAnimNotifyState_MeleeTrace();

	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

	virtual FString GetNotifyName_Implementation() const override;

public:
	// 추적할 본(Bone) 또는 소켓(Socket) 이름 (예: "hand_l", "hand_r", "LeftHand")
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trace")
	FName SocketName = TEXT("hand_r");

	// 스윕 판정 구체의 반지름
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trace", meta = (ClampMin = "5.0"))
	float TraceRadius = 40.0f;

	// 이번 타격에 사용할 데미지/모션 배율 데이터 에셋
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	TObjectPtr<UBaseAttackData> AttackData;

	// 에디터/인게임 디버그 드로잉 표시 여부
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	bool bDrawDebug = false;
};