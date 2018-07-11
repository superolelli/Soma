#pragma once

#include "Combatant.hpp"
#include "../Framework/Gameengine.hpp"
#include "Resources\Resources.hpp"


const float PLAYER_SCALE = 0.6f;
const int WALKING_ANIMATION_SPEED = 18;
const int IDLE_ANIMATION_SPEED = 7;

class Player : public Combatant
{
public:

	virtual void Init(int _id, CGameEngine *_engine) override;
	void Update(int _xMove, bool _is_walking);
	virtual void Render() override;

	virtual bool IsPlayer() override { return true; }
	virtual int GetID() override { return -3; }

protected:

	bool AimChosen();

	void DoCurrentAbility();
	bool CombatantClicked(int _id);
	bool CurrentAbilityCanAimAtCombatant(int i);

	int selectedTarget;

	bool is_walking;
};