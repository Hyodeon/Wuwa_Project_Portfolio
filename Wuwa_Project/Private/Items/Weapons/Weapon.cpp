// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/Weapons/Weapon.h"
#include "Characters/ParagonCharacter.h"
#include "Components/SphereComponent.h"
#include "Components/BoxComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Enemy/Enemy.h"
#include "Data/BaseAttackData.h"

AWeapon::AWeapon()
{
	// Init
	WeaponBox = CreateDefaultSubobject<UBoxComponent>(TEXT("WeaponBox"));
	WeaponBox->SetupAttachment(GetRootComponent());

	// Setting
	WeaponBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WeaponBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
	WeaponBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);

	// Setting Trace Points
	BoxTraceStart = CreateDefaultSubobject<USceneComponent>(TEXT("BoxTraceStart"));
	BoxTraceStart->SetupAttachment(GetRootComponent());

	BoxTraceEnd = CreateDefaultSubobject<USceneComponent>(TEXT("BoxTraceEnd"));
	BoxTraceEnd->SetupAttachment(GetRootComponent());
}

void AWeapon::BeginPlay()
{
	Super::BeginPlay();

	WeaponBox->OnComponentBeginOverlap.AddDynamic(this, &AWeapon::OnBoxOverlap);
}

void AWeapon::Equip(USceneComponent* InParent, FName InSocketName, AActor* NewOwner, APawn* NewInstigator)
{
	ItemState = EItemState::EIS_Equipped;

	SetOwner(NewOwner);
	SetInstigator(NewInstigator);

	ItemMesh->AttachToComponent(InParent, FAttachmentTransformRules::SnapToTargetIncludingScale, InSocketName);
	DisableSphereCollision();
}

void AWeapon::DisableSphereCollision()
{
	if (Sphere)
	{
		Sphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
}

void AWeapon::OnBoxOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor == GetOwner()) return;
	if (OtherActor->ActorHasTag(TEXT("Floor"))) return;
	if (ActorIsSameType(OtherActor)) return;

	// 공격 판정 작동 =======================================================================
	FHitResult BoxHit;
	BoxTrace(BoxHit);

	if (BoxHit.GetActor())
	{
		if (BoxHit.GetActor() == GetOwner()) return;
		if (BoxHit.GetActor()->ActorHasTag(TEXT("Floor"))) return;
		if (ActorIsSameType(BoxHit.GetActor())) return;

		if (GEngine)
		{
			UE_LOG(LogTemp, Display, TEXT("%s Applies Damage To %s"), *GetOwner()->GetName(), *BoxHit.GetActor()->GetName());
			GEngine->AddOnScreenDebugMessage(-1, 0.f, FColor::Red, BoxHit.GetHitObjectHandle().GetName(), true);
		}

		// ★ 기존 ApplyDamage 대신 공격자의 GAS 데미지 파이프라인 호출
		if (ABaseCharacter* AttackerCharacter = Cast<ABaseCharacter>(GetOwner()))
		{
			const UBaseAttackData* AttackData = AttackerCharacter->GetCurrentAttackData();
			AttackerCharacter->ApplyDamageToTarget(BoxHit.GetActor(), AttackData, BoxHit.ImpactPoint);
		}

		// 기존 피격 인터페이스 및 필드 생성 로직 유지
		ExecuteGetHit(BoxHit);
		CreateFields(BoxHit.ImpactPoint);
	}
}

bool AWeapon::ActorIsSameType(AActor* OtherActor)
{
	return GetOwner()->ActorHasTag(TEXT("Enemy")) && OtherActor->ActorHasTag(TEXT("Enemy"));
}

void AWeapon::ExecuteGetHit(FHitResult& BoxHit)
{
	if (IHitInterface* HitInterface = Cast<IHitInterface>(BoxHit.GetActor()))
	{
		HitInterface->Execute_GetHit(BoxHit.GetActor(), BoxHit.ImpactPoint, GetOwner());
	}
}

void AWeapon::BoxTrace(FHitResult& BoxHit)
{
	const FVector Start = BoxTraceStart->GetComponentLocation();
	const FVector End = BoxTraceEnd->GetComponentLocation();

	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(this);
	ActorsToIgnore.Add(GetOwner());

	for (AActor* Actor : IgnoreActors)
	{
		ActorsToIgnore.AddUnique(Actor);
	}

	UKismetSystemLibrary::BoxTraceSingle(
		this, Start, End,
		BoxTraceExtent, BoxTraceStart->GetComponentRotation(),
		ETraceTypeQuery::TraceTypeQuery1,
		false,
		ActorsToIgnore,
		bShowBoxDebug ? EDrawDebugTrace::ForDuration : EDrawDebugTrace::None,
		BoxHit,
		true
	);

	IgnoreActors.AddUnique(BoxHit.GetActor());
}
