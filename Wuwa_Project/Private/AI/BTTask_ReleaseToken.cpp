#include "AI/BTTask_ReleaseToken.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/Character.h"
#include "Components/CombatManagerComponent.h"

UBTTask_ReleaseToken::UBTTask_ReleaseToken()
{
	NodeName = TEXT("Release Combat Token");
}

EBTNodeResult::Type UBTTask_ReleaseToken::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AActor* TargetActor = Cast<AActor>(OwnerComp.GetBlackboardComponent()->GetValueAsObject(TargetKey.SelectedKeyName));
	AAIController* AIController = OwnerComp.GetAIOwner();
	
	if (!TargetActor || !AIController)
	{
		return EBTNodeResult::Failed;
	}

	APawn* ControlledPawn = AIController->GetPawn();
	if (!ControlledPawn)
	{
		return EBTNodeResult::Failed;
	}

	UCombatManagerComponent* CombatManager = TargetActor->FindComponentByClass<UCombatManagerComponent>();
	if (CombatManager)
	{
		CombatManager->ReleaseToken(ControlledPawn);
	}
	
	OwnerComp.GetBlackboardComponent()->SetValueAsBool(TEXT("HasToken"), false);

	return EBTNodeResult::Succeeded;
}
