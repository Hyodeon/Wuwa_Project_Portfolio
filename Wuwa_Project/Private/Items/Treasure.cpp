// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/Treasure.h"
#include "Characters/ParagonCharacter.h"
#include "Kismet/GameplayStatics.h"

void ATreasure::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	AParagonCharacter* ParagonCharacter = Cast<AParagonCharacter>(OtherActor);
	if (ParagonCharacter)
	{
		if (HitSound)
		{
			UGameplayStatics::PlaySoundAtLocation(this, HitSound, GetActorLocation());
		}

		Destroy();
	}
}

void ATreasure::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{

}
