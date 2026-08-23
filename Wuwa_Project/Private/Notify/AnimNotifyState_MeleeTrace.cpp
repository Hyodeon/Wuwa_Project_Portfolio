// Fill out your copyright notice in the Description page of Project Settings.

#include "Notify/AnimNotifyState_MeleeTrace.h"
#include "Characters/BaseCharacter.h"
#include "Data/BaseAttackData.h"
#include "Kismet/KismetSystemLibrary.h"

UAnimNotifyState_MeleeTrace::UAnimNotifyState_MeleeTrace()
{
#if WITH_EDITORONLY_DATA
	NotifyColor = FColor(255, 60, 60, 255);
#endif
}

FString UAnimNotifyState_MeleeTrace::GetNotifyName_Implementation() const
{
	return FString::Printf(TEXT("MeleeTrace: %s"), *SocketName.ToString());
}

void UAnimNotifyState_MeleeTrace::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	if (!MeshComp) return;

	if (ABaseCharacter* Character = Cast<ABaseCharacter>(MeshComp->GetOwner()))
	{
		// 중복 타격 방지 목록 초기화
		Character->ClearDamagedActors();

		// 시작 지점 소켓 위치 기록
		const FVector CurrentSocketLoc = MeshComp->GetSocketLocation(SocketName);
		Character->SetLastMeleeTraceLocation(CurrentSocketLoc);

		// 현재 공격 데이터를 캐릭터에 세팅
		if (AttackData)
		{
			Character->SetCurrentAttackData(AttackData);
		}
	}
}

void UAnimNotifyState_MeleeTrace::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyTick(MeshComp, Animation, FrameDeltaTime, EventReference);

	if (!MeshComp) return;

	ABaseCharacter* Attacker = Cast<ABaseCharacter>(MeshComp->GetOwner());
	if (!Attacker || !Attacker->GetWorld()) return;

	const FVector StartLoc = Attacker->GetLastMeleeTraceLocation();
	const FVector EndLoc = MeshComp->GetSocketLocation(SocketName);

	// 자기 자신을 트레이스 무시 목록에 추가
	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(Attacker);

	// 충돌 대상 탐색 (Pawn 채널 기준)
	TArray<FHitResult> HitResults;
	EDrawDebugTrace::Type DebugType = bDrawDebug ? EDrawDebugTrace::ForOneFrame : EDrawDebugTrace::None;

	const bool bHit = UKismetSystemLibrary::SphereTraceMulti(
		Attacker->GetWorld(),
		StartLoc,
		EndLoc,
		TraceRadius,
		UEngineTypes::ConvertToTraceType(ECC_Pawn),
		false,
		ActorsToIgnore,
		DebugType,
		HitResults,
		true
	);

	if (bHit)
	{
		const UBaseAttackData* CurrentData = AttackData ? AttackData.Get() : Attacker->GetCurrentAttackData();

		for (const FHitResult& Hit : HitResults)
		{
			AActor* HitActor = Hit.GetActor();
			if (!HitActor || HitActor == Attacker) continue;

			// 아군끼리 타격 방지
			if (Attacker->ActorHasTag(TEXT("Enemy")) && HitActor->ActorHasTag(TEXT("Enemy")))
			{
				continue;
			}

			// 이번 공격에서 아직 안 맞은 액터에게만 단 1회 데미지 적용
			if (!Attacker->HasAlreadyBeenDamaged(HitActor))
			{
				Attacker->AddDamagedActor(HitActor);
				Attacker->ApplyDamageToTarget(HitActor, CurrentData, Hit.ImpactPoint);

				// 피격 리액션 인터페이스 호출
				if (IHitInterface* HitInterface = Cast<IHitInterface>(HitActor))
				{
					HitInterface->Execute_GetHit(HitActor, Hit.ImpactPoint, Attacker);
				}
			}
		}
	}

	// 다음 틱을 위해 현재 위치를 LastLocation으로 갱신
	Attacker->SetLastMeleeTraceLocation(EndLoc);
}

void UAnimNotifyState_MeleeTrace::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	if (!MeshComp) return;

	if (ABaseCharacter* Character = Cast<ABaseCharacter>(MeshComp->GetOwner()))
	{
		Character->ClearDamagedActors();
	}
}