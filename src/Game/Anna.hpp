#pragma once

#include "Player.hpp"
#include "PlayerEnum.hpp"


class PlayerAnna: public Player
{
public:
	virtual void Init(int _id) override;

	virtual int GetID() override { return PlayerID::Anna; }

	virtual bool DoAbility(int _id, std::vector<Combatant*> &_targets) override;

private:

	void indomitableHappiness(Combatant* _target);
	void wordGush(Combatant* _target);
	void sideOfNoseBlow(Combatant* _target);
	void deathHug(Combatant* _target);

};