// Fill out your copyright notice in the Description page of Project Settings.


#include "Breakable/BreakableActor.h"
#include "GeometryCollection/GeometryCollectionComponent.h"
#include "Components/CapsuleComponent.h"
#include "Items/Treasure.h"


ABreakableActor::ABreakableActor()
{
	PrimaryActorTick.bCanEverTick = true;

	GeometryCollectionComponent = CreateDefaultSubobject<UGeometryCollectionComponent>(TEXT("GeometryCollectionComponent"));
	SetRootComponent(GeometryCollectionComponent);

	GeometryCollectionComponent->SetGenerateOverlapEvents(true);
	GeometryCollectionComponent->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GeometryCollectionComponent->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
	GeometryCollectionComponent->SetCollisionResponseToChannel(ECollisionChannel::ECC_Destructible, ECollisionResponse::ECR_Ignore);

	CapsuleComponent = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CapsuleComponent"));
	CapsuleComponent->SetupAttachment(RootComponent);
	CapsuleComponent->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	CapsuleComponent->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Block);

	GeometryCollectionComponent->BodyInstance.bStartAwake = false;

}

void ABreakableActor::BeginPlay()
{
	Super::BeginPlay();
	
}

void ABreakableActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ABreakableActor::GetHit_Implementation(const FVector& HitVector, AActor* Hitter)
{
	if (bBroken) return;

	bBroken = true;
	UWorld* World = GetWorld();

	if (World && TreasureClasses.Num() > 0)
	{
		FVector Location = GetActorLocation();
		Location.Z += 100.f;

		int32 RandomIndex = FMath::RandRange(0, TreasureClasses.Num() - 1);
		TSubclassOf<ATreasure> SelectedTreasureClass = TreasureClasses[RandomIndex];

		World->SpawnActor<ATreasure>(SelectedTreasureClass, Location, GetActorRotation());
	}
}

