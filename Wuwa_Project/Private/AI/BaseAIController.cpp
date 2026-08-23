#include "AI/BaseAIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTree.h"
#include "Enemy/Enemy.h"

#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"

ABaseAIController::ABaseAIController()
{
	BehaviorTreeComponent = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BehaviorTreeComponent"));
	BlackboardComponent = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComponent"));
	AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComponent"));

	SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
	if (SightConfig)
	{
		SightConfig->SightRadius = 1500.f;
		SightConfig->LoseSightRadius = 2000.f;
		// 360도 디버그용 시야에서 정상 시야각(전방 120도)으로 복구
		SightConfig->PeripheralVisionAngleDegrees = 60.f;
		SightConfig->SetMaxAge(5.0f);

		SightConfig->DetectionByAffiliation.bDetectEnemies = true;
		SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
		SightConfig->DetectionByAffiliation.bDetectFriendlies = true;

		AIPerceptionComponent->ConfigureSense(*SightConfig);
		AIPerceptionComponent->SetDominantSense(SightConfig->GetSenseImplementation());
	}
}

void ABaseAIController::BeginPlay()
{
	Super::BeginPlay();

	if (AIPerceptionComponent)
	{
		AIPerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &ABaseAIController::OnTargetPerceptionUpdated);
	}
}

void ABaseAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	if (AEnemy* Enemy = Cast<AEnemy>(InPawn))
	{
		if (UBehaviorTree* Tree = Enemy->GetBehaviorTree())
		{
			// 불필요한 임시 변수들을 줄이고 직관적으로 초기화
			if (Tree->BlackboardAsset)
			{
				UBlackboardComponent* BBComp = BlackboardComponent;
				UseBlackboard(Tree->BlackboardAsset, BBComp);
			}

			RunBehaviorTree(Tree);
		}
	}
}

void ABaseAIController::OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
	if (!Actor || !Actor->ActorHasTag(FName("EngageableTarget")))
	{
		return;
	}

	UBlackboardComponent* BB = GetBlackboardComponent();
	if (!BB) return;

	if (Stimulus.WasSuccessfullySensed())
	{
		// 시야에 다시 들어오면 '망각 타이머'를 즉시 취소하고 타겟 유지
		GetWorld()->GetTimerManager().ClearTimer(TargetLoseTimer);
		BB->SetValueAsObject(FName("TargetActor"), Actor);

		if (AEnemy* Enemy = Cast<AEnemy>(GetPawn()))
		{
			Enemy->SetInterest(Actor);
		}

		UE_LOG(LogTemp, Warning, TEXT("[AI] Target Sensed & Timer Cleared: %s"), *Actor->GetName());
	}
	else
	{
		// 시야에서 벗어나면 즉시 지우지 않고 3.5초 뒤에 지우도록 타이머 가동
		GetWorld()->GetTimerManager().SetTimer(TargetLoseTimer, this, &ABaseAIController::ClearTarget, 3.5f, false);
		UE_LOG(LogTemp, Warning, TEXT("[AI] Target Lost! 3.5s Forget Timer Started."));
	}
}

// 3.5초 동안 시야에 안 들어오면 최종적으로 블랙보드를 비워 패트롤로 돌려보냄
void ABaseAIController::ClearTarget()
{
	if (BlackboardComponent)
	{
		// 거리 기반 이탈 체크 (시야에서 놓쳤더라도 가까우면 타깃 유지)
		AActor* CurrentTarget = Cast<AActor>(BlackboardComponent->GetValueAsObject(FName("TargetActor")));
		if (CurrentTarget && GetPawn())
		{
			float Distance = FVector::Dist(GetPawn()->GetActorLocation(), CurrentTarget->GetActorLocation());
			if (Distance <= 2500.f) // 25m 이내면 계속 전투
			{
				UE_LOG(LogTemp, Warning, TEXT("[AI] Target lost from sight, but still close (%.0f). Keeping Aggro."), Distance);
				return; // 타깃을 지우지 않고 유지
			}
		}

		BlackboardComponent->SetValueAsObject(FName("TargetActor"), nullptr);
		if (AEnemy* Enemy = Cast<AEnemy>(GetPawn()))
		{
			Enemy->LoseInterest();
		}
		UE_LOG(LogTemp, Warning, TEXT("[AI] Aggro Reset by Distance. Returning to Patrol."));
	}
}