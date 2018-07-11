#pragma once


#include "Player.hpp"
#include "PlayerEnum.hpp"


class PlayerOle : public Player
{
public:
	virtual void Init(int _id, CGameEngine *_engine) override;

	virtual int GetID() override { return PlayerID::Ole; }

	virtual bool DoAbility(int _id, std::vector<Combatant*> &_targets);

private:

	void gummiArm(Combatant* _target);
	void wholeBodyKick(Combatant* _target);
	void hackIntoTheSystem(Combatant* _target);
	void poetrySlam(Combatant* _target);


};

