#include "Components/CombatManagerComponent.h"
#include "GameFramework/Actor.h"

UCombatManagerComponent::UCombatManagerComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	
	// Default token capacities
	MaxMeleeTokens = 2;
	MaxSupportTokens = 1;
}

void UCombatManagerComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UCombatManagerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	CleanUpTokens();
}

bool UCombatManagerComponent::RequestToken(AActor* Attacker, ECombatTokenType TokenType)
{
	if (!Attacker) return false;

	CleanUpTokens();

	// If already has a token of any type, return true (assuming they just keep their current one)
	if (HasToken(Attacker))
	{
		return true;
	}

	if (TokenType == ECombatTokenType::Melee)
	{
		if (ActiveMeleeAttackers.Num() < MaxMeleeTokens)
		{
			ActiveMeleeAttackers.Add(Attacker);
			return true;
		}
	}
	else if (TokenType == ECombatTokenType::Support)
	{
		if (ActiveSupportAttackers.Num() < MaxSupportTokens)
		{
			ActiveSupportAttackers.Add(Attacker);
			return true;
		}
	}

	return false;
}

void UCombatManagerComponent::ReleaseToken(AActor* Attacker)
{
	if (!Attacker) return;

	ActiveMeleeAttackers.Remove(Attacker);
	ActiveSupportAttackers.Remove(Attacker);
}

bool UCombatManagerComponent::HasToken(AActor* Attacker) const
{
	if (!Attacker) return false;

	return ActiveMeleeAttackers.Contains(Attacker) || ActiveSupportAttackers.Contains(Attacker);
}

void UCombatManagerComponent::CleanUpTokens()
{
	// Remove any null or destroyed attackers
	ActiveMeleeAttackers.RemoveAll([](AActor* Actor) { return !IsValid(Actor); });
	ActiveSupportAttackers.RemoveAll([](AActor* Actor) { return !IsValid(Actor); });
}
