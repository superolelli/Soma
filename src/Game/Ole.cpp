#include "Ole.hpp"



void PlayerOle::Init(int _id, CGameEngine *_engine)
{
	combatantObject = g_pModels->modelOle->getNewEntityInstance("Ole");

	this->Player::Init(_id, _engine);
}
