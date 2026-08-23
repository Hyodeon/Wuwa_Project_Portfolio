
#include "AI/BTTask_FindPatrolLocation.h"
#include "AIController.h"
#include "NavigationSystem.h"
#include "BehaviorTree/BlackboardComponent.h"



UBTTask_FindPatrolLocation::UBTTask_FindPatrolLocation()
{
	NodeName = TEXT("Find Patrol Location");

	// Vector 타입의 블랙보드 키만 에디터 드롭다운에 노출되도록 필터링
	PatrolLocationKey.AddVectorFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_FindPatrolLocation, PatrolLocationKey));
}

EBTNodeResult::Type UBTTask_FindPatrolLocation::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AICon = OwnerComp.GetAIOwner();
	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();

	if (!AICon || !BlackboardComp)
	{
		return EBTNodeResult::Failed;
	}

	APawn* ControllingPawn = AICon->GetPawn();
	if (!ControllingPawn)
	{
		return EBTNodeResult::Failed;
	}

	// 내비메시 시스템을 통해 폰 위치 기준 반경 내 도달 가능한 랜덤 지점 탐색
	UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());
	if (!NavSys)
	{
		return EBTNodeResult::Failed;
	}

	FNavLocation RandomNavLocation;
	const FVector Origin = ControllingPawn->GetActorLocation();

	if (NavSys->GetRandomReachablePointInRadius(Origin, PatrolRadius, RandomNavLocation))
	{
		BlackboardComp->SetValueAsVector(PatrolLocationKey.SelectedKeyName, RandomNavLocation.Location);
		return EBTNodeResult::Succeeded;
	}

	return EBTNodeResult::Failed;
}