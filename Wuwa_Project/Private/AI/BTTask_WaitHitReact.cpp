#include "AI/BTTask_WaitHitReact.h"
#include "AIController.h"
#include "GameFramework/Character.h"
#include "Animation/AnimInstance.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTTask_WaitHitReact::UBTTask_WaitHitReact()
{
	NodeName = TEXT("Wait Hit React");
	bNotifyTick = true; // 비동기 대기를 위한 틱 활성화

	// Bool 타입 키만 선택 가능하도록 필터 추가
	HitStateKey.AddBoolFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_WaitHitReact, HitStateKey));
}

EBTNodeResult::Type UBTTask_WaitHitReact::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AICon = OwnerComp.GetAIOwner();
	if (!AICon)
	{
		return EBTNodeResult::Failed;
	}

	ACharacter* Character = Cast<ACharacter>(AICon->GetPawn());
	if (!Character)
	{
		return EBTNodeResult::Failed;
	}

	// 몽타주가 실행 중이면 InProgress로 대기 시작
	UAnimInstance* AnimInstance = Character->GetMesh() ? Character->GetMesh()->GetAnimInstance() : nullptr;
	if (AnimInstance && AnimInstance->IsAnyMontagePlaying())
	{
		return EBTNodeResult::InProgress;
	}

	// 재생 중인 몽타주가 없으면 즉시 종료 처리
	if (UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent())
	{
		BB->SetValueAsBool(HitStateKey.SelectedKeyName, false);
	}
	return EBTNodeResult::Succeeded;
}

void UBTTask_WaitHitReact::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickTask(OwnerComp, NodeMemory, DeltaSeconds);

	AAIController* AICon = OwnerComp.GetAIOwner();
	ACharacter* Character = AICon ? Cast<ACharacter>(AICon->GetPawn()) : nullptr;
	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();

	if (!Character || !BB)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	UAnimInstance* AnimInstance = Character->GetMesh() ? Character->GetMesh()->GetAnimInstance() : nullptr;

	// 피격 애니메이션(몽타주)이 모두 끝났는지 검사
	if (AnimInstance && !AnimInstance->IsAnyMontagePlaying())
	{
		// 1. 블랙보드의 IsHit 플래그를 자동으로 false 처리
		BB->SetValueAsBool(HitStateKey.SelectedKeyName, false);

		// 2. 태스크 정상 종료 -> BT가 다음 우선순위(전투/추적)로 자동 전이
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	}
}