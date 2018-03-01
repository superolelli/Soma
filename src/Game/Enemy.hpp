#pragma once

#include "Combatant.hpp"
#include "Resources\Resources.hpp"

class Enemy : public Combatant
{
public:

	virtual void Init(int _id) override;

	virtual int GetID() override { return -1; }

	virtual bool DoAbility(int _id, std::vector<Combatant*> &_targets);

	void Render();

private:


};