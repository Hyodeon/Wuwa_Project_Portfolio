#include "Notify/AnimNotify_AoEHit.h"
#include "Characters/BaseCharacter.h"
#include "Data/BaseAttackData.h"
#include "Interfaces/HitInterface.h"
#include "Kismet/KismetSystemLibrary.h"
#include "DrawDebugHelpers.h"

UAnimNotify_AoEHit::UAnimNotify_AoEHit()
{
#if WITH_EDITORONLY_DATA
	NotifyColor = FColor(255, 180, 0, 255);
#endif
}

FString UAnimNotify_AoEHit::GetNotifyName_Implementation() const
{
	return FString::Printf(TEXT("AoE Hit (R: %.0f)"), AoERadius);
}

void UAnimNotify_AoEHit::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (!MeshComp) return;

	ABaseCharacter* Attacker = Cast<ABaseCharacter>(MeshComp->GetOwner());
	if (!Attacker || !Attacker->GetWorld()) return;

	// AttackData가 설정되어 있지 않으면 경고 후 리턴 (데이터 누락 방어)
	if (!AttackData)
	{
		UE_LOG(LogTemp, Warning, TEXT("[AnimNotify_AoEHit] AttackData is NOT set in animation: %s"), *GetNameSafe(Animation));
		return;
	}

	// 소켓 위치 또는 액터 위치 계산
	FVector OriginLocation = Attacker->GetActorLocation();
	if (SocketName != NAME_None && MeshComp->DoesSocketExist(SocketName))
	{
		OriginLocation = MeshComp->GetSocketLocation(SocketName);
	}

	// 디버그 드로잉
	if (bDrawDebug)
	{
		DrawDebugSphere(Attacker->GetWorld(), OriginLocation, AoERadius, 16, FColor::Orange, false, 2.0f, 0, 2.0f);
	}

	// 오버랩 검색 조건
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));

	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(Attacker);

	TArray<AActor*> OverlappingActors;
	const bool bFound = UKismetSystemLibrary::SphereOverlapActors(
		Attacker->GetWorld(),
		OriginLocation,
		AoERadius,
		ObjectTypes,
		AActor::StaticClass(),
		ActorsToIgnore,
		OverlappingActors
	);

	if (bFound)
	{
		for (AActor* HitActor : OverlappingActors)
		{
			if (!HitActor || HitActor == Attacker) continue;

			// 아군(Enemy끼리) 타격 무시
			if (Attacker->ActorHasTag(TEXT("Enemy")) && HitActor->ActorHasTag(TEXT("Enemy")))
			{
				continue;
			}

			const FVector HitPoint = HitActor->GetActorLocation();

			// 1. GAS 데미지 파이프라인 (BaseCharacter 내부에서 저스트 회피 체크 및 데미지 GE 적용)
			Attacker->ApplyDamageToTarget(HitActor, AttackData, HitPoint);

			// 2. 피격 인터페이스 호출
			if (IHitInterface* HitInterface = Cast<IHitInterface>(HitActor))
			{
				HitInterface->Execute_GetHit(HitActor, HitPoint, Attacker);
			}
		}
	}
}