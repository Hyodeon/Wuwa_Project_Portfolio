#include "WuwaGameplayTags.h"

namespace WuwaGameplayTags
{
	// SetByCaller
	UE_DEFINE_GAMEPLAY_TAG(Data_Damage_MotionValue, "Data.Damage.MotionValue");
	UE_DEFINE_GAMEPLAY_TAG(Data_Damage_ResonanceValue, "Data.Damage.ResonanceValue");
	
	UE_DEFINE_GAMEPLAY_TAG(Data_Resource_Valor, "Data.Resource.Valor");
	UE_DEFINE_GAMEPLAY_TAG(Data_Resource_BladeDance, "Data.Resource.BladeDance");
	UE_DEFINE_GAMEPLAY_TAG(Data_Resource_Concerto, "Data.Resource.Concerto");
	UE_DEFINE_GAMEPLAY_TAG(Data_Resource_Ultimate, "Data.Resource.Ultimate");

	// State
	UE_DEFINE_GAMEPLAY_TAG(State_Groggy, "State.Groggy");
	UE_DEFINE_GAMEPLAY_TAG(State_SuperArmor, "State.SuperArmor");
	UE_DEFINE_GAMEPLAY_TAG(State_HyperArmor, "State.HyperArmor");
	UE_DEFINE_GAMEPLAY_TAG(State_Invulnerable, "State.Invulnerable");
	UE_DEFINE_GAMEPLAY_TAG(State_DodgeWindow, "State.DodgeWindow");
	UE_DEFINE_GAMEPLAY_TAG(State_Dead, "State.Dead");
	UE_DEFINE_GAMEPLAY_TAG(State_JustDodge_Active, "State.JustDodge.Active");

	// Ability
	UE_DEFINE_GAMEPLAY_TAG(Ability_Attack, "Ability.Attack");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Dodge, "Ability.Dodge");
	UE_DEFINE_GAMEPLAY_TAG(Ability_JustDodge, "Ability.JustDodge");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Dodge_Counter, "Ability.Dodge.Counter");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Skill_E, "Ability.Skill.E");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Skill_Ultimate, "Ability.Skill.Ultimate");

	// Event
	UE_DEFINE_GAMEPLAY_TAG(Event_Combat_Hit, "Event.Combat.Hit");
	UE_DEFINE_GAMEPLAY_TAG(Event_Combat_Parry, "Event.Combat.Parry");
	UE_DEFINE_GAMEPLAY_TAG(Event_JustDodge_Success, "Event.JustDodge.Success");
	UE_DEFINE_GAMEPLAY_TAG(Event_Combat_AttackEnd, "Event.Combat.AttackEnd");
	UE_DEFINE_GAMEPLAY_TAG(Event_Combat_DodgeEnd, "Event.Combat.DodgeEnd");
	UE_DEFINE_GAMEPLAY_TAG(Event_Combat_CheckCombo, "Event.Combat.CheckCombo");
	UE_DEFINE_GAMEPLAY_TAG(Event_Combat_AttackInput, "Event.Combat.AttackInput");

	// Cooldown
	UE_DEFINE_GAMEPLAY_TAG(Ability_Cooldown_Valor_ESkill, "Ability.Cooldown.Valor.ESkill");
}