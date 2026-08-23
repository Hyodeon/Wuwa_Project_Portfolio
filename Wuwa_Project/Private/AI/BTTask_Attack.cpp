

#include "AI/BTTask_Attack.h"
#include "AIController.h"
#include "Enemy/Enemy.h"


UBTTask_Attack::UBTTask_Attack()
{
	NodeName = TEXT("Attack Target");
}

EBTNodeResult::Type UBTTask_Attack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AICon = OwnerComp.GetAIOwner();
	if (!AICon) return EBTNodeResult::Failed;

	AEnemy* Enemy = Cast<AEnemy>(AICon->GetPawn());
	if (!Enemy) return EBTNodeResult::Failed;

	// 기존처럼 Attack()을 호출하여 기본 공격 상태(EnemyState) 세팅
	Enemy->Attack();

	// 만약 에디터에서 특정 섹션 이름(예: "Slam")을 적어두었다면, 해당 섹션으로 강제 덮어쓰기 재생
	if (AttackSectionName != NAME_None)
	{
		Enemy->PlayAttackMontageSection(AttackSectionName);
	}

	return EBTNodeResult::Succeeded; // (또는 진행형으로 InProgress 리턴 등 기존 로직 유지)
}

void UBTTask_Attack::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickTask(OwnerComp, NodeMemory, DeltaSeconds);

	AAIController* AICon = OwnerComp.GetAIOwner();
	AEnemy* Enemy = AICon ? Cast<AEnemy>(AICon->GetPawn()) : nullptr;

	if (!Enemy)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	// 3. 몽타주가 끝났는지 체크 (어떤 몽타주든 재생 중이 아니면 끝난 것으로 간주)
	UAnimInstance* AnimInstance = Enemy->GetMesh()->GetAnimInstance();
	if (AnimInstance && !AnimInstance->IsAnyMontagePlaying())
	{
		// 4. 애니메이션이 끝나면 태스크 최종 성공 처리
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	}
}