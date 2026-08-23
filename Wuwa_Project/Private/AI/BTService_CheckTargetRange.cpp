#include "AI/BTService_CheckTargetRange.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/Pawn.h"
#include "Enemy/Enemy.h"
#include "Kismet/KismetSystemLibrary.h"

UBTService_CheckTargetRange::UBTService_CheckTargetRange()
{
	NodeName = TEXT("Check Target Range");
	Interval = 0.5f;
	RandomDeviation = 0.1f;
}

void UBTService_CheckTargetRange::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	AAIController* AICon = OwnerComp.GetAIOwner();
	if (!BB || !AICon) return;

	AActor* ControlledPawn = AICon->GetPawn();
	AActor* Target = Cast<AActor>(BB->GetValueAsObject(TargetKey.SelectedKeyName));

	if (!ControlledPawn || !Target)
	{
		BB->SetValueAsBool(InAttackRangeKey.SelectedKeyName, false);
		BB->SetValueAsBool(CanSlamKey.SelectedKeyName, false);
		return;
	}

	float Distance = FVector::Dist2D(ControlledPawn->GetActorLocation(), Target->GetActorLocation());
	
	if (TargetDistanceKey.SelectedKeyName != NAME_None)
	{
		BB->SetValueAsFloat(TargetDistanceKey.SelectedKeyName, Distance);
	}

	if (Distance > MaxLoseInterestDistance)
	{
		BB->SetValueAsObject(TargetKey.SelectedKeyName, nullptr);
		BB->SetValueAsBool(InAttackRangeKey.SelectedKeyName, false);
		BB->SetValueAsBool(CanSlamKey.SelectedKeyName, false);
		
		if (AEnemy* Enemy = Cast<AEnemy>(ControlledPawn))
		{
			Enemy->LoseInterest();
		}
		return;
	}

	bool bInAttackRange = (Distance <= AttackRange);
	if (InAttackRangeKey.SelectedKeyName != NAME_None)
	{
		BB->SetValueAsBool(InAttackRangeKey.SelectedKeyName, bInAttackRange);
	}

	bool bCanSlam = (Distance > AttackRange && Distance <= SlamRange);
	
	if (bCanSlam)
	{
		TArray<AActor*> ActorsToIgnore;
		ActorsToIgnore.Add(ControlledPawn);
		ActorsToIgnore.Add(Target);

		FHitResult HitResult;
		bool bHit = UKismetSystemLibrary::SphereTraceSingle(
			GetWorld(),
			ControlledPawn->GetActorLocation(),
			Target->GetActorLocation(),
			60.f, // 충돌 판정 반경 (다른 적의 두께 정도)
			TraceTypeQuery1, // 카메라/비지빌리티 채널
			false, // trace complex
			ActorsToIgnore,
			EDrawDebugTrace::None,
			HitResult,
			true
		);

		// 중간에 누군가(다른 적이나 벽)가 있다면 슬램 불가능 -> 스트레이핑 분기로 자연스럽게 넘어가서 우회함
		if (bHit)
		{
			bCanSlam = false; 
		}
	}

	if (CanSlamKey.SelectedKeyName != NAME_None)
	{
		BB->SetValueAsBool(CanSlamKey.SelectedKeyName, bCanSlam);
	}
}
