// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interfaces/HitInterface.h"

#include "BreakableActor.generated.h"


class UGeometryCollectionComponent;

UCLASS()
class WUWA_PROJECT_API ABreakableActor : public AActor, public IHitInterface
{
	GENERATED_BODY()
	
public:
	ABreakableActor();
	virtual void Tick(float DeltaTime) override;
	virtual void GetHit_Implementation(const FVector& ImpactPoint, AActor* Hitter) override;

protected:

	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	UGeometryCollectionComponent* GeometryCollectionComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	class UCapsuleComponent* CapsuleComponent;

private:

	UPROPERTY(EditAnywhere, Category = "Breakable")
	TArray<TSubclassOf<class ATreasure>> TreasureClasses;

	bool bBroken = false;
};
