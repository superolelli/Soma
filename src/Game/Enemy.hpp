#pragma once

#include "Combatant.hpp"
#include "Resources\Resources.hpp"

class Enemy : public Combatant
{
public:

	virtual void Init(int _id) override;

	void Render();

private:


};