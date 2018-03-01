#pragma once

#include "Combatant.hpp"
#include "../Framework/Gameengine.hpp"
#include "Resources\Resources.hpp"

class Player : public Combatant
{
public:

	virtual void Init(int _id) override;
	void Update(int _xMove, bool _is_walking);
	void Render();

	virtual bool IsPlayer() override { return true; }
	virtual int GetID() override { return -3; }

protected:

	bool is_walking;
};