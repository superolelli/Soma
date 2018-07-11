#pragma once

#include "Player.hpp"
#include "PlayerEnum.hpp"


class PlayerSimon : public Player
{
public:
	virtual void Init(int _id, CGameEngine *_engine) override;

	virtual int GetID() override { return PlayerID::Simon; }

	virtual bool DoAbility(int _id, std::vector<Combatant*> &_targets) override;

private:

	void messAround(Combatant* _target);
	void allOnOne(Combatant* _target);
	void marshmallowFlash();
	void repair(Combatant* _target);


};