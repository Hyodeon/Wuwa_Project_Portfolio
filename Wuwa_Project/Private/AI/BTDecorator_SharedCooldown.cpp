#include "AI/BTDecorator_SharedCooldown.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Engine/World.h"

UBTDecorator_SharedCooldown::UBTDecorator_SharedCooldown()
{
	NodeName = TEXT("Shared Cooldown");
	bNotifyActivation = true; // Ȱȭ  OnNodeActivation ȣ
}

bool UBTDecorator_SharedCooldown::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (!BB) return false;

	float LastAttackTime = BB->GetValueAsFloat(LastAttackTimeKey.SelectedKeyName);
	float CurrentTime = GetWorld()->GetTimeSeconds();

	//   ð Ÿ(CooldownTime)   ( )
	return (CurrentTime >= LastAttackTime + CooldownTime);
}

void UBTDecorator_SharedCooldown::OnNodeActivation(FBehaviorTreeSearchData& SearchData)
{
	Super::OnNodeActivation(SearchData);

	UBlackboardComponent* BB = SearchData.OwnerComp.GetBlackboardComponent();
	if (BB)
	{
		//  Ǵ    ð 
		BB->SetValueAsFloat(LastAttackTimeKey.SelectedKeyName, GetWorld()->GetTimeSeconds());
	}
}
