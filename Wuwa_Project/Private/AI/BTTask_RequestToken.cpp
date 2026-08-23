#include "AI/BTTask_RequestToken.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/Character.h"
#include "Components/CombatManagerComponent.h"

UBTTask_RequestToken::UBTTask_RequestToken()
{
	NodeName = TEXT("Request Combat Token");
}

EBTNodeResult::Type UBTTask_RequestToken::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
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
	if (!CombatManager)
	{
		// If target doesn't have a combat manager, we just succeed to avoid blocking combat logic
		return EBTNodeResult::Succeeded;
	}

	bool bGranted = CombatManager->RequestToken(ControlledPawn, TokenType);
	
	if (bGranted)
	{
		// Set BB value so the AI knows it has a token
		OwnerComp.GetBlackboardComponent()->SetValueAsBool(TEXT("HasToken"), true);
		return EBTNodeResult::Succeeded;
	}
	else
	{
		OwnerComp.GetBlackboardComponent()->SetValueAsBool(TEXT("HasToken"), false);
		return EBTNodeResult::Failed;
	}
}
