#include "Notify/AnimNotify_CheckCombo.h"
#include "Characters/BaseCharacter.h"
#include "AbilitySystemComponent.h"
#include "WuwaGameplayTags.h"

void UAnimNotify_CheckCombo::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (!MeshComp || !MeshComp->GetOwner()) return;

	ABaseCharacter* Character = Cast<ABaseCharacter>(MeshComp->GetOwner());
	if (!Character) return;

	UAbilitySystemComponent* ASC = Character->GetAbilitySystemComponent();
	if (ASC)
	{
		FGameplayEventData Payload;
		Payload.Instigator = Character;
		Payload.Target = Character;
		
		ASC->HandleGameplayEvent(WuwaGameplayTags::Event_Combat_CheckCombo, &Payload);
	}
}
