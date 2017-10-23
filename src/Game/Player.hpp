#pragma once

#include "Combatant.hpp"
#include "../Framework/Gameengine.hpp"
#include "Resources\Resources.hpp"

class Player : public Combatant
{
public:

	virtual void Init(int _id) override;
	void Quit();
	void Update(int _xMove, bool _is_walking);
	void Render();

private:

	bool is_walking;

};