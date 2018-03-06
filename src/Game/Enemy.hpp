#pragma once

#include "Combatant.hpp"
#include "Resources\Resources.hpp"

const float ENEMY_SCALE = 0.6f;
const int ENEMY_ANIMATION_SPEED = 10;

class Enemy : public Combatant
{
public:

	virtual void Init(int _id) override;

	virtual int GetID() override { return -1; }

	virtual bool DoAbility(int _id, std::vector<Combatant*> &_targets);

	void Render();

private:


};