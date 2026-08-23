#include "AI/BTTask_FindStrafeLocation.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "NavigationSystem.h"

UBTTask_FindStrafeLocation::UBTTask_FindStrafeLocation()
{
	NodeName = TEXT("Find Strafe Location");
}

EBTNodeResult::Type UBTTask_FindStrafeLocation::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AActor* TargetActor = Cast<AActor>(OwnerComp.GetBlackboardComponent()->GetValueAsObject(TargetKey.SelectedKeyName));
	AAIController* AICon = OwnerComp.GetAIOwner();

	if (!TargetActor || !AICon) return EBTNodeResult::Failed;

	APawn* ControlledPawn = AICon->GetPawn();
	if (!ControlledPawn) return EBTNodeResult::Failed;

	FVector TargetLoc = TargetActor->GetActorLocation();
	FVector MyLoc = ControlledPawn->GetActorLocation();
	
	// 1. 플레이어에서 나를 향하는 벡터
	FVector PlayerToMe = MyLoc - TargetLoc;
	PlayerToMe.Z = 0.f;

	// 2. 좌/우 중 한 방향으로 45도 회전 (플레이어를 가로지르지 않고 외곽을 돎)
	float Angle = FMath::RandBool() ? 45.f : -45.f;
	FVector RotatedDir = PlayerToMe.RotateAngleAxis(Angle, FVector::UpVector).GetSafeNormal();

	// 3. 목표 거리에 맞춰서 좌표 생성
	float IdealDistance = FMath::RandRange(MinStrafeDistance, MaxStrafeDistance);
	FVector DesiredLocation = TargetLoc + (RotatedDir * IdealDistance);

	UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());
	if (NavSys)
	{
		FNavLocation NavLocation;
		if (NavSys->ProjectPointToNavigation(DesiredLocation, NavLocation, FVector(500.f, 500.f, 500.f)))
		{
			OwnerComp.GetBlackboardComponent()->SetValueAsVector(StrafeLocationKey.SelectedKeyName, NavLocation.Location);
			return EBTNodeResult::Succeeded;
		}
	}

	return EBTNodeResult::Failed;
}
