// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AttributeComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class WUWA_PROJECT_API UAttributeComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UAttributeComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	void RegenStamina(float DeltaTime);

protected:
	virtual void BeginPlay() override;	

private:
	// Current
	UPROPERTY(EditAnywhere, Category = "Actor Attributes")
	float Health;

	UPROPERTY(EditAnywhere, Category = "Actor Attributes")
	float MaxHealth;

	// Current
	UPROPERTY(EditAnywhere, Category = "Actor Attributes")
	float Stamina;

	UPROPERTY(EditAnywhere, Category = "Actor Attributes")
	float MaxStamina;

	UPROPERTY(EditAnywhere, Category = "Actor Attributes")
	float DodgeCost = 10.f;

	UPROPERTY(EditAnywhere, Category = "Actor Attributes")
	float StaminaRegenRate = 5.f;

public:
	void ReceiveDamage(float Damage);
	void UseStamina(float StaminaCost);
	float GetHealthPercent() const { return Health / MaxHealth; }
	float GetStaminaPercent() const { return Stamina / MaxStamina; }
	bool IsAlive() const { return Health > 0.f; }

	// GETTER
	FORCEINLINE float GetHealth() const { return Health; }
	FORCEINLINE float GetMaxHealth() const { return MaxHealth; }
	FORCEINLINE float GetStamina() const { return Stamina; }
	FORCEINLINE float GetMaxStamina() const { return MaxStamina; }
	FORCEINLINE float GetDodgeCost() const { return DodgeCost; }

	// SETTER
	FORCEINLINE void SetHealth(float NewHealth) { Health = NewHealth; }
	FORCEINLINE void SetMaxHealth(float NewMaxHealth) { MaxHealth = NewMaxHealth; }
	FORCEINLINE void SetStamina(float NewStamina) { Stamina = NewStamina; }
	FORCEINLINE void SetMaxStamina(float NewMaxStamina) { MaxStamina = NewMaxStamina; }
};
