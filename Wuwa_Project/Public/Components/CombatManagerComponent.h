#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CombatManagerComponent.generated.h"

UENUM(BlueprintType)
enum class ECombatTokenType : uint8
{
	Melee,
	Support // e.g., Slam Attack
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class WUWA_PROJECT_API UCombatManagerComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UCombatManagerComponent();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// Request a token for an attacker. Returns true if granted.
	UFUNCTION(BlueprintCallable, Category = "Combat|Token")
	bool RequestToken(AActor* Attacker, ECombatTokenType TokenType);

	// Release any token held by the attacker.
	UFUNCTION(BlueprintCallable, Category = "Combat|Token")
	void ReleaseToken(AActor* Attacker);

	// Check if the attacker currently holds any token.
	UFUNCTION(BlueprintPure, Category = "Combat|Token")
	bool HasToken(AActor* Attacker) const;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Token")
	int32 MaxMeleeTokens;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Token")
	int32 MaxSupportTokens;

private:
	UPROPERTY()
	TArray<AActor*> ActiveMeleeAttackers;

	UPROPERTY()
	TArray<AActor*> ActiveSupportAttackers;

	// Helper to clean up invalid (destroyed) attackers
	void CleanUpTokens();
};
