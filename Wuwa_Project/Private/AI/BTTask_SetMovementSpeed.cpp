#include "AI/BTTask_SetMovementSpeed.h"
#include "AIController.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

UBTTask_SetMovementSpeed::UBTTask_SetMovementSpeed()
{
	NodeName = TEXT("Set Movement Speed");
}

EBTNodeResult::Type UBTTask_SetMovementSpeed::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	if (AAIController* AICon = OwnerComp.GetAIOwner())
	{
		if (ACharacter* Char = Cast<ACharacter>(AICon->GetPawn()))
		{
			Char->GetCharacterMovement()->MaxWalkSpeed = NewSpeed;
			return EBTNodeResult::Succeeded;
		}
	}
	return EBTNodeResult::Failed;
}
