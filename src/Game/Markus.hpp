#pragma once


#include "Player.hpp"
#include "PlayerEnum.hpp"


class PlayerMarkus : public Player
{
public:
	virtual void Init(int _id, CGameEngine *_engine) override;

	virtual int GetID() override { return PlayerID::Markus; }

	virtual bool DoAbility(int _id, std::vector<Combatant*> &_targets) override;

private:

	void fistOfRevenge(Combatant* _target);
	void noseRam(Combatant* _target);
	void hardDestruction(Combatant* _target);
	void horribleGrin(Combatant* _target);

};