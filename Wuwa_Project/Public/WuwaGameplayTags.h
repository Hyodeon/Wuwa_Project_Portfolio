#pragma once

#include "CoreMinimal.h"
#include "NativeGameplayTags.h"

namespace WuwaGameplayTags
{
	// ================= [SetByCaller 데이터 태그] =================
	WUWA_PROJECT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Damage_MotionValue);
	WUWA_PROJECT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Damage_ResonanceValue);

	// ================= [상태 (State) 태그] =================
	WUWA_PROJECT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Groggy);
	WUWA_PROJECT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Invulnerable);
	WUWA_PROJECT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Attack);
	WUWA_PROJECT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_DodgeWindow);
	WUWA_PROJECT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Dead);
	WUWA_PROJECT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_JustDodge_Active);

	// ================= [어빌리티 식별 (Ability) 태그] =================
	WUWA_PROJECT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Attack);
	WUWA_PROJECT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Dodge);
	WUWA_PROJECT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_JustDodge);
	WUWA_PROJECT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Dodge_Counter);
	WUWA_PROJECT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Skill_E);
	WUWA_PROJECT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Skill_Ultimate);

	// ================= [이벤트 (Event) 태그] =================
	WUWA_PROJECT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Combat_Hit);
	WUWA_PROJECT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Combat_Parry);
	WUWA_PROJECT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_JustDodge_Success);
	WUWA_PROJECT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Combat_AttackEnd);
	WUWA_PROJECT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Combat_DodgeEnd);
	WUWA_PROJECT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Combat_CheckCombo);
	WUWA_PROJECT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Combat_AttackInput);

	// ================= [스킬 쿨타임 (Cooldown) 태그] =================
	WUWA_PROJECT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Cooldown_Valor_ESkill);
}