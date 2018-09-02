#pragma once

#include "Combatant.hpp"
#include "../Framework/Gameengine.hpp"
#include "Resources\Resources.hpp"


const float PLAYER_SCALE = 0.6f;
const float WALKING_ANIMATION_SPEED = 0.85f;

class Player : public Combatant
{
public:

	virtual void Init(int _id, CGameEngine *_engine) override;
	void Update(int _xMove, bool _is_walking);
	virtual void Render() override;

	virtual bool IsPlayer() override { return true; }
	virtual int GetID() override { return -3; }

	PossibleAims possibleAbilityAims[4];

protected:

	bool AimChosen();

	void DoCurrentAbility();
	bool CombatantClicked(Combatant* _combatant);
	bool CurrentAbilityCanAimAtCombatant(Combatant* _combatant);

	void SelectAdditionalTargets();

	virtual void StartAbilityAnimation(int _ability) {};

	bool is_walking;
};