#include "Notify/AnimNotifyState_SuperArmor.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "WuwaGameplayTags.h"

UAnimNotifyState_SuperArmor::UAnimNotifyState_SuperArmor()
{
#if WITH_EDITORONLY_DATA
	NotifyColor = FColor(255, 128, 0, 255); // 주황색
#endif
}

FString UAnimNotifyState_SuperArmor::GetNotifyName_Implementation() const
{
	return TEXT("Super Armor");
}

void UAnimNotifyState_SuperArmor::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	if (!MeshComp || !MeshComp->GetOwner()) return;

	if (UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(MeshComp->GetOwner()))
	{
		ASC->AddLooseGameplayTag(WuwaGameplayTags::State_SuperArmor);
	}
}

void UAnimNotifyState_SuperArmor::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	if (!MeshComp || !MeshComp->GetOwner()) return;

	if (UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(MeshComp->GetOwner()))
	{
		ASC->RemoveLooseGameplayTag(WuwaGameplayTags::State_SuperArmor);
	}
}
