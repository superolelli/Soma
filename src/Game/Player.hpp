#pragma once

#include "Combatant.hpp"
#include "../Framework/Gameengine.hpp"
#include "Resources\Resources.hpp"


const float WALKING_ANIMATION_SPEED = 0.85f;

class Player : public Combatant
{
public:
	Player(int _id, CGameEngine *_engine, NotificationRenderer *_notificationRenderer);

	virtual void Init() override;
	void Update(int _xMove, bool _is_walking);
	virtual void Render() override;

	virtual bool IsPlayer() override { return true; }
	virtual int GetID() override { return -3; }

	void SetEquipment(CombatantAttributes &_equipmentStats);

	float GetAdditionalDamageForCurrentlyAimedCombatant();
	bool CurrentAbilityCanAimAtCombatant(Combatant* _combatant);


protected:

	bool AimChosen();

	void DoCurrentAbility();
	bool CombatantClicked(Combatant* _combatant);

	void RenderAbilityTargetMarker();
	void StartAbilityAnimation(int _ability);

	void SelectAdditionalTargets();

	int NumberOfTargetsForCurrentAbility();
	bool CurrentAbilityAttacksAll();
	bool CurrentAbilityAttacksAllPlayers();

	void ChooseCombatantsForConfusionAttack(Combatant *_originallyAttackedCombatant);
	void SelectAdditionalPlayers();
	void SelectAdditionalEnemies();

	bool is_walking;
};