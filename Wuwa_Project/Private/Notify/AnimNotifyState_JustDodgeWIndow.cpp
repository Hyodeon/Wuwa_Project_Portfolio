// Fill out your copyright notice in the Description page of Project Settings.

#include "Notify/AnimNotifyState_JustDodgeWindow.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "WuwaGameplayTags.h"

UAnimNotifyState_JustDodgeWindow::UAnimNotifyState_JustDodgeWindow()
{
#if WITH_EDITORONLY_DATA
	NotifyColor = FColor(0, 200, 255, 255);
#endif
}

FString UAnimNotifyState_JustDodgeWindow::GetNotifyName_Implementation() const
{
	return TEXT("Just Dodge Window");
}

void UAnimNotifyState_JustDodgeWindow::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	if (!MeshComp || !MeshComp->GetOwner()) return;

	// 캐릭터의 ASC를 찾아 State.DodgeWindow 태그 부여
	if (UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(MeshComp->GetOwner()))
	{
		ASC->AddLooseGameplayTag(WuwaGameplayTags::State_DodgeWindow);
	}
}

void UAnimNotifyState_JustDodgeWindow::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	if (!MeshComp || !MeshComp->GetOwner()) return;

	// 태그 제거 (애니메이션이 끝나거나 캔슬되어도 보장됨)
	if (UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(MeshComp->GetOwner()))
	{
		ASC->RemoveLooseGameplayTag(WuwaGameplayTags::State_DodgeWindow);
	}
}