// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BaseAIController.generated.h"

class UBehaviorTreeComponent;
class UBlackboardComponent;
class UAIPerceptionComponent;
class UAISenseConfig_Sight;

/**
 * 
 */
UCLASS()
class WUWA_PROJECT_API ABaseAIController : public AAIController
{
	GENERATED_BODY()
	
public:
	ABaseAIController();

protected:
	virtual void OnPossess(APawn* InPawn) override;
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

	UFUNCTION()
	void ClearTarget();


	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	TObjectPtr<UBehaviorTreeComponent> BehaviorTreeComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	TObjectPtr<UBlackboardComponent> BlackboardComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	TObjectPtr<UAIPerceptionComponent> AIPerceptionComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	TObjectPtr<UAISenseConfig_Sight> SightConfig;

	FTimerHandle TargetLoseTimer;
};
